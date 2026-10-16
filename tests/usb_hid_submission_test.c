#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "usb_retirement.h"
#include "xc.h"
/* Reopening is outside this selected submission harness; fail closed. */
bool USB_RetirementHidReplacementReady(void){return false;}
bool USB_RetirementDeviceReplacementReady(void){return false;}
bool USB_RetirementDriverReplacementReady(void){return false;}
uint32_t U1PWRC=1;static bool irq=true;
uint32_t mock_disable(void){uint32_t old=irq;irq=false;return old;}
void mock_restore(uint32_t old){irq=old!=0;}
#define USB_DEVICE_HID_QUEUE_DEPTH_COMBINED 2U
#define USB_DEVICE_HID_INSTANCES_NUMBER 1U
#define USB_DEVICE_IRP_STATUS_COMPLETED_SHORT 1
#define USB_DEVICE_IRP_STATUS_SETUP 2
#define USB_DEVICE_IRP_STATUS_COMPLETED 0
#define USB_DEVICE_IRP_STATUS_ABORTED_ENDPOINT_HALT -3
#define USB_DEVICE_IRP_STATUS_TERMINATED_BY_HOST -4
#define USB_ERROR_NONE 0
#define OSAL_RESULT_TRUE 1
#define OSAL_WAIT_FOREVER 0
#define OSAL_CRIT_TYPE_HIGH 0
#define USB_DEVICE_HID_TRANSFER_HANDLE_INVALID ((uintptr_t)-1)
#define USB_DEVICE_HID_RESULT_OK 0
#define USB_DEVICE_HID_RESULT_ERROR 1
#define USB_DEVICE_HID_RESULT_ERROR_INSTANCE_INVALID 2
#define USB_DEVICE_HID_RESULT_ERROR_INSTANCE_NOT_CONFIGURED 3
#define USB_DEVICE_HID_RESULT_ERROR_TRANSFER_QUEUE_FULL 4
#define USB_DEVICE_HID_RESULT_ERROR_ENDPOINT_HALTED 5
#define USB_DEVICE_HID_RESULT_ERROR_TERMINATED_BY_HOST 6
#define USB_DEVICE_HID_EVENT_REPORT_SENT 1
#define USB_DEVICE_HID_EVENT_REPORT_RECEIVED 2
#define SYS_ASSERT(condition,message) ((void)(condition),(void)(message))
typedef unsigned USB_DEVICE_HID_INDEX,SYS_MODULE_INDEX;
typedef uintptr_t USB_DEVICE_HID_TRANSFER_HANDLE;
typedef int USB_DEVICE_HID_RESULT,USB_ERROR,OSAL_RESULT,OSAL_CRITSECT_DATA_TYPE;
typedef unsigned USB_DEVICE_HID_TRANSFER_FLAGS;
typedef struct IRP {int status;size_t size;void *data;void(*callback)(struct IRP*);uintptr_t userData;unsigned flags;} USB_DEVICE_IRP;
typedef struct {size_t queueSizeReportSend,queueSizeReportReceive;} HID_INIT;
typedef struct {unsigned interruptEpTxReady,interruptEpRxReady;} FLAGS;
typedef struct {FLAGS flags;HID_INIT *hidFuncInit;unsigned currentTxQueueSize,currentRxQueueSize;uintptr_t devLayerHandle;unsigned endpointTx,endpointRx;void(*appCallBack)(unsigned,int,void*,uintptr_t);uintptr_t userData;} USB_DEVICE_HID_INSTANCE;
typedef struct {size_t length;uintptr_t handle;int status;} USB_DEVICE_HID_EVENT_DATA_REPORT_SENT,USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED;
static USB_DEVICE_IRP gUSBDeviceHIDIRP[2];static USB_DEVICE_HID_INSTANCE gUsbDeviceHidInstance[1];
static struct {unsigned mutexHIDIRP;} gUSBDeviceHidCommonDataObj;
static int mode;static unsigned notifications;static USB_DEVICE_IRP *pending;
static OSAL_RESULT OSAL_MUTEX_Lock(unsigned *m,unsigned t){(void)m;(void)t;return 1;}
static OSAL_RESULT OSAL_MUTEX_Unlock(unsigned *m){(void)m;return 1;}
static OSAL_CRITSECT_DATA_TYPE OSAL_CRIT_Enter(unsigned type){(void)type;return mock_disable();}
static void OSAL_CRIT_Leave(unsigned type,OSAL_CRITSECT_DATA_TYPE old){(void)type;mock_restore(old);}
void F_USB_DEVICE_HID_ReportSendCallBack(USB_DEVICE_IRP*);
void F_USB_DEVICE_HID_ReportReceiveCallBack(USB_DEVICE_IRP*);
static USB_ERROR USB_DEVICE_IRPSubmit(uintptr_t handle,unsigned ep,USB_DEVICE_IRP *irp)
{
    (void)handle;(void)ep;
    if(mode==1)return 9; /* unowned rejection */
    irp->status=4;pending=irp;
    if(mode==2){irp->status=0;USB_RETIREMENT_NOTIFY(irp->callback(irp));pending=NULL;}
    return mode==3?9:0; /* queued-but-error retains source */
}
static void Application(unsigned i,int event,void *data,uintptr_t context)
{
    (void)event;(void)data;(void)context;assert(i==0);++notifications;
    assert(!USB_RetirementHidSlotAvailable(0)||!USB_RetirementHidSlotAvailable(1));
}
#include "submission.inc"
int main(void)
{
    HID_INIT init={2,2};gUsbDeviceHidInstance[0]=(USB_DEVICE_HID_INSTANCE){.flags={1,1},.hidFuncInit=&init,.appCallBack=Application};
    uint8_t data[64]={0};uintptr_t handle;mode=1;
    assert(USB_DEVICE_HID_ReportSend(0,&handle,data,64)==9&&handle==USB_DEVICE_HID_TRANSFER_HANDLE_INVALID);
    assert(gUsbDeviceHidInstance[0].currentTxQueueSize==0&&USB_RetirementHidSlotAvailable(0));
    mode=2;assert(USB_DEVICE_HID_ReportReceive(0,&handle,data,64)==0);
    assert(notifications==1&&gUsbDeviceHidInstance[0].currentRxQueueSize==0&&USB_RetirementHidSlotAvailable(0));
    mode=3;assert(USB_DEVICE_HID_ReportSend(0,&handle,data,64)==9&&handle!=(uintptr_t)-1);
    assert(gUsbDeviceHidInstance[0].currentTxQueueSize==1&&!USB_RetirementHidSlotAvailable(0));
    pending->status=-2;USB_RETIREMENT_NOTIFY(pending->callback(pending));pending=NULL;
    assert(notifications==2&&gUsbDeviceHidInstance[0].currentTxQueueSize==0&&USB_RetirementHidSlotAvailable(0));
    assert(USB_RetirementCallbacksIdle());
    puts("PASS: exact HID APIs rollback unowned failure, retain queued-error ownership and hold callback-before-submit-return ledger; mocked lower driver only.");return 0;
}
