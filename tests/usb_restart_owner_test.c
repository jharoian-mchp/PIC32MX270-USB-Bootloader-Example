#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "usb_restart_owner.h"
#include "usb_rebuild.h"
#include "../bootloader/src/usb_probe.h"
struct SYSTEM_OBJECTS sysObj;
volatile USB_PROBE_DIAGNOSTICS usb_probe_diagnostics;
static USB_DEVICE_EVENT_HANDLER device_event;
static USB_DEVICE_HID_EVENT_HANDLER hid_event;
static uintptr_t device_context,hid_context;
static bool blocked,off,busy,irq=true;
static unsigned poweroffs,retires,rebuilds,attaches,tasks,cancels;
static uint32_t mock_gen=1;
static uint8_t endpoints[4];
bool EVIC_INT_Disable(void){bool old=irq;irq=false;return old;}
void EVIC_INT_Restore(bool old){irq=old;}
void USB_DEVICE_EventHandlerSet(USB_DEVICE_HANDLE h,USB_DEVICE_EVENT_HANDLER cb,uintptr_t ctx){assert(h==1);device_event=cb;device_context=ctx;}
int USB_DEVICE_HID_EventHandlerSet(int i,USB_DEVICE_HID_EVENT_HANDLER cb,uintptr_t ctx){assert(i==0);hid_event=cb;hid_context=ctx;return 0;}
void USB_DEVICE_Attach(USB_DEVICE_HANDLE h){assert(h==1&&!off&&!blocked);attaches++;}
void USB_DEVICE_Detach(USB_DEVICE_HANDLE h){assert(h==1);}
int USB_DEVICE_ControlStatus(USB_DEVICE_HANDLE h,int status){(void)h;(void)status;assert(!blocked);return 0;}
int USB_DEVICE_ControlSend(USB_DEVICE_HANDLE h,void *p,size_t n){(void)h;(void)p;(void)n;assert(!blocked);return 0;}
int USB_DEVICE_IRPCancelAll(USB_DEVICE_HANDLE h,uint8_t ep){assert(h==1&&blocked&&!irq);endpoints[cancels++%4]=ep;return 0;}
void USB_DEVICE_Tasks(uintptr_t object){(void)object;assert(!off&&!blocked);tasks++;}
void USB_PROBE_Initialize(USB_DEVICE_HANDLE h){assert(h==1);usb_probe_diagnostics=(USB_PROBE_DIAGNOSTICS){0};}
void USB_PROBE_Configure(bool active){usb_probe_diagnostics.active=active;}
void USB_PROBE_Event(USB_DEVICE_HID_EVENT event,void *data){(void)event;(void)data;}
void USB_PROBE_Tasks(uint32_t now){(void)now;assert(!off&&!blocked);}
bool USB_PROBE_RetireReady(void){return true;}
bool USB_RetirementFence(void){assert(!blocked);blocked=true;return true;}
bool USB_RetirementBlocked(void){return blocked;}
bool USB_RetirementHidDrained(void){return blocked;}
bool USB_RetirementDeviceDrained(void){return blocked;}
bool USB_RetirementDriverDrained(void){return blocked;}
bool USB_RetirementPowerOff(void){assert(blocked);off=busy=true;poweroffs++;return true;}
bool USB_RetirementOffIdle(void){return off&&!busy;}
bool USB_RetirementHidRetire(void){assert(off&&!busy&&blocked);retires++;return true;}
bool USB_RetirementDeviceRetire(void){assert(off&&!busy&&blocked);retires++;return true;}
bool USB_RetirementDriverRetire(void){assert(off&&!busy&&blocked);retires++;return true;}
bool USB_RebuildDetached(uint32_t generation,USB_DEVICE_EVENT_HANDLER dev,USB_DEVICE_HID_EVENT_HANDLER hid,uintptr_t context,USB_DEVICE_HANDLE *handle)
{assert(off&&!busy&&blocked&&retires%3==0&&generation==mock_gen+1&&context==generation);mock_gen=generation;off=blocked=false;rebuilds++;device_event=dev;hid_event=hid;device_context=hid_context=context;*handle=1;return true;}
int main(int argc,char **argv)
{
    assert(!USB_RestartOwnerInit(USB_DEVICE_HANDLE_INVALID,100));assert(USB_RestartOwnerInit(1,100));
    assert(device_context==1&&hid_context==1);USB_RestartOwnerTasks(0,true,true);assert(attaches==1);
    device_event(USB_DEVICE_EVENT_CONFIGURED,NULL,1);device_event(USB_DEVICE_EVENT_RESET,NULL,1);
    USB_RestartOwnerTasks(0,true,true);assert(!usb_restart_owner_diagnostics.configured);
    device_event(USB_DEVICE_EVENT_RESET,NULL,1);device_event(USB_DEVICE_EVENT_CONFIGURED,NULL,1);
    USB_RestartOwnerTasks(0,true,true);assert(usb_restart_owner_diagnostics.configured);
    uint32_t ticket;assert(USB_RestartOwnerPause(10,1,1,&ticket)==0&&ticket==1);
    assert(USB_RestartOwnerPause(10,1,1,&ticket)==1&&ticket==1);
    assert(USB_RestartOwnerPause(10,2,1,&ticket)==2);
    assert(USB_RestartOwnerPause(9,1,1,&ticket)==2);
    USB_RestartOwnerAck(ticket+1,true,2);assert(cancels==0);
    USB_RestartOwnerAck(ticket,true,3);assert(blocked&&cancels==4&&!memcmp(endpoints,(uint8_t[]){1,129,0,128},4));
    unsigned old=tasks;USB_RestartOwnerTasks(4,true,true);assert(poweroffs==1&&tasks==old);
    USB_RestartOwnerTasks(5,true,true);assert(rebuilds==0);busy=false;
    USB_RestartOwnerTasks(6,true,true);USB_RestartOwnerTasks(7,true,true);USB_RestartOwnerTasks(8,true,true);
    assert(rebuilds==1&&attaches==2&&device_context==2&&hid_context==2);
    device_event(USB_DEVICE_EVENT_CONFIGURED,NULL,1);assert(!usb_restart_owner_diagnostics.configured&&usb_restart_owner_diagnostics.stale_events==1);
    device_event(USB_DEVICE_EVENT_RESET,NULL,2);device_event(USB_DEVICE_EVENT_CONFIGURED,NULL,2);
    USB_RestartOwnerTasks(9,true,true);assert(usb_restart_owner_diagnostics.state==USB_RESTART_DONE&&usb_restart_owner_diagnostics.completed_cycles==1);
    assert(USB_RestartOwnerRequest(10,&ticket)&&ticket==2);USB_RestartOwnerAck(ticket,true,11);
    USB_RestartOwnerTasks(12,true,true);busy=false;USB_RestartOwnerTasks(13,true,true);USB_RestartOwnerTasks(14,true,true);USB_RestartOwnerTasks(15,true,true);
    assert(rebuilds==2&&device_context==3);device_event(USB_DEVICE_EVENT_CONFIGURED,NULL,3);
    USB_RestartOwnerTasks(16,true,true);assert(usb_restart_owner_diagnostics.completed_cycles==2);
    if(argc>1&&strcmp(argv[1],"error")==0)device_event(USB_DEVICE_EVENT_ERROR,NULL,3);
    else if(argc>1&&strcmp(argv[1],"ack-reset")==0)
    {
        assert(USB_RestartOwnerRequest(17,&ticket));unsigned before=cancels;
        device_event(USB_DEVICE_EVENT_RESET,NULL,3);USB_RestartOwnerAck(ticket,true,18);assert(cancels==before);
    }
    else device_event(USB_DEVICE_EVENT_RESET,NULL,3);
    old=tasks;USB_RestartOwnerTasks(19,true,true);
    assert(usb_restart_owner_diagnostics.state==USB_RESTART_FAILED&&tasks==old);
    assert(!usb_restart_owner_diagnostics.configured&&!USB_RestartOwnerRequest(20,&ticket));
    puts("PASS: inactive target owner cancels HID+EP0, gates tasks/off-idle/retire/rebuild, routes generation callbacks and repeated resets/fault quarantine; mocked stack APIs only.");return 0;
}
