/**
 ****************************************************************************************
 * @file user_barebone_beacon.c
 * @brief BLE Beacon-based Buzzer Control Main Application
 * 
 * This file contains the main application logic for beacon-based buzzer control.
 * It initializes beacon advertising, parses beacon commands, and controls the buzzer.
 ****************************************************************************************
 */

#include "swt_t1_beacon.h"
#include <stdint.h>
#include <stdbool.h>

/*
 * DEFINES
 ****************************************************************************************
 */

/// GPIO definitions for buzzer control
#define BUZZER_GPIO_PORT        0
#define BUZZER_GPIO_PIN         6

/// GPIO definitions for power sensing
#define POWER_SENSE_GPIO_PORT   0
#define POWER_SENSE_GPIO_PIN    7

/// GPIO definitions for status LED
#define STATUS_LED_GPIO_PORT    0
#define STATUS_LED_GPIO_PIN     9

/// Buzzer patterns
#define BUZZER_PATTERN_SHORT    100   // ms
#define BUZZER_PATTERN_LONG     500   // ms
#define BUZZER_PATTERN_PAUSE    200   // ms

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Buzzer state
typedef enum {
    BUZZER_STATE_IDLE,
    BUZZER_STATE_PLAYING,
    BUZZER_STATE_ENABLED,
    BUZZER_STATE_DISABLED
} buzzer_state_t;

/// Application context
typedef struct {
    buzzer_state_t buzzer_state;
    bool power_connected;
    uint32_t last_command_time;
    uint8_t device_name[4];
} app_context_t;

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */

/// Application context
static app_context_t app_ctx;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

static void gpio_init(void);
static void buzzer_on(void);
static void buzzer_off(void);
static void status_led_on(void);
static void status_led_off(void);
static void status_led_blink(void);
static bool power_sense_read(void);
static void process_beacon_command(uint8_t command, uint32_t timestamp);
static void buzzer_play_pattern(void);
static uint32_t get_timestamp(void);

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize GPIO pins for buzzer, power sense, and status LED
 ****************************************************************************************
 */
static void gpio_init(void)
{
    // In real implementation, use Dialog SDK GPIO functions
    // Example:
    // GPIO_ConfigurePin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, OUTPUT, PID_GPIO, false);
    // GPIO_ConfigurePin(POWER_SENSE_GPIO_PORT, POWER_SENSE_GPIO_PIN, INPUT_PULLUP, PID_GPIO, false);
    // GPIO_ConfigurePin(STATUS_LED_GPIO_PORT, STATUS_LED_GPIO_PIN, OUTPUT, PID_GPIO, false);
}

/**
 ****************************************************************************************
 * @brief Turn buzzer on
 ****************************************************************************************
 */
static void buzzer_on(void)
{
    // In real implementation: GPIO_SetActive(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
}

/**
 ****************************************************************************************
 * @brief Turn buzzer off
 ****************************************************************************************
 */
static void buzzer_off(void)
{
    // In real implementation: GPIO_SetInactive(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
}

/**
 ****************************************************************************************
 * @brief Turn status LED on
 ****************************************************************************************
 */
static void status_led_on(void)
{
    // In real implementation: GPIO_SetActive(STATUS_LED_GPIO_PORT, STATUS_LED_GPIO_PIN);
}

/**
 ****************************************************************************************
 * @brief Turn status LED off
 ****************************************************************************************
 */
static void status_led_off(void)
{
    // In real implementation: GPIO_SetInactive(STATUS_LED_GPIO_PORT, STATUS_LED_GPIO_PIN);
}

/**
 ****************************************************************************************
 * @brief Blink status LED to indicate activity
 ****************************************************************************************
 */
static void status_led_blink(void)
{
    status_led_on();
    // In real implementation, use timer for delay
    // app_easy_timer(10, status_led_off_callback);
}

/**
 ****************************************************************************************
 * @brief Read power sense GPIO
 * @return true if power connected, false otherwise
 ****************************************************************************************
 */
static bool power_sense_read(void)
{
    // In real implementation: return GPIO_GetPinStatus(POWER_SENSE_GPIO_PORT, POWER_SENSE_GPIO_PIN);
    return true;  // Placeholder
}

/**
 ****************************************************************************************
 * @brief Get current timestamp
 * @return Current timestamp in milliseconds
 ****************************************************************************************
 */
static uint32_t get_timestamp(void)
{
    // In real implementation, use RTC or timer
    // Example: return (uint32_t)(rtc_get_time_ms());
    return 0;  // Placeholder
}

/**
 ****************************************************************************************
 * @brief Play buzzer pattern
 ****************************************************************************************
 */
static void buzzer_play_pattern(void)
{
    if (app_ctx.buzzer_state == BUZZER_STATE_DISABLED) {
        return;
    }
    
    // Short beep pattern
    buzzer_on();
    // In real implementation, use timer
    // app_easy_timer(BUZZER_PATTERN_SHORT / 10, buzzer_off_callback);
}

/**
 ****************************************************************************************
 * @brief Process received beacon command
 * @param[in] command       Command code
 * @param[in] timestamp     Command timestamp
 ****************************************************************************************
 */
static void process_beacon_command(uint8_t command, uint32_t timestamp)
{
    // Check if command is too old (prevent replay attacks)
    uint32_t current_time = get_timestamp();
    if (app_ctx.last_command_time > 0 && timestamp <= app_ctx.last_command_time) {
        // Ignore old command
        return;
    }
    
    app_ctx.last_command_time = timestamp;
    status_led_blink();
    
    switch (command) {
        case BEACON_CMD_PLAY:
            if (app_ctx.buzzer_state != BUZZER_STATE_DISABLED) {
                app_ctx.buzzer_state = BUZZER_STATE_PLAYING;
                buzzer_play_pattern();
                beacon_send_status(BEACON_STATUS_PLAYING);
            }
            break;
            
        case BEACON_CMD_STOP:
            buzzer_off();
            app_ctx.buzzer_state = BUZZER_STATE_IDLE;
            beacon_send_status(BEACON_STATUS_IDLE);
            break;
            
        case BEACON_CMD_ENABLE:
            app_ctx.buzzer_state = BUZZER_STATE_ENABLED;
            beacon_send_status(BEACON_STATUS_ENABLED);
            break;
            
        case BEACON_CMD_DISABLE:
            buzzer_off();
            app_ctx.buzzer_state = BUZZER_STATE_DISABLED;
            beacon_send_status(BEACON_STATUS_DISABLED);
            break;
            
        case BEACON_CMD_STATUS:
            // Send current status
            switch (app_ctx.buzzer_state) {
                case BUZZER_STATE_IDLE:
                    beacon_send_status(BEACON_STATUS_IDLE);
                    break;
                case BUZZER_STATE_PLAYING:
                    beacon_send_status(BEACON_STATUS_PLAYING);
                    break;
                case BUZZER_STATE_ENABLED:
                    beacon_send_status(BEACON_STATUS_ENABLED);
                    break;
                case BUZZER_STATE_DISABLED:
                    beacon_send_status(BEACON_STATUS_DISABLED);
                    break;
            }
            break;
            
        default:
            // Unknown command
            break;
    }
}

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Application initialization
 ****************************************************************************************
 */
void user_app_init(void)
{
    // Initialize application context
    app_ctx.buzzer_state = BUZZER_STATE_IDLE;
    app_ctx.power_connected = false;
    app_ctx.last_command_time = 0;
    
    // Set default device name
    app_ctx.device_name[0] = 'B';
    app_ctx.device_name[1] = 'Z';
    app_ctx.device_name[2] = 'R';
    app_ctx.device_name[3] = '1';
    
    // Initialize GPIO
    gpio_init();
    
    // Initialize beacon
    beacon_init(app_ctx.device_name);
    
    // Start beacon advertising
    beacon_start_advertising();
    
    // Turn on status LED to indicate ready
    status_led_on();
}

/**
 ****************************************************************************************
 * @brief Application main loop / periodic callback
 ****************************************************************************************
 */
void user_app_on_periodic(void)
{
    // Check power sense
    app_ctx.power_connected = power_sense_read();
    
    // Update status LED based on power
    if (app_ctx.power_connected) {
        status_led_on();
    } else {
        status_led_blink();
    }
    
    // In real implementation, this would be called periodically by the SDK
}

/**
 ****************************************************************************************
 * @brief Handle incoming BLE advertising data (for scanning mode)
 * @param[in] data      Advertising data
 * @param[in] len       Data length
 * @param[in] rssi      RSSI value
 ****************************************************************************************
 */
void user_app_on_adv_report(const uint8_t *data, uint8_t len, int8_t rssi)
{
    beacon_payload_t payload;
    
    // Parse beacon payload from advertising data
    // Look for Service Data with UUID 0xAA28
    uint8_t idx = 0;
    while (idx < len) {
        uint8_t field_len = data[idx++];
        if (idx >= len) break;
        
        uint8_t field_type = data[idx++];
        
        // Check for Service Data (0x16)
        if (field_type == 0x16 && field_len >= 3) {
            uint16_t uuid = data[idx] | (data[idx+1] << 8);
            
            if (uuid == BEACON_SERVICE_UUID) {
                // Found our beacon service data
                if (beacon_parse_payload(&data[idx+2], field_len - 3, &payload)) {
                    // Check if this is for our device
                    if (memcmp(payload.device_name, app_ctx.device_name, BEACON_DEVICE_NAME_LEN) == 0) {
                        // Process the command
                        process_beacon_command(payload.command, payload.timestamp);
                    }
                }
                break;
            }
        }
        
        idx += (field_len - 1);
    }
}

/**
 ****************************************************************************************
 * @brief Application sleep mode check
 * @return Sleep mode (in real implementation)
 ****************************************************************************************
 */
int user_app_check_sleep_mode(void)
{
    // In real implementation, return appropriate sleep mode
    // Example: return arch_SLEEP_OFF or arch_SLEEP_ON
    return 0;
}
