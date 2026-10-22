#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "debugger_rehearsal.h"
static void SnapshotCut(const RAM_REHEARSAL_MAILBOX *m);
static void PublishCut(RAM_REHEARSAL *r);
#define RAM_REHEARSAL_TEST_RENDEZVOUS 1
#include "rehearsal-body.inc"
static RAM_REHEARSAL r;static RAM_REHEARSAL_MAILBOX m;
static RAM_REHEARSAL_RAW raw;
static unsigned samples,lock_at,mutate_at,pre_calls,readout_calls,cut,fields,odd_reads;
static unsigned mutation,cut_kind;
static uint32_t step,pre_edit,pre_count;
static bool pre_unsafe,readout_unsafe;
RAM_REHEARSAL_RAW RAM_RehearsalHwRead(void)
{
    ++samples;raw.count+=step;if(samples>=lock_at)raw.osccon|=32U;
    if(samples==mutate_at)switch(mutation){
    case 1:raw.pwrc=1;break;case 2:m.guard_low^=1;break;case 3:m.sequence+=2;break;
    case 4:raw.count=r.report.resume_count_first;break;case 5:m.edit_word=pre_edit;break;
    case 6:raw.count=UINT32_MAX;break;default:assert(0);}
    return raw;
}
void RAM_RehearsalPreEdit(void)
{
    ++pre_calls;assert(pre_calls==1&&r.preedit_called&&!r.spent&&m.edit_word==0);
    RAM_REHEARSAL_REPORT out;assert(RAM_RehearsalSnapshot(&m,&out)&&out.state==RAM_WAIT_EDIT);
    raw.count+=pre_count;m.edit_word=pre_edit;if(pre_unsafe)raw.status|=1U;
}
void RAM_RehearsalReadout(void)
{
    ++readout_calls;assert(readout_calls==1&&r.terminal&&r.readout_attempted&&!r.readout_denied);
    assert(!(raw.pwrc&9U)&&!(raw.dma&0x8000U)&&!(raw.nvm&0x8000U));
    RAM_REHEARSAL_REPORT out;assert(RAM_RehearsalSnapshot(&m,&out));
    if(readout_unsafe)raw.dma=0x8000U;
}
static void SnapshotCut(const RAM_REHEARSAL_MAILBOX *box)
{
    if(++fields==cut){RAM_REHEARSAL_MAILBOX *writer=(RAM_REHEARSAL_MAILBOX *)box;
        if(cut_kind==1)writer->guard_high^=1;else {writer->sequence+=2;writer->report.after.count^=1;}}
}
static void PublishCut(RAM_REHEARSAL *owner)
{
    RAM_REHEARSAL_REPORT out;assert(owner->mailbox->sequence&1U);
    assert(!RAM_RehearsalSnapshot(owner->mailbox,&out));++odd_reads;
}
static void Reset(void)
{
    raw=(RAM_REHEARSAL_RAW){.osccon=0x08073300U,.cfg0=0x7FFFFFE0U,
        .cfg1=0xFD74CE5BU,.cfg2=0xFFF97AFAU,.cfg3=UINT32_MAX};
    samples=mutate_at=pre_calls=readout_calls=cut=fields=mutation=cut_kind=0;
    lock_at=1;step=1;pre_edit=pre_count=0;pre_unsafe=readout_unsafe=false;
    memset(&r,0,sizeof(r));memset(&m,0xA5,sizeof(m));
}
static void Ready(void){Reset();assert(RAM_RehearsalInit(&r,&m));assert(pre_calls==1&&!r.spent&&!r.terminal&&samples==4);}
static void Complete(void)
{m.edit_word=RAM_REHEARSAL_CHALLENGE;RAM_RehearsalTasks(&r);assert(r.report.state==RAM_COMPLETE&&r.terminal&&r.report.edit_attempts==1&&m.edit_word==0);}
static void SnapshotReject(void)
{
    RAM_REHEARSAL_REPORT out,before;memset(&out,0xA5,sizeof(out));before=out;
    assert(!RAM_RehearsalSnapshot(&m,&out)&&!memcmp(&out,&before,sizeof(out)));
}
int main(void)
{
    _Static_assert(sizeof(RAM_REHEARSAL_REPORT)/sizeof(uint32_t)==33U,"snapshot cuts");
    Ready();RAM_REHEARSAL_REPORT out;assert(RAM_RehearsalSnapshot(&m,&out));
    assert(out.version==1&&out.mode==1&&out.state==RAM_WAIT_EDIT&&out.before.count==4U);
    uint32_t seq=m.sequence;for(unsigned i=0;i<300U;i++)RAM_RehearsalTasks(&r);
    assert(!r.spent&&!r.terminal&&m.sequence==seq&&pre_calls==1&&readout_calls==0);
    Complete();assert(r.report.challenge_observed==RAM_REHEARSAL_CHALLENGE&&r.report.observations==1U&&r.report.resume_count_delta==2U);
    RAM_REHEARSAL_MAILBOX terminal=m;RAM_RehearsalTasks(&r);assert(readout_calls==1&&!memcmp(&terminal,&m,sizeof(m)));
    m.edit_word=RAM_REHEARSAL_CHALLENGE;RAM_REHEARSAL_MAILBOX edited=m;
    for(unsigned i=0;i<3;i++)RAM_RehearsalTasks(&r);
    assert(readout_calls==1&&pre_calls==1&&!memcmp(&edited,&m,sizeof(m))); /* no repeated edit clear */
    /* Debug delay/wrap excluded; edit at inert pre-point consumed only by Tasks. */
    Reset();pre_count=UINT32_MAX-10U;pre_edit=RAM_REHEARSAL_CHALLENGE;
    assert(RAM_RehearsalInit(&r,&m)&&!r.spent&&m.edit_word==pre_edit);
    uint32_t resumed=raw.count;RAM_RehearsalTasks(&r);
    assert(r.report.state==RAM_COMPLETE&&r.report.resume_count_first==resumed+1U&&r.report.resume_count_delta==2U);
    /* Missing count/lock finite; no busy waits, automatic cleanup or pre-edit. */
    Reset();step=0;assert(!RAM_RehearsalInit(&r,&m)&&samples==257U&&r.report.fault==RAM_FAULT_COUNT&&!pre_calls);
    RAM_RehearsalTasks(&r);assert(readout_calls==1); /* safe no-count failure can be inspected */
    Reset();lock_at=UINT32_MAX;assert(!RAM_RehearsalInit(&r,&m)&&samples==257U&&!pre_calls);
    RAM_RehearsalTasks(&r);assert(!readout_calls&&r.readout_denied);
    Reset();lock_at=5;assert(RAM_RehearsalInit(&r,&m)&&samples==7U);
    Reset();raw.count=UINT32_MAX-1U;mutate_at=3;mutation=6;
    assert(!RAM_RehearsalInit(&r,&m)&&r.report.fault==RAM_FAULT_COUNT&&!pre_calls);
    /* Every exact fuse bit is pinned; faults cannot force hardware off. */
    for(unsigned word=0;word<4U;word++)for(unsigned bit=0;bit<32U;bit++) {
        Reset();switch(word){case 0:raw.cfg0^=1U<<bit;break;case 1:raw.cfg1^=1U<<bit;break;
        case 2:raw.cfg2^=1U<<bit;break;default:raw.cfg3^=1U<<bit;}
        assert(!RAM_RehearsalInit(&r,&m)&&r.report.fault==RAM_FAULT_PROFILE&&samples==1U&&!pre_calls);
        RAM_RehearsalTasks(&r);assert(!readout_calls&&r.readout_denied);
    }
    for(unsigned fault=0;fault<9U;fault++) {
        Reset();switch(fault){case 0:raw.pwrc=8;break;case 1:raw.dma=0x8000;break;
        case 2:raw.status=1;break;case 3:raw.cause=0x08000000;break;case 4:raw.debug=0x40000000;break;
        case 5:raw.nvm=0x8000;break;case 6:raw.nvm=0x2000;break;case 7:raw.osccon|=8;break;default:raw.osccon|=1;}
        assert(!RAM_RehearsalInit(&r,&m)&&samples==1&&!pre_calls);RAM_RehearsalTasks(&r);assert(!readout_calls);
    }
    /* Observed profile/activity failure cannot disappear into a safe BP. */
    Ready();raw.pwrc=1U;RAM_RehearsalTasks(&r);assert(r.terminal&&r.readout_denied);
    raw.pwrc=0U;RAM_RehearsalTasks(&r);assert(!readout_calls&&r.report.fault==RAM_FAULT_PROFILE);
    /* COUNTDM is intentionally uninterpreted outside DM. */
    Reset();raw.debug=0x02000000;assert(RAM_RehearsalInit(&r,&m));Complete();
    for(unsigned value=0;value<4U;value++) {
        Ready();m.edit_word=value==0?1U:value==1?0x4E564D31U:value==2?UINT32_MAX:0x52414D30U;
        RAM_RehearsalTasks(&r);assert(r.spent&&m.edit_word==0&&r.report.edit_attempts==1&&r.report.fault==RAM_FAULT_CHALLENGE);
        RAM_RehearsalTasks(&r);assert(readout_calls==1);m.edit_word=RAM_REHEARSAL_CHALLENGE;
        RAM_RehearsalTasks(&r);assert(m.edit_word==RAM_REHEARSAL_CHALLENGE&&r.report.state==RAM_FAILED);
    }
    /* Simultaneous nonzero+guard+profile failure spends before validation. */
    Ready();m.edit_word=RAM_REHEARSAL_CHALLENGE;m.guard_low^=1;raw.pwrc=1;
    RAM_RehearsalTasks(&r);assert(r.spent&&m.edit_word==0&&r.report.fault==RAM_FAULT_GUARDS&&r.readout_denied);
    SnapshotReject();RAM_REHEARSAL_MAILBOX damaged=m;m.guard_low=RAM_REHEARSAL_GUARD_LOW;
    RAM_RehearsalTasks(&r);assert(!readout_calls);m=damaged;
    /* Nonzero premature pre-point edit is rejected/spent without rendezvous. */
    Reset();mutate_at=4;mutation=5;pre_edit=RAM_REHEARSAL_CHALLENGE;
    assert(!RAM_RehearsalInit(&r,&m)&&r.spent&&m.edit_word==0&&!pre_calls&&r.report.fault==RAM_FAULT_CHALLENGE);
    /* Debug return IRQ change is detected, never re-masked. */
    Reset();pre_edit=RAM_REHEARSAL_CHALLENGE;pre_unsafe=true;assert(RAM_RehearsalInit(&r,&m));
    RAM_RehearsalTasks(&r);assert(r.spent&&r.report.fault==RAM_FAULT_PROFILE&&raw.status==1);RAM_RehearsalTasks(&r);assert(!readout_calls);
    /* Fresh resumed anchor, stopped timebase and final-sample regressions. */
    Ready();step=0;m.edit_word=RAM_REHEARSAL_CHALLENGE;unsigned before_samples=samples;
    RAM_RehearsalTasks(&r);assert(r.report.fault==RAM_FAULT_COUNT&&r.report.observations==256U&&samples==before_samples+257U);
    Ready();raw.count=UINT32_MAX-1U;Complete();assert(r.report.resume_count_first==UINT32_MAX&&r.report.resume_count_delta==2U);
    for(unsigned mutation_kind=1;mutation_kind<=4U;mutation_kind++) {
        Ready();m.edit_word=RAM_REHEARSAL_CHALLENGE;mutate_at=samples+3;mutation=mutation_kind;
        RAM_RehearsalTasks(&r);assert(r.terminal&&r.report.state==RAM_FAILED);
        if(mutation_kind==1)assert(r.report.fault==RAM_FAULT_PROFILE);
        if(mutation_kind==2)assert(r.report.fault==RAM_FAULT_GUARDS);
        if(mutation_kind==3){assert(r.sequence_invalid);SnapshotReject();}
        if(mutation_kind==4)assert(r.report.fault==RAM_FAULT_COUNT);
    }
    /* All33 field snapshots reject deterministic sequence/guard interleavings. */
    for(unsigned kind=0;kind<2U;kind++)for(unsigned pos=1;pos<=33U;pos++) {
        Ready();cut=pos;fields=0;cut_kind=kind;SnapshotReject();assert(fields==33U);
    }
    assert(odd_reads>0);
    for(unsigned value=0;value<3U;value++) {Ready();m.sequence=value==0?0U:value==1?3U:RAM_REHEARSAL_EXHAUSTED;SnapshotReject();}
    /* Terminal profile/guard denial no payload publication; sequence exception. */
    Ready();Complete();terminal=m;raw.dma=0x8000;RAM_RehearsalTasks(&r);
    assert(r.readout_denied&&!readout_calls&&!memcmp(&terminal,&m,sizeof(m))&&r.report.state==RAM_COMPLETE);
    raw.dma=0;RAM_RehearsalTasks(&r);assert(!readout_calls);
    Ready();Complete();m.guard_high^=1;terminal=m;RAM_RehearsalTasks(&r);
    assert(r.readout_denied&&!memcmp(&terminal,&m,sizeof(m)));SnapshotReject();
    for(unsigned phase=0;phase<2;phase++) {
        Ready();if(phase)Complete();RAM_REHEARSAL_REPORT payload=m.report;
        m.sequence+=2;m.edit_word=RAM_REHEARSAL_CHALLENGE;RAM_RehearsalTasks(&r);
        assert(r.sequence_invalid&&m.sequence==RAM_REHEARSAL_EXHAUSTED&&!memcmp((const void *)&m.report,&payload,sizeof(payload)));
        if(!phase)assert(r.spent&&m.edit_word==0);else assert(m.edit_word==RAM_REHEARSAL_CHALLENGE);
        terminal=m;RAM_RehearsalTasks(&r);assert(!memcmp(&terminal,&m,sizeof(m)));SnapshotReject();
    }
    Ready();r.expected_sequence=m.sequence=RAM_REHEARSAL_EXHAUSTED-2U;
    m.edit_word=RAM_REHEARSAL_CHALLENGE;RAM_RehearsalTasks(&r);assert(r.sequence_invalid&&r.terminal);SnapshotReject();
    Ready();Complete();readout_unsafe=true;terminal=m;RAM_RehearsalTasks(&r);
    assert(readout_calls==1&&r.readout_denied&&!memcmp(&terminal,&m,sizeof(m)));RAM_RehearsalTasks(&r);assert(readout_calls==1);
    /* Inputs rejected without mutation before dereferencing invalid alignment. */
    Reset();RAM_REHEARSAL before_r=r;RAM_REHEARSAL_MAILBOX before_m=m;
    assert(!RAM_RehearsalInit(&r,(RAM_REHEARSAL_MAILBOX *)(void *)&r)&&!memcmp(&before_r,&r,sizeof(r)));
    assert(!RAM_RehearsalInit((RAM_REHEARSAL *)(void *)((unsigned char *)&r+1),&m)&&!memcmp(&before_m,&m,sizeof(m)));
    assert(!RAM_RehearsalInit(&r,(RAM_REHEARSAL_MAILBOX *)(void *)((unsigned char *)&m+1)));
    assert(!RAM_RehearsalInit(NULL,&m)&&!RAM_RehearsalInit(&r,NULL));RAM_RehearsalTasks(NULL);
    Ready();before_m=m;assert(!RAM_RehearsalSnapshot(&m,(RAM_REHEARSAL_REPORT *)(void *)&m.report)&&!memcmp(&m,&before_m,sizeof(m)));
    assert(!RAM_RehearsalSnapshot((RAM_REHEARSAL_MAILBOX *)(void *)((unsigned char *)&m+1),&out));
    assert(!RAM_RehearsalSnapshot(&m,(RAM_REHEARSAL_REPORT *)(void *)((unsigned char *)&out+1)));
    puts("PASS: RAM-only rehearsal exact profile, 33-word guarded snapshots, finite Count, spend-before-validation, one-shot inert rendezvous and terminal readout denial; no NVM/Flash linked, all hardware mocked.");
}
