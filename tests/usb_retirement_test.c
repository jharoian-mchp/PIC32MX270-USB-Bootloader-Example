#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "usb_retirement.h"
#include "xc.h"
uint32_t U1PWRC;
static bool irq=true;
static uint32_t count;static unsigned count_reads;
uint32_t mock_count(void){assert(!irq);++count_reads;return count;}
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
static uint32_t con_value;static unsigned con_reads;
static uint32_t ReadCon(void){assert(!irq);++con_reads;return con_value;}
#define U1CON ReadCon()
typedef unsigned INT_SOURCE;
static bool irq_source_enabled;
static bool EVIC_SourceIsEnabled(INT_SOURCE source){(void)source;return irq_source_enabled;}
static void F_USB_DEVICE_EventHandler(void){}
static void Callback(void){}
static void SetHandler(uintptr_t handle,uintptr_t context,void(*cb)(void));
typedef struct {struct {int status;} irpEp0Rx,irpEp0Tx;uintptr_t usbCDHandle;int usbDeviceInstanceState,clientState,taskState;bool inUse;USB_DEVICE_EVENT_HANDLER callBackFunc;uintptr_t context;void *ptrMasterDescTable;INTERFACE *driverInterface;struct {int usbDeviceState;} usbDeviceStatusStruct;} USB_DEVICE_OBJ;
static USB_DEVICE_OBJ usbDeviceInstance[1];
typedef union {uint8_t byte[8];uint32_t word[2];} DRV_USBFS_BDT_ENTRY;
typedef struct {void *irpQueue;} ENDPOINT;
typedef struct {bool inUse,inInterruptContext,isAttached,isOpened;int status;void(*pEventCallBack)(void);ENDPOINT *endpointTable;DRV_USBFS_BDT_ENTRY *pBDT;unsigned interruptSource,operationMode;uintptr_t usbID,hClientArg;} DRV_USBFS_OBJ;
static struct {DRV_USBFS_OBJ gDrvUSBObj;ENDPOINT gDrvUSBEndpoints[4];} gDrvUSBGroup[1];
static DRV_USBFS_BDT_ENTRY bdt[8];
static unsigned deinit,poweroffs;
static bool M_DRV_USBFS_InterruptSourceDisable(unsigned source){(void)source;assert(!irq);bool old=irq_source_enabled;irq_source_enabled=false;return old;}
static void PLIB_USB_Disable(uintptr_t id){(void)id;assert(!irq);U1PWRC=8;poweroffs++;}
static void DRV_USBFS_Deinitialize(unsigned index)
{assert(index==0&&U1PWRC==0);deinit++;gDrvUSBGroup[0].gDrvUSBObj.inUse=false;}
#include "hooks.inc"
#include "nvm-usb-gate.inc"
typedef uintptr_t DRV_HANDLE;
typedef void (*DRV_USB_EVENT_CALLBACK)(void);
typedef unsigned USB_INTERRUPTS;
typedef unsigned USB_OTG_INTERRUPTS;
#define USB_OPMODE_DEVICE 1U
#define USB_OTG_INT_SESSION_VALID 1U
#define USB_OTG_INT_ACTIVITY_DETECT 2U
#define USB_OTG_INT_ALL 3U
#define USB_INT_ALL 255U
#define USB_INT_RESUME 4U
#define USB_ERR_INT_ALL 255U
#define SYS_DEBUG(...) ((void)0)
#define SYS_DEBUG_MESSAGE(...) ((void)0)
static unsigned session_enables,source_enables,attaches;
static bool session_enabled;
static void PLIB_USB_OTG_InterruptEnable(uintptr_t id,unsigned event)
{(void)id;assert(!irq&&event==USB_OTG_INT_SESSION_VALID);session_enabled=true;++session_enables;}
static void M_DRV_USBFS_InterruptSourceEnable(unsigned source)
{(void)source;assert(!irq&&!USB_RetirementBlocked()&&session_enabled);irq_source_enabled=true;++source_enables;}
static void PLIB_USB_OperatingModeSelect(uintptr_t id,unsigned mode)
{(void)id;assert(!irq&&!USB_RetirementBlocked()&&mode==USB_OPMODE_DEVICE);++attaches;}
static void PLIB_USB_AllInterruptEnable(uintptr_t id,USB_INTERRUPTS usb,unsigned errors,USB_OTG_INTERRUPTS otg)
{(void)id;(void)usb;assert(!irq&&!USB_RetirementBlocked()&&errors==USB_ERR_INT_ALL&&(otg&USB_OTG_INT_SESSION_VALID)!=0);session_enabled=true;}
#include "binding.inc"
static void SetHandler(uintptr_t handle,uintptr_t context,void(*cb)(void))
{assert(handle&&context&&cb);DRV_USBFS_ClientEventCallBackSet((uintptr_t)&gDrvUSBGroup[0].gDrvUSBObj,context,cb);}
static void AssertAttachBlocked(void)
{
    unsigned old_attaches=attaches,old_enables=source_enables;bool old=irq;
    DRV_USBFS_DEVICE_Attach((uintptr_t)&gDrvUSBGroup[0].gDrvUSBObj);
    assert(irq==old&&attaches==old_attaches&&source_enables==old_enables);
}
static USB_POWER_EVIDENCE captured;
static unsigned commits,copies;
static void PowerObserve(const USB_POWER_EVIDENCE *p,bool commit)
{assert(!irq);captured=*p;++copies;if(commit)++commits;}
static void PLIB_USB_Enable(unsigned usbID)
{(void)usbID;assert(!irq&&captured.meta==1&&commits==1);count=4;U1PWRC=9;}
static void Enable(void){unsigned usbID=0;
#include "power-enable.inc"
}
static uint32_t observed_detail;
static void ObserveDetail(uint32_t detail){observed_detail=detail;}
static bool Guarded(bool early)
{
    USB_RETIREMENT_ADMISSION(false);
    assert(!USB_RetirementFence());
    if(early)return true;
    return true;
}
int main(void)
{
    USB_RetirementDetailObserver(ObserveDetail);
    USB_RetirementPowerObserver(PowerObserve);USB_RetirementPowerReset();
    irq=false;USB_RetirementPowerEnableSample(false);USB_RetirementPowerReadySample(9);irq=true;
    assert(!count_reads&&!copies); /* ordinary startup adds no sample */
    /* The actual flash USB exclusion gate remains power0 AND busy0. */
    for(unsigned power=0;power<4;power++) {
        U1PWRC=((power&1U)?1U:0U)|((power&2U)?8U:0U);
        assert(UPDATE_NvmPortUsbActive()==(power!=0U));
        assert(USB_RetirementOffIdle()==(power==0U));
    }
    U1PWRC=1;assert(!USB_RetirementBlocked());
    /* Ordinary registration still enables both interrupt layers; invalid and
     * non-device calls do not. Default-off source is also checked by the overlay. */
    DRV_USBFS_OBJ *driver=&gDrvUSBGroup[0].gDrvUSBObj;
    *driver=(DRV_USBFS_OBJ){.isOpened=true,.operationMode=USB_OPMODE_DEVICE};
    irq=false;DRV_USBFS_ClientEventCallBackSet(DRV_HANDLE_INVALID,123,Callback);
    assert(!source_enables&&!session_enables);
    driver->operationMode=0;DRV_USBFS_ClientEventCallBackSet((uintptr_t)driver,123,Callback);
    assert(!source_enables&&!session_enables&&driver->hClientArg==123);
    driver->operationMode=USB_OPMODE_DEVICE;DRV_USBFS_ClientEventCallBackSet((uintptr_t)driver,456,Callback);
    assert(!irq&&irq_source_enabled&&session_enabled&&source_enables==1&&session_enables==1&&driver->hClientArg==456);
    irq=true;
    assert(Guarded(true)&&Guarded(false));assert(USB_RetirementCallbacksIdle());
    assert(USB_RetirementHidReserve(&gUSBDeviceHIDIRP[0],false));
    assert(!USB_RetirementHidReserve(&gUSBDeviceHIDIRP[0],true));
    assert(USB_RetirementHidSubmitResult(&gUSBDeviceHIDIRP[0],false));
    assert(USB_RetirementHidSlotAvailable(0));
    assert(USB_RetirementAdmissionBegin());assert(!USB_RetirementFence());USB_RetirementAdmissionEnd();
    USB_RetirementCallbackEnter();assert(!USB_RetirementFence());USB_RetirementCallbackLeave();
    assert(USB_RetirementFence()&&USB_RetirementBlocked());assert(!USB_RetirementAdmissionBegin());
    AssertAttachBlocked();
    unsigned saved_source_enables=source_enables,saved_session_enables=session_enables;
    irq=false;DRV_USBFS_ClientEventCallBackSet((uintptr_t)driver,789,Callback);irq=true;
    assert(source_enables==saved_source_enables&&session_enables==saved_session_enables&&driver->hClientArg==789);
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
    assert(USB_RetirementPowerOff()&&U1PWRC==8&&irq&&poweroffs==1&&!irq_source_enabled);
    session_enabled=false; /* Mock retired/rebuilt peripheral interrupt state. */
    assert(!USB_RetirementOffIdle()&&!USB_RetirementDriverRetire()&&deinit==0&&bdt[0].byte[0]==0x84);
    U1PWRC=0;assert(USB_RetirementHidRetire()&&USB_RetirementDeviceRetire());
    assert(gUSBDeviceHIDIRP[0].data==0&&gUsbDeviceHidInstance[0].appCallBack==NULL);
    assert(USB_RetirementDriverRetire()&&deinit==1&&bdt[0].word[0]==0);
    assert(USB_RetirementBlocked()&&!USB_RetirementAdmissionBegin());
    assert(!USB_RetirementPrepare(1));assert(USB_RetirementPrepare(2));
    assert(!USB_RetirementPrepare(3));assert(!USB_RetirementReopen(2));AssertAttachBlocked();
    assert(!count_reads); /* earlier readiness failure never samples */
    irq=false;count=UINT32_MAX-5;U1PWRC=0;Enable();
    assert(captured.meta==99&&captured.pre_count==UINT32_MAX-5&&captured.post_delta==10&&count_reads==2&&commits==2);
    USB_RetirementPowerEnableSample(false);USB_RetirementPowerEnableSample(true);assert(count_reads==2&&copies==2);
    irq=true;
    INTERFACE interface={SetHandler};USB_DEVICE_HANDLE handle;
    assert(!USB_RetirementBindDevice(Callback,123,&handle));
    usbDeviceInstance[0]=(USB_DEVICE_OBJ){.usbCDHandle=1,.usbDeviceInstanceState=1,.taskState=1,.ptrMasterDescTable=&interface,.driverInterface=&interface};
    gDrvUSBGroup[0].gDrvUSBObj=(DRV_USBFS_OBJ){.inUse=true,.isOpened=true,.status=1,.operationMode=USB_OPMODE_DEVICE,.endpointTable=gDrvUSBGroup[0].gDrvUSBEndpoints,.pBDT=bdt};
    usbDeviceInstance[0].driverInterface=NULL;assert(!USB_RetirementBindDevice(Callback,123,&handle));
    usbDeviceInstance[0].driverInterface=&interface;interface.eventHandlerSet=NULL;
    assert(!USB_RetirementBindDevice(Callback,123,&handle));interface.eventHandlerSet=SetHandler;
    irq=false;assert(USB_RetirementBindDevice(Callback,123,&handle));
    assert(!irq&&!irq_source_enabled&&!session_enabled&&source_enables==saved_source_enables&&session_enables==saved_session_enables);
    assert(driver->pEventCallBack==F_USB_DEVICE_EventHandler&&driver->hClientArg==(uintptr_t)&usbDeviceInstance[0]);
    irq=true;AssertAttachBlocked();
    assert(USB_RetirementBindHid(Callback,123));gUSBDeviceHidCommonDataObj.isMutexHidIrpInitialized=true;
    /* The first READY sample still sees powered BUSY; USBEN now rejects next. */
    U1PWRC=9;con_value=1;count=14;assert(!USB_RetirementReopen(2));
    assert(captured.meta==487&&captured.ready_delta==20&&count_reads==3&&copies==3&&commits==2);
    assert(((observed_detail>>8)&65535U)==2048U&&(observed_detail>>24)==7U);
    U1PWRC=1;con_value=1;assert(!USB_RetirementReopen(2));
    assert(count_reads==3&&captured.ready_delta==20); /* first one-shot sample remains powered BUSY */
    assert((observed_detail&255U)==USB_REBUILD_DETAIL_DRIVER_READY&&((observed_detail>>8)&65535U)==2048U);
    AssertAttachBlocked();
    /* Power-off (idle or busy) must reject before ordinary USB-register reads. */
    for(unsigned power=0;power<=8;power+=8) {
        U1PWRC=power;unsigned before=con_reads;assert(!USB_RetirementReopen(2));
        assert(con_reads==before&&((observed_detail>>8)&65535U)==16384U&&(observed_detail>>24)==(power?2U:0U));
        AssertAttachBlocked();
    }
    /* Powered BUSY does not bypass USBEN, queue, BDT or IRQ prerequisites. */
    U1PWRC=9;con_value=0;driver->endpointTable[0].irpQueue=(void*)1;
    assert(!USB_RetirementReopen(2)&&((observed_detail>>8)&65535U)==4096U);driver->endpointTable[0].irpQueue=NULL;
    bdt[0].byte[0]=USBFS_UOWN;assert(!USB_RetirementReopen(2)&&((observed_detail>>8)&65535U)==8192U);bdt[0].byte[0]=0;
    driver->pEventCallBack=NULL;unsigned before_con=con_reads;
    assert(!USB_RetirementReopen(2)&&((observed_detail>>8)&65535U)==16U&&con_reads==before_con);driver->pEventCallBack=Callback;
    irq=false;assert(USB_RetirementDriverReplacementReady());irq=true;
    assert((observed_detail>>24)==3U&&((observed_detail>>8)&65535U)==0U&&USB_RetirementBlocked());
    /* Every earlier driver check still short-circuits before U1CON, even busy+powered. */
    for(unsigned check=0;check<9;check++) {
        DRV_USBFS_OBJ saved=*driver;unsigned reads=con_reads;U1PWRC=9;
        switch(check) {
            case 0:driver->inUse=false;break;
            case 1:driver->isOpened=false;break;
            case 2:driver->status=0;break;
            case 3:driver->pEventCallBack=NULL;break;
            case 4:driver->isAttached=true;break;
            case 5:driver->inInterruptContext=true;break;
            case 6:driver->endpointTable=NULL;break;
            case 7:driver->pBDT=NULL;break;
            case 8:irq_source_enabled=true;break;
        }
        assert(!USB_RetirementReopen(2)&&con_reads==reads);
        assert(((observed_detail>>8)&65535U)==(2U<<check)&&(observed_detail>>24)==0U);
        *driver=saved;irq_source_enabled=false;AssertAttachBlocked();
    }
    /* Both powered busy values are accepted by the final driver predicate,
     * but neither alone thaws the full stack. */
    for(unsigned power=1;power<=9;power+=8) {
        U1PWRC=power;con_value=0;irq=false;assert(USB_RetirementDriverReplacementReady());irq=true;
        assert(USB_RetirementBlocked()&&((observed_detail>>8)&65535U)==0U&&(observed_detail>>24)==(power==9?3U:1U));
    }
    usbDeviceInstance[0].irpEp0Rx.status=3;assert(!USB_RetirementReopen(2));
    assert((observed_detail&255U)==USB_REBUILD_DETAIL_DEVICE_READY&&((observed_detail>>8)&65535U)==256U);
    usbDeviceInstance[0].irpEp0Rx.status=0;
    gUsbDeviceHidInstance[0].currentRxQueueSize=1;assert(!USB_RetirementReopen(2));
    assert((observed_detail&255U)==USB_REBUILD_DETAIL_HID_READY&&((observed_detail>>8)&65535U)==32U);
    gUsbDeviceHidInstance[0].currentRxQueueSize=0;
    U1PWRC=1;irq_source_enabled=true;assert(!USB_RetirementReopen(2));
    assert(((observed_detail>>8)&65535U)==512U&&(observed_detail>>24)==0U);
    irq_source_enabled=false;
    assert(!USB_RetirementReopen(3));assert((observed_detail&255U)==USB_REBUILD_DETAIL_REOPEN&&((observed_detail>>8)&65535U)==2U);
    U1PWRC=9;assert(USB_RetirementReopen(2)&&!USB_RetirementBlocked());
    assert((observed_detail&255U)==USB_REBUILD_DETAIL_DRIVER_READY&&((observed_detail>>8)&65535U)==0U);
    assert(!irq_source_enabled&&!session_enabled); /* Reopen does not unmask or attach. */
    assert(count_reads==3&&captured.meta==487&&copies==3);
    assert((observed_detail>>24)==3U); /* successful powered BUSY remains retained evidence */
    irq=false;DRV_USBFS_DEVICE_Attach((uintptr_t)driver);assert(!irq);irq=true;
    assert(irq_source_enabled&&session_enabled&&attaches==1&&driver->isAttached&&source_enables==saved_source_enables+1);
    assert(USB_RetirementCallbacksIdle());
    assert(USB_RetirementAdmissionBegin());USB_RetirementAdmissionEnd();assert(!USB_RetirementReopen(2));
    assert(USB_RetirementFence());assert(!USB_RetirementPrepare(2));
    USB_RetirementCallbackLeave();assert(USB_RetirementBlocked()&&!USB_RetirementFence()); /* underflow poisons */
    AssertAttachBlocked();
    irq=false;DRV_USBFS_ClientEventCallBackSet((uintptr_t)driver,999,Callback);irq=true;
    assert(source_enables==saved_source_enables+1&&session_enables==saved_session_enables&&driver->hClientArg==999);
    puts("PASS: opt-in owning-TU drain/retire hooks fence callbacks+IRPs, poweroff retains state until offidle, private data reset only drained/off; no target restart binding.");return 0;
}
