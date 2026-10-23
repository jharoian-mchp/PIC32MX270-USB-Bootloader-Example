/* Source/compile-only RAM_REHEARSAL_DEBUG_V1. No functions/peripherals.
 * Object bytes checked against exact words. No fuse programming permission
 * and no debugger-installed executive/content equivalence follows. */
#include <xc.h>
#if !defined(__32MX270F256D__)
#error "rehearsal configuration requires PIC32MX270F256D"
#endif
#pragma config DEBUG = ON
#pragma config JTAGEN = OFF
#pragma config ICESEL = ICS_PGx4
#pragma config PWP = OFF
#pragma config BWP = OFF
#pragma config CP = OFF
#pragma config FNOSC = PRIPLL
#pragma config FPBDIV = DIV_1
#pragma config FSOSCEN = OFF
#pragma config IESO = OFF
#pragma config POSCMOD = HS
#pragma config OSCIOFNC = OFF
#pragma config FCKSM = CSDCMD
#pragma config WDTPS = PS1048576
#pragma config FWDTEN = OFF
#pragma config WINDIS = OFF
#pragma config FWDTWINSZ = WINSZ_50
#pragma config FPLLIDIV = DIV_3
#pragma config FPLLMUL = MUL_24
#pragma config FPLLODIV = DIV_2
#pragma config UPLLEN = ON
#pragma config UPLLIDIV = DIV_3
#pragma config FVBUSONIO = ON
#pragma config USERID = 0xffff
#pragma config PMDL1WAY = ON
#pragma config IOL1WAY = ON
#pragma config FUSBIDIO = ON
