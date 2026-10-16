/* Selected generated function bodies are extracted byte-for-byte at build time.
 * Mock types retain only the fields those functions access. No hardware code
 * runs; observed gaps are reproduced, not certified safe. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef unsigned USB_DEVICE_HID_INDEX;
typedef unsigned SYS_MODULE_INDEX;
typedef int OSAL_RESULT;
#define OSAL_RESULT_TRUE 1
#define SYS_ERROR_INFO 0
#define SYS_DEBUG_MESSAGE(level,message) ((void)(level),(void)(message))
#define SYS_DEBUG(level,message) ((void)(level),(void)(message))
#define DRV_USBFS_INSTANCES_NUMBER 1U
typedef uintptr_t SYS_MODULE_OBJ;
#define USB_DEVICE_HID_INSTANCES_NUMBER 1U
#define SYS_MODULE_OBJ_INVALID ((SYS_MODULE_OBJ)-1)
#define SYS_STATUS_UNINITIALIZED 0
#define OSAL_RESULT_FALSE 0
#define USB_DEVICE_HID_RESULT_OK 0
#define USB_DEVICE_HID_RESULT_ERROR_INSTANCE_INVALID 1
#define USB_DEVICE_HID_RESULT_ERROR_PARAMETER_INVALID 2
#define USB_DEVICE_HID_ENDPOINT_TX 0
#define USB_DEVICE_HID_ENDPOINT_RX 1
#define USB_DEVICE_HID_ENDPOINT_ALL 2
#define USB_DEVICE_HID_FLAGS_ALL_CLEAR 0
static unsigned assertions;
#define SYS_ASSERT(condition,message) do { if(!(condition))++assertions; } while(0)
typedef unsigned OSAL_MUTEX_HANDLE_TYPE;
typedef unsigned USB_DEVICE_HID_RESULT;
typedef void (*USB_DEVICE_HID_EVENT_HANDLER)(int event,void *data,uintptr_t context);
typedef struct {
    union { unsigned allFlags;struct {unsigned interruptEpTxReady:1,interruptEpRxReady:1;}; } flags;
    uintptr_t devLayerHandle;
    unsigned endpointTx,endpointRx;
    unsigned currentTxQueueSize,currentRxQueueSize;
    USB_DEVICE_HID_EVENT_HANDLER appCallBack;
    uintptr_t userData;
} USB_DEVICE_HID_INSTANCE;
static USB_DEVICE_HID_INSTANCE gUsbDeviceHidInstance[1];
static struct {bool isMutexHidIrpInitialized;OSAL_MUTEX_HANDLE_TYPE mutexHIDIRP;} gUSBDeviceHidCommonDataObj;
typedef struct {int usbDeviceInstanceState;bool inUse;} USB_DEVICE_OBJ;
static USB_DEVICE_OBJ usbDeviceInstance[1];
typedef struct {
    bool inUse,isOpened;
    unsigned mutexID;
    int status;
    USB_DEVICE_HID_EVENT_HANDLER pEventCallBack;
    unsigned interruptSource;
    uintptr_t usbID;
} DRV_USBFS_OBJ;
static struct {DRV_USBFS_OBJ gDrvUSBObj;} gDrvUSBGroup[1];
static bool powered,hardware_busy,irq_enabled,pending_irp,create_fails,synchronous_cancel;
static unsigned creates,cancels,disables;
static char events[64];static size_t event_count;
static void Event(char e){assert(event_count<sizeof(events));events[event_count++]=e;}
static int OSAL_MUTEX_Create(OSAL_MUTEX_HANDLE_TYPE *mutex)
{++creates;if(create_fails)return OSAL_RESULT_FALSE;*mutex=99;return 1;}
static int OSAL_MUTEX_Delete(unsigned *mutex)
{(void)mutex;assert(powered&&irq_enabled&&pending_irp);Event('M');return 1;}
static int USB_DEVICE_IRPCancelAll(uintptr_t handle,unsigned endpoint)
{
    (void)handle;++cancels;Event('C');
    if(synchronous_cancel)
    {
        if(endpoint==0x81U)gUsbDeviceHidInstance[0].currentTxQueueSize=0;
        else gUsbDeviceHidInstance[0].currentRxQueueSize=0;
        pending_irp=gUsbDeviceHidInstance[0].currentTxQueueSize!=0U||gUsbDeviceHidInstance[0].currentRxQueueSize!=0U;
    }
    return 0;
}
static int USB_DEVICE_EndpointDisable(uintptr_t handle,unsigned endpoint)
{(void)handle;(void)endpoint;++disables;Event('E');return 0;}
static bool M_DRV_USBFS_InterruptSourceDisable(unsigned source)
{(void)source;Event('I');irq_enabled=false;return true;}
static void M_DRV_USBFS_InterruptSourceClear(unsigned source){(void)source;Event('F');}
static void PLIB_USB_Disable(uintptr_t id){(void)id;Event('P');powered=false;hardware_busy=true;}
static void Callback(int event,void *data,uintptr_t context)
{(void)event;(void)data;(void)context;}
#include "functions.inc"
int main(void)
{
    /* Global HID mutex persists; reinitialisation is not pool retirement. */
    gUSBDeviceHidCommonDataObj.isMutexHidIrpInitialized=false;
    F_USB_DEVICE_HID_GlobalInitialize();assert(creates==1&&gUSBDeviceHidCommonDataObj.isMutexHidIrpInitialized);
    F_USB_DEVICE_HID_GlobalInitialize();assert(creates==1);
    assert(USB_DEVICE_HID_EventHandlerSet(0,Callback,77)==USB_DEVICE_HID_RESULT_OK);
    assert(USB_DEVICE_HID_EventHandlerSet(0,NULL,0)==USB_DEVICE_HID_RESULT_ERROR_PARAMETER_INVALID);
    assert(gUsbDeviceHidInstance[0].appCallBack==Callback&&gUsbDeviceHidInstance[0].userData==77);
    /* Function deinit requests cleanup; it does not prove callback/counter drain. */
    USB_DEVICE_HID_INSTANCE *hid=&gUsbDeviceHidInstance[0];
    hid->endpointTx=0x81U;hid->endpointRx=0x01U;
    hid->flags.interruptEpTxReady=hid->flags.interruptEpRxReady=1;
    hid->currentTxQueueSize=hid->currentRxQueueSize=1;pending_irp=true;
    F_USB_DEVICE_HID_DeInitialize(0);
    assert(cancels==2&&disables==2&&hid->flags.allFlags==0&&pending_irp);
    assert(hid->currentTxQueueSize==1&&hid->currentRxQueueSize==1&&hid->appCallBack==Callback);
    /* A synchronous cancel port can drain counts, but deinit itself does not
     * verify that outcome; callback/mutex state still persists. */
    synchronous_cancel=true;hid->flags.interruptEpTxReady=hid->flags.interruptEpRxReady=1;
    F_USB_DEVICE_HID_DeInitialize(0);
    assert(!pending_irp&&hid->currentTxQueueSize==0&&hid->currentRxQueueSize==0&&hid->appCallBack==Callback);
    /* Device deinit is status-only; no function cancellation or controller close. */
    unsigned old=cancels;usbDeviceInstance[0].usbDeviceInstanceState=1;usbDeviceInstance[0].inUse=true;
    USB_DEVICE_Deinitialize(0);assert(usbDeviceInstance[0].usbDeviceInstanceState==0&&usbDeviceInstance[0].inUse&&cancels==old);
    assert(assertions==1); /* NULL callback registration was rejected. */
    /* Actual driver body calls mutex delete before interrupt mask/module
     * disable. Basic OSAL delete is a no-op; this records call order, not free.
     * Neither drain nor off-idle wait is enforced by this function. */
    event_count=0;powered=irq_enabled=pending_irp=true;hardware_busy=false;
    gDrvUSBGroup[0].gDrvUSBObj=(DRV_USBFS_OBJ){.inUse=true,.isOpened=true,.mutexID=99,.status=1,.pEventCallBack=Callback};
    DRV_USBFS_Deinitialize(0);
    assert(event_count==4&&!memcmp(events,"MIFP",4));
    assert(!powered&&hardware_busy&&pending_irp&&!irq_enabled);
    assert(!gDrvUSBGroup[0].gDrvUSBObj.inUse&&!gDrvUSBGroup[0].gDrvUSBObj.isOpened&&gDrvUSBGroup[0].gDrvUSBObj.pEventCallBack==NULL);
    puts("REPRODUCED: HID mutex/callback state persists; counters depend on cancel completion with no drain verification; device deinit status-only; driver mutex-delete call precedes mask/power-off, without off-idle wait (basic OSAL delete is no-op).");
    return 0;
}
