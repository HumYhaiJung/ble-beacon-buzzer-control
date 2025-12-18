/**
 ****************************************************************************************
 * @file swt_t1_beacon.h
 * @brief BLE Beacon-based Buzzer Control Header
 * 
 * This header defines the beacon payload format and command codes for controlling
 * the buzzer through BLE beacon advertising data.
 ****************************************************************************************
 */

#ifndef _SWT_T1_BEACON_H_
#define _SWT_T1_BEACON_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * DEFINES
 ****************************************************************************************
 */

/// Beacon Service UUID (16-bit)
#define BEACON_SERVICE_UUID                 0xAA28

/// Beacon payload sizes
#define BEACON_DEVICE_NAME_LEN              4
#define BEACON_COMMAND_LEN                  1
#define BEACON_TIMESTAMP_LEN                4
#define BEACON_PAYLOAD_TOTAL_LEN            (BEACON_DEVICE_NAME_LEN + BEACON_COMMAND_LEN + BEACON_TIMESTAMP_LEN)

/// Command codes
#define BEACON_CMD_STOP                     0x00
#define BEACON_CMD_PLAY                     0x01
#define BEACON_CMD_ENABLE                   0x02
#define BEACON_CMD_DISABLE                  0x03
#define BEACON_CMD_STATUS                   0xFF

/// Status codes
#define BEACON_STATUS_IDLE                  0x00
#define BEACON_STATUS_PLAYING               0x01
#define BEACON_STATUS_ENABLED               0x02
#define BEACON_STATUS_DISABLED              0x03

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Beacon payload structure
typedef struct {
    uint8_t device_name[BEACON_DEVICE_NAME_LEN];   ///< Device name (4 bytes)
    uint8_t command;                                 ///< Command code
    uint32_t timestamp;                              ///< Timestamp (4 bytes)
} beacon_payload_t;

/// Beacon configuration structure
typedef struct {
    uint8_t device_name[BEACON_DEVICE_NAME_LEN];   ///< Device name
    uint8_t current_status;                          ///< Current device status
    bool advertising_enabled;                        ///< Advertising state
} beacon_config_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize beacon advertising
 * @param[in] device_name   Device name (4 bytes)
 * @return void
 ****************************************************************************************
 */
void beacon_init(const uint8_t *device_name);

/**
 ****************************************************************************************
 * @brief Set command in beacon payload
 * @param[in] command       Command code to set
 * @param[in] timestamp     Timestamp value
 * @return void
 ****************************************************************************************
 */
void beacon_set_command(uint8_t command, uint32_t timestamp);

/**
 ****************************************************************************************
 * @brief Send status response via beacon
 * @param[in] status        Current status to send
 * @return void
 ****************************************************************************************
 */
void beacon_send_status(uint8_t status);

/**
 ****************************************************************************************
 * @brief Parse received beacon payload
 * @param[in] data          Pointer to beacon data
 * @param[in] len           Length of beacon data
 * @param[out] payload      Parsed payload structure
 * @return true if parse successful, false otherwise
 ****************************************************************************************
 */
bool beacon_parse_payload(const uint8_t *data, uint8_t len, beacon_payload_t *payload);

/**
 ****************************************************************************************
 * @brief Start beacon advertising
 * @return void
 ****************************************************************************************
 */
void beacon_start_advertising(void);

/**
 ****************************************************************************************
 * @brief Stop beacon advertising
 * @return void
 ****************************************************************************************
 */
void beacon_stop_advertising(void);

/**
 ****************************************************************************************
 * @brief Update beacon advertising data
 * @return void
 ****************************************************************************************
 */
void beacon_update_advertising_data(void);

/**
 ****************************************************************************************
 * @brief Get current beacon configuration
 * @return Pointer to beacon configuration
 ****************************************************************************************
 */
beacon_config_t* beacon_get_config(void);

#endif // _SWT_T1_BEACON_H_
