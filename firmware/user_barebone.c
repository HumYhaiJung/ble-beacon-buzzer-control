/**
 ****************************************************************************************
 *
 * @file user_barebone.c
 *
 * @brief BLE Connectable device with P0_7 power sensing + P0_9 passive buzzer control
 * Integrated with swt-t1 vendor service for GATT write control
 *
 ****************************************************************************************
 */
#include "stdio.h"
#include "rwip_config.h"
#include "gap.h"
#include "app_easy_timer.h"
#include "user_barebone.h"
#include "co_bt.h"
#include "gattc_task.h"
#include "gpio.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "swt_t1_lib.h"
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

struct mnf_specific_data_ad_structure
{
    uint8_t ad_structure_size;
    uint8_t ad_structure_type;
    uint8_t company_id[2];
    uint8_t proprietary_data[APP_AD_MSD_DATA_LEN];
};

/* Beacon payload: device_name(4) + command(1) + timestamp(4) = 9 bytes */
struct svc_data_16_ad_structure
{
    uint8_t ad_structure_size;
    uint8_t ad_structure_type;
    uint8_t uuid[2];
    uint8_t device_name[4];
    uint8_t command;
    uint8_t timestamp[4];
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

uint8_t app_connection_idx __SECTION_ZERO("retention_mem_area0");               //@RETENTION MEMORY
timer_hnd app_adv_data_update_timer_used __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

struct mnf_specific_data_ad_structure mnf_data __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t mnf_data_index __SECTION_ZERO("retention_mem_area0");                         //@RETENTION MEMORY

struct svc_data_16_ad_structure svc_data __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t svc_data_index __SECTION_ZERO("retention_mem_area0");                   //@RETENTION MEMORY

uint8_t stored_adv_data_len __SECTION_ZERO("retention_mem_area0");           //@RETENTION MEMORY
uint8_t stored_adv_data[ADV_DATA_LEN] __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

static const char device_name[] = "T1-1000";
static const char beacon_device_name[] = "BZR1"; /* 4-byte device identifier for beacon */
static uint32_t beacon_timestamp_ms = 0;         /* Millisecond timestamp for beacon */

#define APP_AD_SVC_UUID 0xAA28 /* Beacon Service UUID */
#define APP_AD_SVC_DATA_LEN 9  /* device_name(4) + command(1) + timestamp(4) bytes */

#ifndef GAP_AD_TYPE_SERVICE_DATA_16
#define GAP_AD_TYPE_SERVICE_DATA_16 0x16
#endif

#define POWER_SENSE_PORT GPIO_PORT_0
#define POWER_SENSE_PIN GPIO_PIN_7
#define POWER_DEBOUNCE_MS 20

#define BUZZER_PIN_PORT GPIO_PORT_0
#define BUZZER_PIN GPIO_PIN_9
#define BUZZER_DEFAULT_FREQ_HZ 2000u
#define BUZZER_DEFAULT_DUR_MS 500u

/*
 * Forward declarations
 ****************************************************************************************
 */
static void adv_data_update_timer_cb(void);
static void power_pin_isr(void);

void user_buzzer_command(uint8_t cmd);
static void buzzer_pwm_toggle_cb(void);
static void buzzer_duration_cb(void);
static void user_buzzer_start(uint32_t freq_hz, uint32_t duration_ms);
static void user_buzzer_stop(void);

/*
 * Buzzer state
 ****************************************************************************************
 */
static bool buzzer_running = false;
static bool allow_buzzer = false;
static uint32_t buzzer_freq_hz = BUZZER_DEFAULT_FREQ_HZ;
static uint32_t buzzer_duration_ms = BUZZER_DEFAULT_DUR_MS;
static timer_hnd buzzer_pwm_timer = EASY_TIMER_INVALID_TIMER;
static timer_hnd buzzer_duration_timer = EASY_TIMER_INVALID_TIMER;
static bool buzzer_pin_state = false;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

static void mnf_data_init()
{
    mnf_data.ad_structure_size = sizeof(struct mnf_specific_data_ad_structure) - sizeof(uint8_t);
    mnf_data.ad_structure_type = GAP_AD_TYPE_MANU_SPECIFIC_DATA;

    mnf_data.company_id[0] = APP_AD_MSD_COMPANY_ID & 0xFF;
    mnf_data.company_id[1] = (APP_AD_MSD_COMPANY_ID >> 8) & 0xFF;

    mnf_data.proprietary_data[0] = 0x02;
    mnf_data.proprietary_data[1] = 0x15;
    mnf_data.proprietary_data[2] = 0;
    mnf_data.proprietary_data[3] = 0;
}

static void svc_data_init()
{
    svc_data.ad_structure_size = sizeof(struct svc_data_16_ad_structure) - sizeof(uint8_t);
    svc_data.ad_structure_type = GAP_AD_TYPE_SERVICE_DATA_16;
    svc_data.uuid[0] = APP_AD_SVC_UUID & 0xFF;
    svc_data.uuid[1] = (APP_AD_SVC_UUID >> 8) & 0xFF;

    /* Initialize beacon device name (4 bytes, ASCII) */
    memcpy(svc_data.device_name, beacon_device_name, sizeof(svc_data.device_name));

    /* Initialize command to 0x00 (STOP) */
    svc_data.command = 0x00;

    /* Initialize timestamp to 0 */
    svc_data.timestamp[0] = 0;
    svc_data.timestamp[1] = 0;
    svc_data.timestamp[2] = 0;
    svc_data.timestamp[3] = 0;
}

static void beacon_timestamp_update(void)
{
    /* Update timestamp in little-endian format */
    svc_data.timestamp[0] = (uint8_t)(beacon_timestamp_ms & 0xFF);
    svc_data.timestamp[1] = (uint8_t)((beacon_timestamp_ms >> 8) & 0xFF);
    svc_data.timestamp[2] = (uint8_t)((beacon_timestamp_ms >> 16) & 0xFF);
    svc_data.timestamp[3] = (uint8_t)((beacon_timestamp_ms >> 24) & 0xFF);
    beacon_timestamp_ms++;
}

static void mnf_data_update()
{
    uint16_t seq;
    seq = mnf_data.proprietary_data[2] | (mnf_data.proprietary_data[3] << 8);
    seq++;
    mnf_data.proprietary_data[2] = seq & 0xFF;
    mnf_data.proprietary_data[3] = (seq >> 8) & 0xFF;

    if (seq == 0xFFFF)
    {
        mnf_data.proprietary_data[2] = 0;
        mnf_data.proprietary_data[3] = 0;
    }
}

static void app_add_ad_struct(struct gapm_start_advertise_cmd *cmd, void *ad_struct_data, uint8_t ad_struct_len)
{
    if ((ADV_DATA_LEN - cmd->info.host.adv_data_len) >= ad_struct_len)
    {
        memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len], ad_struct_data, ad_struct_len);
        cmd->info.host.adv_data_len += ad_struct_len;

        uint8_t *p = (uint8_t *)ad_struct_data;
        if (ad_struct_len >= 2 && p[1] == GAP_AD_TYPE_MANU_SPECIFIC_DATA)
        {
            mnf_data_index = cmd->info.host.adv_data_len - sizeof(struct mnf_specific_data_ad_structure);
        }
        else if (ad_struct_len >= 2 && p[1] == GAP_AD_TYPE_SERVICE_DATA_16)
        {
            svc_data_index = cmd->info.host.adv_data_len - sizeof(struct svc_data_16_ad_structure);
        }
    }
    else
    {
        ASSERT_WARNING(0);
    }

    stored_adv_data_len = cmd->info.host.adv_data_len;
    memcpy(stored_adv_data, cmd->info.host.adv_data, stored_adv_data_len);
}

static void add_complete_local_name(struct gapm_start_advertise_cmd *cmd)
{
    uint8_t name_len = (uint8_t)strlen(device_name);
    uint8_t name_ad_struct_len = (uint8_t)(name_len + 2);

    uint8_t name_ad_struct[ADV_DATA_LEN];
    if (name_ad_struct_len > sizeof(name_ad_struct))
        return;

    name_ad_struct[0] = name_len + 1;
    name_ad_struct[1] = GAP_AD_TYPE_COMPLETE_NAME;
    memcpy(&name_ad_struct[2], device_name, name_len);

    app_add_ad_struct(cmd, name_ad_struct, name_ad_struct_len);
}

static void add_service_data_poweron(struct gapm_start_advertise_cmd *cmd)
{
    uint8_t svc_ad_struct_len = (uint8_t)sizeof(struct svc_data_16_ad_structure);

    uint8_t svc_ad_struct[ADV_DATA_LEN];
    if (svc_ad_struct_len > sizeof(svc_ad_struct))
        return;

    svc_ad_struct[0] = (uint8_t)(svc_ad_struct_len - 1);
    memcpy(&svc_ad_struct[1], &svc_data.ad_structure_type, svc_ad_struct_len - 1);

    app_add_ad_struct(cmd, svc_ad_struct, svc_ad_struct_len);
}

static void add_128bit_service_uuid_adv(struct gapm_start_advertise_cmd *cmd)
{
    uint8_t uuid_ad[18];
    uuid_ad[0] = 1 + 16; // length = type + 16
    uuid_ad[1] = 0x07;   // Complete List of 128-bit UUIDs
    memcpy(&uuid_ad[2], SWT_T1_SVC_UUID, 16);
    app_add_ad_struct(cmd, uuid_ad, sizeof(uuid_ad));
}
static void power_pin_isr(void)
{
    /* Power sensing is detected via interrupt, but beacon payload is managed
       by beacon_timestamp_update() which is called in adv_data_update_timer_cb */
    bool pin_high = GPIO_GetPinStatus(POWER_SENSE_PORT, POWER_SENSE_PIN);
    (void)pin_high; /* Not currently used in beacon payload */

    GPIO_ResetIRQ(GPIO0_IRQn);
}

/*
 * Buzzer implementation
 */

static void buzzer_pwm_toggle_cb(void)
{
    buzzer_pin_state = !buzzer_pin_state;
    if (buzzer_pin_state)
        GPIO_SetActive(BUZZER_PIN_PORT, BUZZER_PIN);
    else
        GPIO_SetInactive(BUZZER_PIN_PORT, BUZZER_PIN);

    if (buzzer_running)
    {
        uint32_t half_period_us = (1000000u / buzzer_freq_hz) / 2u;
        uint32_t half_period_ms = (half_period_us + 500u) / 1000u;
        if (half_period_ms == 0)
            half_period_ms = 1;
        buzzer_pwm_timer = app_easy_timer(half_period_ms, buzzer_pwm_toggle_cb);
    }
    else
    {
        GPIO_SetInactive(BUZZER_PIN_PORT, BUZZER_PIN);
    }
}

static void buzzer_duration_cb(void)
{
    user_buzzer_stop();
    buzzer_duration_timer = EASY_TIMER_INVALID_TIMER;
}

static void user_buzzer_start(uint32_t freq_hz, uint32_t duration_ms)
{
    if (!allow_buzzer)
        return;

    if (buzzer_running)
    {
        user_buzzer_stop();
    }

    if (freq_hz == 0)
        freq_hz = BUZZER_DEFAULT_FREQ_HZ;
    buzzer_freq_hz = freq_hz;
    buzzer_duration_ms = duration_ms;

    GPIO_ConfigurePin(BUZZER_PIN_PORT, BUZZER_PIN, OUTPUT, PID_GPIO, false);
    buzzer_pin_state = false;
    GPIO_SetInactive(BUZZER_PIN_PORT, BUZZER_PIN);

    buzzer_running = true;

    uint32_t half_period_us = (1000000u / buzzer_freq_hz) / 2u;
    uint32_t half_period_ms = (half_period_us + 500u) / 1000u;
    if (half_period_ms == 0)
        half_period_ms = 1;
    buzzer_pwm_timer = app_easy_timer(half_period_ms, buzzer_pwm_toggle_cb);

    if (buzzer_duration_ms > 0)
    {
        buzzer_duration_timer = app_easy_timer(buzzer_duration_ms, buzzer_duration_cb);
    }
}

static void user_buzzer_stop(void)
{
    buzzer_running = false;

    if (buzzer_pwm_timer != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(buzzer_pwm_timer);
        buzzer_pwm_timer = EASY_TIMER_INVALID_TIMER;
    }
    if (buzzer_duration_timer != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(buzzer_duration_timer);
        buzzer_duration_timer = EASY_TIMER_INVALID_TIMER;
    }

    GPIO_SetInactive(BUZZER_PIN_PORT, BUZZER_PIN);
    GPIO_ConfigurePin(BUZZER_PIN_PORT, BUZZER_PIN, INPUT, PID_GPIO, false);
}

void user_buzzer_command(uint8_t cmd)
{
    switch (cmd)
    {
    case 0x01: /* PLAY */
        if (allow_buzzer)
            user_buzzer_start(BUZZER_DEFAULT_FREQ_HZ, BUZZER_DEFAULT_DUR_MS);
        break;
    case 0x00: /* STOP */
        user_buzzer_stop();
        break;
    case 0x02: /* ENABLE */
        allow_buzzer = true;
        break;
    case 0x03: /* DISABLE */
        allow_buzzer = false;
        user_buzzer_stop();
        break;
    case 0xFF: /* STATUS - return current status via beacon */
        /* Status is implicitly communicated via beacon timestamp and payload */
        beacon_timestamp_update();
        break;
    default:
        break;
    }
}

static void adv_data_update_timer_cb(void)
{
    mnf_data_update();
    beacon_timestamp_update();

    if (mnf_data_index + sizeof(struct mnf_specific_data_ad_structure) <= stored_adv_data_len)
    {
        memcpy(stored_adv_data + mnf_data_index, &mnf_data, sizeof(struct mnf_specific_data_ad_structure));
    }

    if (svc_data_index + sizeof(struct svc_data_16_ad_structure) <= stored_adv_data_len)
    {
        memcpy(stored_adv_data + svc_data_index, &svc_data, sizeof(struct svc_data_16_ad_structure));
    }

    app_easy_gap_update_adv_data(stored_adv_data, stored_adv_data_len, NULL, 0);

    app_adv_data_update_timer_used = app_easy_timer(APP_ADV_DATA_UPDATE_TO, adv_data_update_timer_cb);
}

/**
 ****************************************************************************************
 * @brief Initialize user app
 ****************************************************************************************
 */
void user_app_init(void)
{
    app_adv_data_update_timer_used = EASY_TIMER_INVALID_TIMER;

    mnf_data_init();
    svc_data_init();

#if DEVELOPMENT_DEBUG && !defined(GPIO_DRV_PIN_ALLOC_MON_DISABLED)
    RESERVE_GPIO(POWER_SENSE, POWER_SENSE_PORT, POWER_SENSE_PIN, PID_GPIO);
    RESERVE_GPIO(BUZZER, BUZZER_PIN_PORT, BUZZER_PIN, PID_GPIO);
#endif

    GPIO_ConfigurePin(POWER_SENSE_PORT, POWER_SENSE_PIN, INPUT_PULLUP, PID_GPIO, false);
    GPIO_ConfigurePin(BUZZER_PIN_PORT, BUZZER_PIN, INPUT, PID_GPIO, false);

    GPIO_RegisterCallback(GPIO0_IRQn, power_pin_isr);
    GPIO_EnableIRQ(POWER_SENSE_PORT,
                   POWER_SENSE_PIN,
                   GPIO0_IRQn,
                   false,
                   false,
                   POWER_DEBOUNCE_MS);

    struct gapm_start_advertise_cmd *cmd;
    cmd = app_easy_gap_undirected_advertise_get_active();

    cmd->op.code = GAPM_ADV_UNDIRECT;
    cmd->channel_map = 0x07;
    cmd->intv_min = 160;
    cmd->intv_max = 240;
    cmd->info.host.adv_data_len = 0;

    add_complete_local_name(cmd);
    app_add_ad_struct(cmd, &mnf_data, sizeof(struct mnf_specific_data_ad_structure));
    add_service_data_poweron(cmd);
    add_128bit_service_uuid_adv(cmd);
    default_app_on_init();
    swt_t1_init();
    swt_t1_register_write_cb(user_buzzer_command);
}

/**
 ****************************************************************************************
 * @brief Start advertising (connectable)
 ****************************************************************************************
 */
void user_app_adv_start(void)
{
    struct gapm_start_advertise_cmd *cmd;
    cmd = app_easy_gap_undirected_advertise_get_active();

    cmd->op.code = GAPM_ADV_UNDIRECT;

    app_easy_gap_undirected_advertise_start();

    app_adv_data_update_timer_used = app_easy_timer(APP_ADV_DATA_UPDATE_TO, adv_data_update_timer_cb);
}

/**
 ****************************************************************************************
 * @brief Set powerOn value and update advertising payload
 ****************************************************************************************
 */
void user_set_power_on(uint8_t value)
{
    /* This function is deprecated - power sensing is now handled via power_pin_isr */
    (void)value;
    /* Beacon payload is updated via beacon_timestamp_update in adv_data_update_timer_cb */
}

/**
 ****************************************************************************************
 * @brief GATT write handler
 ****************************************************************************************
 */
void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
    switch (msgid)
    {
    case GATTC_WRITE_REQ_IND:
    {
        const struct gattc_write_req_ind *wr = (const struct gattc_write_req_ind *)param;

        // Check if write is to our SWT-T1 characteristic
        if (wr != NULL && wr->handle == swt_t1_get_char_handle() && wr->length > 0)
        {
            // Forward write value to buzzer command handler
            user_buzzer_command(wr->value[0]);
        }

        // Send write confirmation
        struct gattc_write_cfm *cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
        cfm->handle = wr->handle;
        cfm->status = ATT_ERR_NO_ERROR;
        KE_MSG_SEND(cfm);
    }
    break;

    case GATTC_EVENT_REQ_IND:
    {
        struct gattc_event_ind const *ind = (struct gattc_event_ind const *)param;
        struct gattc_event_cfm *cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);
        cfm->handle = ind->handle;
        KE_MSG_SEND(cfm);
    }
    break;

    default:
        break;
    }
}
/**
 ****************************************************************************************
 * @brief Connection established
 ****************************************************************************************
 */
void user_app_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    app_connection_idx = connection_idx;

    if (app_adv_data_update_timer_used != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(app_adv_data_update_timer_used);
        app_adv_data_update_timer_used = EASY_TIMER_INVALID_TIMER;
    }

    (void)param;
}

/**
 ****************************************************************************************
 * @brief Disconnected � restart advertising
 ****************************************************************************************
 */
void user_app_disconnect(struct gapc_disconnect_ind const *param)
{
    (void)param;

    app_connection_idx = 0xFF;

    struct gapm_start_advertise_cmd *cmd = app_easy_gap_undirected_advertise_get_active();
    if (cmd)
    {
        cmd->op.code = GAPM_ADV_UNDIRECT;
        cmd->channel_map = 0x07;
        cmd->intv_min = 160;
        cmd->intv_max = 240;
    }

    app_easy_gap_undirected_advertise_start();

    if (app_adv_data_update_timer_used == EASY_TIMER_INVALID_TIMER)
    {
        app_adv_data_update_timer_used = app_easy_timer(APP_ADV_DATA_UPDATE_TO, adv_data_update_timer_cb);
    }
}

void user_app_adv_undirect_complete(uint8_t status)
{
    (void)status;
}