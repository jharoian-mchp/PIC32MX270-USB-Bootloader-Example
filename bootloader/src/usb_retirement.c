#include "usb_retirement.h"
#if USB_RETIREMENT_HOOKS
#include <xc.h>
#include <limits.h>
static volatile bool fenced,poisoned,power_off_authorized;
static volatile uint32_t callbacks,admissions;
bool USB_RetirementBlocked(void){return fenced||poisoned;}
bool USB_RetirementFence(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    bool ok=!poisoned&&callbacks==0U&&admissions==0U;
    if(ok)fenced=true;
    __builtin_mtc0(12,0,irq);return ok;
}
bool USB_RetirementCallbacksIdle(void){return !poisoned&&callbacks==0U&&admissions==0U;}
bool USB_RetirementAdmissionBegin(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    bool ok=!fenced&&!poisoned&&admissions!=UINT32_MAX;
    if(ok)++admissions;
    __builtin_mtc0(12,0,irq);return ok;
}
void USB_RetirementAdmissionEnd(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    if(admissions==0U)poisoned=true;else --admissions;
    __builtin_mtc0(12,0,irq);
}
void USB_RetirementPowerOffAuthorized(void){power_off_authorized=true;}
bool USB_RetirementRetirementAuthorized(void)
{return fenced&&power_off_authorized&&USB_RetirementCallbacksIdle()&&USB_RetirementOffIdle();}
void USB_RetirementCallbackEnter(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    if(callbacks==UINT32_MAX)poisoned=true;else ++callbacks;
    __builtin_mtc0(12,0,irq);
}
void USB_RetirementCallbackLeave(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    if(callbacks==0U)poisoned=true;else --callbacks;
    __builtin_mtc0(12,0,irq);
}
bool USB_RetirementOffIdle(void)
{return (U1PWRC&(_U1PWRC_USBPWR_MASK|_U1PWRC_USBBUSY_MASK))==0U;}
#endif
