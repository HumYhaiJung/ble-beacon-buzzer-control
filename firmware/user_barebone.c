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
#include "arch_system.h"
#include "co_bt.h"
#include "gattc_task.h"
#include "gpio.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "swt_t1_lib.h"
#include "ke_msg.h" // for KE_MSG_ALLOC / KE_MSG_SEND (depends on SDK, adjust include if different)
#include "timer0_2.h"
#include "timer0.h"

// PWM settings for 3000Hz buzzer tone
// With 2MHz Timer0 clock (16MHz / 8 divider):
// For 3000Hz: Period = 1/3000s = 333.3us = 667 ticks at 2MHz
#define TIMER_ON 667
#define PWM_HIGH 334 // ~50% duty cycle high
#define PWM_LOW 333  // ~50% duty cycle low
#define ALL_NOTES 26

const uint16_t notes[ALL_NOTES] = {1046, 987, 767, 932, 328, 880, 830,
                                   609, 783, 991, 739, 989, 698, 456,
                                   659, 255, 622, 254, 587, 554, 365,
                                   523, 251, 493, 466, 440};

static tim0_2_clk_div_config_t clk_div_config =
    {
        .clk_div = TIM0_2_CLK_DIV_8};

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
#define APP_UPDATE_ADV_REQ ((ke_msg_id_t)0xC000)

/* message struct (empty payload is OK) */
struct app_update_adv_req
{
    uint8_t dummy;
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

static const char device_name[] = USER_DEVICE_NAME;

#define APP_AD_SVC_UUID 28151
#define APP_AD_SVC_DATA_LEN 1

#ifndef GAP_AD_TYPE_SERVICE_DATA_16
#define GAP_AD_TYPE_SERVICE_DATA_16 0x16
#endif

#define POWER_SENSE_PORT GPIO_PORT_0
#define POWER_SENSE_PIN GPIO_PIN_7
#define POWER_DEBOUNCE_MS 20

#define BUZZER_PIN_PORT GPIO_PORT_0
#define BUZZER_PIN GPIO_PIN_9
#define BUZZER_DEFAULT_FREQ_HZ 3000u
#define BUZZER_DEFAULT_DUR_MS 200u

/* previous power state: -1 = unknown, 0 = low/off, 1 = high/on */
static int8_t prev_power_state = -1;

/* Fallback flag if KE_MSG_ALLOC fails in ISR */
static volatile uint8_t adv_update_flag = 0;

/*
 * Forward declarat0ons
 ****************************************************************************************
 */
static void adv_data_update_timer_cb(void);
static void power_pin_isr(void);

/* Siren callback forward declaration */
static void siren_timer_cb(void);

/* Buzzer forward declarations */
void user_buzzer_command(uint8_t cmd);
static void buzzer_duration_cb(void);
static void user_buzzer_start(uint32_t freq_hz, uint32_t duration_ms);
static void user_buzzer_stop(void);
/* Rhythmic beep (new) */
static void user_buzzer_start_rhythm(uint32_t total_duration_ms);
static void rhythm_timer_cb(void);

/*
 * Buzzer state
 ****************************************************************************************
 */
/* Buzzer state for Timer0 hardware PWM */
static bool allow_buzzer = true;
static bool hw_pwm_running = false;
static timer_hnd buzzer_duration_timer = EASY_TIMER_INVALID_TIMER;
/* Rhythm state */
static timer_hnd rhythm_timer = EASY_TIMER_INVALID_TIMER;
static uint32_t rhythm_remaining_ms = 0;
static const uint32_t rhythm_on_ms = 200;
static const uint32_t rhythm_off_ms = 100;
static bool rhythm_state_on = false;

/* Siren (find-device) state */
static timer_hnd siren_timer = EASY_TIMER_INVALID_TIMER;
static uint32_t siren_remaining_ms = 0;
static uint32_t siren_current_freq = 800; /* Hz */
static const uint32_t siren_min_freq = 800;
static const uint32_t siren_max_freq = 2000;
static const uint32_t siren_step_hz = 100;
static const uint32_t siren_step_ms = 50; /* change freq every 50ms */
static int8_t siren_dir = 1;              /* 1 = up, -1 = down */

/* Helper: set Timer0 PWM frequency (50% duty) */
static void set_buzzer_freq(uint32_t freq_hz)
{
    if (freq_hz == 0)
        return;

    /* Timer0 input clock is 2MHz (16MHz / 8). Period ticks = 2,000,000 / freq */
    uint32_t period = 2000000u / freq_hz;
    if (period < 3)
        period = 3;

    uint16_t on = (uint16_t)period;
    uint16_t high = (uint16_t)(period / 2);
    uint16_t low = (uint16_t)(period - high);

    timer0_set(on, high, low);
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
    // Add 16-bit service UUID instead of 128-bit for better compatibility
    uint8_t uuid_ad[4];
    uuid_ad[0] = 3;                                  // length = type (1) + UUID (2)
    uuid_ad[1] = 0x03;                               // Complete List of 16-bit Service UUIDs
    uuid_ad[2] = (SWT_T1_SVC_UUID_16 & 0xFF);        // Low byte
    uuid_ad[3] = ((SWT_T1_SVC_UUID_16 >> 8) & 0xFF); // High byte
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
 * Buzzer implementation (Timer0 hardware PWM)
 */

static void buzzer_duration_cb(void)
{
    user_buzzer_stop();
    buzzer_duration_timer = EASY_TIMER_INVALID_TIMER;
}

/* Rhythm timer callback toggles PWM on/off until total duration reached */
static void rhythm_timer_cb(void)
{
    uint32_t next_interval = 0;

    if (rhythm_remaining_ms == 0)
    {
        /* finished: ensure PWM stopped and clocks disabled */
        timer0_stop();
        timer0_2_clk_disable();
        hw_pwm_running = false;
        rhythm_state_on = false;
        rhythm_timer = EASY_TIMER_INVALID_TIMER;
        return;
    }

    /* Toggle state */
    if (rhythm_state_on)
    {
        timer0_stop();
        hw_pwm_running = false;
        rhythm_state_on = false;
        next_interval = rhythm_off_ms;
    }
    else
    {
        timer0_start();
        hw_pwm_running = true;
        rhythm_state_on = true;
        next_interval = rhythm_on_ms;
    }

    if (rhythm_remaining_ms < next_interval)
        next_interval = rhythm_remaining_ms;

    rhythm_remaining_ms -= next_interval;

    /* schedule next toggle; store handle so it can be cancelled externally */
    rhythm_timer = app_easy_timer(next_interval, rhythm_timer_cb);
}

/**
 * @brief Timer0 interrupt callback - maintain constant 3000Hz tone
 * Fixed PWM frequency without melody/note changes (simple single tone)
 * Callback fires every ~333.3us (3000Hz)
 */
/* No Timer0 IRQ callback: run PWM without registering an ISR to avoid
   high-frequency interrupt load that can block the BLE stack. */
/**
 * @brief Start Timer0 hardware PWM buzzer (P0_9 PWM0 output)
 * @param freq_hz Desired frequency in Hz (0 uses default 3000 Hz)
 * @param duration_ms Tone duration in milliseconds (0 uses default 200 ms)
 */
static void user_buzzer_start(uint32_t freq_hz, uint32_t duration_ms)
{
    if (!allow_buzzer)
        return;

    if (hw_pwm_running)
        user_buzzer_stop();

    if (freq_hz == 0)
        freq_hz = BUZZER_DEFAULT_FREQ_HZ;
    if (duration_ms == 0)
        duration_ms = BUZZER_DEFAULT_DUR_MS;

    /* Enable Timer0/2 clock (following SDK example timer0_pwm_test) */
    timer0_2_clk_enable();

    /* Set clock division: 16 MHz / 8 = 2 MHz Timer0 input clock */
    timer0_2_clk_div_set(&clk_div_config);

    /* Initialize Timer0 in PWM mode */
    timer0_init(TIM0_CLK_FAST, PWM_MODE_ONE, TIM0_CLK_NO_DIV);

    /* Set Timer0 PWM counters (TIMER_ON=667 ticks, PWM_HIGH=334, PWM_LOW=333 => 3000Hz) */
    timer0_set(TIMER_ON, PWM_HIGH, PWM_LOW);

    /* No IRQ/callback registration to keep ISR load low. PWM output will
        run from hardware; automatic stop is handled by `app_easy_timer`. */

    /* Start hardware PWM on P0_9 (PWM0) */
    timer0_start();

    /* Mark as running and schedule automatic stop after duration */
    hw_pwm_running = true;
    if (duration_ms > 0)
    {
        buzzer_duration_timer = app_easy_timer(duration_ms, buzzer_duration_cb);
    }
}

static void user_buzzer_stop(void)
{
    if (buzzer_duration_timer != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(buzzer_duration_timer);
        buzzer_duration_timer = EASY_TIMER_INVALID_TIMER;
    }

    /* stop rhythm timer if active */
    if (rhythm_timer != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(rhythm_timer);
        rhythm_timer = EASY_TIMER_INVALID_TIMER;
        rhythm_remaining_ms = 0;
        rhythm_state_on = false;
    }

    /* stop siren timer if active */
    if (siren_timer != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(siren_timer);
        siren_timer = EASY_TIMER_INVALID_TIMER;
        siren_remaining_ms = 0;
        siren_current_freq = siren_min_freq;
        siren_dir = 1;
    }

    if (hw_pwm_running)
    {
        timer0_stop();
        timer0_2_clk_disable();
        hw_pwm_running = false;
    }
}

void user_buzzer_command(uint8_t cmd)
{

    switch (cmd)
    {
    case 0x01: /* SIREN (find-device) 5s */
        if (allow_buzzer)
        {
            /* cancel any existing siren */
            if (siren_timer != EASY_TIMER_INVALID_TIMER)
            {
                app_easy_timer_cancel(siren_timer);
                siren_timer = EASY_TIMER_INVALID_TIMER;
            }
            /* Initialize PWM hardware */
            timer0_2_clk_enable();
            timer0_2_clk_div_set(&clk_div_config);
            timer0_init(TIM0_CLK_FAST, PWM_MODE_ONE, TIM0_CLK_NO_DIV);
            /* start at min freq */
            siren_current_freq = siren_min_freq;
            set_buzzer_freq(siren_current_freq);
            timer0_start();
            hw_pwm_running = true;

            /* start siren state machine for 5s */
            siren_remaining_ms = 5000;
            siren_dir = 1;
            /* kickstart immediately */
            siren_timer = app_easy_timer(0, siren_timer_cb);
        }
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

#if DEVELOPMENT_DEBUG && !defined(GPIO_DRV_PIN_ALLOC_MON_DISABLED)
    RESERVE_GPIO(POWER_SENSE, POWER_SENSE_PORT, POWER_SENSE_PIN, PID_GPIO);
    RESERVE_GPIO(BUZZER, BUZZER_PIN_PORT, BUZZER_PIN, PID_PWM0);
#endif

    /* Configure pins - adjust pull mode if your hardware requires active-low sensing */
    GPIO_ConfigurePin(POWER_SENSE_PORT, POWER_SENSE_PIN, INPUT_PULLUP, PID_GPIO, false);
    /* Configure P0_9 as PWM0 output for Timer0 hardware PWM (true = drive strength) */
    GPIO_ConfigurePin(BUZZER_PIN_PORT, BUZZER_PIN, OUTPUT, PID_PWM0, true);

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
                   low_input, /* low_input: TRUE => IRQ when input is LOW */
                   false,     /* release_wait: FALSE (don't wait release) */
                   POWER_DEBOUNCE_MS);

    /* Also set IRQ input level explicitly to match (safer) */
    if (low_input)
        GPIO_SetIRQInputLevel(GPIO0_IRQn, GPIO_IRQ_INPUT_LEVEL_LOW);
    else
        GPIO_SetIRQInputLevel(GPIO0_IRQn, GPIO_IRQ_INPUT_LEVEL_HIGH);

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

    /* Initialize custom service AFTER default app init when BLE stack is ready */
    swt_t1_init();
    swt_t1_register_write_cb(user_buzzer_command);

    /* Publish initial advertising data */
    if (stored_adv_data_len && svc_data_index + sizeof(struct svc_data_16_ad_structure) <= stored_adv_data_len)
    {
        memcpy(stored_adv_data + svc_data_index, &svc_data, sizeof(struct svc_data_16_ad_structure));
        app_easy_gap_update_adv_data(stored_adv_data, stored_adv_data_len, NULL, 0);
    }
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
    }

    switch (msgid)
    {
    case GATTC_WRITE_REQ_IND:
    {
        const struct gattc_write_req_ind *wr = (const struct gattc_write_req_ind *)param;

        // Handle ANY write request as a potential buzzer command
        if (wr != NULL && wr->length > 0)
        {
            // First try the SWT-T1 registered callback path
            int handled = swt_t1_on_write(wr->value, wr->length);
            if (!handled)
            {
                // Fallback: directly handle as buzzer command
                uint8_t cmd = wr->value[0];
                user_buzzer_command(cmd);
            }
        }

        // Send write confirmation
        struct gattc_write_cfm *cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
        cfm->handle = wr ? wr->handle : 0;
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

    // Connection event triggers buzzer - connectionless approach
    //  user_buzzer_start(BUZZER_DEFAULT_FREQ_HZ, BUZZER_DEFAULT_DUR_MS);
    user_buzzer_start_rhythm(1000); // 1 second of rhythmic beeps
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

/* Start rhythmic beeps for total_duration_ms (non-blocking) */
static void user_buzzer_start_rhythm(uint32_t total_duration_ms)
{
    if (!allow_buzzer)
        return;

    /* stop any existing timers */
    if (buzzer_duration_timer != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(buzzer_duration_timer);
        buzzer_duration_timer = EASY_TIMER_INVALID_TIMER;
    }
    if (rhythm_timer != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(rhythm_timer);
        rhythm_timer = EASY_TIMER_INVALID_TIMER;
    }

    /* Initialize PWM hardware (same as single-tone start) */
    timer0_2_clk_enable();
    timer0_2_clk_div_set(&clk_div_config);
    timer0_init(TIM0_CLK_FAST, PWM_MODE_ONE, TIM0_CLK_NO_DIV);
    timer0_set(TIMER_ON, PWM_HIGH, PWM_LOW);

    /* Start in ON state immediately and schedule first OFF */
    rhythm_remaining_ms = total_duration_ms;
    rhythm_state_on = true;
    timer0_start();
    hw_pwm_running = true;

    uint32_t first_interval = (rhythm_remaining_ms < rhythm_on_ms) ? rhythm_remaining_ms : rhythm_on_ms;
    rhythm_remaining_ms -= first_interval;
    rhythm_timer = app_easy_timer(first_interval, rhythm_timer_cb);
}

/* Siren timer callback: sweep frequency up/down until time expires */
static void siren_timer_cb(void)
{
    uint32_t next_interval = siren_step_ms;

    if (siren_remaining_ms == 0)
    {
        /* finished: ensure PWM stopped and clocks disabled */
        timer0_stop();
        timer0_2_clk_disable();
        hw_pwm_running = false;
        siren_timer = EASY_TIMER_INVALID_TIMER;
        return;
    }

    /* advance frequency */
    siren_current_freq += (siren_dir > 0) ? siren_step_hz : -siren_step_hz;
    if (siren_current_freq >= siren_max_freq)
    {
        siren_current_freq = siren_max_freq;
        siren_dir = -1;
    }
    else if (siren_current_freq <= siren_min_freq)
    {
        siren_current_freq = siren_min_freq;
        siren_dir = 1;
    }

    /* Apply new frequency */
    set_buzzer_freq(siren_current_freq);

    if (siren_remaining_ms < next_interval)
        next_interval = siren_remaining_ms;

    siren_remaining_ms -= next_interval;
    siren_timer = app_easy_timer(next_interval, siren_timer_cb);
}