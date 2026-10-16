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
#define USB_DEVICE_IRP_STATUS_COMPLETED_SHORT 1
#define USB_DEVICE_IRP_STATUS_SETUP 2
#define USBFS_UOWN 0x80U
#define USBFS_STALL_SET 0x04U
typedef struct {int status;uintptr_t data,userData;} USB_DEVICE_IRP;
static USB_DEVICE_IRP gUSBDeviceHIDIRP[2];
typedef void (*USB_DEVICE_HID_EVENT_HANDLER)(void);
typedef void (*USB_DEVICE_EVENT_HANDLER)(void);
typedef uintptr_t USB_DEVICE_HANDLE;
typedef struct {unsigned currentTxQueueSize,currentRxQueueSize;USB_DEVICE_HID_EVENT_HANDLER appCallBack;uintptr_t userData;} USB_DEVICE_HID_INSTANCE;
static USB_DEVICE_HID_INSTANCE gUsbDeviceHidInstance[1];
static struct {bool isMutexHidIrpInitialized;} gUSBDeviceHidCommonDataObj;
typedef struct {void (*eventHandlerSet)(uintptr_t,uintptr_t,void (*)(void));} INTERFACE;
#define SYS_STATUS_READY 1
#define DRV_HANDLE_INVALID ((uintptr_t)-1)
#define USB_DEVICE_CLIENT_STATUS_READY 1
#define USB_DEVICE_TASK_STATE_RUNNING 1
#define USB_DEVICE_STATE_DETACHED 0
#define _U1CON_USBEN_MASK 1U
uint32_t U1CON;
typedef unsigned INT_SOURCE;
static bool EVIC_SourceIsEnabled(INT_SOURCE source){(void)source;return false;}
static void F_USB_DEVICE_EventHandler(void){}
static void Callback(void){}
static void SetHandler(uintptr_t handle,uintptr_t context,void(*cb)(void)){assert(handle&&context&&cb);}
typedef struct {struct {int status;} irpEp0Rx,irpEp0Tx;uintptr_t usbCDHandle;int usbDeviceInstanceState,clientState,taskState;bool inUse;USB_DEVICE_EVENT_HANDLER callBackFunc;uintptr_t context;void *ptrMasterDescTable;INTERFACE *driverInterface;struct {int usbDeviceState;} usbDeviceStatusStruct;} USB_DEVICE_OBJ;
static USB_DEVICE_OBJ usbDeviceInstance[1];
typedef union {uint8_t byte[8];uint32_t word[2];} DRV_USBFS_BDT_ENTRY;
typedef struct {void *irpQueue;} ENDPOINT;
typedef struct {bool inUse,inInterruptContext,isAttached,isOpened;int status;void(*pEventCallBack)(void);ENDPOINT *endpointTable;DRV_USBFS_BDT_ENTRY *pBDT;unsigned interruptSource;uintptr_t usbID;} DRV_USBFS_OBJ;
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
    assert(USB_RetirementHidReserve(&gUSBDeviceHIDIRP[0],false));
    assert(!USB_RetirementHidReserve(&gUSBDeviceHIDIRP[0],true));
    assert(USB_RetirementHidSubmitResult(&gUSBDeviceHIDIRP[0],false));
    assert(USB_RetirementHidSlotAvailable(0));
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
    gUSBDeviceHIDIRP[0].data=123;gUsbDeviceHidInstance[0].appCallBack=Callback;
    assert(!USB_RetirementHidRetire()&&!USB_RetirementDeviceRetire()&&!USB_RetirementDriverRetire());
    assert(USB_RetirementPowerOff()&&U1PWRC==8&&irq&&poweroffs==1);
    assert(!USB_RetirementOffIdle()&&!USB_RetirementDriverRetire()&&deinit==0&&bdt[0].byte[0]==0x84);
    U1PWRC=0;assert(USB_RetirementHidRetire()&&USB_RetirementDeviceRetire());
    assert(gUSBDeviceHIDIRP[0].data==0&&gUsbDeviceHidInstance[0].appCallBack==NULL);
    assert(USB_RetirementDriverRetire()&&deinit==1&&bdt[0].word[0]==0);
    assert(USB_RetirementBlocked()&&!USB_RetirementAdmissionBegin());
    assert(!USB_RetirementPrepare(1));assert(USB_RetirementPrepare(2));
    assert(!USB_RetirementPrepare(3));assert(!USB_RetirementReopen(2));
    INTERFACE interface={SetHandler};USB_DEVICE_HANDLE handle;
    assert(!USB_RetirementBindDevice(Callback,123,&handle));
    usbDeviceInstance[0]=(USB_DEVICE_OBJ){.usbCDHandle=1,.usbDeviceInstanceState=1,.taskState=1,.ptrMasterDescTable=&interface,.driverInterface=&interface};
    gDrvUSBGroup[0].gDrvUSBObj=(DRV_USBFS_OBJ){.inUse=true,.isOpened=true,.status=1,.pEventCallBack=Callback,.endpointTable=gDrvUSBGroup[0].gDrvUSBEndpoints,.pBDT=bdt};
    usbDeviceInstance[0].driverInterface=NULL;assert(!USB_RetirementBindDevice(Callback,123,&handle));
    usbDeviceInstance[0].driverInterface=&interface;interface.eventHandlerSet=NULL;
    assert(!USB_RetirementBindDevice(Callback,123,&handle));interface.eventHandlerSet=SetHandler;
    assert(USB_RetirementBindDevice(Callback,123,&handle));
    assert(USB_RetirementBindHid(Callback,123));gUSBDeviceHidCommonDataObj.isMutexHidIrpInitialized=true;
    U1PWRC=1;U1CON=1;assert(!USB_RetirementReopen(2));U1CON=0;
    assert(!USB_RetirementReopen(3));assert(USB_RetirementReopen(2)&&!USB_RetirementBlocked());
    assert(USB_RetirementAdmissionBegin());USB_RetirementAdmissionEnd();assert(!USB_RetirementReopen(2));
    assert(USB_RetirementFence());assert(!USB_RetirementPrepare(2));
    USB_RetirementCallbackLeave();assert(USB_RetirementBlocked()&&!USB_RetirementFence()); /* underflow poisons */
    puts("PASS: opt-in owning-TU drain/retire hooks fence callbacks+IRPs, poweroff retains state until offidle, private data reset only drained/off; no target restart binding.");return 0;
}
