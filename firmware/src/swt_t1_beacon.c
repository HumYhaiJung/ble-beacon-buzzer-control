/**
 ****************************************************************************************
 * @file swt_t1_beacon.c
 * @brief BLE Beacon Service Data Implementation
 * 
 * This file implements the beacon advertising with custom service data format.
 * Service UUID: 0xAA28
 * Payload: [device_name:4bytes][command:1byte][timestamp:4bytes]
 ****************************************************************************************
 */

#include "swt_t1_beacon.h"
#include <string.h>

/*
 * LOCAL VARIABLES
 ****************************************************************************************
 */

/// Beacon configuration
static beacon_config_t beacon_cfg;

/// Current beacon payload
static beacon_payload_t current_payload;

/// Advertising data buffer
static uint8_t adv_data[31];
static uint8_t adv_data_len = 0;

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Build advertising data with beacon payload
 * @return Length of advertising data
 ****************************************************************************************
 */
static uint8_t beacon_build_adv_data(void)
{
    uint8_t idx = 0;
    
    // Flags
    adv_data[idx++] = 0x02;  // Length
    adv_data[idx++] = 0x01;  // Type: Flags
    adv_data[idx++] = 0x06;  // General discoverable, BR/EDR not supported
    
    // Service Data (UUID 0xAA28)
    adv_data[idx++] = BEACON_PAYLOAD_TOTAL_LEN + 3;  // Length (payload + UUID)
    adv_data[idx++] = 0x16;  // Type: Service Data
    adv_data[idx++] = (BEACON_SERVICE_UUID & 0xFF);        // UUID LSB
    adv_data[idx++] = (BEACON_SERVICE_UUID >> 8) & 0xFF;   // UUID MSB
    
    // Device name (4 bytes)
    memcpy(&adv_data[idx], current_payload.device_name, BEACON_DEVICE_NAME_LEN);
    idx += BEACON_DEVICE_NAME_LEN;
    
    // Command (1 byte)
    adv_data[idx++] = current_payload.command;
    
    // Timestamp (4 bytes, little-endian)
    adv_data[idx++] = (current_payload.timestamp) & 0xFF;
    adv_data[idx++] = (current_payload.timestamp >> 8) & 0xFF;
    adv_data[idx++] = (current_payload.timestamp >> 16) & 0xFF;
    adv_data[idx++] = (current_payload.timestamp >> 24) & 0xFF;
    
    adv_data_len = idx;
    return adv_data_len;
}

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void beacon_init(const uint8_t *device_name)
{
    // Initialize configuration
    memset(&beacon_cfg, 0, sizeof(beacon_config_t));
    memset(&current_payload, 0, sizeof(beacon_payload_t));
    
    // Set device name
    if (device_name != NULL) {
        memcpy(beacon_cfg.device_name, device_name, BEACON_DEVICE_NAME_LEN);
        memcpy(current_payload.device_name, device_name, BEACON_DEVICE_NAME_LEN);
    } else {
        // Default device name
        beacon_cfg.device_name[0] = 'B';
        beacon_cfg.device_name[1] = 'Z';
        beacon_cfg.device_name[2] = 'R';
        beacon_cfg.device_name[3] = '1';
        memcpy(current_payload.device_name, beacon_cfg.device_name, BEACON_DEVICE_NAME_LEN);
    }
    
    beacon_cfg.current_status = BEACON_STATUS_IDLE;
    beacon_cfg.advertising_enabled = false;
    
    // Initialize payload with idle state
    current_payload.command = BEACON_CMD_STOP;
    current_payload.timestamp = 0;
}

void beacon_set_command(uint8_t command, uint32_t timestamp)
{
    current_payload.command = command;
    current_payload.timestamp = timestamp;
    
    // Update status based on command
    switch (command) {
        case BEACON_CMD_STOP:
            beacon_cfg.current_status = BEACON_STATUS_IDLE;
            break;
        case BEACON_CMD_PLAY:
            beacon_cfg.current_status = BEACON_STATUS_PLAYING;
            break;
        case BEACON_CMD_ENABLE:
            beacon_cfg.current_status = BEACON_STATUS_ENABLED;
            break;
        case BEACON_CMD_DISABLE:
            beacon_cfg.current_status = BEACON_STATUS_DISABLED;
            break;
        default:
            break;
    }
    
    // Update advertising data
    beacon_update_advertising_data();
}

void beacon_send_status(uint8_t status)
{
    // Get current timestamp (simplified - in real implementation, use RTC)
    uint32_t timestamp = 0;  // Replace with actual timestamp
    
    beacon_cfg.current_status = status;
    current_payload.command = BEACON_CMD_STATUS;
    current_payload.timestamp = timestamp;
    
    // Update advertising data
    beacon_update_advertising_data();
}

bool beacon_parse_payload(const uint8_t *data, uint8_t len, beacon_payload_t *payload)
{
    if (data == NULL || payload == NULL || len < BEACON_PAYLOAD_TOTAL_LEN) {
        return false;
    }
    
    uint8_t idx = 0;
    
    // Parse device name
    memcpy(payload->device_name, &data[idx], BEACON_DEVICE_NAME_LEN);
    idx += BEACON_DEVICE_NAME_LEN;
    
    // Parse command
    payload->command = data[idx++];
    
    // Parse timestamp (little-endian)
    payload->timestamp = data[idx] | 
                        (data[idx+1] << 8) | 
                        (data[idx+2] << 16) | 
                        (data[idx+3] << 24);
    
    return true;
}

void beacon_start_advertising(void)
{
    if (!beacon_cfg.advertising_enabled) {
        beacon_cfg.advertising_enabled = true;
        beacon_build_adv_data();
        
        // In real implementation, call Dialog SDK advertising start function
        // Example: app_easy_gap_undirected_advertise_start();
    }
}

void beacon_stop_advertising(void)
{
    if (beacon_cfg.advertising_enabled) {
        beacon_cfg.advertising_enabled = false;
        
        // In real implementation, call Dialog SDK advertising stop function
        // Example: app_easy_gap_advertise_stop();
    }
}

void beacon_update_advertising_data(void)
{
    if (beacon_cfg.advertising_enabled) {
        beacon_build_adv_data();
        
        // In real implementation, update advertising data through Dialog SDK
        // Example: app_easy_gap_update_adv_data(adv_data, adv_data_len, NULL, 0);
    }
}

beacon_config_t* beacon_get_config(void)
{
    return &beacon_cfg;
}
