#include "usb_restart.h"
#include <stddef.h>
static bool Overlap(const void *a,size_t an,const void *b,size_t bn)
{
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    return x<=y?y-x<an:x-y<bn;
}
static bool Expired(const USB_RESTART *s,uint32_t now)
{return (uint32_t)(now-s->since)>=s->timeout_ms;}
static USB_RESTART_STATE Fail(USB_RESTART *s)
{s->state=USB_RESTART_FAILED;return s->state;}
bool USB_RestartInit(USB_RESTART *s,const USB_RESTART_PORT *p,uint32_t timeout)
{
    if(s==NULL||p==NULL||timeout==0U||timeout>0x7FFFFFFFU||
       !p->fence_submissions||!p->cancel_and_detach||!p->transfers_drained||
       !p->mask_and_power_off||!p->module_off_idle||!p->retire_resources||
       !p->initialize_usb_only||!p->configured)return false;
    USB_RESTART_PORT copy=*p;
    *s=(USB_RESTART){.port=copy,.generation=1,.timeout_ms=timeout,.initialized=true};return true;
}
bool USB_RestartRequest(USB_RESTART *s,uint32_t now,uint32_t *ticket)
{
    if(ticket!=NULL&&s!=NULL&&Overlap(ticket,sizeof(*ticket),s,sizeof(*s)))return false;
    if(ticket!=NULL)*ticket=0;
    if(s==NULL||!s->initialized||ticket==NULL||
       (s->state!=USB_RESTART_READY&&s->state!=USB_RESTART_DONE)||
       s->ticket==UINT32_MAX||s->generation==UINT32_MAX)return false;
    ++s->ticket;s->since=now;s->offline_confirmed=false;s->state=USB_RESTART_ACK_WAIT;
    *ticket=s->ticket;return true;
}
void USB_RestartAck(USB_RESTART *s,uint32_t ticket,bool success,uint32_t now)
{
    if(s==NULL||!s->initialized||s->state!=USB_RESTART_ACK_WAIT||ticket!=s->ticket)return;
    if(!success||Expired(s,now)){Fail(s);return;}
    if(!s->port.fence_submissions(s->port.context)||!s->port.cancel_and_detach(s->port.context))
    {Fail(s);return;}
    s->since=now;s->state=USB_RESTART_DRAIN;
}
USB_RESTART_STATE USB_RestartPoll(USB_RESTART *s,uint32_t now)
{
    if(s==NULL||!s->initialized)return USB_RESTART_FAILED;
    if(s->state==USB_RESTART_READY||s->state==USB_RESTART_DONE||s->state==USB_RESTART_FAILED)return s->state;
    if(Expired(s,now))return Fail(s);
    switch(s->state)
    {
        case USB_RESTART_DRAIN:
            if(s->port.transfers_drained(s->port.context))
            {
                if(!s->port.mask_and_power_off(s->port.context))return Fail(s);
                s->state=USB_RESTART_OFF_WAIT;s->since=now;
            }
            break;
        case USB_RESTART_OFF_WAIT:
            /* No other USB port operation while powered-down module drains. */
            if(s->port.module_off_idle(s->port.context))
            {s->offline_confirmed=true;s->state=USB_RESTART_RETIRE;s->since=now;}
            break;
        case USB_RESTART_RETIRE:
            if(!s->port.module_off_idle(s->port.context)||!s->port.transfers_drained(s->port.context)||
               !s->port.retire_resources(s->port.context))return Fail(s);
            s->state=USB_RESTART_INIT;s->since=now;
            break;
        case USB_RESTART_INIT:
            if(!s->port.module_off_idle(s->port.context))return Fail(s);
            ++s->generation;
            if(!s->port.initialize_usb_only(s->port.context,s->generation))return Fail(s);
            s->offline_confirmed=false;s->state=USB_RESTART_CONFIG_WAIT;s->since=now;
            break;
        case USB_RESTART_CONFIG_WAIT:
            if(s->port.configured(s->port.context,s->generation))s->state=USB_RESTART_DONE;
            break;
        default: break;
    }
    return s->state;
}
void USB_RestartCancel(USB_RESTART *s)
{if(s!=NULL&&s->initialized)Fail(s);}
