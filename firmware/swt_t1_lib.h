/**
 ****************************************************************************************
 *
 * @file swt_t1.h
 *
 * @brief SWT-T1 Vendor Service Profile Header
 *
 ****************************************************************************************
 */

#ifndef SWT_T1_H_
#define SWT_T1_H_

#include <stdint.h>
#include "attm_db.h"

/* Enable feature guard (kept for compatibility) */
#ifndef BLE_SWT_T1_ENABLED
#define BLE_SWT_T1_ENABLED 1
#endif

/* Use 16-bit UUIDs for now to test basic GATT functionality */
#define SWT_T1_SVC_UUID_16 0xFFF0  /* Use a different custom service UUID */
#define SWT_T1_CHAR_UUID_16 0xFFF1 /* Custom characteristic UUID */

/* 128-bit UUIDs (for future use) */
static const uint8_t SWT_T1_SVC_UUID[16] = {
    0x73, 0x77, 0x74, 0x2d, 0x74, 0x31, 0x2d, 0x63, 0x61, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00};

static const uint8_t SWT_T1_CHAR_UUID[16] = {
    0x73, 0x77, 0x74, 0x2d, 0x74, 0x31, 0x2d, 0x63, 0x61, 0x6c, 0x6c, 0x11, 0x11, 0x11, 0x11, 0x11};

/* Attribute indices inside the service */
enum swt_t1_att_idx
{
    SWT_T1_IDX_SVC = 0,
    SWT_T1_IDX_CHAR_DECL,
    SWT_T1_IDX_CHAR_VAL,
    SWT_T1_IDX_NB
};

/* Callback type for writes */
typedef void (*swt_t1_write_cb_t)(uint8_t cmd);

/* Public API */
void swt_t1_init(void);
void swt_t1_register_write_cb(swt_t1_write_cb_t cb);
int swt_t1_on_write(const uint8_t *buf, uint16_t len);
uint16_t swt_t1_get_char_handle(void);

#endif // SWT_T1_H_