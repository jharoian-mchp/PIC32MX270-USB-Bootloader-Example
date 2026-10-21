#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "usb_restart_owner.h"
#include "usb_rebuild.h"
#include "../bootloader/src/usb_probe.h"
#include "update_protocol.h"
struct SYSTEM_OBJECTS sysObj;
static USB_DEVICE_EVENT_HANDLER device_event;
static USB_DEVICE_HID_EVENT_HANDLER hid_event;
static uintptr_t device_context,hid_context;
static bool blocked,off,busy,irq=true,rx_owned,tx_owned;
static unsigned poweroffs,retires,rebuilds,attaches,tasks,cancels;
static uint32_t mock_gen=1;
static uint8_t *rx,*tx,sent[64];
void USB_RetirementPowerReset(void){}
void USB_RetirementPowerObserver(void (*observer)(const USB_POWER_EVIDENCE *,bool)){(void)observer;}
void USB_RetirementDetailObserver(void (*observer)(uint32_t)){(void)observer;}
bool EVIC_INT_Disable(void){bool old=irq;irq=false;return old;}
void EVIC_INT_Restore(bool old){irq=old;}
void USB_DEVICE_EventHandlerSet(USB_DEVICE_HANDLE h,USB_DEVICE_EVENT_HANDLER cb,uintptr_t ctx){assert(h==1);device_event=cb;device_context=ctx;}
int USB_DEVICE_HID_EventHandlerSet(int i,USB_DEVICE_HID_EVENT_HANDLER cb,uintptr_t ctx){assert(i==0);hid_event=cb;hid_context=ctx;return 0;}
void USB_DEVICE_Attach(USB_DEVICE_HANDLE h){assert(h==1&&!off&&!blocked);attaches++;}
void USB_DEVICE_Detach(USB_DEVICE_HANDLE h){assert(h==1);}
int USB_DEVICE_ControlStatus(USB_DEVICE_HANDLE h,int s){(void)h;(void)s;assert(!blocked);return 0;}
int USB_DEVICE_ControlSend(USB_DEVICE_HANDLE h,void *p,size_t n){(void)h;(void)p;(void)n;assert(!blocked);return 0;}
void USB_DEVICE_Tasks(uintptr_t object){(void)object;assert(!off&&!blocked);tasks++;}
USB_DEVICE_HID_RESULT USB_DEVICE_HID_ReportReceive(int i,USB_DEVICE_HID_TRANSFER_HANDLE *handle,void *data,size_t n)
{assert(i==0&&n==64&&!irq&&!blocked&&!rx_owned);*handle=100;rx=data;rx_owned=true;return 0;}
USB_DEVICE_HID_RESULT USB_DEVICE_HID_ReportSend(int i,USB_DEVICE_HID_TRANSFER_HANDLE *handle,void *data,size_t n)
{assert(i==0&&n==64&&!irq&&!blocked&&!tx_owned);*handle=200;tx=data;memcpy(sent,tx,64);tx_owned=true;return 0;}
int USB_DEVICE_IRPCancelAll(USB_DEVICE_HANDLE h,uint8_t ep)
{
    assert(h==1&&blocked&&!irq);cancels++;
    USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED d={0,ep==1?100U:200U,-1};
    if(ep==1&&rx_owned){rx_owned=false;hid_event(0,USB_DEVICE_HID_EVENT_REPORT_RECEIVED,&d,hid_context);}
    if(ep==129&&tx_owned){tx_owned=false;hid_event(0,USB_DEVICE_HID_EVENT_REPORT_SENT,&d,hid_context);}
    return 0;
}
bool USB_RetirementFence(void){assert(!blocked&&!irq);blocked=true;return true;}
bool USB_RetirementBlocked(void){return blocked;}
bool USB_RetirementHidDrained(void){return blocked&&!rx_owned&&!tx_owned;}
bool USB_RetirementDeviceDrained(void){return blocked;}
bool USB_RetirementDriverDrained(void){return blocked&&!rx_owned&&!tx_owned;}
bool USB_RetirementPowerOff(void){assert(blocked);off=busy=true;poweroffs++;return true;}
bool USB_RetirementOffIdle(void){return off&&!busy;}
bool USB_RetirementHidRetire(void){assert(off&&!busy&&blocked);retires++;return true;}
bool USB_RetirementDeviceRetire(void){assert(off&&!busy&&blocked);retires++;return true;}
bool USB_RetirementDriverRetire(void){assert(off&&!busy&&blocked);retires++;return true;}
bool USB_RebuildDetached(uint32_t generation,USB_DEVICE_EVENT_HANDLER dev,USB_DEVICE_HID_EVENT_HANDLER hid,uintptr_t context,USB_DEVICE_HANDLE *handle)
{assert(off&&!busy&&blocked&&retires%3==0&&generation==mock_gen+1);mock_gen=generation;off=blocked=false;rebuilds++;device_event=dev;hid_event=hid;device_context=hid_context=context;*handle=1;return true;}
static void Receive(const uint8_t *p,size_t size,uint32_t now)
{
    size_t n;uint8_t wire[64];assert(rx_owned&&UPDATE_FrameEncode(p,size,wire,sizeof(wire),&n));
    memcpy(rx,wire,n);rx_owned=false;USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED d={n,100,0};
    hid_event(0,USB_DEVICE_HID_EVENT_REPORT_RECEIVED,&d,hid_context);USB_RestartOwnerTasks(now,true,true);
}
int main(int argc,char **argv)
{
    assert(USB_RestartOwnerInit(1,10000));USB_RestartOwnerTasks(0,true,true);
    device_event(USB_DEVICE_EVENT_RESET,NULL,1);device_event(USB_DEVICE_EVENT_CONFIGURED,NULL,1);
    USB_RestartOwnerTasks(10,true,true);USB_RestartOwnerTasks(20,true,true);assert(rx_owned);
    if(argc>1)
    {
        uint8_t retained[64];memcpy(retained,rx,64);uint32_t ticket;
        assert(USB_RestartOwnerRequest(21,&ticket));
        unsigned old=tasks;
        if(strcmp(argv[1],"failed-ack")==0)USB_RestartOwnerAck(ticket,false,22);
        else if(strcmp(argv[1],"reset-wait")==0)device_event(USB_DEVICE_EVENT_RESET,NULL,1);
        USB_RestartOwnerTasks(strcmp(argv[1],"expired-ack")==0?10022:22,true,true);
        assert(usb_restart_owner_diagnostics.state==USB_RESTART_FAILED&&!usb_restart_owner_diagnostics.attached);
        assert(rx_owned&&memcmp(retained,rx,64)==0&&tasks==old&&poweroffs==0&&rebuilds==0);
        /* RX remains statically owned until actual completion/reset, never
         * reused just because the failed owner detached from the host. */
        USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED cancelled={0,100,-1};rx_owned=false;
        hid_event(0,USB_DEVICE_HID_EVENT_REPORT_RECEIVED,&cancelled,hid_context);
        assert(USB_PROBE_RetireReady());
        USB_RestartOwnerTasks(10023,true,true);assert(tasks==old&&poweroffs==0);
        puts("PASS: failed/expired/reset ACK detaches but retains pending real-probe RX storage until actual completion; no task/rebuild/retry.");
        return 0;
    }
    Receive((uint8_t[]){0x66,1,10,0,0,0,1,0,0,0},10,21);assert(tx_owned&&!blocked&&!poweroffs);
    assert(!memcmp(tx,sent,64));tx_owned=false;USB_DEVICE_HID_EVENT_DATA_REPORT_SENT done={64,200,0};
    hid_event(0,USB_DEVICE_HID_EVENT_REPORT_SENT,&done,hid_context);
    unsigned old=tasks;USB_RestartOwnerTasks(22,true,true);
    assert(blocked&&cancels==4&&tasks==old&&!rx_owned); /* no same-task EP0/rx submit after ACK */
    USB_RestartOwnerTasks(23,true,true);assert(poweroffs==1);busy=false;
    USB_RestartOwnerTasks(24,true,true);USB_RestartOwnerTasks(25,true,true);USB_RestartOwnerTasks(26,true,true);
    assert(rebuilds==1&&device_context==2&&attaches==2);
    device_event(USB_DEVICE_EVENT_RESET,NULL,2);device_event(USB_DEVICE_EVENT_CONFIGURED,NULL,2);
    USB_RestartOwnerTasks(30,true,true);USB_RestartOwnerTasks(40,true,true);assert(rx_owned);
    Receive((uint8_t[]){0x67,1,11,0,0,0},6,41);assert(tx_owned);
    UPDATE_FRAME frame;UPDATE_FrameReset(&frame);size_t used;assert(UPDATE_FrameFeed(&frame,sent,64,&used)==UPDATE_FRAME_READY);
    assert(frame.length==21&&frame.bytes[0]==0xE7&&frame.bytes[8]==1&&frame.bytes[12]==2&&frame.bytes[16]==1);
    puts("PASS: real probe -> owner pause ACK -> cancellation/off/retire/mock rebuild -> generation/status, no same-task submission after fence.");return 0;
}
