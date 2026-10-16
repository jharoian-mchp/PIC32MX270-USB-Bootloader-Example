#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "usb_restart.h"
static bool pending,hardware_busy,powered,configured,retired,fenced;
static unsigned reject_at,call,resource_retired,initializations;
static uint32_t live_generation,configured_generation;
static char trace[128];static size_t used;
static bool Event(char c)
{assert(used<sizeof(trace));trace[used++]=c;return ++call!=reject_at;}
static bool Stop(void *c){(void)c;if(!Event('S'))return false;fenced=true;return true;}
static bool Cancel(void *c){(void)c;return Event('C');}
static bool Drained(void *c){(void)c;assert(fenced&&(powered||!hardware_busy));return !pending;}
static bool Off(void *c)
{(void)c;assert(fenced&&!pending);if(!Event('O'))return false;powered=false;hardware_busy=true;return true;}
static bool OffIdle(void *c){(void)c;return !powered&&!hardware_busy;}
static bool Retire(void *c)
{(void)c;assert(!powered&&!hardware_busy&&!pending);if(!Event('R'))return false;retired=true;++resource_retired;return true;}
static bool Initialize(void *c,uint32_t generation)
{
    (void)c;assert(fenced&&retired&&!pending&&!powered&&!hardware_busy);
    if(!Event('I'))return false;
    powered=true;retired=false;fenced=false;live_generation=generation;++initializations;return true;
}
static bool Configured(void *c,uint32_t generation)
{(void)c;return configured&&generation==live_generation&&generation==configured_generation;}
static const USB_RESTART_PORT port={NULL,Stop,Cancel,Drained,Off,OffIdle,Retire,Initialize,Configured};
static void Reset(USB_RESTART *s)
{
    pending=true;hardware_busy=configured=retired=fenced=false;powered=true;
    reject_at=call=resource_retired=initializations=used=0;live_generation=1;configured_generation=0;
    assert(USB_RestartInit(s,&port,100));
}
static void Drain(USB_RESTART *s,uint32_t ticket,uint32_t now)
{
    USB_RestartAck(s,ticket,true,now);assert(s->state==USB_RESTART_DRAIN);
    USB_RestartPoll(s,now+1);assert(s->state==USB_RESTART_DRAIN&&powered&&!resource_retired);
    pending=false;USB_RestartPoll(s,now+2);assert(s->state==USB_RESTART_OFF_WAIT&&!powered);
    unsigned old=call;USB_RestartPoll(s,now+3);assert(s->state==USB_RESTART_OFF_WAIT&&call==old);
    hardware_busy=false;USB_RestartPoll(s,now+4);assert(s->state==USB_RESTART_RETIRE);
    USB_RestartPoll(s,now+5);assert(s->state==USB_RESTART_INIT&&resource_retired==1);
    USB_RestartPoll(s,now+6);assert(s->state==USB_RESTART_CONFIG_WAIT&&initializations==1);
}
int main(void)
{
    USB_RESTART s;uint32_t ticket;Reset(&s);
    USB_RESTART before=s;assert(!USB_RestartRequest(&s,0,&s.ticket));assert(!memcmp(&s,&before,sizeof(s)));
    assert(USB_RestartRequest(&s,0,&ticket)&&ticket==1);assert(!USB_RestartRequest(&s,0,&ticket));
    USB_RestartAck(&s,99,true,1);assert(s.state==USB_RESTART_ACK_WAIT&&used==0);
    USB_RestartPoll(&s,2);assert(used==0);Drain(&s,1,3);
    configured=true;configured_generation=1;USB_RestartPoll(&s,10);assert(s.state==USB_RESTART_CONFIG_WAIT);
    configured_generation=2;USB_RestartPoll(&s,11);assert(s.state==USB_RESTART_DONE);
    assert(used==5&&!memcmp(trace,"SCORI",5));
    USB_RestartAck(&s,1,true,12);assert(used==5);
    /* Repeated cycle: old callback/generation cannot authorise/configure. */
    pending=true;configured=false;resource_retired=initializations=used=call=0;
    assert(USB_RestartRequest(&s,20,&ticket)&&ticket==2);
    USB_RestartAck(&s,1,true,21);assert(s.state==USB_RESTART_ACK_WAIT&&used==0);
    Drain(&s,2,22);configured=true;configured_generation=2;
    USB_RestartPoll(&s,29);assert(s.state==USB_RESTART_CONFIG_WAIT);
    configured_generation=3;USB_RestartPoll(&s,30);assert(s.state==USB_RESTART_DONE);
    /* Failed/late ACK has no side effects; cancelled late callback no retry. */
    Reset(&s);assert(USB_RestartRequest(&s,0,&ticket));USB_RestartAck(&s,ticket,false,1);
    assert(s.state==USB_RESTART_FAILED&&used==0);USB_RestartAck(&s,ticket,true,2);assert(used==0);
    Reset(&s);assert(USB_RestartRequest(&s,UINT32_MAX-5,&ticket));
    USB_RestartAck(&s,ticket,true,95);assert(s.state==USB_RESTART_FAILED&&used==0);
    /* Timeout at drain/off/config keeps storage and performs no hidden retire. */
    Reset(&s);assert(USB_RestartRequest(&s,0,&ticket));USB_RestartAck(&s,ticket,true,1);
    USB_RestartPoll(&s,101);assert(s.state==USB_RESTART_FAILED&&resource_retired==0&&powered);
    Reset(&s);assert(USB_RestartRequest(&s,0,&ticket));USB_RestartAck(&s,ticket,true,1);
    pending=false;USB_RestartPoll(&s,2);USB_RestartPoll(&s,102);
    assert(s.state==USB_RESTART_FAILED&&!powered&&resource_retired==0&&hardware_busy);
    Reset(&s);assert(USB_RestartRequest(&s,0,&ticket));Drain(&s,ticket,1);
    USB_RestartPoll(&s,107);assert(s.state==USB_RESTART_FAILED&&resource_retired==1&&initializations==1);
    /* Every mutating port failure stops, without reinitialization/retry. */
    for(unsigned failure=1;failure<=5;failure++)
    {
        Reset(&s);reject_at=failure;assert(USB_RestartRequest(&s,0,&ticket));USB_RestartAck(&s,ticket,true,1);
        pending=false;hardware_busy=false;
        for(uint32_t now=2;now<10&&s.state!=USB_RESTART_FAILED;now++)
        {if(s.state==USB_RESTART_OFF_WAIT)hardware_busy=false;USB_RestartPoll(&s,now);}
        assert(s.state==USB_RESTART_FAILED&&call==failure);
        unsigned old=call;USB_RestartPoll(&s,200);assert(call==old);
    }
    Reset(&s);assert(USB_RestartRequest(&s,0,&ticket));USB_RestartCancel(&s);USB_RestartAck(&s,ticket,true,1);
    assert(s.state==USB_RESTART_FAILED&&used==0);
    Reset(&s);assert(USB_RestartRequest(&s,0,&ticket));Drain(&s,ticket,1);
    configured=true;configured_generation=s.generation;USB_RestartPoll(&s,8);assert(s.state==USB_RESTART_DONE);
    USB_RestartCancel(&s);assert(s.state==USB_RESTART_FAILED);assert(!USB_RestartRequest(&s,9,&ticket));
    Reset(&s);s.generation=UINT32_MAX;assert(!USB_RestartRequest(&s,0,&ticket));
    assert(!USB_RestartInit(NULL,&port,100));assert(!USB_RestartInit(&s,&port,0));
    puts("PASS: read-only restart ACK/drain/off-idle/retire/init/config ordering, alias/callback generations, timeout/wrap/failure quarantine and repeated cycles; no USB/NVM hardware binding.");return 0;
}
