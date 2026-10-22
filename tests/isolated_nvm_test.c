#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "isolated_nvm.h"
/* Internal fixture uses the actual owner TU to challenge private wrapper calls;
 * no extra production API or hardware operation is exposed. */
static unsigned snapshot_cut,reader_fields,odd_publications;
static void SnapshotCut(const ISOLATED_NVM_MAILBOX *m);
static void PublishCut(ISOLATED_NVM *o);
#include "isolated-body.inc"
static void SnapshotCut(const ISOLATED_NVM_MAILBOX *m)
{
    ++reader_fields;
    if(snapshot_cut==reader_fields) {
        /* Single-thread deterministic interleaving, not a C data race. */
        ISOLATED_NVM_MAILBOX *writer=(ISOLATED_NVM_MAILBOX *)m;
        writer->sequence+=2U;writer->report.page^=1024U;
    }
}
static void PublishCut(ISOLATED_NVM *o)
{
    ISOLATED_NVM_REPORT report;assert(!ISOLATED_NvmSnapshot(o->mailbox,&report));++odd_publications;
}
typedef struct {
    uint32_t now,unsafe,slow_erase,slow_write,slow_read,slow_busy,slow_unsafe,fail_unsafe;
    unsigned inits,erases,writes,reads,busies;
    bool busy,settle,auto_complete,reject_erase,reject_write,reject_init,corrupt;
    uint8_t memory[1024];const uint32_t *source;
    ISOLATED_NVM *owner;
} MOCK;
static uint32_t Tick(void *p){return ((MOCK*)p)->now;}
static uint32_t Unsafe(void *p){MOCK *m=p;m->now+=m->slow_unsafe;return m->unsafe;}
static bool MockBusy(void *p)
{
    MOCK *m=p;++m->busies;m->now+=m->slow_busy;
    if(m->slow_busy)m->busy=false;
    if(!m->busy)m->settle=false;
    return m->busy;
}
static uint32_t MockError(void *p){(void)p;return 0;}
static bool MockErase(void *p,uint32_t address)
{
    MOCK *m=p;assert(address==m->owner->config.page&&m->owner->report.erase_attempts==1);
    ISOLATED_NVM_REPORT r;assert(ISOLATED_NvmSnapshot(m->owner->mailbox,&r)&&r.erase_attempts==1);
    ++m->erases;m->now+=m->slow_erase;m->unsafe|=m->fail_unsafe;
    if(m->reject_erase)return false;
    memset(m->memory,255,sizeof(m->memory));m->busy=!m->auto_complete;m->settle=m->busy;return true;
}
static bool MockWrite(void *p,const uint32_t *row,uint32_t address)
{
    MOCK *m=p;assert(address==m->owner->config.page&&row==m->owner->flash.row&&m->owner->report.row_attempts==1);
    ++m->writes;m->source=row;m->now+=m->slow_write;m->unsafe|=m->fail_unsafe;
    if(m->reject_write)return false;
    memcpy(m->memory,row,128);m->busy=!m->auto_complete;m->settle=m->busy;return true;
}
static bool MockRead(void *p,uint32_t a,uint8_t *out,size_t n)
{
    MOCK *m=p;assert(!m->busy&&!m->settle&&a>=m->owner->config.page&&n<=m->owner->config.page+1024-a);
    ++m->reads;memcpy(out,m->memory+a-m->owner->config.page,n);if(m->corrupt)out[0]^=1;
    m->now+=m->slow_read;m->unsafe|=m->fail_unsafe;return true;
}
static bool InitBackend(void *p,UPDATE_FLASH_BACKEND *b)
{
    MOCK *m=p;assert(m->owner->spent&&m->owner->report.arm_attempts==1);++m->inits;
    *b=(UPDATE_FLASH_BACKEND){p,MockBusy,MockError,MockErase,MockWrite,MockRead};return !m->reject_init;
}
static void Init(ISOLATED_NVM *o,ISOLATED_NVM_MAILBOX *mb,MOCK *m)
{
    memset(m,0,sizeof(*m));memset(o,0,sizeof(*o));memset(mb,0xA5,sizeof(*mb));
    m->owner=o;m->auto_complete=true;
    ISOLATED_NVM_CONFIG c={0x1D008000,24000000,10,200,1000};
    ISOLATED_NVM_PORT p={m,Tick,Unsafe,InitBackend};
    assert(ISOLATED_NvmInit(o,mb,&c,&p));assert(mb->token==0&&mb->sequence==2);
    assert(!m->inits&&!m->erases&&!m->writes&&!m->reads);
}
static void Step(ISOLATED_NVM *o,MOCK *m,bool pressed,unsigned ticks)
{m->now+=ticks;ISOLATED_NvmTasks(o,pressed);}
static void Reserve(ISOLATED_NVM *o,MOCK *m)
{
    Step(o,m,false,0);Step(o,m,false,10);
    o->mailbox->token=ISOLATED_NVM_TOKEN;Step(o,m,false,0);
    assert(o->report.state==ISOLATED_ARMED&&o->spent&&!m->inits);
}
static void Arm(ISOLATED_NVM *o,MOCK *m)
{
    Reserve(o,m);Step(o,m,true,1);Step(o,m,true,10);Step(o,m,false,1);Step(o,m,false,10);
    assert(o->report.state==ISOLATED_RUNNING&&!m->inits);
}
static void To(ISOLATED_NVM *o,MOCK *m,ISOLATED_NVM_PHASE phase)
{for(unsigned i=0;i<100&&o->report.phase!=(unsigned)phase&&!o->failed;i++)Step(o,m,false,1);assert(o->report.phase==(unsigned)phase&&!o->failed);}
static void Finish(ISOLATED_NVM *o,MOCK *m)
{for(unsigned i=0;i<100&&!o->failed&&o->report.state!=ISOLATED_PASS_SETTLED;i++)Step(o,m,false,1);}
static void Aliases(void)
{
    ISOLATED_NVM o;ISOLATED_NVM_MAILBOX mb;
    ISOLATED_NVM_CONFIG c={0x1D008000,24000000,10,200,1000};
    ISOLATED_NVM_PORT p={NULL,Tick,Unsafe,InitBackend};
    memset(&o,0x5A,sizeof(o));memset(&mb,0xA5,sizeof(mb));
    o.config=c;o.port=p;
    ISOLATED_NVM before=o;ISOLATED_NVM_MAILBOX before_mb=mb;
    assert(!ISOLATED_NvmInit(&o,(ISOLATED_NVM_MAILBOX *)(void *)&o,&c,&p));
    assert(!memcmp(&o,&before,sizeof(o))&&!memcmp(&mb,&before_mb,sizeof(mb)));
    assert(!ISOLATED_NvmInit(&o,&mb,&o.config,&p));
    assert(!ISOLATED_NvmInit(&o,&mb,&c,&o.port));
    assert(!memcmp(&o,&before,sizeof(o))&&!memcmp(&mb,&before_mb,sizeof(mb)));
    /* Real overlap with valid inputs; no unsupported misaligned pointers. */
    union {max_align_t alignment;unsigned char bytes[128];} storage;
    memset(&storage,0x55,sizeof(storage));
    ISOLATED_NVM_MAILBOX *m=(void *)storage.bytes;
    ISOLATED_NVM_CONFIG *in_c=(void *)(storage.bytes+8);*in_c=c;
    unsigned char saved[128];memcpy(saved,storage.bytes,128);
    assert(!ISOLATED_NvmInit(&o,m,in_c,&p));assert(!memcmp(saved,storage.bytes,128));
    ISOLATED_NVM_PORT *in_p=(void *)(storage.bytes+8);*in_p=p;
    memcpy(saved,storage.bytes,128);
    assert(!ISOLATED_NvmInit(&o,m,&c,in_p));assert(!memcmp(saved,storage.bytes,128));
    memset(&storage,0x55,sizeof(storage));in_c=(void *)storage.bytes;*in_c=c;
    in_p=(void *)(storage.bytes+16);*in_p=p;in_p->context=(void *)(uintptr_t)1000;
    /* context's low word is the valid operation_ticks; inputs overlap. */
    assert(in_c->operation_ticks==1000);memcpy(saved,storage.bytes,128);
    assert(!ISOLATED_NvmInit(&o,&mb,in_c,in_p));assert(!memcmp(saved,storage.bytes,128));
    assert(!Overlap((void *)(uintptr_t)(UINTPTR_MAX-16U),8,(void *)(uintptr_t)8,8));
}
int main(void)
{
    ISOLATED_NVM o;ISOLATED_NVM_MAILBOX mb;MOCK m;ISOLATED_NVM_REPORT r;
    Aliases();
    Init(&o,&mb,&m);Arm(&o,&m);Finish(&o,&m);
    assert(o.report.state==ISOLATED_PASS_SETTLED&&m.inits==1&&m.erases==1&&m.writes==1&&m.reads==17);
    for(unsigned i=0;i<1024;i++)assert(m.memory[i]==(i<128?(uint8_t)(i^0xA5):255));
    assert(ISOLATED_NvmSnapshot(&mb,&r)&&r.verify_offset==1024&&r.erase_attempts==1&&r.row_attempts==1);
    uint32_t seq=mb.sequence;mb.token=ISOLATED_NVM_TOKEN;Step(&o,&m,true,100);assert(mb.sequence==seq&&m.erases==1&&m.writes==1);
    /* Wrong token/held button consume the boot's one opportunity irreversibly. */
    Init(&o,&mb,&m);mb.token=123;Step(&o,&m,false,0);assert(o.failed&&o.spent&&o.report.fault==ISOLATED_FAULT_TOKEN);
    mb.token=ISOLATED_NVM_TOKEN;Step(&o,&m,false,20);assert(!m.inits&&!m.erases&&o.report.state==ISOLATED_FAIL_NO_LAUNCH);
    Init(&o,&mb,&m);Step(&o,&m,true,20);mb.token=ISOLATED_NVM_TOKEN;Step(&o,&m,true,1);assert(o.failed&&!m.inits);
    Init(&o,&mb,&m);Step(&o,&m,false,0);Step(&o,&m,false,10);
    Step(&o,&m,true,1);Step(&o,&m,false,1);mb.token=ISOLATED_NVM_TOKEN;Step(&o,&m,false,0);
    assert(o.failed&&o.spent&&!m.inits&&o.report.fault==ISOLATED_FAULT_GESTURE); /* release bounce cannot reuse historical qualification */
    Init(&o,&mb,&m);Reserve(&o,&m);Step(&o,&m,false,200);assert(o.report.fault==ISOLATED_FAULT_ARM_TIMEOUT&&!m.inits);
    Init(&o,&mb,&m);Reserve(&o,&m);Step(&o,&m,true,1);Step(&o,&m,true,10);Step(&o,&m,false,1);Step(&o,&m,false,188);
    assert(o.report.fault==ISOLATED_FAULT_ARM_TIMEOUT&&!m.inits); /* exact deadline beats release */
    Init(&o,&mb,&m);Reserve(&o,&m);Step(&o,&m,true,1);Step(&o,&m,true,10);Step(&o,&m,false,1);
    m.slow_unsafe=200;Step(&o,&m,false,10);
    assert(o.failed&&o.report.fault==ISOLATED_FAULT_ARM_TIMEOUT&&!m.inits); /* exclusion check crosses deadline before gesture acceptance */
    /* Invalid config never opens an adapter or launches. */
    ISOLATED_NVM_CONFIG c={0x1D008000,24000000,10,200,1000};ISOLATED_NVM_PORT p={&m,Tick,Unsafe,InitBackend};
    for(unsigned i=0;i<4;i++){uint32_t pages[]={0x1D000000,0x1D03FC00,0x1D008001,0xFFFFFC00};c.page=pages[i];assert(!ISOLATED_NvmInit(&o,&mb,&c,&p));}
    c.page=0x1D008000;c.count_hz=1;assert(!ISOLATED_NvmInit(&o,&mb,&c,&p));
    assert(!ISOLATED_NvmInit(NULL,&mb,&c,&p)&&!ISOLATED_NvmSnapshot(NULL,&r)&&!ISOLATED_NvmSnapshot(&mb,NULL));
    /* Every call boundary can fail exclusion; never start the next operation. */
    for(unsigned bit=1;bit<=16;bit<<=1){Init(&o,&mb,&m);Reserve(&o,&m);m.unsafe=bit;Step(&o,&m,true,1);assert(o.failed&&!m.inits&&!m.erases);}
    Init(&o,&mb,&m);Arm(&o,&m);m.reject_init=true;Step(&o,&m,false,1);assert(o.failed&&!m.erases);
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ERASE);m.reject_erase=true;Step(&o,&m,false,1);
    assert(o.failed&&m.erases==1&&!m.writes);Step(&o,&m,false,1);assert(o.report.state==ISOLATED_FAIL_SETTLED);
    /* Actual private wrapper whitelist challenged without new public API. */
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ERASE);
    assert(!Erase(&o,o.config.page+1024)&&m.erases==0&&o.failed);
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ERASE_VERIFY);
    uint8_t b[128];assert(!Read(&o,o.config.page+1000,b,128)&&o.failed);
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ROW);
    uint32_t row[32];assert(!Write(&o,row,o.config.page)&&o.failed&&!m.writes);
    /* Late complete erase/write/read/settlement may not become success. */
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ERASE);m.slow_erase=1000;Step(&o,&m,false,1);
    assert(o.failed&&o.report.fault==ISOLATED_FAULT_TIMEOUT&&!m.writes&&!m.reads);
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ROW);m.slow_write=1000;Step(&o,&m,false,1);
    assert(o.failed&&o.report.fault==ISOLATED_FAULT_TIMEOUT&&m.writes==1);
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ERASE_VERIFY);m.slow_read=1000;Step(&o,&m,false,1);
    assert(o.failed&&m.reads==1&&!m.writes);
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ERASE_VERIFY);m.slow_busy=1000;Step(&o,&m,false,1);
    assert(o.failed&&!m.reads&&!m.writes);
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_FINAL_READ);m.slow_read=1000;Step(&o,&m,false,1);assert(o.failed&&o.report.state!=ISOLATED_PASS_SETTLED);
    /* Pending timeout retains row and only busy/settlement may run afterwards. */
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ROW);m.auto_complete=false;Step(&o,&m,false,1);
    assert(m.source==o.flash.row&&m.busy&&m.settle);
    uint8_t saved[128];memcpy(saved,m.source,128);Step(&o,&m,false,1000);assert(o.failed&&o.unsettled);
    unsigned reads=m.reads,writes=m.writes;Step(&o,&m,false,10);assert(o.unsettled&&reads==m.reads&&writes==m.writes&&!memcmp(saved,m.source,128));
    m.busy=false;Step(&o,&m,false,1);assert(o.report.state==ISOLATED_FAIL_SETTLED&&!m.settle);
    seq=mb.sequence;mb.token=ISOLATED_NVM_TOKEN;Step(&o,&m,true,100);assert(seq==mb.sequence&&m.writes==1);
    /* Corruption and exclusion introduced inside callbacks fail closed. */
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ERASE_VERIFY);m.corrupt=true;Step(&o,&m,false,1);assert(o.failed&&!m.writes);
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ERASE);m.fail_unsafe=1;Step(&o,&m,false,1);assert(o.failed&&!m.writes&&!m.reads);
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_FINAL_READ);
    m.memory[896]=0;Finish(&o,&m);assert(o.failed&&o.report.state!=ISOLATED_PASS_SETTLED); /* untouched rows are also verified */
    /* Counter wrap is modular, not mistaken for a deadline reset. */
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ERASE);m.now=UINT32_MAX-20;Step(&o,&m,false,0);Finish(&o,&m);assert(o.report.state==ISOLATED_PASS_SETTLED);
    /* Budgets longer than one second must not time out at Flash's old limit. */
    Init(&o,&mb,&m);o.config.operation_ticks=48000000U;Arm(&o,&m);
    To(&o,&m,ISOLATED_PHASE_ERASE);m.auto_complete=false;Step(&o,&m,false,1);
    assert(o.flash.timeout_ms==2000U&&m.busy);Step(&o,&m,false,24000000U);
    assert(!o.failed&&o.flash.status==UPDATE_FLASH_BUSY&&!m.reads&&!m.writes);
    m.busy=false;m.auto_complete=true;Finish(&o,&m);assert(o.report.state==ISOLATED_PASS_SETTLED);
    Init(&o,&mb,&m);o.config.operation_ticks=24001U;Arm(&o,&m);
    To(&o,&m,ISOLATED_PHASE_ERASE);Step(&o,&m,false,1);assert(o.flash.timeout_ms==2U);
    Init(&o,&mb,&m);o.config.operation_ticks=INT32_MAX;Arm(&o,&m);
    To(&o,&m,ISOLATED_PHASE_ERASE);Step(&o,&m,false,1);assert(o.flash.timeout_ms==89479U);
    Init(&o,&mb,&m);Arm(&o,&m);To(&o,&m,ISOLATED_PHASE_ERASE_VERIFY);
    o.flash.timeout_ms=0;m.busy=true;Step(&o,&m,false,1);
    assert(o.failed&&o.report.fault==ISOLATED_FAULT_TIMEOUT); /* generic timeout keeps its fault class */
    /* Coherent-but-stale payload cannot be accepted on exhaustion. */
    Init(&o,&mb,&m);Reserve(&o,&m);mb.sequence=o.publication_sequence=SNAPSHOT_EXHAUSTED-2U;
    ISOLATED_NVM_REPORT stale=mb.report;assert(!Phase(&o,ISOLATED_PHASE_INIT));
    assert(o.failed&&mb.sequence==SNAPSHOT_EXHAUSTED&&!m.inits&&!m.erases);
    assert(!memcmp(&stale,(const void *)&mb.report,sizeof(stale)));
    memset(&r,0xA5,sizeof(r));ISOLATED_NVM_REPORT untouched=r;
    assert(!ISOLATED_NvmSnapshot(&mb,&r)&&!memcmp(&r,&untouched,sizeof(r)));
    /* Output aliases, including partial overlap, must not mutate the mailbox. */
    Init(&o,&mb,&m);ISOLATED_NVM_MAILBOX saved_mb=mb;
    assert(!ISOLATED_NvmSnapshot(&mb,(ISOLATED_NVM_REPORT *)(void *)&mb.report));
    assert(!ISOLATED_NvmSnapshot(&mb,(ISOLATED_NVM_REPORT *)(void *)&mb));
    assert(!memcmp(&mb,&saved_mb,sizeof(mb)));
    mb.sequence=0;assert(!ISOLATED_NvmSnapshot(&mb,&r));
    /* Odd or exhausted snapshots cannot authorize a launch. */
    Init(&o,&mb,&m);Reserve(&o,&m);mb.sequence=o.publication_sequence=UINT32_MAX-1;Step(&o,&m,true,1);Step(&o,&m,true,10);Step(&o,&m,false,1);Step(&o,&m,false,10);
    assert(o.failed&&o.snapshot_exhausted&&!m.inits&&!m.erases&&mb.sequence==UINT32_MAX-1);
    mb.sequence=3;assert(!ISOLATED_NvmSnapshot(&mb,&r));
    Init(&o,&mb,&m);Reserve(&o,&m);mb.sequence=0;Step(&o,&m,true,1);Step(&o,&m,true,10);Step(&o,&m,false,1);Step(&o,&m,false,10);
    assert(o.failed&&o.snapshot_exhausted&&!m.inits&&mb.sequence==0);
    for(unsigned cut=1;cut<=14;cut++) {
        snapshot_cut=0;Init(&o,&mb,&m);reader_fields=0;snapshot_cut=cut;
        assert(!ISOLATED_NvmSnapshot(&mb,&r));
    }
    snapshot_cut=0;Init(&o,&mb,&m);reader_fields=0;
    assert(ISOLATED_NvmSnapshot(&mb,&r)&&reader_fields==14&&odd_publications>0);
    puts("PASS: inactive isolated arming/page wrapper, once-only transaction, canonical pattern, late-OK budgets, settlement quarantine, snapshots and no runtime binding.");
}
