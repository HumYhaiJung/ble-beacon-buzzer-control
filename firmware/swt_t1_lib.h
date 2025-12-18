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
#include "attm_db_128.h"

/* Enable feature guard (kept for compatibility) */
#ifndef BLE_SWT_T1_ENABLED
#define BLE_SWT_T1_ENABLED 1
#endif

/* 128-bit UUIDs (derived from ASCII "swt-t1-call") */
static const uint8_t SWT_T1_SVC_UUID[16] = {
    's','w','t','-','t','1','-','c','a','l','l', 0x00,0x00,0x00,0x00,0x00
};

static const uint8_t SWT_T1_CHAR_UUID[16] = {
    's','w','t','-','t','1','-','c','a','l','l', 0x11,0x11,0x11,0x11,0x11
};

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
void swt_t1_on_write(const uint8_t *buf, uint16_t len);
uint16_t swt_t1_get_char_handle(void);

#endif // SWT_T1_H_