#ifndef USB_RETIREMENT_H
#define USB_RETIREMENT_H
#include <stdbool.h>
#include <stdint.h>
/* Opt-in generated-stack overlay; OFF in normal/diagnostic builds. No restart
 * command, target coordinator port or NVM binding is installed by these hooks. */
#ifndef USB_RETIREMENT_HOOKS
#define USB_RETIREMENT_HOOKS 0
#endif
#if USB_RETIREMENT_HOOKS
bool USB_RetirementBlocked(void);
bool USB_RetirementFence(void);
bool USB_RetirementCallbacksIdle(void); /* Also no admitted API calls. */
void USB_RetirementCallbackEnter(void);
void USB_RetirementCallbackLeave(void);
void USB_RetirementFault(void); /* Quarantine invalid/stale completion. */
bool USB_RetirementHidCompletionValid(const void *irp,bool transmit);
bool USB_RetirementAdmissionBegin(void);
void USB_RetirementAdmissionEnd(void);
/* Latch is set only by a successful reviewed driver power-off hook. Fence is
 * one-shot today; no thaw/reopen until a replacement initializer is reviewed. */
void USB_RetirementPowerOffAuthorized(void);
bool USB_RetirementRetirementAuthorized(void);
typedef struct { bool admitted; } USB_RETIREMENT_ADMISSION_SCOPE;
static inline void USB_RetirementAdmissionCleanup(USB_RETIREMENT_ADMISSION_SCOPE *scope)
{if(scope->admitted)USB_RetirementAdmissionEnd();}
#define USB_RETIREMENT_ADMISSION(result) \
    USB_RETIREMENT_ADMISSION_SCOPE usb_retirement_scope __attribute__((cleanup(USB_RetirementAdmissionCleanup)))={USB_RetirementAdmissionBegin()}; \
    if(!usb_retirement_scope.admitted)return result
bool USB_RetirementOffIdle(void);
bool USB_RetirementHidDrained(void);
bool USB_RetirementDeviceDrained(void);
bool USB_RetirementDriverDrained(void);
bool USB_RetirementPowerOff(void);
bool USB_RetirementHidRetire(void);
bool USB_RetirementDeviceRetire(void);
bool USB_RetirementDriverRetire(void);
#define USB_RETIREMENT_NOTIFY(call) do { USB_RetirementCallbackEnter(); call; USB_RetirementCallbackLeave(); } while(0)
#else
#define USB_RETIREMENT_NOTIFY(call) do { call; } while(0)
#endif
#endif
