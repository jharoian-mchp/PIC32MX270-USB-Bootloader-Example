#include "debugger_rehearsal.h"
#include <debugger_rehearsal_hw.h>
static void Order(void)
{
#if defined(__XC32)
    __asm__ volatile("sync":::"memory");
#else
    __asm__ volatile("":::"memory");
#endif
}
#if defined(RAM_REHEARSAL_TEST_RENDEZVOUS)
/* Test TU supplies these two functions. Production has no callback hooks. */
#else
__attribute__((noinline)) void RAM_RehearsalPreEdit(void){__asm__ volatile("nop":::"memory");}
__attribute__((noinline)) void RAM_RehearsalReadout(void){__asm__ volatile("nop":::"memory");}
#endif
static bool Aligned(const void *p,size_t n){return (uintptr_t)p%n==0U;}
static bool Overlap(const void *a,size_t an,const void *b,size_t bn)
{uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x>=y?x-y<bn:y-x<an;}
#define HEADER(C) C(version);C(mode);C(state);C(fault);C(edit_attempts);C(observations);C(challenge_observed);C(resume_count_first);C(resume_count_delta)
#define RAW(C) C(osccon);C(count);C(status);C(cause);C(debug);C(pwrc);C(dma);C(nvm);C(cfg0);C(cfg1);C(cfg2);C(cfg3)
static bool Guards(const RAM_REHEARSAL_MAILBOX *m)
{return m->guard_low==RAM_REHEARSAL_GUARD_LOW&&m->guard_high==RAM_REHEARSAL_GUARD_HIGH;}
static bool Profile(const RAM_REHEARSAL_RAW *r,bool locked)
{
    /* Exact RAM_REHEARSAL_DEBUG_V1, never the production DEBUG-OFF port.
     * Only DM is interpreted from Debug; COUNTDM outside DM is undefined. */
    return (r->osccon&0x381F7709U)==0x08073300U&&(!locked||(r->osccon&32U))&&
        !(r->status&1U)&&!(r->cause&0x08000000U)&&!(r->debug&0x40000000U)&&
        !(r->pwrc&9U)&&!(r->dma&0x8000U)&&!(r->nvm&0xB000U)&&
        r->cfg0==0x7FFFFFE0U&&r->cfg1==0xFD74CE5BU&&r->cfg2==0xFFF97AFAU&&r->cfg3==UINT32_MAX;
}
static void Deny(RAM_REHEARSAL *r,uint32_t reason)
{if(!r->readout_denied)r->readout_reason=reason;r->readout_denied=true;}
static void Invalidate(RAM_REHEARSAL *r)
{
    if(r->sequence_invalid)return;
    Order();r->mailbox->sequence=RAM_REHEARSAL_EXHAUSTED;Order();
    r->expected_sequence=RAM_REHEARSAL_EXHAUSTED;r->sequence_invalid=true;
    if(!r->terminal){r->report.state=RAM_FAILED;r->report.fault=RAM_FAULT_SEQUENCE;r->terminal=true;}
    Deny(r,RAM_FAULT_SEQUENCE);
}
static bool Sequence(RAM_REHEARSAL *r)
{
    if(r->sequence_invalid)return false;
    if(r->mailbox->sequence!=r->expected_sequence||(r->expected_sequence&1U)||
       r->expected_sequence==RAM_REHEARSAL_EXHAUSTED){Invalidate(r);return false;}
    return true;
}
static bool Publish(RAM_REHEARSAL *r)
{
    if(r->terminal)return false;
    if(!Sequence(r))return false;
    uint32_t seq=r->expected_sequence;
    if(seq>=RAM_REHEARSAL_EXHAUSTED-2U){Invalidate(r);return false;}
    r->mailbox->sequence=seq+1U;Order();
#define COPY(n) r->mailbox->report.n=r->report.n
    HEADER(COPY);
#undef COPY
#define COPY(n) r->mailbox->report.before.n=r->report.before.n
    RAW(COPY);
#undef COPY
#define COPY(n) r->mailbox->report.after.n=r->report.after.n
    RAW(COPY);
#undef COPY
    Order();r->mailbox->sequence=seq+2U;r->expected_sequence=seq+2U;return true;
}
bool RAM_RehearsalSnapshot(const RAM_REHEARSAL_MAILBOX *m,RAM_REHEARSAL_REPORT *out)
{
    if(m==NULL||out==NULL||!Aligned(m,_Alignof(RAM_REHEARSAL_MAILBOX))||
       !Aligned(out,_Alignof(RAM_REHEARSAL_REPORT))||Overlap(m,sizeof(*m),out,sizeof(*out)))return false;
    uint32_t seq=m->sequence;
    if(!seq||(seq&1U)||seq==RAM_REHEARSAL_EXHAUSTED||!Guards(m))return false;
    Order();RAM_REHEARSAL_REPORT copy;
#define COPY(n) copy.n=m->report.n
    HEADER(COPY);
#undef COPY
#define COPY(n) copy.before.n=m->report.before.n
    RAW(COPY);
#undef COPY
#define COPY(n) copy.after.n=m->report.after.n
    RAW(COPY);
#undef COPY
    Order();if(seq!=m->sequence||!Guards(m))return false;*out=copy;return true;
}
static void Finish(RAM_REHEARSAL *r,uint32_t fault)
{
    if(r->terminal)return;
    r->report.state=fault?RAM_FAILED:RAM_COMPLETE;r->report.fault=fault;
    /* A sampled exclusion/profile violation cannot become breakpoint-safe
     * merely because hardware later looks clean. Only RAM challenge/Count
     * failures can use the conservative independent readout gate. */
    if(fault==RAM_FAULT_PROFILE||fault==RAM_FAULT_GUARDS)Deny(r,fault);
    if(Publish(r))r->terminal=true;
}
static bool Integrity(RAM_REHEARSAL *r)
{
    if(!Sequence(r))return false;
    if(!Guards(r->mailbox)){
        if(!r->terminal)Finish(r,RAM_FAULT_GUARDS);
        Deny(r,RAM_FAULT_GUARDS);return false;
    }
    return true;
}
/* Conservative gate: FULL profile even for failure readout. No breakpoint
 * if halted Count failure has invalid clock/control/bus state. */
static void Readout(RAM_REHEARSAL *r)
{
    if(!Integrity(r)||r->readout_denied)return;
    RAM_REHEARSAL_RAW raw=RAM_RehearsalHwRead();
    if(!Profile(&raw,true)){r->denied_raw=raw;Deny(r,RAM_FAULT_PROFILE);return;}
    if(r->readout_attempted)return;
    r->readout_attempted=true;RAM_RehearsalReadout();
    /* Debug return must not re-mask IRQ, clear a bus or revalidate a report. */
    if(!Integrity(r))return;
    raw=RAM_RehearsalHwRead();
    if(!Profile(&raw,true)){r->denied_raw=raw;Deny(r,RAM_FAULT_PROFILE);}
}
bool RAM_RehearsalInit(RAM_REHEARSAL *r,RAM_REHEARSAL_MAILBOX *m)
{
    if(r==NULL||m==NULL||!Aligned(r,_Alignof(RAM_REHEARSAL))||
       !Aligned(m,_Alignof(RAM_REHEARSAL_MAILBOX))||Overlap(r,sizeof(*r),m,sizeof(*m)))return false;
    *r=(RAM_REHEARSAL){.mailbox=m,.initialized=true};
    m->guard_low=RAM_REHEARSAL_GUARD_LOW;m->guard_high=RAM_REHEARSAL_GUARD_HIGH;m->edit_word=0;m->sequence=0;
    r->report=(RAM_REHEARSAL_REPORT){.version=1,.mode=1,.state=RAM_CHECKING};
    if(!Publish(r))return false;
    RAM_REHEARSAL_RAW raw=RAM_RehearsalHwRead();r->report.before=raw;
    if(!Profile(&raw,false)){Finish(r,RAM_FAULT_PROFILE);return false;}
    uint32_t first=raw.count;bool changed=false;
    for(unsigned i=0;i<RAM_REHEARSAL_OBSERVATIONS;i++) {
        raw=RAM_RehearsalHwRead();r->report.before=raw;
        if(!Integrity(r))return false;
        if(!Profile(&raw,false)){Finish(r,RAM_FAULT_PROFILE);return false;}
        changed=changed||raw.count!=first;
        if(changed&&(raw.osccon&32U)) {
            raw=RAM_RehearsalHwRead();r->report.before=raw;
            if(!Profile(&raw,true)){Finish(r,RAM_FAULT_PROFILE);return false;}
            if(raw.count==first){Finish(r,RAM_FAULT_COUNT);return false;}
            r->report.state=RAM_WAIT_EDIT;
            if(!Publish(r))return false;
            raw=RAM_RehearsalHwRead(); /* fresh pre-edit gate */
            uint32_t premature=m->edit_word;
            if(premature){r->spent=true;r->report.edit_attempts=1;
                r->report.challenge_observed=premature;m->edit_word=0;}
            if(!Integrity(r))return false;
            r->report.before=raw;
            if(!Profile(&raw,true)){Finish(r,RAM_FAULT_PROFILE);return false;}
            if(premature){Finish(r,RAM_FAULT_CHALLENGE);return false;}
            if(!Publish(r))return false;
            r->preedit_called=true;RAM_RehearsalPreEdit();return true;
        }
    }
    Finish(r,changed?RAM_FAULT_PROFILE:RAM_FAULT_COUNT);return false;
}
void RAM_RehearsalTasks(RAM_REHEARSAL *r)
{
    if(r==NULL||!r->initialized)return;
    if(r->terminal){Readout(r);return;}
    /* First nonzero edit spends before ANY validation, including guards. */
    uint32_t edit=r->mailbox->edit_word;
    if(edit&&!r->spent){r->spent=true;r->report.edit_attempts=1;r->report.challenge_observed=edit;r->mailbox->edit_word=0;}
    if(!Integrity(r))return;
    RAM_REHEARSAL_RAW raw=RAM_RehearsalHwRead();
    if(!Profile(&raw,true)){r->report.after=raw;Finish(r,RAM_FAULT_PROFILE);return;}
    if(!edit)return; /* zero never arms or mutates an otherwise safe report */
    if(edit!=RAM_REHEARSAL_CHALLENGE){Finish(r,RAM_FAULT_CHALLENGE);return;}
    r->report.state=RAM_OBSERVING;
    /* Anchor only the fresh sample AFTER consumption, never pre-debug Count. */
    r->report.resume_count_first=raw.count;r->report.after=raw;
    if(!Publish(r))return;
    bool changed=false;
    for(unsigned i=0;i<RAM_REHEARSAL_OBSERVATIONS;i++) {
        raw=RAM_RehearsalHwRead();r->report.after=raw;r->report.observations=i+1U;
        r->report.resume_count_delta=raw.count-r->report.resume_count_first;
        if(!Integrity(r))return;
        if(!Profile(&raw,true)){Finish(r,RAM_FAULT_PROFILE);return;}
        changed=changed||r->report.resume_count_delta!=0U;
        if(!Publish(r))return;
        if(changed) {
            raw=RAM_RehearsalHwRead();r->report.after=raw;
            r->report.resume_count_delta=raw.count-r->report.resume_count_first;
            if(!Integrity(r))return;
            if(!Profile(&raw,true)){Finish(r,RAM_FAULT_PROFILE);return;}
            Finish(r,r->report.resume_count_delta?RAM_FAULT_NONE:RAM_FAULT_COUNT);return;
        }
    }
    Finish(r,RAM_FAULT_COUNT);
}
