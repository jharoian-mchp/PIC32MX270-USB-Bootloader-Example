#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "isolated_nvm_target.h"
#include <isolated_nvm_target_hw.h>
#include <update_nvm_port.h>
static ISOLATED_TARGET_RAW raw;
static unsigned samples,gpio_calls,lock_at,bad_at;
static uint32_t count_step,injected_usb;
static bool pressed,reject_gpio;
static unsigned nvm_writes,snapshot_cut,reader_fields,odd_publications;
static void TargetSnapshotCut(const ISOLATED_TARGET_MAILBOX *m);
static void TargetPublishCut(ISOLATED_NVM_TARGET *t);
#include "isolated-target-body.inc"
static void TargetSnapshotCut(const ISOLATED_TARGET_MAILBOX *m)
{
    if(++reader_fields==snapshot_cut) {
        /* Deterministic single-thread interleaving; no C data race. */
        ISOLATED_TARGET_MAILBOX *writer=(ISOLATED_TARGET_MAILBOX *)m;
        writer->sequence+=2U;writer->report.raw.count^=1U;
    }
}
static void TargetPublishCut(ISOLATED_NVM_TARGET *t)
{
    ISOLATED_TARGET_REPORT report;
    assert(t->mailbox->sequence&1U);
    assert(!ISOLATED_TargetSnapshot(t->mailbox,&report));++odd_publications;
}
static ISOLATED_NVM owner;
static ISOLATED_NVM_MAILBOX owner_mailbox;
static uint8_t memory[1024];
static const uint32_t *source;
static uint32_t operation,address;
static unsigned launches,reads,settles,disarms,init_busy_calls;
static bool automatic,low,init_reject,exclude_after_init;
static void Complete(void)
{
    assert((raw.nvm&0x8000U)&&address==owner.config.page);
    raw.nvm&=~0x8000U;
    if(operation==4U)memset(memory,255,sizeof(memory));
    else {assert(operation==3U);memcpy(memory,source,128U);}
}
ISOLATED_TARGET_RAW ISOLATED_TargetHwRead(void)
{
    ++samples;raw.count+=count_step;
    if(samples>=lock_at)raw.osccon|=32U;
    if(samples==bad_at)raw.pwrc=injected_usb;
    return raw;
}
void ISOLATED_TargetHwGpio(void)
{++gpio_calls;assert(raw.pwrc==0&&raw.dma==0&&(raw.status&1U)==0);if(!reject_gpio){raw.anselb&=~4U;raw.trisb|=4U;}}
uint32_t ISOLATED_TargetHwCount(void){return raw.count;}
bool ISOLATED_TargetHwPressed(void){return pressed;}
uint32_t UPDATE_NvmPortInterruptDisable(void){assert(!(raw.status&1U));return 0;}
void UPDATE_NvmPortInterruptRestore(uint32_t status){assert(status==0);}
bool UPDATE_NvmPortBusy(void)
{
    ++init_busy_calls;
    if(init_reject)return true;
    if(exclude_after_init)raw.dma=0x8000U;
    return (raw.nvm&0x8000U)!=0U;
}
bool UPDATE_NvmPortDmaEnabled(void){return (raw.dma&0x8000U)!=0U;}
bool UPDATE_NvmPortUsbActive(void){return (raw.pwrc&9U)!=0U;}
uint32_t UPDATE_NvmPortError(void){return raw.nvm&0x3000U;}
bool UPDATE_NvmPortLowVoltage(void){return low;}
void UPDATE_NvmPortWait(uint32_t t){raw.count+=t;if(t==24U)++settles;}
bool UPDATE_NvmPortSource(const uint32_t *s,uint32_t *p)
{assert(s==owner.flash.row);source=s;*p=0x1200U;return true;}
void UPDATE_NvmPortMode(uint32_t op)
{assert(!(raw.status&1U)&&!(raw.pwrc&9U)&&!(raw.dma&0x8000U));operation=op;++nvm_writes;}
void UPDATE_NvmPortAddress(uint32_t a){assert(a==owner.config.page);address=a;++nvm_writes;}
void UPDATE_NvmPortSourceAddress(uint32_t a){assert(a==0x1200U);++nvm_writes;}
void UPDATE_NvmPortLaunch(void){raw.nvm|=0x8000U;++nvm_writes;++launches;if(automatic)Complete();}
void UPDATE_NvmPortDisarm(void){assert(!(raw.nvm&0x8000U));++disarms;++nvm_writes;}
bool UPDATE_NvmPortRead(uint32_t a,uint8_t *out,size_t n)
{
    assert(!(raw.nvm&0x8000U)&&a>=owner.config.page&&n<=owner.config.page+1024U-a);
    ++reads;memcpy(out,memory+a-owner.config.page,n);return true;
}
static void Reset(void)
{
    raw=(ISOLATED_TARGET_RAW){.osccon=0x08073300U,.cfg0=0x7FFFFFE3U,
        .cfg1=0xFD74CE5BU,.cfg2=0xFFF97AFAU,.cfg3=UINT32_MAX,.anselb=UINT32_MAX};
    samples=gpio_calls=nvm_writes=bad_at=0;lock_at=1;count_step=1;injected_usb=0;
    reject_gpio=pressed=low=init_reject=exclude_after_init=false;automatic=true;
    source=NULL;operation=address=launches=reads=settles=disarms=init_busy_calls=0;
    snapshot_cut=reader_fields=0;memset(memory,0x33,sizeof(memory));
}
static void InitOwner(ISOLATED_NVM_TARGET *t,ISOLATED_TARGET_MAILBOX *m)
{
    Reset();assert(ISOLATED_TargetPreflight(t,m));
    ISOLATED_NVM_PORT p;assert(ISOLATED_TargetPort(t,&p));
    /* Mock page/timing inputs only; no target entrypoint or page default. */
    ISOLATED_NVM_CONFIG c={0x1D008000U,24000000U,24U,240000U,24000000U};
    assert(ISOLATED_NvmInit(&owner,&owner_mailbox,&c,&p));
    assert(t->initialize_attempts==0U&&!t->adapter.initialized&&!nvm_writes&&!init_busy_calls);
}
static void Step(bool button,uint32_t ticks)
{pressed=button;raw.count+=ticks;ISOLATED_NvmTasks(&owner,ISOLATED_TargetPressed());}
static void Arm(ISOLATED_NVM_TARGET *t)
{
    Step(false,0);Step(false,24);owner_mailbox.token=ISOLATED_NVM_TOKEN;Step(false,0);
    Step(true,1);Step(true,24);Step(false,1);Step(false,24);
    assert(owner.report.state==ISOLATED_RUNNING&&t->initialize_attempts==0&&!nvm_writes);
    Step(false,24);assert(owner.report.phase==ISOLATED_PHASE_ERASE&&t->initialize_attempts==1);
}
int main(void)
{
    ISOLATED_NVM_TARGET t;ISOLATED_TARGET_MAILBOX m;ISOLATED_TARGET_REPORT r;
    ISOLATED_NVM_PORT p;UPDATE_FLASH_BACKEND b;
    Reset();assert(ISOLATED_TargetPreflight(&t,&m));
    assert(t.prepared&&gpio_calls==1&&samples==3&&nvm_writes==0);
    assert(ISOLATED_TargetSnapshot(&m,&r)&&r.state==2&&r.unsafe_bits==0&&r.count_last>r.count_first);
    assert(ISOLATED_TargetPort(&t,&p)&&p.unsafe(p.context)==0&&!nvm_writes);
    assert(p.count(p.context)==raw.count&&!ISOLATED_TargetPressed());pressed=true;assert(ISOLATED_TargetPressed());
    /* Initialization is deferred; accepted WR/errors belong to backend after it. */
    assert(p.initialize(p.context,&b)&&t.initialize_attempts==1&&t.lifecycle==ISOLATED_TARGET_INITIALIZED&&!nvm_writes);
    assert(t.adapter.lvd_ticks==240&&t.adapter.settle_ticks==24);
    raw.nvm=0xB000;assert(p.unsafe(p.context)==0&&b.busy(b.context)&&b.error(b.context)==0x3000);
    raw.nvm=0;assert(!p.initialize(p.context,&b)&&t.lifecycle==ISOLATED_TARGET_INITIALIZED&&!nvm_writes);
    raw.pwrc=1;assert(p.unsafe(p.context)&ISOLATED_TARGET_USB);
    assert(!t.prepared&&t.lifecycle==ISOLATED_TARGET_INIT_FAILED);
    assert(ISOLATED_TargetSnapshot(&m,&r)&&r.state==3&&(r.unsafe_bits&ISOLATED_TARGET_USB));
    uint32_t seq=m.sequence;raw.pwrc=0;assert(p.unsafe(p.context)&&m.sequence==seq); /* failure never disappears */
    /* Every disallowed status fails before GPIO if already present. */
    for(unsigned fault=0;fault<11;fault++) {
        Reset();uint32_t expected;
        switch(fault) {
        case 0:raw.pwrc=8;expected=ISOLATED_TARGET_USB;break;
        case 1:raw.dma=0x8000;expected=ISOLATED_TARGET_DMA;break;
        case 2:raw.status=1;expected=ISOLATED_TARGET_IRQ;break;
        case 3:raw.osccon|=1;expected=ISOLATED_TARGET_CLOCK;break;
        case 4:raw.cause=0x08000000;expected=ISOLATED_TARGET_COUNT;break;
        case 5:raw.debug=0x40000000;expected=ISOLATED_TARGET_DEBUG;break;
        case 6:raw.nvm=0x2000;expected=ISOLATED_TARGET_NVM;break;
        case 7:raw.pullup=4;expected=ISOLATED_TARGET_GPIO;break;
        case 8:raw.pulldown=4;expected=ISOLATED_TARGET_GPIO;break;
        case 9:raw.cfg0^=4;expected=ISOLATED_TARGET_STATE;break;
        default:raw.osccon^=0x1000;expected=ISOLATED_TARGET_CLOCK;break;
        }
        assert(!ISOLATED_TargetPreflight(&t,&m)&&!gpio_calls&&!nvm_writes);
        assert(ISOLATED_TargetSnapshot(&m,&r)&&r.state==3&&(r.unsafe_bits&expected));
        assert(!ISOLATED_TargetPort(&t,&p));
    }
    /* Every configuration word is pinned, even fields not used for timing. */
    for(unsigned word=0;word<4;word++) {
        Reset();switch(word){case 0:raw.cfg0^=0x40;break;case 1:raw.cfg1^=0x10;break;case 2:raw.cfg2^=0x100;break;default:raw.cfg3^=1;}
        assert(!ISOLATED_TargetPreflight(&t,&m)&&t.unsafe_bits&&!nvm_writes);
    }
    Reset();lock_at=5;assert(ISOLATED_TargetPreflight(&t,&m)&&samples==6);
    Reset();lock_at=UINT32_MAX;assert(!ISOLATED_TargetPreflight(&t,&m)&&samples==257&&t.unsafe_bits==ISOLATED_TARGET_CLOCK);
    Reset();count_step=0;assert(!ISOLATED_TargetPreflight(&t,&m)&&samples==257&&(t.unsafe_bits&ISOLATED_TARGET_COUNT));
    Reset();reject_gpio=true;assert(!ISOLATED_TargetPreflight(&t,&m)&&gpio_calls==1&&(t.unsafe_bits&ISOLATED_TARGET_GPIO));
    Reset();bad_at=3;injected_usb=1;assert(!ISOLATED_TargetPreflight(&t,&m)&&!t.prepared&&(t.unsafe_bits&ISOLATED_TARGET_USB)); /* fresh final sample */
    /* Pre-init NVM error is terminal; it cannot be cleared/retried via port. */
    Reset();assert(ISOLATED_TargetPreflight(&t,&m)&&ISOLATED_TargetPort(&t,&p));
    raw.nvm=0x1000;assert(!p.initialize(p.context,&b)&&t.lifecycle==ISOLATED_TARGET_INIT_FAILED&&!nvm_writes);
    raw.nvm=0;assert(!p.initialize(p.context,&b)&&t.initialize_attempts==1);
    /* Null/aliased storage rejected without overwriting existing inputs. */
    Reset();memset(&t,0x55,sizeof(t));ISOLATED_NVM_TARGET before=t;
    assert(!ISOLATED_TargetPreflight(&t,(ISOLATED_TARGET_MAILBOX *)(void *)&t)&&!memcmp(&before,&t,sizeof(t)));
    assert(!ISOLATED_TargetPreflight(NULL,&m)&&!ISOLATED_TargetPort(NULL,&p));
    assert(ISOLATED_TargetPreflight(&t,&m));ISOLATED_TARGET_MAILBOX saved=m;
    assert(!ISOLATED_TargetSnapshot(&m,(ISOLATED_TARGET_REPORT *)(void *)&m.report)&&!memcmp(&saved,&m,sizeof(m)));
    assert(!ISOLATED_TargetPort(&t,(ISOLATED_NVM_PORT *)(void *)&t));
    m.sequence=0;assert(!ISOLATED_TargetSnapshot(&m,&r));m.sequence=3;assert(!ISOLATED_TargetSnapshot(&m,&r));
    m.sequence=ISOLATED_NVM_SNAPSHOT_EXHAUSTED;assert(!ISOLATED_TargetSnapshot(&m,&r));
    assert(!ISOLATED_TargetSnapshot(NULL,&r)&&!ISOLATED_TargetSnapshot(&m,NULL));
    assert(!nvm_writes);
    /* Held SW1 does not prevent digital input setup or authorize init. */
    Reset();pressed=true;assert(ISOLATED_TargetPreflight(&t,&m)&&gpio_calls==1&&!nvm_writes);
    /* All 22 aligned report/raw words reject interrupted snapshots without
     * writing output; every odd publication cut rejects a reader. */
    _Static_assert(sizeof(ISOLATED_TARGET_REPORT)==22U*sizeof(uint32_t),"snapshot fixture drift");
    for(unsigned cut=1;cut<=22U;cut++) {
        Reset();assert(ISOLATED_TargetPreflight(&t,&m));
        snapshot_cut=cut;reader_fields=0;memset(&r,0xA5,sizeof(r));ISOLATED_TARGET_REPORT before_r=r;
        assert(!ISOLATED_TargetSnapshot(&m,&r)&&reader_fields==22U&&!memcmp(&r,&before_r,sizeof(r)));
    }
    assert(odd_publications>0);
    /* Changed-even, odd and unpublished corruption invalidate stale READY. */
    for(unsigned corruption=0;corruption<3U;corruption++) {
        Reset();assert(ISOLATED_TargetPreflight(&t,&m)&&ISOLATED_TargetPort(&t,&p));
        if(corruption==0)m.sequence+=2U;else m.sequence=corruption==1?3U:0U;
        assert(p.unsafe(p.context)&&t.lifecycle==ISOLATED_TARGET_INIT_FAILED&&!t.prepared);
        assert(t.report.state==3U&&m.sequence==ISOLATED_NVM_SNAPSHOT_EXHAUSTED);
        memset(&r,0xA5,sizeof(r));ISOLATED_TARGET_REPORT before_r=r;
        assert(!ISOLATED_TargetSnapshot(&m,&r)&&!memcmp(&r,&before_r,sizeof(r)));
    }
    /* Reserved exhaustion marker makes stale READY payload unavailable. */
    Reset();assert(ISOLATED_TargetPreflight(&t,&m)&&ISOLATED_TargetPort(&t,&p));
    t.publication_sequence=m.sequence=ISOLATED_NVM_SNAPSHOT_EXHAUSTED-2U;
    raw.dma=0x8000U;assert(p.unsafe(p.context)&&t.report.state==3U);
    assert(m.sequence==ISOLATED_NVM_SNAPSHOT_EXHAUSTED&&!ISOLATED_TargetSnapshot(&m,&r));
    /* Adapter rejection and post-init exclusion failure publish terminal state;
     * no retry, launch or backend handed to caller. */
    for(unsigned failure=0;failure<4U;failure++) {
        Reset();assert(ISOLATED_TargetPreflight(&t,&m)&&ISOLATED_TargetPort(&t,&p));
        memset(&b,0xA5,sizeof(b));UPDATE_FLASH_BACKEND before_b=b;
        if(failure==0)init_reject=true;
        if(failure==1)exclude_after_init=true;
        UPDATE_FLASH_BACKEND *out=failure==2?NULL:failure==3?(UPDATE_FLASH_BACKEND *)(void *)&t:&b;
        assert(!p.initialize(p.context,out)&&t.initialize_attempts==1&&t.lifecycle==ISOLATED_TARGET_INIT_FAILED);
        assert(!memcmp(&b,&before_b,sizeof(b))&&!nvm_writes&&!launches);
        assert(ISOLATED_TargetSnapshot(&m,&r)&&r.state==3U&&r.unsafe_bits);
        init_reject=exclude_after_init=false;raw.dma=0;
        assert(!p.initialize(p.context,&b)&&t.initialize_attempts==1);
    }
    /* Exclusion checks stay active in both PRE_INIT and INITIALIZED. */
    for(unsigned initialized=0;initialized<2U;initialized++)for(unsigned fault=0;fault<6U;fault++) {
        Reset();assert(ISOLATED_TargetPreflight(&t,&m)&&ISOLATED_TargetPort(&t,&p));
        if(initialized)assert(p.initialize(p.context,&b));
        switch(fault){case 0:raw.pwrc=8U;break;case 1:raw.dma=0x8000U;break;
        case 2:raw.status=1U;break;case 3:raw.osccon^=0x1000U;break;
        case 4:raw.cause=0x08000000U;break;default:raw.debug=0x40000000U;}
        assert(p.unsafe(p.context)&&t.lifecycle==ISOLATED_TARGET_INIT_FAILED&&!nvm_writes);
        assert(ISOLATED_TargetSnapshot(&m,&r)&&r.state==3U&&r.unsafe_bits);
    }
    /* Real owner -> target port -> real adapter, with all hardware mocked. */
    InitOwner(&t,&m);Arm(&t);
    for(unsigned i=0;i<100U&&!owner.failed&&owner.report.state!=ISOLATED_PASS_SETTLED;i++)Step(false,24);
    assert(owner.report.state==ISOLATED_PASS_SETTLED&&launches==2U&&settles==2U&&disarms==2U&&reads==17U);
    assert(t.lifecycle==ISOLATED_TARGET_INITIALIZED&&!t.adapter.pending);
    unsigned saved_reads=reads,saved_launches=launches;
    for(unsigned i=0;i<4U;i++)Step(true,24);
    assert(reads==saved_reads&&launches==saved_launches&&t.initialize_attempts==1U);
    /* Missing token, held-at-boot gesture, bad token and expired arming cannot init. */
    for(unsigned failure=0;failure<4U;failure++) {
        InitOwner(&t,&m);
        if(failure==0){Step(true,100);Step(false,100);Step(false,100);assert(!owner.spent);}
        else if(failure==1){Step(true,24);owner_mailbox.token=ISOLATED_NVM_TOKEN;Step(true,24);}
        else {Step(false,0);Step(false,24);owner_mailbox.token=failure==2?1U:ISOLATED_NVM_TOKEN;Step(false,0);
            if(failure==3)Step(false,240000);}
        assert(!t.initialize_attempts&&!nvm_writes&&!launches);
        if(failure)assert(owner.failed&&owner.report.state==ISOLATED_FAIL_NO_LAUNCH);
    }
    /* PRE_INIT error after valid gesture blocks initialization and retry. */
    InitOwner(&t,&m);
    Step(false,0);Step(false,24);owner_mailbox.token=ISOLATED_NVM_TOKEN;Step(false,0);
    Step(true,1);Step(true,24);Step(false,1);Step(false,24);
    raw.nvm=0x2000U;Step(false,24);assert(owner.failed&&!launches&&!t.initialize_attempts);
    raw.nvm=0;Step(false,24);assert(!launches&&t.lifecycle==ISOLATED_TARGET_INIT_FAILED);
    /* Expected WR/pending remains backend-owned; terminal timeout retains row
     * through later settlement, without further readback or launch. */
    InitOwner(&t,&m);Arm(&t);automatic=false;Step(false,24);
    assert(launches==1U&&(raw.nvm&0x8000U)&&t.adapter.pending&&t.lifecycle==ISOLATED_TARGET_INITIALIZED);
    Step(false,24);assert(!owner.failed&&t.unsafe_bits==0U);
    Step(false,24000000U);assert(owner.failed&&owner.unsettled&&t.adapter.pending);
    saved_reads=reads;Step(false,24);assert(reads==saved_reads&&launches==1U);
    Complete();Step(false,24);assert(owner.report.state==ISOLATED_FAIL_SETTLED&&!t.adapter.pending&&reads==saved_reads&&launches==1U);
    /* Hardware errors after init reach backend error reporting, not startup. */
    InitOwner(&t,&m);Arm(&t);automatic=false;Step(false,24);
    raw.nvm|=0x2000U;Complete();Step(false,24);
    assert(owner.failed&&owner.report.backend_error==0x2000U&&t.unsafe_bits==0U&&t.lifecycle==ISOLATED_TARGET_INITIALIZED);
    Step(false,24);assert(owner.report.state==ISOLATED_FAIL_SETTLED&&launches==1U);
    puts("PASS: inactive target preflight, 22-word snapshot cuts, deferred owner/adapter lifecycle, failure reporting and settlement; all hardware mocked.");
}
