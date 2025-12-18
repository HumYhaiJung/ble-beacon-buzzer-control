/**
 ****************************************************************************************
 *
 * @file swt_t1_lib.c
 *
 * @brief SWT-T1 Profile:  create 128-bit service + 1 write-only characteristic
 *
 * Uses attm_svc_create_db_128() from attm_db_128.c (present in your SDK).
 *
 ****************************************************************************************
 */

#include "swt_t1_lib.h"
#include "attm_db_128.h"
#include "attm.h"
#include "att.h"
#include "ke_mem.h"
#include "ke_task.h"
#include "stdio.h"
#include <string.h>

/* Internal storage for callback and handle */
static swt_t1_write_cb_t s_write_cb = NULL;
static uint16_t s_char_handle = 0;

/* 16-bit UUID bytes for Characteristic Declaration (0x2803) */
static const uint8_t char_decl_uuid16[2] = { 0x03, 0x28 };

/* Attribute database description (128-bit aware) */
static const struct attm_desc_128 swt_t1_att_db[SWT_T1_IDX_NB] =
{
    /* Service declaration entry (index 0): service UUID is provided in value */
    [SWT_T1_IDX_SVC] = {
        .uuid = SWT_T1_SVC_UUID,
        .uuid_size = ATT_UUID_128_LEN,
        .perm = PERM(RD, ENABLE),
        .max_length = 0,
        .length = ATT_UUID_128_LEN,
        .value = (uint8_t *)SWT_T1_SVC_UUID
    },

    /* Characteristic declaration (index 1) - 0x2803 (16-bit UUID) */
    [SWT_T1_IDX_CHAR_DECL] = {
        .uuid = char_decl_uuid16,
        .uuid_size = ATT_UUID_16_LEN,
        .perm = PERM(RD, ENABLE),
        .max_length = 0,
        .length = 0,
        .value = NULL
    },

    /* Characteristic value (index 2) - 128-bit UUID, write-only, 1 byte */
    [SWT_T1_IDX_CHAR_VAL] = {
        .uuid = SWT_T1_CHAR_UUID,
        .uuid_size = ATT_UUID_128_LEN,
        .perm = PERM(WR, ENABLE),
        .max_length = 1,
        .length = 0,
        .value = NULL
    }
};

/**
 * Create SWT-T1 service in ATT DB (128-bit)
 */
void swt_t1_init(void)
{
    uint16_t start_hdl = 0;
    uint8_t att_tbl[SWT_T1_IDX_NB];
    uint8_t status;

#if defined(TASK_APP)
    ke_task_id_t dest_id = (ke_task_id_t) TASK_APP;
#else
    ke_task_id_t dest_id = (ke_task_id_t) 0;
#endif

    status = attm_svc_create_db_128(0, &start_hdl, NULL, SWT_T1_IDX_NB,
                                   att_tbl, dest_id, swt_t1_att_db, PERM_MASK_SVC_PRIMARY);

    if (status == ATT_ERR_NO_ERROR)
    {
        s_char_handle = start_hdl + SWT_T1_IDX_CHAR_VAL;
    }
    else
    {
        s_char_handle = 0;
    }
}

/* Register write callback */
void swt_t1_register_write_cb(swt_t1_write_cb_t cb)
{
    s_write_cb = cb;
}

/* Forward write to registered callback */
void swt_t1_on_write(const uint8_t *buf, uint16_t len)
{
    if (len == 0) return;
    if (s_write_cb) s_write_cb(buf[0]);
}

/* Return handle (0 if not created) */
uint16_t swt_t1_get_char_handle(void)
{
    return s_char_handle;
}