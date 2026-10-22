#include "isolated_nvm_target.h"
#include <isolated_nvm_target_hw.h>
#include <limits.h>
static void Order(void)
{
#if defined(__XC32)
    __asm__ volatile("sync":::"memory");
#else
    __asm__ volatile("":::"memory");
#endif
}
static bool Overlap(const void *a,size_t an,const void *b,size_t bn)
{uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;return x>=y?x-y<bn:y-x<an;}
#define RAW_FIELDS(C) C(osccon);C(count);C(status);C(cause);C(debug);C(pwrc);C(dma);C(nvm);C(cfg0);C(cfg1);C(cfg2);C(cfg3);C(anselb);C(trisb);C(pullup);C(pulldown)
#define REPORT_FIELDS(C) C(version);C(state);C(unsafe_bits);C(observations);C(count_first);C(count_last)
static bool Publish(ISOLATED_NVM_TARGET *t)
{
    uint32_t seq=t->publication_sequence;
    bool consistent=t->mailbox->sequence==seq&&(seq&1U)==0U;
    if(!consistent||seq>=ISOLATED_NVM_SNAPSHOT_EXHAUSTED-2U) {
        t->unsafe_bits|=ISOLATED_TARGET_STATE;t->report.unsafe_bits=t->unsafe_bits;
        t->report.state=3U;t->prepared=false;t->lifecycle=ISOLATED_TARGET_INIT_FAILED;
        /* Invalidate stale payload even if a changed even sequence would
         * otherwise let a reader accept old READY. Never recover/publish it. */
        Order();t->mailbox->sequence=ISOLATED_NVM_SNAPSHOT_EXHAUSTED;
        t->publication_sequence=ISOLATED_NVM_SNAPSHOT_EXHAUSTED;
        return false;
    }
    t->mailbox->sequence=seq+1U;Order();
#define COPY(n) t->mailbox->report.n=t->report.n
    REPORT_FIELDS(COPY);
#undef COPY
#define COPY(n) t->mailbox->report.raw.n=t->report.raw.n
    RAW_FIELDS(COPY);
#undef COPY
    Order();t->mailbox->sequence=seq+2U;t->publication_sequence=seq+2U;return true;
}
bool ISOLATED_TargetSnapshot(const ISOLATED_TARGET_MAILBOX *m,ISOLATED_TARGET_REPORT *out)
{
    if(m==NULL||out==NULL||Overlap(m,sizeof(*m),out,sizeof(*out)))return false;
    uint32_t seq=m->sequence;if(seq==0U||(seq&1U)||seq==ISOLATED_NVM_SNAPSHOT_EXHAUSTED)return false;
    Order();ISOLATED_TARGET_REPORT r;
#define COPY(n) r.n=m->report.n
    REPORT_FIELDS(COPY);
#undef COPY
#define COPY(n) r.raw.n=m->report.raw.n
    RAW_FIELDS(COPY);
#undef COPY
    Order();if(seq!=m->sequence)return false;*out=r;return true;
}
static uint32_t Check(const ISOLATED_TARGET_RAW *r,bool nvm,bool gpio,bool locked)
{
    uint32_t bad=0;
    if(r->pwrc&9U)bad|=ISOLATED_TARGET_USB;
    if(r->dma&0x8000U)bad|=ISOLATED_TARGET_DMA;
    if(r->status&1U)bad|=ISOLATED_TARGET_IRQ;
    /* Board profile: primary PLL x24 /2, PB /1; config input /3, HS,
     * primary+PLL and watchdog OFF. No software clock switching. */
    if((r->osccon&0x381F7709U)!=0x08073300U||
       (locked&&(r->osccon&32U)==0U)||
       (r->cfg1&0x00803307U)!=0x00000203U||
       (r->cfg2&0x00070077U)!=0x00010072U)bad|=ISOLATED_TARGET_CLOCK;
    if(r->cause&0x08000000U)bad|=ISOLATED_TARGET_COUNT;
    /* No approved debug mode/profile yet; do not infer COUNTDM safety. */
    if((r->debug&0x40000000U)||(r->cfg0&3U)!=3U)bad|=ISOLATED_TARGET_DEBUG;
    if(r->cfg0!=0x7FFFFFE3U||r->cfg1!=0xFD74CE5BU||
       r->cfg2!=0xFFF97AFAU||r->cfg3!=UINT32_MAX)bad|=ISOLATED_TARGET_STATE;
    if(nvm&&(r->nvm&0xB000U))bad|=ISOLATED_TARGET_NVM;
    if((r->pullup|r->pulldown)&4U)bad|=ISOLATED_TARGET_GPIO;
    if(gpio&&((r->anselb&4U)||(r->trisb&4U)==0U))bad|=ISOLATED_TARGET_GPIO;
    return bad;
}
static void Fail(ISOLATED_NVM_TARGET *t,uint32_t bad)
{
    t->unsafe_bits|=bad;t->report.unsafe_bits=t->unsafe_bits;
    t->report.state=3U;t->prepared=false;t->lifecycle=ISOLATED_TARGET_INIT_FAILED;
    (void)Publish(t);
}
bool ISOLATED_TargetPreflight(ISOLATED_NVM_TARGET *t,ISOLATED_TARGET_MAILBOX *m)
{
    if(t==NULL||m==NULL||Overlap(t,sizeof(*t),m,sizeof(*m)))return false;
    *t=(ISOLATED_NVM_TARGET){.mailbox=m};m->sequence=0;
    t->report=(ISOLATED_TARGET_REPORT){.version=1,.state=1};
    if(!Publish(t))return false;
    ISOLATED_TARGET_RAW r=ISOLATED_TargetHwRead();uint32_t bad=Check(&r,false,false,false);
    /* WR/errors are terminal even before permitted GPIO setup. */
    if(r.nvm&0xB000U)bad|=ISOLATED_TARGET_NVM;
    t->report.raw=r;t->report.count_first=r.count;t->report.count_last=r.count;
    if(bad){Fail(t,bad);return false;}
    ISOLATED_TargetHwGpio(); /* Only RB2 ANSEL clear / TRIS input set. */
    bool changed=false;
    for(unsigned i=0;i<ISOLATED_TARGET_OBSERVATIONS;i++) {
        r=ISOLATED_TargetHwRead();t->report.raw=r;t->report.observations=i+1U;t->report.count_last=r.count;
        changed=changed||r.count!=t->report.count_first;
        bad=Check(&r,true,true,false);
        if(bad){Fail(t,bad);return false;}
        if(!Publish(t))return false;
        if(changed&&(r.osccon&32U)) {
            /* Fresh final sample must still match all preflight conditions. */
            r=ISOLATED_TargetHwRead();t->report.raw=r;t->report.count_last=r.count;
            bad=Check(&r,true,true,true);if(bad){Fail(t,bad);return false;}
            t->prepared=true;t->report.state=2U;return Publish(t);
        }
    }
    bad=Check(&r,true,true,true);if(!changed)bad|=ISOLATED_TARGET_COUNT;
    Fail(t,bad?bad:ISOLATED_TARGET_CLOCK);return false;
}
static uint32_t Unsafe(void *context)
{
    ISOLATED_NVM_TARGET *t=context;
    if(!t->prepared||t->lifecycle==ISOLATED_TARGET_INIT_FAILED)return t->unsafe_bits|ISOLATED_TARGET_STATE;
    if(t->mailbox->sequence!=t->publication_sequence||t->publication_sequence==ISOLATED_NVM_SNAPSHOT_EXHAUSTED)
        {Fail(t,ISOLATED_TARGET_STATE);return t->unsafe_bits;}
    ISOLATED_TARGET_RAW r=ISOLATED_TargetHwRead();
    uint32_t bad=Check(&r,t->lifecycle==ISOLATED_TARGET_PRE_INIT,true,true);
    /* Latch any exclusion violation. Never reinterpret active WR or NVM errors
     * as startup-unsafe after successful initialization. Backend owns those. */
    if(bad!=0U){t->report.raw=r;Fail(t,bad);}
    return t->unsafe_bits;
}
static uint32_t Count(void *context){(void)context;return ISOLATED_TargetHwCount();}
static bool Initialize(void *context,UPDATE_FLASH_BACKEND *out)
{
    ISOLATED_NVM_TARGET *t=context;
    if(t->lifecycle!=ISOLATED_TARGET_PRE_INIT||t->initialize_attempts!=0U)return false;
    t->initialize_attempts=1U;
    if(out==NULL||Overlap(t,sizeof(*t),out,sizeof(*out))||
       Overlap(t->mailbox,sizeof(*t->mailbox),out,sizeof(*out))) {
        Fail(t,ISOLATED_TARGET_STATE);return false;
    }
    if(Unsafe(t)!=0U){t->lifecycle=ISOLATED_TARGET_INIT_FAILED;return false;}
    UPDATE_FLASH_BACKEND b={0};
    bool ok=UPDATE_NvmInit(&t->adapter,48000000U,&b);
    t->lifecycle=ok?ISOLATED_TARGET_INITIALIZED:ISOLATED_TARGET_INIT_FAILED;
    if(!ok){t->report.raw=ISOLATED_TargetHwRead();Fail(t,ISOLATED_TARGET_NVM);return false;}
    if(Unsafe(t)!=0U){t->lifecycle=ISOLATED_TARGET_INIT_FAILED;return false;}
    *out=b;return true;
}
bool ISOLATED_TargetPort(ISOLATED_NVM_TARGET *t,ISOLATED_NVM_PORT *out)
{
    if(t==NULL||out==NULL||!t->prepared||t->lifecycle!=ISOLATED_TARGET_PRE_INIT||
       Overlap(t,sizeof(*t),out,sizeof(*out))||Overlap(t->mailbox,sizeof(*t->mailbox),out,sizeof(*out)))return false;
    if(Unsafe(t)!=0U)return false;
    *out=(ISOLATED_NVM_PORT){t,Count,Unsafe,Initialize};return true;
}
bool ISOLATED_TargetPressed(void){return ISOLATED_TargetHwPressed();}
