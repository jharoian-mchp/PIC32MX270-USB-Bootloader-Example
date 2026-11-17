#ifndef DEBUGGER_REHEARSAL_HW_H
#define DEBUGGER_REHEARSAL_HW_H
#include <xc.h>
#include "debugger_rehearsal.h"
#if !defined(__32MX270F256D__)
#error "RAM rehearsal register interface requires PIC32MX270F256D"
#endif
_Static_assert(_OSCCON_COSC_MASK==0x7000U&&_OSCCON_PLLMULT_MASK==0x70000U&&
    _OSCCON_PLLODIV_MASK==0x38000000U&&_OSCCON_PBDIV_MASK==0x180000U,"clock field drift");
_Static_assert(_OSCCON_OSWEN_MASK==1U&&_OSCCON_CF_MASK==8U&&_OSCCON_SLOCK_MASK==32U,"clock status drift");
_Static_assert(_U1PWRC_USBPWR_MASK==1U&&_U1PWRC_USBBUSY_MASK==8U&&_DMACON_ON_MASK==0x8000U,"bus field drift");
_Static_assert(_NVMCON_WR_MASK==0x8000U&&_NVMCON_WRERR_MASK==0x2000U&&_NVMCON_LVDERR_MASK==0x1000U,"NVM field drift");
_Static_assert(_DEVCFG0_DEBUG_MASK==3U&&_DEVCFG0_ICESEL_MASK==0x18U,"debug profile drift");
/* Status reads ONLY. No GPIO/setup, IRQ disable/restore, Count/Compare/Debug
 * writes, USB normal-register access or any NVM write/initialization. */
static inline RAM_REHEARSAL_RAW RAM_RehearsalHwRead(void)
{
    return (RAM_REHEARSAL_RAW){.osccon=OSCCON,.count=__builtin_mfc0(9,0),
        .status=__builtin_mfc0(12,0),.cause=__builtin_mfc0(13,0),.debug=__builtin_mfc0(23,0),
        .pwrc=U1PWRC,.dma=DMACON,.nvm=NVMCON,.cfg0=DEVCFG0,.cfg1=DEVCFG1,.cfg2=DEVCFG2,.cfg3=DEVCFG3};
}
#endif
