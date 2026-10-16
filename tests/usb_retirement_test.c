#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "usb_retirement.h"
#include "xc.h"
uint32_t U1PWRC;
static bool irq=true;
uint32_t mock_disable(void){uint32_t old=irq;irq=false;return old;}
void mock_restore(uint32_t value){irq=value!=0;}
#define USB_DEVICE_HID_QUEUE_DEPTH_COMBINED 2U
#define USB_DEVICE_HID_INSTANCES_NUMBER 1U
#define USB_DEVICE_INSTANCES_NUMBER 1U
#define DRV_USBFS_INSTANCES_NUMBER 1U
#define DRV_USBFS_ENDPOINTS_NUMBER 2U
#define USB_DEVICE_IRP_STATUS_COMPLETED_SHORT 1U
#define USB_DEVICE_IRP_STATUS_SETUP 2U
#define USBFS_UOWN 0x80U
#define USBFS_STALL_SET 0x04U
static struct {unsigned status;uintptr_t data;} gUSBDeviceHIDIRP[2];
static struct {unsigned currentTxQueueSize,currentRxQueueSize;uintptr_t callback;} gUsbDeviceHidInstance[1];
typedef struct {struct {unsigned status;} irpEp0Rx,irpEp0Tx;uintptr_t callback;} USB_DEVICE_OBJ;
static USB_DEVICE_OBJ usbDeviceInstance[1];
typedef union {uint8_t byte[8];uint32_t word[2];} DRV_USBFS_BDT_ENTRY;
typedef struct {void *irpQueue;} ENDPOINT;
typedef struct {bool inUse,inInterruptContext;ENDPOINT *endpointTable;DRV_USBFS_BDT_ENTRY *pBDT;unsigned interruptSource;uintptr_t usbID;} DRV_USBFS_OBJ;
static struct {DRV_USBFS_OBJ gDrvUSBObj;ENDPOINT gDrvUSBEndpoints[4];} gDrvUSBGroup[1];
static DRV_USBFS_BDT_ENTRY bdt[8];
static unsigned deinit,poweroffs;
static bool M_DRV_USBFS_InterruptSourceDisable(unsigned source){(void)source;assert(!irq);return true;}
static void PLIB_USB_Disable(uintptr_t id){(void)id;assert(!irq);U1PWRC=8;poweroffs++;}
static void DRV_USBFS_Deinitialize(unsigned index)
{assert(index==0&&U1PWRC==0);deinit++;gDrvUSBGroup[0].gDrvUSBObj.inUse=false;}
#include "hooks.inc"
static bool Guarded(bool early)
{
    USB_RETIREMENT_ADMISSION(false);
    assert(!USB_RetirementFence());
    if(early)return true;
    return true;
}
int main(void)
{
    U1PWRC=1;assert(!USB_RetirementBlocked());
    assert(Guarded(true)&&Guarded(false));assert(USB_RetirementCallbacksIdle());
    assert(USB_RetirementAdmissionBegin());assert(!USB_RetirementFence());USB_RetirementAdmissionEnd();
    USB_RetirementCallbackEnter();assert(!USB_RetirementFence());USB_RetirementCallbackLeave();
    assert(USB_RetirementFence()&&USB_RetirementBlocked());assert(!USB_RetirementAdmissionBegin());
    U1PWRC=0;assert(!USB_RetirementHidRetire()&&!USB_RetirementDeviceRetire()&&!USB_RetirementDriverRetire());
    U1PWRC=1;
    gDrvUSBGroup[0].gDrvUSBObj=(DRV_USBFS_OBJ){.inUse=true,.endpointTable=gDrvUSBGroup[0].gDrvUSBEndpoints,.pBDT=bdt};
    gUSBDeviceHIDIRP[0].status=3;assert(!USB_RetirementHidDrained());gUSBDeviceHIDIRP[0].status=0;
    gUsbDeviceHidInstance[0].currentTxQueueSize=1;assert(!USB_RetirementHidDrained());gUsbDeviceHidInstance[0].currentTxQueueSize=0;
    usbDeviceInstance[0].irpEp0Rx.status=3;assert(!USB_RetirementDeviceDrained());usbDeviceInstance[0].irpEp0Rx.status=0;
    gDrvUSBGroup[0].gDrvUSBEndpoints[0].irpQueue=(void *)1;
    assert(!USB_RetirementDriverDrained()&&!USB_RetirementPowerOff()&&poweroffs==0);
    gDrvUSBGroup[0].gDrvUSBEndpoints[0].irpQueue=NULL;
    USB_RetirementCallbackEnter();assert(!USB_RetirementDriverDrained());USB_RetirementCallbackLeave();
    bdt[0].byte[0]=0x80;assert(!USB_RetirementDriverDrained());
    memset(bdt,0,sizeof(bdt));for(unsigned i=0;i<8;i++)bdt[i].byte[0]=0x84;
    gUSBDeviceHIDIRP[0].data=123;gUsbDeviceHidInstance[0].callback=456;
    assert(!USB_RetirementHidRetire()&&!USB_RetirementDeviceRetire()&&!USB_RetirementDriverRetire());
    assert(USB_RetirementPowerOff()&&U1PWRC==8&&irq&&poweroffs==1);
    assert(!USB_RetirementOffIdle()&&!USB_RetirementDriverRetire()&&deinit==0&&bdt[0].byte[0]==0x84);
    U1PWRC=0;assert(USB_RetirementHidRetire()&&USB_RetirementDeviceRetire());
    assert(gUSBDeviceHIDIRP[0].data==0&&gUsbDeviceHidInstance[0].callback==0);
    assert(USB_RetirementDriverRetire()&&deinit==1&&bdt[0].word[0]==0);
    assert(USB_RetirementBlocked()&&!USB_RetirementAdmissionBegin()); /* no unsafe thaw */
    USB_RetirementCallbackLeave();assert(USB_RetirementBlocked()&&!USB_RetirementFence()); /* underflow poisons */
    puts("PASS: opt-in owning-TU drain/retire hooks fence callbacks+IRPs, poweroff retains state until offidle, private data reset only drained/off; no target restart binding.");return 0;
}
