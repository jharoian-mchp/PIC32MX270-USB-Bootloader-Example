#ifndef USB_RETIREMENT_H
#define USB_RETIREMENT_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
/* Opt-in generated-stack overlay; OFF in normal/diagnostic builds. No restart
 * command, target coordinator port or NVM binding is installed by these hooks. */
#ifndef USB_RETIREMENT_HOOKS
#define USB_RETIREMENT_HOOKS 0
#endif
#if USB_RETIREMENT_HOOKS
typedef enum {
    USB_REBUILD_DETAIL_NONE=0,USB_REBUILD_DETAIL_INPUT,USB_REBUILD_DETAIL_PREPARE,
    USB_REBUILD_DETAIL_DRIVER,USB_REBUILD_DETAIL_DEVICE,USB_REBUILD_DETAIL_OPEN,
    USB_REBUILD_DETAIL_BIND_DEVICE,USB_REBUILD_DETAIL_BIND_HID,USB_REBUILD_DETAIL_REOPEN,
    USB_REBUILD_DETAIL_HID_READY,USB_REBUILD_DETAIL_DEVICE_READY,USB_REBUILD_DETAIL_DRIVER_READY,
    USB_REBUILD_DETAIL_COMPLETE
} USB_REBUILD_DETAIL_STAGE;
/* Diagnostic-only detail word: stage8 | rejected-mask16 | observation3.
 * Observer must be a non-reentrant evidence-only RAM writer; caller holds IRQ
 * exclusion. Register before rebuild; no permission/phase decisions use it. */
void USB_RetirementDetailObserver(void (*observer)(uint32_t));
void USB_RetirementDetail(USB_REBUILD_DETAIL_STAGE stage,uint16_t rejected,uint8_t observation);
bool USB_RetirementBlocked(void);
bool USB_RetirementFence(void);
bool USB_RetirementCallbacksIdle(void); /* Also no admitted API calls. */
void USB_RetirementCallbackEnter(void);
void USB_RetirementCallbackLeave(void);
void USB_RetirementFault(void); /* Quarantine invalid/stale completion. */
bool USB_RetirementHidCompletionValid(const void *irp,bool transmit); /* Structural only. */
bool USB_RetirementHidSlotAvailable(unsigned slot);
bool USB_RetirementHidReserve(void *irp,bool transmit);
bool USB_RetirementHidSubmitResult(void *irp,bool success); /* true -> rollback unowned submit. */
bool USB_RetirementHidClaim(void *irp,bool transmit);
void USB_RetirementHidRelease(void *irp);
typedef struct { void *irp; } USB_RETIREMENT_HID_CALLBACK_SCOPE;
static inline void USB_RetirementHidCallbackCleanup(USB_RETIREMENT_HID_CALLBACK_SCOPE *scope)
{if(scope->irp!=NULL)USB_RetirementHidRelease(scope->irp);}
bool USB_RetirementAdmissionBegin(void);
void USB_RetirementAdmissionEnd(void);
/* Reopen is opt-in and permitted only after all retired layers, a one-use
 * preparation lease and independently validated detached replacement state. */
#define USB_RETIREMENT_LAYER_HID 1U
#define USB_RETIREMENT_LAYER_DEVICE 2U
#define USB_RETIREMENT_LAYER_DRIVER 4U
void USB_RetirementLayerRetired(unsigned layer);
bool USB_RetirementPrepare(uint32_t generation);
bool USB_RetirementPreparing(void);
bool USB_RetirementReopen(uint32_t generation);
bool USB_RetirementHidReplacementReady(void);
bool USB_RetirementDeviceReplacementReady(void);
bool USB_RetirementDriverReplacementReady(void);
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
