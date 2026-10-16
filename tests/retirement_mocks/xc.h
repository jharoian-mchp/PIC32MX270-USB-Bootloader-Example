#ifndef MOCK_XC_H
#define MOCK_XC_H
#include <stdint.h>
extern uint32_t U1PWRC;
#define _U1PWRC_USBPWR_MASK 1U
#define _U1PWRC_USBBUSY_MASK 8U
uint32_t mock_disable(void);
void mock_restore(uint32_t value);
#define __builtin_disable_interrupts mock_disable
#define __builtin_mtc0(reg,sel,value) mock_restore(value)
#endif
