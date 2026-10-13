/* Toolchain verification only. Not MCC-generated or intended for programming. */
#include <xc.h>
#if !defined(__32MX270F256D__)
#error Wrong target MCU
#endif
#pragma config FNOSC = PRIPLL
#pragma config POSCMOD = HS
#pragma config FPLLIDIV = DIV_3
#pragma config FPLLMUL = MUL_24
#pragma config FPLLODIV = DIV_2
#pragma config UPLLIDIV = DIV_3
#pragma config UPLLEN = ON
#pragma config FWDTEN = OFF
#pragma config ICESEL = ICS_PGx4
#pragma config FUSBIDIO = OFF
#pragma config FVBUSONIO = OFF
int main(void)
{
    volatile unsigned int part = DEVID;
    (void)part;
    while (1) { __asm__ volatile("nop"); }
    return 0;
}
