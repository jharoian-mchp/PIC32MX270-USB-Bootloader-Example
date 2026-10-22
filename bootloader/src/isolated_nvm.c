#include "isolated_nvm.h"
#include "update_protocol.h"
#include "update_image.h"
#include <limits.h>
#include <string.h>
_Static_assert(UPDATE_ROW_SIZE==128U&&UPDATE_ERASE_SIZE==1024U,"isolated target geometry");
/* Reserve the final even word as a detectable exhaustion marker, not a
 * published snapshot. Pointer differences avoid end-address overflow. */
#define SNAPSHOT_EXHAUSTED ISOLATED_NVM_SNAPSHOT_EXHAUSTED
static bool Overlap(const void *a,size_t an,const void *b,size_t bn)
{
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    return x>=y?x-y<bn:y-x<an;
}
static void Order(void)
{
#if defined(__XC32)
    __asm__ volatile("sync":::"memory");
#else
    __asm__ volatile("":::"memory");
#endif
}
static bool Publish(ISOLATED_NVM *o)
{
    uint32_t sequence=o->publication_sequence;
    bool consistent=o->mailbox->sequence==sequence&&(sequence&1U)==0U;
    if(!consistent||sequence>=SNAPSHOT_EXHAUSTED-2U) {
        o->snapshot_exhausted=o->failed=true;
        if(o->report.fault==ISOLATED_FAULT_NONE)o->report.fault=ISOLATED_FAULT_SNAPSHOT;
        o->unsettled=o->report.erase_attempts!=0U||o->report.row_attempts!=0U;
        if(consistent) {
            /* Keep the old payload but make its staleness detectable. */
            Order();o->mailbox->sequence=SNAPSHOT_EXHAUSTED;
            o->publication_sequence=SNAPSHOT_EXHAUSTED;
        }
        return false;
    }
    o->mailbox->sequence=sequence+1U;Order();
    /* Explicit word stores avoid structure padding in debugger snapshots. */
#define COPY(name) o->mailbox->report.name=o->report.name
    COPY(version);COPY(state);COPY(phase);COPY(fault);COPY(page);COPY(count_hz);
    COPY(arm_attempts);COPY(erase_attempts);COPY(row_attempts);COPY(anchor);
    COPY(elapsed_ticks);COPY(unsafe_bits);COPY(backend_error);COPY(verify_offset);
#undef COPY
    Order();o->mailbox->sequence=sequence+2U;o->publication_sequence=sequence+2U;return true;
}
bool ISOLATED_NvmSnapshot(const ISOLATED_NVM_MAILBOX *m,ISOLATED_NVM_REPORT *r)
{
    if(m==NULL||r==NULL||Overlap(m,sizeof(*m),r,sizeof(*r)))return false;
    uint32_t a=m->sequence;
    if(a==0U||a==SNAPSHOT_EXHAUSTED||(a&1U)!=0U)return false;
    Order();
    ISOLATED_NVM_REPORT copy;
#define COPY(name) copy.name=m->report.name
    COPY(version);COPY(state);COPY(phase);COPY(fault);COPY(page);COPY(count_hz);
    COPY(arm_attempts);COPY(erase_attempts);COPY(row_attempts);COPY(anchor);
    COPY(elapsed_ticks);COPY(unsafe_bits);COPY(backend_error);COPY(verify_offset);
#undef COPY
    Order();if(m->sequence!=a)return false;*r=copy;return true;
}
static uint32_t Count(ISOLATED_NVM *o){return o->port.count(o->port.context);}
static void Fail(ISOLATED_NVM *o,ISOLATED_NVM_FAULT fault)
{
    if(o->failed)return;
    o->failed=true;o->report.fault=fault;
    o->unsettled=o->report.erase_attempts!=0U||o->report.row_attempts!=0U;
    o->report.state=o->unsettled?ISOLATED_FAIL_UNSETTLED:ISOLATED_FAIL_NO_LAUNCH;
    (void)Publish(o);
}
static bool Guard(ISOLATED_NVM *o)
{
    if(o->failed)return false;
    if(o->mailbox->sequence!=o->publication_sequence||(o->publication_sequence&1U)!=0U) {
        o->snapshot_exhausted=true;Fail(o,ISOLATED_FAULT_SNAPSHOT);return false;
    }
    uint32_t unsafe=o->port.unsafe(o->port.context);
    if(unsafe!=0U){o->report.unsafe_bits|=unsafe;Fail(o,ISOLATED_FAULT_PREFLIGHT);return false;}
    if(o->report.state==ISOLATED_RUNNING&&o->budget_active) {
        o->report.elapsed_ticks=Count(o)-o->report.anchor;
        if(o->report.elapsed_ticks>=o->config.operation_ticks){Fail(o,ISOLATED_FAULT_TIMEOUT);return false;}
    }
    return true;
}
static bool Busy(void *context)
{
    ISOLATED_NVM *o=context;
    if(!Guard(o))return true;
    bool busy=o->real.busy(o->real.context);
    return !Guard(o)||busy;
}
static uint32_t Error(void *context)
{
    ISOLATED_NVM *o=context;if(!Guard(o))return UINT32_MAX;
    uint32_t error=o->real.error(o->real.context);o->report.backend_error=error;
    if(!Guard(o))return UINT32_MAX;
    return error;
}
static bool Erase(void *context,uint32_t address)
{
    ISOLATED_NVM *o=context;
    if(!Guard(o))return false;
    if(address!=o->config.page||o->report.phase!=ISOLATED_PHASE_ERASE||o->report.erase_attempts!=0U)
        {Fail(o,ISOLATED_FAULT_BACKEND);return false;}
    o->report.erase_attempts=1;
    if(!Publish(o)||!Guard(o))return false;
    bool result=o->real.erase(o->real.context,address);
    return Guard(o)&&result;
}
static bool Write(void *context,const uint32_t *row,uint32_t address)
{
    ISOLATED_NVM *o=context;
    if(!Guard(o))return false;
    if(address!=o->config.page||row!=o->flash.row||o->report.phase!=ISOLATED_PHASE_ROW||
       !o->erase_verified||o->report.erase_attempts!=1U||o->report.row_attempts!=0U)
        {Fail(o,ISOLATED_FAULT_BACKEND);return false;}
    o->report.row_attempts=1;
    if(!Publish(o)||!Guard(o))return false;
    bool result=o->real.write(o->real.context,row,address);
    return Guard(o)&&result;
}
static bool Read(void *context,uint32_t address,uint8_t *out,size_t size)
{
    ISOLATED_NVM *o=context;if(!Guard(o))return false;
    if(out==NULL||size==0U||address<o->config.page||address>=o->config.page+1024U||
       size>o->config.page+1024U-address||
       (o->report.phase!=ISOLATED_PHASE_ERASE_VERIFY&&o->report.phase!=ISOLATED_PHASE_ROW_VERIFY&&
        o->report.phase!=ISOLATED_PHASE_FINAL_READ))
        {Fail(o,ISOLATED_FAULT_BACKEND);return false;}
    bool busy=o->real.busy(o->real.context);if(!Guard(o)||busy)return false;
    bool result=o->real.read(o->real.context,address,out,size);
    return Guard(o)&&result;
}
bool ISOLATED_NvmInit(ISOLATED_NVM *o,ISOLATED_NVM_MAILBOX *m,
    const ISOLATED_NVM_CONFIG *c,const ISOLATED_NVM_PORT *p)
{
    if(o==NULL||m==NULL||c==NULL||p==NULL||p->count==NULL||p->unsafe==NULL||p->initialize==NULL||
       c->page<UPDATE_APP_PHYSICAL_START||c->page>UPDATE_IMAGE_META_ADDRESS-1024U||c->page%1024U||
       c->count_hz!=ISOLATED_NVM_COUNT_HZ||c->debounce_ticks==0U||
       c->arm_window_ticks<=c->debounce_ticks||c->arm_window_ticks>INT32_MAX||
       c->operation_ticks==0U||c->operation_ticks>INT32_MAX||
       Overlap(o,sizeof(*o),m,sizeof(*m))||Overlap(o,sizeof(*o),c,sizeof(*c))||
       Overlap(o,sizeof(*o),p,sizeof(*p))||Overlap(m,sizeof(*m),c,sizeof(*c))||
       Overlap(m,sizeof(*m),p,sizeof(*p))||Overlap(c,sizeof(*c),p,sizeof(*p)))return false;
    *o=(ISOLATED_NVM){.port=*p,.config=*c,.mailbox=m,.initialized=true};
    o->report=(ISOLATED_NVM_REPORT){.version=1,.state=ISOLATED_READY,.page=c->page,.count_hz=c->count_hz};
    m->token=0;m->sequence=0;
    return Publish(o);
}
static bool Phase(ISOLATED_NVM *o,ISOLATED_NVM_PHASE phase)
{
    o->report.phase=phase;return Publish(o)&&Guard(o);
}
static void SettleFailure(ISOLATED_NVM *o)
{
    if(!o->unsettled||!o->backend_ready)return;
    /* Only existing backend busy/settle/disarm may run after failure. No Flash
     * reset, new launch or ordinary readback; source and state remain allocated. */
    if(o->real.busy(o->real.context))return;
    o->unsettled=false;o->report.state=ISOLATED_FAIL_SETTLED;
    if(!o->snapshot_exhausted)(void)Publish(o);
}
static void Run(ISOLATED_NVM *o)
{
    if(!Guard(o))return;
    switch(o->report.phase) {
    case ISOLATED_PHASE_INIT: {
        UPDATE_FLASH_BACKEND b={0};
        bool ok=o->port.initialize(o->port.context,&b);
        if(ok&&b.busy&&b.error&&b.erase&&b.write&&b.read){o->real=b;o->backend_ready=true;}
        if(!Guard(o))return;
        if(!o->backend_ready){Fail(o,ISOLATED_FAULT_BACKEND);return;}
        UPDATE_FLASH_BACKEND wrapped={o,Busy,Error,Erase,Write,Read};
        /* Round up to milliseconds so the generic Flash timeout cannot
         * precede the authoritative Count budget. Its since value stays zero. */
        uint32_t per_ms=o->config.count_hz/1000U;
        uint32_t timeout_ms=(o->config.operation_ticks+per_ms-1U)/per_ms;
        if(!UPDATE_FlashInit(&o->flash,&wrapped,o->config.page,o->config.page+1024U,timeout_ms))
            {Fail(o,ISOLATED_FAULT_BACKEND);return;}
        (void)Phase(o,ISOLATED_PHASE_ERASE);return;
    }
    case ISOLATED_PHASE_ERASE:
    case ISOLATED_PHASE_ROW: {
        bool erase=o->report.phase==ISOLATED_PHASE_ERASE;
        o->report.anchor=Count(o);o->report.elapsed_ticks=0;o->budget_active=true;
        if(!Publish(o)||!Guard(o))return;
        uint8_t pattern[128];for(unsigned i=0;i<128;i++)pattern[i]=(uint8_t)(i^0xA5U);
        bool ok=erase?UPDATE_FlashErase(&o->flash,o->config.page,0):
            UPDATE_FlashWrite(&o->flash,o->config.page,pattern,0);
        if(!Guard(o))return;
        if(!ok){Fail(o,ISOLATED_FAULT_BACKEND);return;}
        (void)Phase(o,erase?ISOLATED_PHASE_ERASE_VERIFY:ISOLATED_PHASE_ROW_VERIFY);return;
    }
    case ISOLATED_PHASE_ERASE_VERIFY:
    case ISOLATED_PHASE_ROW_VERIFY: {
        bool erase=o->report.phase==ISOLATED_PHASE_ERASE_VERIFY;
        UPDATE_FLASH_STATUS status=UPDATE_FlashPoll(&o->flash,o->report.elapsed_ticks/(o->config.count_hz/1000U));
        if(!Guard(o))return;
        o->report.verify_offset=o->flash.verify_offset;
        if(status==UPDATE_FLASH_BUSY||status==UPDATE_FLASH_VERIFY){(void)Publish(o);return;}
        if(status!=UPDATE_FLASH_OK) {
            Fail(o,status==UPDATE_FLASH_TIMEOUT?ISOLATED_FAULT_TIMEOUT:ISOLATED_FAULT_READBACK);return;
        }
        if(erase) {
            o->erase_verified=true;
            bool ok=UPDATE_FlashReset(&o->flash);if(!Guard(o))return;
            if(!ok){Fail(o,ISOLATED_FAULT_BACKEND);return;}
            o->report.verify_offset=0;o->budget_active=false;(void)Phase(o,ISOLATED_PHASE_ROW);
        } else {o->report.verify_offset=0;(void)Phase(o,ISOLATED_PHASE_FINAL_READ);}
        return;
    }
    case ISOLATED_PHASE_FINAL_READ: {
        uint8_t actual[128];uint32_t offset=o->report.verify_offset;
        bool ok=Read(o,o->config.page+offset,actual,sizeof(actual));if(!Guard(o))return;
        if(!ok){Fail(o,ISOLATED_FAULT_READBACK);return;}
        for(unsigned i=0;i<128;i++)if(actual[i]!=(offset==0U?(uint8_t)(i^0xA5U):255U))
            {Fail(o,ISOLATED_FAULT_READBACK);return;}
        o->report.verify_offset+=128U;
        if(o->report.verify_offset==1024U) {
            if(Busy(o)){if(!o->failed)Fail(o,ISOLATED_FAULT_BACKEND);return;}
            if(Error(o)!=0U){if(!o->failed)Fail(o,ISOLATED_FAULT_BACKEND);return;}
            if(!Guard(o))return;
            o->report.state=ISOLATED_PASS_SETTLED;
        }
        (void)Publish(o);return;
    }
    default:Fail(o,ISOLATED_FAULT_BACKEND);return;
    }
}
void ISOLATED_NvmTasks(ISOLATED_NVM *o,bool pressed)
{
    if(o==NULL||!o->initialized)return;
    if(o->failed){SettleFailure(o);return;}
    if(o->report.state==ISOLATED_PASS_SETTLED)return;
    if(o->report.state==ISOLATED_RUNNING){Run(o);return;}
    uint32_t now=Count(o);
    if(!o->button_seen||pressed!=o->button_pressed) {
        o->button_seen=true;o->button_pressed=pressed;o->button_since=now;o->button_stable=false;
    }
    if((uint32_t)(now-o->button_since)>=o->config.debounce_ticks)o->button_stable=true;
    if(o->report.state==ISOLATED_READY) {
        if(!pressed&&o->button_stable)o->startup_released=true;
        uint32_t token=o->mailbox->token;if(token==0U)return;
        bool released=o->startup_released&&!pressed&&o->button_stable;
        o->spent=true;o->report.arm_attempts=1;o->mailbox->token=0;
        o->report.state=ISOLATED_ARMED;o->arm_anchor=now;
        o->fresh_press=false;o->button_stable=false;o->button_since=now;
        if(!Publish(o))return;
        if(token!=ISOLATED_NVM_TOKEN){Fail(o,ISOLATED_FAULT_TOKEN);return;}
        if(!released){Fail(o,ISOLATED_FAULT_GESTURE);return;}
        (void)Guard(o);return;
    }
    if((uint32_t)(now-o->arm_anchor)>=o->config.arm_window_ticks){Fail(o,ISOLATED_FAULT_ARM_TIMEOUT);return;}
    if(!Guard(o))return;
    /* Exclusion checks can consume time; do not accept a gesture using the
     * stale pre-check Count sample. Expiry still wins over a completed release. */
    if((uint32_t)(Count(o)-o->arm_anchor)>=o->config.arm_window_ticks) {
        Fail(o,ISOLATED_FAULT_ARM_TIMEOUT);return;
    }
    if(pressed&&o->button_stable)o->fresh_press=true;
    if(!pressed&&o->button_stable&&o->fresh_press) {
        o->report.state=ISOLATED_RUNNING;(void)Phase(o,ISOLATED_PHASE_INIT);
    }
}
