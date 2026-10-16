#include "usb_restart_owner.h"
#if USB_RETIREMENT_HOOKS
#include "usb_rebuild.h"
#include "usb_probe.h"
static USB_RESTART restart;
static USB_DEVICE_HANDLE handle;
static bool initialized,attached,configured;
static volatile uint32_t callback_generation,event_flags;
#define OWNER_RESET 1U
#define OWNER_CONFIGURED 2U
#define OWNER_ERROR 4U
static uint8_t idle_rate;
static uint32_t pause_id,pause_generation,pause_ticket;
volatile USB_RESTART_OWNER_DIAGNOSTICS usb_restart_owner_diagnostics;
static USB_DEVICE_HID_EVENT_RESPONSE HidEvent(USB_DEVICE_HID_INDEX index,USB_DEVICE_HID_EVENT event,void *data,uintptr_t context)
{
    (void)index;
    if(context!=callback_generation){++usb_restart_owner_diagnostics.stale_events;return USB_DEVICE_HID_EVENT_RESPONSE_NONE;}
    USB_PROBE_Event(event,data);
    if(!USB_RetirementBlocked())
    {
        if(event==USB_DEVICE_HID_EVENT_SET_IDLE&&data!=NULL)
        {idle_rate=((USB_DEVICE_HID_EVENT_DATA_SET_IDLE *)data)->duration;(void)USB_DEVICE_ControlStatus(handle,USB_DEVICE_CONTROL_STATUS_OK);}
        else if(event==USB_DEVICE_HID_EVENT_GET_IDLE)(void)USB_DEVICE_ControlSend(handle,&idle_rate,1);
    }
    return USB_DEVICE_HID_EVENT_RESPONSE_NONE;
}
static void DeviceEvent(USB_DEVICE_EVENT event,void *data,uintptr_t context)
{
    (void)data;
    if(context!=callback_generation){++usb_restart_owner_diagnostics.stale_events;return;}
    /* ISR only records flags. Main owns restart/core/layer transitions. */
    if(event==USB_DEVICE_EVENT_CONFIGURED)event_flags|=OWNER_CONFIGURED;
    else if(event==USB_DEVICE_EVENT_RESET||event==USB_DEVICE_EVENT_DECONFIGURED)
        event_flags=(event_flags|OWNER_RESET)&~OWNER_CONFIGURED; /* Latest reset invalidates prior configuration. */
    else if(event==USB_DEVICE_EVENT_ERROR)event_flags|=OWNER_ERROR;
}
static bool Fence(void *c){(void)c;return USB_RetirementFence();}
static bool CancelDetach(void *c)
{
    (void)c;bool irq=EVIC_INT_Disable();
    USB_PROBE_Configure(false);configured=false;usb_restart_owner_diagnostics.configured=false;
    bool ok=true;
    /* HID data endpoints plus both control directions; lower cancel invokes
     * accounted callbacks, whose re-submission is fenced. */
    const uint8_t endpoints[]={0x01,0x81,0x00,0x80};
    for(unsigned i=0;i<sizeof(endpoints);i++)
        if(USB_DEVICE_IRPCancelAll(handle,endpoints[i])!=USB_ERROR_NONE){ok=false;++usb_restart_owner_diagnostics.cancel_errors;}
    USB_DEVICE_Detach(handle);attached=false;
    EVIC_INT_Restore(irq);return ok;
}
static bool Drained(void *c)
{(void)c;return USB_PROBE_RetireReady()&&USB_RetirementHidDrained()&&USB_RetirementDeviceDrained()&&USB_RetirementDriverDrained();}
static bool PowerOff(void *c){(void)c;return USB_RetirementPowerOff();}
static bool OffIdle(void *c){(void)c;return USB_RetirementOffIdle();}
static bool Retire(void *c)
{(void)c;return USB_RetirementHidRetire()&&USB_RetirementDeviceRetire()&&USB_RetirementDriverRetire();}
static bool Rebuild(void *c,uint32_t generation)
{
    (void)c;USB_DEVICE_HANDLE rebuilt=USB_DEVICE_HANDLE_INVALID;
    bool irq=EVIC_INT_Disable();callback_generation=generation;event_flags=0;
    bool ok=USB_RebuildDetached(generation,DeviceEvent,HidEvent,generation,&rebuilt);
    if(ok){handle=rebuilt;attached=configured=false;USB_PROBE_Initialize(handle);}
    EVIC_INT_Restore(irq);return ok;
}
static bool Configured(void *c,uint32_t generation)
{(void)c;return configured&&generation==restart.generation;}
bool USB_RestartOwnerInit(USB_DEVICE_HANDLE initial,uint32_t timeout)
{
    if(initial==USB_DEVICE_HANDLE_INVALID||initialized)return false;
    if(usb_probe_diagnostics.rx_pending||usb_probe_diagnostics.tx_pending)return false;
    const USB_RESTART_PORT port={NULL,Fence,CancelDetach,Drained,PowerOff,OffIdle,Retire,Rebuild,Configured};
    if(!USB_RestartInit(&restart,&port,timeout))return false;
    handle=initial;initialized=true;attached=configured=false;idle_rate=0;callback_generation=1;event_flags=0;
    usb_restart_owner_diagnostics=(USB_RESTART_OWNER_DIAGNOSTICS){.generation=1,.exclusive=true};
    USB_DEVICE_EventHandlerSet(handle,DeviceEvent,1);
    (void)USB_DEVICE_HID_EventHandlerSet(USB_DEVICE_HID_INDEX_0,HidEvent,1);
    USB_DEVICE_Detach(handle);USB_PROBE_Initialize(handle);return true;
}
bool USB_RestartOwnerRequest(uint32_t now,uint32_t *ticket)
{
    if(!initialized||!configured||event_flags!=0U)return false;
    return USB_RestartRequest(&restart,now,ticket);
}
unsigned USB_RestartOwnerPause(uint32_t id,uint32_t generation,uint32_t now,uint32_t *ticket)
{
    if(ticket==NULL||id==0U)return 2;
    if(id==pause_id)
    {if(generation!=pause_generation)return 2;*ticket=pause_ticket;return 1;}
    if(id<pause_id||generation!=restart.generation||!USB_RestartOwnerRequest(now,ticket))return 2;
    pause_id=id;pause_generation=generation;pause_ticket=*ticket;return 0;
}
void USB_RestartOwnerStatus(uint32_t *ticket,uint32_t *generation)
{*ticket=pause_ticket;*generation=restart.generation;usb_restart_owner_diagnostics.state=restart.state;}
void USB_RestartOwnerAck(uint32_t ticket,bool success,uint32_t now)
{
    if(!initialized)return;
    bool irq=EVIC_INT_Disable();
    if(event_flags&(OWNER_RESET|OWNER_ERROR))USB_RestartCancel(&restart);
    else USB_RestartAck(&restart,ticket,success,now);
    EVIC_INT_Restore(irq);
}
void USB_RestartOwnerTasks(uint32_t now,bool cable,bool mux)
{
    if(!initialized)return;
    /* Hold the single-core global IRQ fence through event application and
     * scheduling. A new ISR reset cannot race an old CONFIGURED snapshot. */
    bool irq=EVIC_INT_Disable();uint32_t events=event_flags;event_flags=0;
    if(events&OWNER_RESET)
    {
        USB_PROBE_Configure(false);configured=false;usb_restart_owner_diagnostics.configured=false;
        if(restart.state==USB_RESTART_ACK_WAIT||restart.state==USB_RESTART_DONE)USB_RestartCancel(&restart);
    }
    if(events&OWNER_ERROR)
    {
        USB_PROBE_Configure(false);configured=false;usb_restart_owner_diagnostics.configured=false;
        USB_RestartCancel(&restart);
    }
    if((events&OWNER_CONFIGURED)&&!(events&OWNER_ERROR))
    {
        if(restart.state==USB_RESTART_READY||restart.state==USB_RESTART_CONFIG_WAIT||restart.state==USB_RESTART_DONE)
        {configured=true;usb_restart_owner_diagnostics.configured=true;USB_PROBE_Configure(true);}
        else USB_RestartCancel(&restart);
    }
    if(!cable||!mux)
    {
        configured=false;usb_restart_owner_diagnostics.configured=false;USB_PROBE_Configure(false);
        if(restart.state!=USB_RESTART_READY)USB_RestartCancel(&restart);
    }
    USB_RESTART_STATE previous=restart.state;
    USB_RestartPoll(&restart,now);
    if(previous!=USB_RESTART_DONE&&restart.state==USB_RESTART_DONE)++usb_restart_owner_diagnostics.completed_cycles;
    usb_restart_owner_diagnostics.state=restart.state;usb_restart_owner_diagnostics.generation=restart.generation;
    if(restart.state==USB_RESTART_READY||restart.state==USB_RESTART_ACK_WAIT||
       restart.state==USB_RESTART_CONFIG_WAIT||restart.state==USB_RESTART_DONE)
    {
        if(cable&&mux&&!attached)
        {USB_DEVICE_Attach(handle);attached=true;}
        else if((!cable||!mux)&&attached)
        {USB_PROBE_Configure(false);USB_DEVICE_Detach(handle);attached=false;configured=false;
         usb_restart_owner_diagnostics.configured=false;}
        if((event_flags&(OWNER_RESET|OWNER_ERROR))==0U)USB_PROBE_Tasks(now);
        /* Successful pause send completion can fence/detach inside probe
         * Tasks. Recheck before any device task can queue EP0 work. */
        if((event_flags&(OWNER_RESET|OWNER_ERROR))==0U&&
           (restart.state==USB_RESTART_READY||restart.state==USB_RESTART_ACK_WAIT||
            restart.state==USB_RESTART_CONFIG_WAIT||restart.state==USB_RESTART_DONE))
            USB_DEVICE_Tasks(sysObj.usbDevObject0);
    }
    usb_restart_owner_diagnostics.attached=attached;
    EVIC_INT_Restore(irq);
    /* DRAIN uses synchronous cancellation/observations only; OFF_WAIT only
     * reads U1PWRC. No normal task/attach/report calls through retirement. */
}
#endif
