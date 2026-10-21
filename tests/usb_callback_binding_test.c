#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "usb_retirement.h"
typedef uintptr_t DRV_HANDLE;
typedef void (*DRV_USB_EVENT_CALLBACK)(void);
typedef unsigned USB_INTERRUPTS;
typedef unsigned USB_OTG_INTERRUPTS;
#define DRV_HANDLE_INVALID UINTPTR_MAX
#define USB_OPMODE_DEVICE 1U
#define USB_INT_ALL 255U
#define USB_INT_RESUME 4U
#define USB_ERR_INT_ALL 255U
#define USB_OTG_INT_ALL 3U
#define USB_OTG_INT_SESSION_VALID 1U
#define USB_OTG_INT_ACTIVITY_DETECT 2U
#define SYS_DEBUG(...) ((void)0)
#define SYS_DEBUG_MESSAGE(...) ((void)0)
typedef struct {bool isOpened,isAttached;unsigned operationMode,interruptSource;uintptr_t usbID,hClientArg;DRV_USB_EVENT_CALLBACK pEventCallBack;} DRV_USBFS_OBJ;
static bool blocked,session,source;
static unsigned attaches,enables,admissions;
#if USB_RETIREMENT_HOOKS
bool USB_RetirementBlocked(void){return blocked;}
bool USB_RetirementAdmissionBegin(void){if(blocked)return false;++admissions;return true;}
void USB_RetirementAdmissionEnd(void){assert(admissions==1);--admissions;}
#endif
static void PLIB_USB_OTG_InterruptEnable(uintptr_t id,unsigned event)
{(void)id;assert(event==USB_OTG_INT_SESSION_VALID);session=true;}
static void M_DRV_USBFS_InterruptSourceEnable(unsigned irq)
{(void)irq;assert(session);source=true;++enables;}
static void PLIB_USB_OperatingModeSelect(uintptr_t id,unsigned mode)
{(void)id;assert(mode==USB_OPMODE_DEVICE);++attaches;}
static void PLIB_USB_AllInterruptEnable(uintptr_t id,USB_INTERRUPTS usb,unsigned errors,USB_OTG_INTERRUPTS otg)
{(void)id;(void)usb;assert(errors==USB_ERR_INT_ALL&&(otg&USB_OTG_INT_SESSION_VALID));session=true;}
#include "binding.inc"
static void Callback(void){}
int main(void)
{
    DRV_USBFS_OBJ driver={.isOpened=true,.operationMode=USB_OPMODE_DEVICE};
    DRV_USBFS_ClientEventCallBackSet(DRV_HANDLE_INVALID,1,Callback);
    DRV_USBFS_DEVICE_Attach(DRV_HANDLE_INVALID);assert(!enables&&!attaches&&!admissions);
    driver.isOpened=false;DRV_USBFS_ClientEventCallBackSet((uintptr_t)&driver,1,Callback);assert(!session&&!source);
    driver.isOpened=true;driver.operationMode=0;
    DRV_USBFS_ClientEventCallBackSet((uintptr_t)&driver,2,Callback);assert(driver.hClientArg==2&&!session&&!source);
    driver.operationMode=USB_OPMODE_DEVICE;blocked=true;
    DRV_USBFS_ClientEventCallBackSet((uintptr_t)&driver,3,Callback);
    assert(driver.hClientArg==3&&driver.pEventCallBack==Callback);
    DRV_USBFS_DEVICE_Attach((uintptr_t)&driver);
#if USB_RETIREMENT_HOOKS
    assert(!source&&!session&&!enables&&!attaches&&!driver.isAttached&&!admissions);
#else
    /* With hooks off, neither deferred binding nor guarded/source-enabling
     * attach changes the generated startup behaviour. */
    assert(source&&session&&enables==1&&attaches==1&&driver.isAttached&&!admissions);
#endif
    blocked=false;source=session=false;attaches=enables=0;driver.isAttached=false;
    DRV_USBFS_ClientEventCallBackSet((uintptr_t)&driver,4,Callback);
    assert(source&&session&&enables==1&&driver.hClientArg==4);
    source=session=false;DRV_USBFS_DEVICE_Attach((uintptr_t)&driver);
    assert(session&&attaches==1&&driver.isAttached&&!admissions);
#if USB_RETIREMENT_HOOKS
    assert(source&&enables==2);
#else
    assert(!source&&enables==1); /* Unmodified default attach never enabled IEC. */
#endif
    puts("PASS: exact callback/attach default-off parity and opt-in interrupt deferral/admission.");
    return 0;
}
