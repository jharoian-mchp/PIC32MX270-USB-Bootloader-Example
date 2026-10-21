#include "usb_restart_owner.h"
#if USB_RETIREMENT_HOOKS
#include "usb_rebuild.h"
#include "usb_probe.h"
#include "usb_restart_journal.h"
#if defined(__XC32)
static USB_RESTART_JOURNAL __attribute__((persistent,section(".persist.usb_restart_journal"),used)) restart_journal;
#else
static USB_RESTART_JOURNAL restart_journal;
#endif
static USB_RESTART_RECORD previous_record;
static bool previous_record_valid,runtime_record_valid;
static USB_RESTART_RECORD runtime_record;
static uint8_t wait_evidence;
static USB_RESTART_CHECKPOINT checkpoint;
static uint32_t owner_now,rebuild_detail;
static USB_POWER_EVIDENCE owner_power;
static void RecordPhase(uint32_t now);
#if USB_RESTART_DIAGNOSTIC
static void PowerChanged(const USB_POWER_EVIDENCE *power,bool commit)
{owner_power=*power;if(commit)RecordPhase(owner_now);}
static void PowerReset(void){owner_power=(USB_POWER_EVIDENCE){0};USB_RetirementPowerReset();}
#else
static void PowerReset(void){owner_power=(USB_POWER_EVIDENCE){0};}
#endif
static void PortEnter(USB_RESTART_CHECKPOINT port)
{checkpoint=port;rebuild_detail=0;usb_restart_owner_diagnostics.checkpoint=port;RecordPhase(owner_now);}
static void DetailChanged(uint32_t detail)
{rebuild_detail=detail;RecordPhase(owner_now);}
static USB_RESTART restart;
static USB_DEVICE_HANDLE handle;
static bool initialized,attached,configured;
static bool usb_registers_accessible=true;
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
static bool Fence(void *c){(void)c;PortEnter(USB_RESTART_PORT_FENCE);return USB_RetirementFence();}
static bool CancelDetach(void *c)
{
    (void)c;bool irq=EVIC_INT_Disable();PortEnter(USB_RESTART_PORT_CANCEL);
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
{
    (void)c;PortEnter(USB_RESTART_PORT_DRAIN);
    bool probe=USB_PROBE_RetireReady(),hid=USB_RetirementHidDrained();
    bool device=USB_RetirementDeviceDrained(),driver=USB_RetirementDriverDrained();
    wait_evidence=(probe?0U:1U)|(hid?0U:2U)|(device?0U:4U)|(driver?0U:8U);
    return probe&&hid&&device&&driver;
}
static bool PowerOff(void *c)
{
    (void)c;PortEnter(USB_RESTART_PORT_POWER_OFF);
    /* After a power-off attempt, do not touch ordinary USB registers even if
     * the port reports failure. Partial shutdown is not idle proof. */
    usb_registers_accessible=false;return USB_RetirementPowerOff();
}
static bool OffIdle(void *c)
{(void)c;PortEnter(USB_RESTART_PORT_OFF_IDLE);bool idle=USB_RetirementOffIdle();wait_evidence=idle?0U:16U;return idle;}
static bool Retire(void *c)
{(void)c;PortEnter(USB_RESTART_PORT_RETIRE);return USB_RetirementHidRetire()&&USB_RetirementDeviceRetire()&&USB_RetirementDriverRetire();}
static bool Rebuild(void *c,uint32_t generation)
{
    (void)c;USB_DEVICE_HANDLE rebuilt=USB_DEVICE_HANDLE_INVALID;
    bool irq=EVIC_INT_Disable();PowerReset();PortEnter(USB_RESTART_PORT_REBUILD);callback_generation=generation;event_flags=0;
    bool ok=USB_RebuildDetached(generation,DeviceEvent,HidEvent,generation,&rebuilt);
    if(ok){handle=rebuilt;attached=configured=false;usb_registers_accessible=true;USB_PROBE_Initialize(handle);}
    EVIC_INT_Restore(irq);return ok;
}
static bool Configured(void *c,uint32_t generation)
{(void)c;PortEnter(USB_RESTART_PORT_CONFIGURED);return configured&&generation==restart.generation;}
bool USB_RestartOwnerInit(USB_DEVICE_HANDLE initial,uint32_t timeout)
{
    if(initial==USB_DEVICE_HANDLE_INVALID||initialized)return false;
    if(usb_probe_diagnostics.rx_pending||usb_probe_diagnostics.tx_pending)return false;
    const USB_RESTART_PORT port={NULL,Fence,CancelDetach,Drained,PowerOff,OffIdle,Retire,Rebuild,Configured};
    if(!USB_RestartInit(&restart,&port,timeout))return false;
    handle=initial;initialized=true;attached=configured=false;idle_rate=0;callback_generation=1;event_flags=0;
    wait_evidence=0;runtime_record_valid=false;checkpoint=USB_RESTART_PORT_NONE;owner_now=0;
    usb_restart_owner_diagnostics=(USB_RESTART_OWNER_DIAGNOSTICS){.generation=1,.exclusive=true};
    USB_DEVICE_EventHandlerSet(handle,DeviceEvent,1);
    (void)USB_DEVICE_HID_EventHandlerSet(USB_DEVICE_HID_INDEX_0,HidEvent,1);
    bool journal_irq=EVIC_INT_Disable();
    previous_record_valid=USB_RestartJournalRead(&restart_journal,&previous_record);
    rebuild_detail=0;PowerReset();USB_RetirementDetailObserver(DetailChanged);
#if USB_RESTART_DIAGNOSTIC
    USB_RetirementPowerObserver(PowerChanged);
#endif
    EVIC_INT_Restore(journal_irq);
    USB_DEVICE_Detach(handle);USB_PROBE_Initialize(handle);return true;
}
bool USB_RestartOwnerRequest(uint32_t now,uint32_t *ticket)
{
    bool irq=EVIC_INT_Disable();
    if(!initialized||!configured||event_flags!=0U){EVIC_INT_Restore(irq);return false;}
    bool ok=USB_RestartRequest(&restart,now,ticket);
    if(ok){owner_now=now;wait_evidence=0;checkpoint=USB_RESTART_PORT_NONE;PowerReset();RecordPhase(now);}
    EVIC_INT_Restore(irq);return ok;
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
bool USB_RestartOwnerPrevious(USB_RESTART_RECORD *record)
{
    if(record==NULL||!previous_record_valid)return false;
    *record=previous_record;return true;
}
static void RecordPhase(uint32_t now)
{
    if(restart.ticket==0U)return;
    uint8_t evidence=wait_evidence;
    USB_RESTART_RECORD record={.ticket=restart.ticket,.generation=restart.generation,.time_ms=now,
        .state=restart.state,.phase=restart.state==USB_RESTART_FAILED?restart.failure_phase:restart.state,
        .fault=restart.fault,.checkpoint=checkpoint,.detail=rebuild_detail,.power=owner_power,.evidence=evidence};
    if(runtime_record_valid&&runtime_record.ticket==record.ticket&&runtime_record.generation==record.generation&&
       runtime_record.state==record.state&&runtime_record.phase==record.phase&&runtime_record.fault==record.fault&&
       runtime_record.evidence==record.evidence&&runtime_record.checkpoint==record.checkpoint&&runtime_record.detail==record.detail&&
       runtime_record.power.pre_count==record.power.pre_count&&runtime_record.power.post_delta==record.power.post_delta&&
       runtime_record.power.ready_delta==record.power.ready_delta&&runtime_record.power.meta==record.power.meta)return;
    if(USB_RestartJournalWrite(&restart_journal,&record)){runtime_record=record;runtime_record_valid=true;}
}
static void QuarantineFailed(void)
{
    if(restart.state!=USB_RESTART_FAILED)return;
    USB_PROBE_Configure(false);configured=false;usb_restart_owner_diagnostics.configured=false;
    /* Pre-shutdown ACK/reset failure must not leave a pull-up advertising an
     * unserviced device. After power-off/partial rebuild retain all resources
     * and never access USB registers whose status may be invalid. */
    if(attached&&usb_registers_accessible){USB_DEVICE_Detach(handle);attached=false;}
    usb_restart_owner_diagnostics.attached=attached;
}
void USB_RestartOwnerAck(uint32_t ticket,bool success,uint32_t now)
{
    if(!initialized)return;
    bool irq=EVIC_INT_Disable();owner_now=now;
    if(event_flags&(OWNER_RESET|OWNER_ERROR))USB_RestartCancel(&restart);
    else USB_RestartAck(&restart,ticket,success,now);
    wait_evidence=0;QuarantineFailed();RecordPhase(now);
    EVIC_INT_Restore(irq);
}
void USB_RestartOwnerTasks(uint32_t now,bool cable,bool mux)
{
    if(!initialized)return;
    /* Hold the single-core global IRQ fence through event application and
     * scheduling. A new ISR reset cannot race an old CONFIGURED snapshot. */
    bool irq=EVIC_INT_Disable();owner_now=now;uint32_t events=event_flags;event_flags=0;
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
    QuarantineFailed();
    if(previous!=USB_RESTART_DONE&&restart.state==USB_RESTART_DONE)++usb_restart_owner_diagnostics.completed_cycles;
    usb_restart_owner_diagnostics.state=restart.state;usb_restart_owner_diagnostics.generation=restart.generation;
    usb_restart_owner_diagnostics.failure_phase=restart.failure_phase;usb_restart_owner_diagnostics.fault=restart.fault;
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
    QuarantineFailed();
    usb_restart_owner_diagnostics.attached=attached;RecordPhase(now);
    EVIC_INT_Restore(irq);
    /* DRAIN uses synchronous cancellation/observations only; OFF_WAIT only
     * reads U1PWRC. No normal task/attach/report calls through retirement. */
}
#endif
