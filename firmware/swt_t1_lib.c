/**
 ****************************************************************************************
 *
 * @file swt_t1_lib.c
 *
 * @brief Minimal SWT-T1 implementation - just handle writes
 *
 ****************************************************************************************
 */

#include "swt_t1_lib.h"

/* Internal storage for callback */
static swt_t1_write_cb_t s_write_cb = NULL;

/**
 * Simplified init with GPIO debugging
 */
void swt_t1_init(void)
{
    // Initialization placeholder. Do NOT reconfigure the buzzer pin here
    // because the application configures it for PWM (PID_PWM0). Overwriting
    // the pin function here prevents hardware PWM from driving the buzzer.
    (void)0;
}

/* Register write callback */
void swt_t1_register_write_cb(swt_t1_write_cb_t cb)
{
    s_write_cb = cb;
}

/* Forward write to registered callback; return 1 if handled, 0 otherwise */
int swt_t1_on_write(const uint8_t *buf, uint16_t len)
{
    if (len == 0)
        return 0;
    if (s_write_cb)
    {
        s_write_cb(buf[0]);
        return 1;
    }
    return 0;
}

/* Return dummy handle */
uint16_t swt_t1_get_char_handle(void)
{
    return 0x0001; // Dummy handle
}