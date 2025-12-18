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
#include "ke_msg.h"   // for KE_MSG_ALLOC / KE_MSG_SEND (depends on SDK, adjust include if different)

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

struct svc_data_16_ad_structure
{
    uint8_t ad_structure_size;
    uint8_t ad_structure_type;
    uint8_t uuid[2];
    uint8_t data[1];
};

/*
 * INTERNAL MESSAGE IDs
 *
 * NOTE: If your SDK provides a macro to allocate app-specific message IDs use that.
 * Here we use a locally-defined constant. Adjust if needed to avoid collision.
 ****************************************************************************************
 */
#define APP_UPDATE_ADV_REQ  ((ke_msg_id_t)0xC000)

/* message struct (empty payload is OK) */
struct app_update_adv_req
{
    uint8_t dummy;
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

uint8_t app_connection_idx                      __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
timer_hnd app_adv_data_update_timer_used        __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

struct mnf_specific_data_ad_structure mnf_data  __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t mnf_data_index                          __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

struct svc_data_16_ad_structure svc_data        __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t svc_data_index                          __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

uint8_t stored_adv_data_len                     __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY
uint8_t stored_adv_data[ADV_DATA_LEN]           __SECTION_ZERO("retention_mem_area0"); //@RETENTION MEMORY

static const char device_name[] = USER_DEVICE_NAME;

#define APP_AD_SVC_UUID            28151
#define APP_AD_SVC_DATA_LEN        1

#ifndef GAP_AD_TYPE_SERVICE_DATA_16
#define GAP_AD_TYPE_SERVICE_DATA_16 0x16
#endif

#define POWER_SENSE_PORT    GPIO_PORT_0
#define POWER_SENSE_PIN     GPIO_PIN_7
#define POWER_DEBOUNCE_MS   20

#define BUZZER_PIN_PORT     GPIO_PORT_0
#define BUZZER_PIN          GPIO_PIN_9
#define BUZZER_DEFAULT_FREQ_HZ   2000u
#define BUZZER_DEFAULT_DUR_MS    500u

/* previous power state: -1 = unknown, 0 = low/off, 1 = high/on */
static int8_t prev_power_state = -1;

/* Fallback flag if KE_MSG_ALLOC fails in ISR */
static volatile uint8_t adv_update_flag = 0;

/*
 * Forward declarations
 ****************************************************************************************
 */
static void adv_data_update_timer_cb(void);
static void power_pin_isr(void);

/* Passive beep forward declarations (prevent implicit decl errors) */
static void passive_beep_stop(void);
static void passive_beep_start(uint32_t freq_hz);
static void passive_beep_once(uint32_t freq_hz, uint32_t duration_ms);

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
static bool allow_buzzer = true;
static uint32_t buzzer_freq_hz = BUZZER_DEFAULT_FREQ_HZ;
static uint32_t buzzer_duration_ms = BUZZER_DEFAULT_DUR_MS;
static timer_hnd buzzer_pwm_timer = EASY_TIMER_INVALID_TIMER;
static timer_hnd buzzer_duration_timer = EASY_TIMER_INVALID_TIMER;
static bool buzzer_pin_state = false;

/* -------- Passive buzzer (software PWM, ms resolution) -------- */
static timer_hnd passive_pwm_timer = EASY_TIMER_INVALID_TIMER;
static bool passive_pwm_pin_state = false;
static uint32_t passive_half_period_ms = 1; // half period in ms (>=1)

static void passive_pwm_toggle_cb(void)
{
    passive_pwm_pin_state = !passive_pwm_pin_state;
    if (passive_pwm_pin_state)
        GPIO_SetActive(BUZZER_PIN_PORT, BUZZER_PIN);
    else
        GPIO_SetInactive(BUZZER_PIN_PORT, BUZZER_PIN);

    /* schedule next toggle while running */
    if (passive_pwm_timer != EASY_TIMER_INVALID_TIMER)
    {
        passive_pwm_timer = app_easy_timer(passive_half_period_ms, passive_pwm_toggle_cb);
    }
}

/* Stop passive beep */
static void passive_beep_stop(void)
{
    if (passive_pwm_timer != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(passive_pwm_timer);
        passive_pwm_timer = EASY_TIMER_INVALID_TIMER;
    }
    passive_pwm_pin_state = false;
    GPIO_SetInactive(BUZZER_PIN_PORT, BUZZER_PIN);
    GPIO_ConfigurePin(BUZZER_PIN_PORT, BUZZER_PIN, INPUT, PID_GPIO, false);
}

/* Start passive beep at freq_hz (limited by ms granularity; freq_hz up to ~500) */
static void passive_beep_start(uint32_t freq_hz)
{
    if (!allow_buzzer) return;

    if (passive_pwm_timer != EASY_TIMER_INVALID_TIMER)
    {
        /* already running: restart with new freq */
        passive_beep_stop();
    }

    if (freq_hz == 0) freq_hz = 400; // reasonable default

    /* compute half period in microseconds then round to ms */
    uint32_t half_period_us = (1000000u / freq_hz) / 2u;
    uint32_t half_period_ms = (half_period_us + 500u) / 1000u; // round
    if (half_period_ms == 0) half_period_ms = 1; // cannot be zero

    passive_half_period_ms = half_period_ms;

    /* configure pin as output and ensure low initially */
    GPIO_ConfigurePin(BUZZER_PIN_PORT, BUZZER_PIN, OUTPUT, PID_GPIO, false);
    passive_pwm_pin_state = false;
    GPIO_SetInactive(BUZZER_PIN_PORT, BUZZER_PIN);

    /* start toggling */
    passive_pwm_timer = app_easy_timer(passive_half_period_ms, passive_pwm_toggle_cb);
}

/* Convenience: beep for duration_ms at freq_hz */
static void passive_beep_once(uint32_t freq_hz, uint32_t duration_ms)
{
    passive_beep_start(freq_hz);
    if (duration_ms > 0)
    {
        /* schedule stop */
        app_easy_timer(duration_ms, passive_beep_stop);
    }
}

/* --- Debug helpers (temporary) --- */
/* helper: short audible pulse to indicate ISR/handler activity, using passive_beep */
static void debug_pulse_buzzer_once(void)
{
    /* Use a quick low frequency pulse that is audible on passive buzzer */
    passive_beep_once(1200, 30); // 400 Hz, 30 ms
}

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************/

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

    svc_data.data[0] = 0;
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
    uuid_ad[0] = 1 + 16;     // length = type + 16
    uuid_ad[1] = 0x07;      // Complete List of 128-bit UUIDs
    memcpy(&uuid_ad[2], SWT_T1_SVC_UUID, 16);
    app_add_ad_struct(cmd, uuid_ad, sizeof(uuid_ad));
}

/*
 * ISR: only prepare and send message to TASK_APP to update advertising data
 * so that BLE stack APIs are called in task context (safer).
 *
 * Important: we flip the IRQ input level after handling so the next transition (the opposite edge)
 * will generate the next IRQ. This is needed because GPIO_EnableIRQ in this SDK is level-based:
 * low_input == TRUE generates IRQ when input is LOW; low_input == FALSE generates IRQ when input is HIGH.
 */
static void power_pin_isr(void)
{
    bool pin_high = GPIO_GetPinStatus(POWER_SENSE_PORT, POWER_SENSE_PIN);
    uint8_t new_state = pin_high ? 1 : 0;

    /* Debug store raw reading so you can inspect via MNF advertising if needed */
    mnf_data.proprietary_data[0] = new_state;

    if (prev_power_state != (int8_t)new_state)
    {
        prev_power_state = new_state;

        svc_data.data[0] = new_state;

        if (svc_data_index + sizeof(struct svc_data_16_ad_structure) <= stored_adv_data_len)
        {
            memcpy(stored_adv_data + svc_data_index, &svc_data, sizeof(struct svc_data_16_ad_structure));

            /* Try to send message to TASK_APP (preferred) */
            struct app_update_adv_req *req = KE_MSG_ALLOC(APP_UPDATE_ADV_REQ, TASK_APP, TASK_APP, app_update_adv_req);
            if (req)
            {
                req->dummy = 0;
                KE_MSG_SEND(req);
            }
            else
            {
                /* Fallback: set flag to be handled in task context */
                adv_update_flag = 1;
            }

            /* Debug: short audible pulse to indicate ISR triggered */
            debug_pulse_buzzer_once();
        }
    }

    /* Flip IRQ input level so next opposite transition triggers IRQ.
     * If new_state == 1 (pin high now), we want next IRQ when it goes LOW -> set IRQ to LOW trigger.
     * If new_state == 0 (pin low now), we want next IRQ when it goes HIGH -> set IRQ to HIGH trigger.
     */
    if (new_state)
    {
        GPIO_SetIRQInputLevel(GPIO0_IRQn, GPIO_IRQ_INPUT_LEVEL_LOW);
    }
    else
    {
        GPIO_SetIRQInputLevel(GPIO0_IRQn, GPIO_IRQ_INPUT_LEVEL_HIGH);
    }

    /* Clear IRQ flag */
    GPIO_ResetIRQ(GPIO0_IRQn);
}

/*
 * Buzzer implementation (original timers kept for longer-duration support)
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
        if (half_period_ms == 0) half_period_ms = 1;
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
    if (! allow_buzzer) return;

    if (buzzer_running)
    {
        user_buzzer_stop();
    }

    if (freq_hz == 0) freq_hz = BUZZER_DEFAULT_FREQ_HZ;
    buzzer_freq_hz = freq_hz;
    buzzer_duration_ms = duration_ms;

    GPIO_ConfigurePin(BUZZER_PIN_PORT, BUZZER_PIN, OUTPUT, PID_GPIO, false);
    buzzer_pin_state = false;
    GPIO_SetInactive(BUZZER_PIN_PORT, BUZZER_PIN);

    buzzer_running = true;

    uint32_t half_period_us = (1000000u / buzzer_freq_hz) / 2u;
    uint32_t half_period_ms = (half_period_us + 500u) / 1000u;
    if (half_period_ms == 0) half_period_ms = 1;
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
        case 0x01:
            if (allow_buzzer) passive_beep_once(BUZZER_DEFAULT_FREQ_HZ, BUZZER_DEFAULT_DUR_MS);
            break;
        case 0x00:
            passive_beep_stop();
            break;
        case 0x02:
            allow_buzzer = true;
            break;
        case 0x03:
            allow_buzzer = false;
            passive_beep_stop();
            break;
        default:
            break;
    }
}

static void adv_data_update_timer_cb(void)
{
    mnf_data_update();
    if (mnf_data_index + sizeof(struct mnf_specific_data_ad_structure) <= stored_adv_data_len)
    {
        memcpy(stored_adv_data + mnf_data_index, &mnf_data, sizeof(struct mnf_specific_data_ad_structure));
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

#if DEVELOPMENT_DEBUG && ! defined(GPIO_DRV_PIN_ALLOC_MON_DISABLED)
    RESERVE_GPIO(POWER_SENSE, POWER_SENSE_PORT, POWER_SENSE_PIN, PID_GPIO);
    RESERVE_GPIO(BUZZER, BUZZER_PIN_PORT, BUZZER_PIN, PID_GPIO);
#endif

    /* Configure pins - adjust pull mode if your hardware requires active-low sensing */
    GPIO_ConfigurePin(POWER_SENSE_PORT, POWER_SENSE_PIN, INPUT_PULLUP, PID_GPIO, false);
    GPIO_ConfigurePin(BUZZER_PIN_PORT, BUZZER_PIN, INPUT, PID_GPIO, false);

    /* Register ISR */
    GPIO_RegisterCallback(GPIO0_IRQn, power_pin_isr);

    /* Read initial pin state */
    bool pin_high = GPIO_GetPinStatus(POWER_SENSE_PORT, POWER_SENSE_PIN);
    prev_power_state = pin_high ? 1 : 0;
    /* set initial svc_data (if hardware is active-low invert here) */
    svc_data.data[0] = prev_power_state;

    /* Enable IRQ: set low_input such that IRQ triggers when pin becomes the opposite of current state.
     * low_input parameter means "generate IRQ when input is LOW (TRUE) or when input is HIGH (FALSE)".
     * So set low_input = (pin is currently HIGH) -> want next IRQ when it goes LOW -> low_input = true.
     */
    bool low_input = pin_high ? true : false;
    GPIO_EnableIRQ(POWER_SENSE_PORT,
                   POWER_SENSE_PIN,
                   GPIO0_IRQn,
                   low_input,   /* low_input: TRUE => IRQ when input is LOW */
                   false,       /* release_wait: FALSE (don't wait release) */
                   POWER_DEBOUNCE_MS);

    /* Also set IRQ input level explicitly to match (safer) */
    if (low_input)
        GPIO_SetIRQInputLevel(GPIO0_IRQn, GPIO_IRQ_INPUT_LEVEL_LOW);
    else
        GPIO_SetIRQInputLevel(GPIO0_IRQn, GPIO_IRQ_INPUT_LEVEL_HIGH);

    struct gapm_start_advertise_cmd* cmd;
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

    /* Publish initial advertising data */
    if (stored_adv_data_len && svc_data_index + sizeof(struct svc_data_16_ad_structure) <= stored_adv_data_len)
    {
        memcpy(stored_adv_data + svc_data_index, &svc_data, sizeof(struct svc_data_16_ad_structure));
        app_easy_gap_update_adv_data(stored_adv_data, stored_adv_data_len, NULL, 0);
    }

    /* Optional quick test beep at init to confirm buzzer wiring.
     * Comment out if not desired.
     */
    // passive_beep_once(400, 200); // 400 Hz for 200 ms - uncomment to test at startup
}

/**
 ****************************************************************************************
 * @brief Start advertising (connectable)
 ****************************************************************************************
 */
void user_app_adv_start(void)
{
    struct gapm_start_advertise_cmd* cmd;
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
    svc_data.data[0] = (value != 0) ? 1 : 0;

    if (stored_adv_data_len && svc_data_index + sizeof(struct svc_data_16_ad_structure) <= stored_adv_data_len)
    {
        memcpy(stored_adv_data + svc_data_index, &svc_data, sizeof(struct svc_data_16_ad_structure));
        app_easy_gap_update_adv_data(stored_adv_data, stored_adv_data_len, NULL, 0);
    }
}

/**
 ****************************************************************************************
 * @brief GATT write handler + message handler for APP_UPDATE_ADV_REQ
 ****************************************************************************************
 */
void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
    /* Handle fallback flag if set (set in ISR when KE_MSG_ALLOC fails) */
    if (adv_update_flag)
    {
        adv_update_flag = 0;
        if (stored_adv_data_len)
        {
            app_easy_gap_update_adv_data(stored_adv_data, stored_adv_data_len, NULL, 0);
        }
        /* debug pulse to indicate fallback handled */
        debug_pulse_buzzer_once();
    }

    switch(msgid)
    {
        case GATTC_WRITE_REQ_IND:
        {
            const struct gattc_write_req_ind *wr = (const struct gattc_write_req_ind *) param;
            
            // Check if write is to our SWT-T1 characteristic
            if (wr != NULL && wr->handle == swt_t1_get_char_handle() && wr->length > 0)
            {
                // Forward write value to registered SWT-T1 write callback
                swt_t1_on_write(wr->value, wr->length);
            }

            // Send write confirmation
            struct gattc_write_cfm *cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
            cfm->handle = wr ? wr->handle : 0;
            cfm->status = ATT_ERR_NO_ERROR;
            KE_MSG_SEND(cfm);
        } break;

        case GATTC_EVENT_REQ_IND:
        {
            struct gattc_event_ind const *ind = (struct gattc_event_ind const *) param;
            struct gattc_event_cfm *cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);
            cfm->handle = ind->handle;
            KE_MSG_SEND(cfm);
        } break;

        case APP_UPDATE_ADV_REQ:
        {
            // Received request from ISR: perform advertising update in task context
            (void)dest_id;
            (void)src_id;
            // param may be NULL or pointer to struct app_update_adv_req; no data needed
            if (stored_adv_data_len)
            {
                app_easy_gap_update_adv_data(stored_adv_data, stored_adv_data_len, NULL, 0);
            }

            /* debug: short double-pulse to indicate handler ran */
            debug_pulse_buzzer_once();
            app_easy_timer(60, debug_pulse_buzzer_once);
        } break;

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
 * @brief Disconnected – restart advertising
 ****************************************************************************************
 */
void user_app_disconnect(struct gapc_disconnect_ind const *param)
{
    (void)param;

    app_connection_idx = 0xFF;

    struct gapm_start_advertise_cmd* cmd = app_easy_gap_undirected_advertise_get_active();
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