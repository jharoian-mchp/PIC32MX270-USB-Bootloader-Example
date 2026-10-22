#ifndef ISOLATED_NVM_TARGET_HW_H
#define ISOLATED_NVM_TARGET_HW_H
/* Unbound PIC32MX270-only register access. Host tests substitute this header.
 * No main, clock/PMD/cache initialization, interrupt enable or NVM write. */
#include <xc.h>
#if !defined(__32MX270F256D__)
#error "isolated preflight requires PIC32MX270F256D"
#endif
#include "isolated_nvm_target.h"
_Static_assert(_OSCCON_COSC_MASK==0x7000U&&_OSCCON_PLLMULT_MASK==0x70000U&&
    _OSCCON_PLLODIV_MASK==0x38000000U&&_OSCCON_PBDIV_MASK==0x180000U,"oscillator field drift");
_Static_assert(_OSCCON_SLOCK_MASK==32U&&_OSCCON_CF_MASK==8U&&_OSCCON_OSWEN_MASK==1U,"oscillator status drift");
_Static_assert(_U1PWRC_USBPWR_MASK==1U&&_U1PWRC_USBBUSY_MASK==8U&&_DMACON_ON_MASK==0x8000U,"bus field drift");
_Static_assert(_NVMCON_WR_MASK==0x8000U&&_NVMCON_WRERR_MASK==0x2000U&&_NVMCON_LVDERR_MASK==0x1000U,"NVM field drift");
_Static_assert(_CNPUB_CNPUB2_MASK==4U&&_CNPDB_CNPDB2_MASK==4U,"RB2 pulls drift");
static inline ISOLATED_TARGET_RAW ISOLATED_TargetHwRead(void)
{
    return (ISOLATED_TARGET_RAW){.osccon=OSCCON,.count=__builtin_mfc0(9,0),
        .status=__builtin_mfc0(12,0),.cause=__builtin_mfc0(13,0),.debug=__builtin_mfc0(23,0),
        .pwrc=U1PWRC,.dma=DMACON,.nvm=NVMCON,.cfg0=DEVCFG0,.cfg1=DEVCFG1,.cfg2=DEVCFG2,.cfg3=DEVCFG3,
        .anselb=ANSELB,.trisb=TRISB,.pullup=CNPUB,.pulldown=CNPDB};
}
static inline void ISOLATED_TargetHwGpio(void){ANSELBCLR=4U;TRISBSET=4U;}
static inline uint32_t ISOLATED_TargetHwCount(void){return __builtin_mfc0(9,0);}
static inline bool ISOLATED_TargetHwPressed(void){return (PORTB&4U)==0U;}
#endif
