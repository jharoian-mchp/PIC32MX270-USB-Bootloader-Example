#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "usb_retirement.h"
#include "xc.h"
/* Reopening is outside this selected callback harness; fail closed. */
bool USB_RetirementHidReplacementReady(void){return false;}
bool USB_RetirementDeviceReplacementReady(void){return false;}
bool USB_RetirementDriverReplacementReady(void){return false;}
uint32_t U1PWRC=1;
static bool irq=true;
uint32_t mock_disable(void){uint32_t old=irq;irq=false;return old;}
void mock_restore(uint32_t value){irq=value!=0;}
typedef unsigned SYS_MODULE_INDEX;
typedef unsigned USB_DEVICE_HID_INDEX;
typedef uintptr_t USB_DEVICE_HID_TRANSFER_HANDLE;
typedef int USB_DEVICE_IRP_STATUS;
#define USB_DEVICE_IRP_STATUS_ABORTED -2
#define USB_DEVICE_IRP_STATUS_ABORTED_ENDPOINT_HALT -3
#define USB_DEVICE_IRP_STATUS_TERMINATED_BY_HOST -4
#define USB_DEVICE_IRP_STATUS_COMPLETED 0
#define USB_DEVICE_IRP_STATUS_COMPLETED_SHORT 1
#define USB_DEVICE_HID_RESULT_OK 0
#define USB_DEVICE_HID_RESULT_ERROR_ENDPOINT_HALTED 1
#define USB_DEVICE_HID_RESULT_ERROR_TERMINATED_BY_HOST 2
#define USB_DEVICE_HID_RESULT_ERROR 3
#define USB_DEVICE_HID_EVENT_REPORT_RECEIVED 1
#define USB_DEVICE_HID_EVENT_REPORT_SENT 2
typedef struct IRP {
    int status;size_t size;uintptr_t userData;
    void (*callback)(struct IRP *);
    struct IRP *next;
    uint8_t *data;
} USB_DEVICE_IRP,USB_DEVICE_IRP_LOCAL;
typedef struct {USB_DEVICE_IRP_LOCAL *irpQueue;} DRV_USBFS_DEVICE_ENDPOINT_OBJ;
typedef struct {size_t length;uintptr_t handle;int status;} USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED,USB_DEVICE_HID_EVENT_DATA_REPORT_SENT;
typedef struct {unsigned currentTxQueueSize,currentRxQueueSize;void(*appCallBack)(unsigned,int,void*,uintptr_t);uintptr_t userData;} USB_DEVICE_HID_INSTANCE;
static USB_DEVICE_HID_INSTANCE gUsbDeviceHidInstance[1];
#define USB_DEVICE_HID_INSTANCES_NUMBER 1U
#define USB_DEVICE_HID_QUEUE_DEPTH_COMBINED 2U
#define USB_DEVICE_IRP_STATUS_SETUP 2
static USB_DEVICE_IRP gUSBDeviceHIDIRP[2];
static unsigned rx,tx,attempts;
static uint8_t expected=0x5A;
static void Application(unsigned instance,int event,void *data,uintptr_t context)
{
    assert(instance==0&&context==123&&!USB_RetirementCallbacksIdle());
    assert(!USB_RetirementFence()); /* running callback blocks another fence */
    USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED *report=data;
    assert(report->status==USB_DEVICE_HID_RESULT_ERROR&&report->length==64);
    USB_DEVICE_IRP *irp=(USB_DEVICE_IRP*)report->handle;assert(irp->data[0]==expected);
    /* Cancellation completion is delivered, but requeue is fenced. */
    ++attempts;assert(!USB_RetirementAdmissionBegin());
    if(event==USB_DEVICE_HID_EVENT_REPORT_RECEIVED)++rx;else {assert(event==USB_DEVICE_HID_EVENT_REPORT_SENT);++tx;}
}
#include "callbacks.inc"
int main(void)
{
    uint8_t rxbuf[64]={0x5A},txbuf[64]={0x5A};
    gUSBDeviceHIDIRP[0]=(USB_DEVICE_IRP){.status=3,.size=64,.callback=F_USB_DEVICE_HID_ReportReceiveCallBack,.data=rxbuf};
    gUSBDeviceHIDIRP[1]=(USB_DEVICE_IRP){.status=3,.size=64,.callback=F_USB_DEVICE_HID_ReportSendCallBack,.data=txbuf};
    USB_DEVICE_IRP *a=&gUSBDeviceHIDIRP[0],*b=&gUSBDeviceHIDIRP[1];a->next=b;
    gUsbDeviceHidInstance[0]=(USB_DEVICE_HID_INSTANCE){.currentRxQueueSize=1,.currentTxQueueSize=1,.appCallBack=Application,.userData=123};
    DRV_USBFS_DEVICE_ENDPOINT_OBJ ep={.irpQueue=a};
    a->status=b->status=0;
    assert(USB_RetirementHidReserve(a,false));a->status=3;USB_RetirementHidSubmitResult(a,true);
    assert(USB_RetirementHidReserve(b,true));b->status=3;USB_RetirementHidSubmitResult(b,true);
    assert(USB_RetirementFence());F_DRV_USBFS_DEVICE_IRPQueueFlush(&ep,USB_DEVICE_IRP_STATUS_ABORTED);
    assert(ep.irpQueue==NULL&&rx==1&&tx==1&&attempts==2&&USB_RetirementCallbacksIdle());
    assert(gUsbDeviceHidInstance[0].currentRxQueueSize==0&&gUsbDeviceHidInstance[0].currentTxQueueSize==0);
    assert(a->status==-2&&b->status==-2&&rxbuf[0]==0x5A&&txbuf[0]==0x5A);
    /* Empty repeated cancellation cannot deliver another completion. */
    F_DRV_USBFS_DEVICE_IRPQueueFlush(&ep,USB_DEVICE_IRP_STATUS_ABORTED);assert(rx==1&&tx==1);
    /* Zero-count completion is now quarantined by the opt-in guard. This
     * injection is not evidence of actual hardware late delivery. */
    assert(!USB_RetirementHidCompletionValid((uint8_t*)a+1,false));
    /* Structural checks alone admit an old terminal entry when another RX
     * count is nonzero; the per-slot ledger rejects its consumed claim. */
    assert(USB_RetirementHidReserve(b,false));
    b->status=3;USB_RetirementHidSubmitResult(b,true);
    gUsbDeviceHidInstance[0].currentRxQueueSize=1;
    assert(USB_RetirementHidCompletionValid(a,false));
    assert(!USB_RetirementHidClaim(a,false)); /* other real slot remains armed */
    b->status=-2;USB_RETIREMENT_NOTIFY(F_USB_DEVICE_HID_ReportReceiveCallBack(b));
    assert(gUsbDeviceHidInstance[0].currentRxQueueSize==0&&rx==2);
    gUsbDeviceHidInstance[0].appCallBack=NULL;
    USB_RETIREMENT_NOTIFY(F_USB_DEVICE_HID_ReportReceiveCallBack(a));
    assert(gUsbDeviceHidInstance[0].currentRxQueueSize==0);
    assert(rx==2&&tx==1&&!USB_RetirementCallbacksIdle()&&USB_RetirementBlocked());
    puts("PASS: exact driver flush -> real HID callbacks -> app release under fence; requeue blocked, active callbacks counted, buffers retained, no duplicate empty flush.");
    puts("PASS: injected invalid/zero-count HID completion rejected and gate poisoned; no true-generation reuse guarantee, thaw/initializer remains blocked.");
    return 0;
}
