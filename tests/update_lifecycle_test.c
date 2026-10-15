#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "update_lifecycle.h"
#define START UPDATE_APP_PHYSICAL_START
#define END UPDATE_FLASH_PHYSICAL_END
static uint8_t memory[END-START];
static bool busy,usb=true,reject;
static uint32_t address,error,now;
static const uint32_t *source;
static unsigned writes,erases;
static bool Busy(void *c){(void)c;return busy;}
static uint32_t Error(void *c){(void)c;return error;}
static bool Erase(void *c,uint32_t a)
{(void)c;assert(!usb);++erases;if(reject)return false;address=a;source=NULL;busy=true;return true;}
static bool Write(void *c,const uint32_t *p,uint32_t a)
{(void)c;assert(!usb);++writes;if(reject)return false;address=a;source=p;busy=true;return true;}
static bool Read(void *c,uint32_t a,uint8_t *p,size_t n)
{(void)c;assert(!usb&&!busy);memcpy(p,memory+a-START,n);return true;}
static const UPDATE_FLASH_BACKEND backend={NULL,Busy,Error,Erase,Write,Read};
static void Finish(void)
{assert(busy);if(source)memcpy(memory+address-START,source,128);else memset(memory+address-START,255,1024);busy=false;}
static void Reset(UPDATE_LIFECYCLE *c)
{
    busy=reject=false;usb=true;error=now=writes=erases=0;source=NULL;
    memset(memory,255,sizeof(memory));assert(UPDATE_LifecycleInit(c,&backend,7,100,10000,true));
}
static void Pump(UPDATE_LIFECYCLE *c)
{
    usb=false;
    for(unsigned i=0;i<10000&&c->state!=UPDATE_RESULT;i++)
    {if(busy)Finish();UPDATE_LifecyclePoll(c,now++,true);}
    assert(c->state==UPDATE_RESULT&&!busy);usb=true;
}
static UPDATE_STAGED_OPERATION Begin(uint32_t id,uint32_t token,uint32_t length,uint32_t crc)
{return (UPDATE_STAGED_OPERATION){.epoch=7,.token=token,.id=id,.kind=UPDATE_STAGE_BEGIN,.length=length,.crc=crc};}
static size_t Rec(uint8_t *p,uint8_t type,uint16_t a,const uint8_t *data,uint8_t n)
{
    p[0]=n;p[1]=(uint8_t)(a>>8);p[2]=(uint8_t)a;p[3]=type;
    if(n)memcpy(p+4,data,n);
    uint8_t sum=0;for(size_t i=0;i<n+4U;i++)sum=(uint8_t)(sum+p[i]);p[n+4U]=(uint8_t)(0U-sum);return n+5U;
}
static void Accepted(UPDATE_LIFECYCLE *c,const UPDATE_STAGED_OPERATION *op)
{
    uint32_t ticket=0;unsigned e=erases,w=writes;
    assert(UPDATE_LifecycleStage(c,op,now,&ticket)&&ticket);
    assert(UPDATE_LifecycleStage(c,op,now,&ticket));assert(e==erases&&w==writes);
    UPDATE_STAGED_OPERATION changed=*op;changed.crc^=1;
    assert(!UPDATE_LifecycleStage(c,&changed,now,&ticket));
    assert(!UPDATE_LifecycleExecute(c,8,c->token,c->last_id,c->ticket,now));
    assert(!UPDATE_LifecycleExecute(c,7,c->token,c->last_id,c->ticket+1,now));
    assert(UPDATE_LifecycleExecute(c,7,c->token,c->last_id,c->ticket,now));
    assert(UPDATE_LifecycleExecute(c,7,c->token,c->last_id,c->ticket,now));
    UPDATE_LifecyclePoll(c,now++,false);assert(c->state==UPDATE_ACK_WAIT&&e==erases&&w==writes);
    UPDATE_LifecycleAck(c,c->ticket+1,true,now);assert(c->state==UPDATE_ACK_WAIT);
    UPDATE_LifecycleAck(c,c->ticket,true,now);assert(c->state==UPDATE_OFF_WAIT);
    UPDATE_LifecyclePoll(c,now++,false);assert(c->state==UPDATE_OFF_WAIT&&e==erases&&w==writes);
}
static void Resume(UPDATE_LIFECYCLE *c)
{
    unsigned e=erases,w=writes;assert(c->outcome==UPDATE_OUTCOME_DONE);
    uint32_t t;assert(UPDATE_LifecycleStage(c,&c->staged,now,&t)&&t==c->ticket);
    assert(UPDATE_LifecycleExecute(c,7,c->token,c->last_id,c->ticket,now));
    assert(!UPDATE_LifecycleResume(c,8,c->token,c->last_id,c->ticket,now));
    assert(UPDATE_LifecycleResume(c,7,c->token,c->last_id,c->ticket,now));
    assert(UPDATE_LifecycleResume(c,7,c->token,c->last_id,c->ticket,now));
    assert(e==erases&&w==writes&&c->state==UPDATE_LINK);
}
static void CompleteImage(UPDATE_LIFECYCLE *c,bool stop_at_commit)
{
    uint8_t image[128];for(unsigned i=0;i<128;i++)image[i]=(uint8_t)i;
    uint32_t crc=UPDATE_Crc32Step(UINT32_MAX,image,128)^UINT32_MAX;
    UPDATE_STAGED_OPERATION op=Begin(1,1,128,crc);Accepted(c,&op);Pump(c);Resume(c);
    op=(UPDATE_STAGED_OPERATION){.epoch=7,.token=1,.id=2,.kind=UPDATE_STAGE_BATCH};
    op.size=Rec(op.records,4,0,(uint8_t[]){0x1D,0},2);
    op.size+=Rec(op.records+op.size,0,0x8000,image,128);
    op.size+=Rec(op.records+op.size,1,0,NULL,0);
    Accepted(c,&op);usb=false;
    if(stop_at_commit)
    {
        for(unsigned i=0;i<1000&&c->session.status!=UPDATE_SESSION_COMMITTING;i++)
        {if(busy)Finish();UPDATE_LifecyclePoll(c,now++,true);}
        assert(busy&&c->session.status==UPDATE_SESSION_COMMITTING);return;
    }
    Pump(c);Resume(c);assert(c->session.status==UPDATE_SESSION_COMPLETE);
}
int main(void)
{
    UPDATE_LIFECYCLE c;UPDATE_STAGED_OPERATION op;uint32_t ticket;
    Reset(&c);op=Begin(1,1,128,0);assert(!UPDATE_LifecycleStage(&c,&op,0,NULL));
    op.epoch=8;assert(!UPDATE_LifecycleStage(&c,&op,0,&ticket));op.epoch=7;
    op.length=3;assert(!UPDATE_LifecycleStage(&c,&op,0,&ticket));op.length=128;
    UPDATE_LIFECYCLE before=c;UPDATE_STAGED_OPERATION original=op;
    assert(!UPDATE_LifecycleStage(&c,&op,0,&c.ticket));assert(!memcmp(&c,&before,sizeof(c)));
    assert(!UPDATE_LifecycleStage(&c,&op,0,&op.id));assert(!memcmp(&op,&original,sizeof(op)));
    Accepted(&c,&op);assert(erases==0);
    uint32_t saved=c.ticket;assert(!UPDATE_LifecycleStage(&c,&c.staged,now,&c.ticket));assert(c.ticket==saved);
    UPDATE_LifecycleAck(&c,c.ticket,true,now); /* duplicate ignored */
    Pump(&c);assert(erases==2&&c.session.status==UPDATE_SESSION_READY);Resume(&c);
    op=(UPDATE_STAGED_OPERATION){.epoch=7,.token=1,.id=2,.kind=UPDATE_STAGE_BATCH,.size=1};
    assert(!UPDATE_LifecycleStage(&c,&op,now,&ticket));assert(c.session.status==UPDATE_SESSION_READY);
    op.size=Rec(op.records,4,0,(uint8_t[]){0x1D,0},2);op.sequence=1;
    assert(!UPDATE_LifecycleStage(&c,&op,now,&ticket));op.sequence=0;Accepted(&c,&op);Pump(&c);Resume(&c);
    assert(c.next_sequence==1);assert(!UPDATE_LifecycleStage(&c,&(UPDATE_STAGED_OPERATION){.epoch=7,.token=1,.id=1,.kind=UPDATE_STAGE_ABORT},now,&ticket));
    /* ACK failed/late: no BEGIN is invoked and old image remains untouched. */
    Reset(&c);op=Begin(1,1,4,0);assert(UPDATE_LifecycleStage(&c,&op,0,&ticket));
    assert(UPDATE_LifecycleExecute(&c,7,1,1,ticket,0));UPDATE_LifecycleAck(&c,ticket,false,1);
    UPDATE_LifecycleAck(&c,ticket,true,2);assert(c.state==UPDATE_RECOVERY);Pump(&c);
    assert(erases==0&&writes==0&&!c.invoked&&c.outcome==UPDATE_OUTCOME_CANCELLED);
    assert(!UPDATE_LifecycleResume(&c,7,1,1,ticket,now));
    Reset(&c);c.phase_timeout_ms=10;now=UINT32_MAX-4;op=Begin(1,1,4,0);
    assert(UPDATE_LifecycleStage(&c,&op,now,&ticket));
    UPDATE_LifecyclePoll(&c,6,false);assert(c.state==UPDATE_RECOVERY&&erases==0);Pump(&c);
    /* Host timeouts after ACK cannot undo execution: lost result is replayable. */
    Reset(&c);CompleteImage(&c,false);unsigned w=writes,e=erases;
    UPDATE_LifecycleCancel(&c,now);assert(c.session.status==UPDATE_SESSION_COMPLETE&&c.state==UPDATE_LINK);
    UPDATE_LifecyclePoll(&c,now+20000,false);assert(w==writes&&e==erases);
    /* Cancel BEFORE complete classification, after a physical commit: do not
     * call it ABORTED until verified metadata invalidation. */
    Reset(&c);CompleteImage(&c,true);uint8_t retained[128];memcpy(retained,source,128);
    UPDATE_LifecycleCancel(&c,now);assert(!memcmp(retained,source,128));
    usb=true;UPDATE_LifecyclePoll(&c,now++,false);assert(c.state==UPDATE_RECOVERY&&busy);
    Finish();assert(memory[UPDATE_IMAGE_META_ADDRESS-START]!=255);
    assert(c.session.status!=UPDATE_SESSION_ABORTED);Pump(&c);
    assert(c.session.status==UPDATE_SESSION_ABORTED&&memory[UPDATE_IMAGE_META_ADDRESS-START]==255);
    /* Unexpected reenable while writing freezes polling/retains row until off. */
    Reset(&c);CompleteImage(&c,true);usb=true;w=writes;e=erases;
    UPDATE_LifecyclePoll(&c,now++,false);assert(c.state==UPDATE_RECOVERY&&w==writes&&e==erases);
    assert(busy);Finish();Pump(&c);assert(c.session.status==UPDATE_SESSION_ABORTED);
    /* Staged explicit ABORT on COMPLETE still needs ACK/off and erases marker. */
    Reset(&c);CompleteImage(&c,false);op=(UPDATE_STAGED_OPERATION){.epoch=7,.token=1,.id=3,.kind=UPDATE_STAGE_ABORT};
    Accepted(&c,&op);Pump(&c);assert(c.session.status==UPDATE_SESSION_ABORTED);Resume(&c);
    /* A new BEGIN cancelled before invocation preserves prior committed image. */
    Reset(&c);CompleteImage(&c,false);uint8_t marker[128];
    memcpy(marker,memory+UPDATE_IMAGE_META_ADDRESS-START,128);
    op=Begin(3,2,128,0);assert(UPDATE_LifecycleStage(&c,&op,now,&ticket));
    UPDATE_LifecycleCancel(&c,now);Pump(&c);
    assert(c.outcome==UPDATE_OUTCOME_CANCELLED&&c.session.status==UPDATE_SESSION_COMPLETE);
    assert(!memcmp(marker,memory+UPDATE_IMAGE_META_ADDRESS-START,128));
    /* Source timeout: no reuse or USB-on cleanup; wait hardware settlement. */
    Reset(&c);op=Begin(1,1,128,0);Accepted(&c,&op);usb=false;
    UPDATE_LifecyclePoll(&c,now++,true);assert(busy);
    UPDATE_LifecyclePoll(&c,now+101,true);assert(c.state==UPDATE_RECOVERY&&busy);
    usb=true;UPDATE_LifecyclePoll(&c,now+102,false);assert(busy);Pump(&c);
    assert(c.session.status==UPDATE_SESSION_ABORTED&&c.outcome==UPDATE_OUTCOME_FAILED);
    /* Permanent backend failure remains recovery-only, no success or reenable. */
    Reset(&c);op=Begin(1,1,128,0);Accepted(&c,&op);reject=true;usb=false;
    UPDATE_LifecyclePoll(&c,now++,true);UPDATE_LifecyclePoll(&c,now++,true);
    assert(c.state==UPDATE_RECOVERY&&c.outcome==UPDATE_OUTCOME_FAILED);
    assert(!UPDATE_LifecycleResume(&c,7,1,1,c.ticket,now));
    /* Expired ACK/off/resume cannot start/continue a mutation. */
    Reset(&c);op=Begin(1,1,128,0);assert(UPDATE_LifecycleStage(&c,&op,now,&ticket));
    c.phase_timeout_ms=10;assert(UPDATE_LifecycleExecute(&c,7,1,1,ticket,now));
    UPDATE_LifecycleAck(&c,ticket,true,now+10);assert(c.state==UPDATE_RECOVERY&&erases==0);Pump(&c);
    Reset(&c);op=Begin(1,1,128,0);Accepted(&c,&op);c.phase_timeout_ms=10;
    now+=11;UPDATE_LifecyclePoll(&c,now,false);assert(c.state==UPDATE_RECOVERY&&erases==0);Pump(&c);
    Reset(&c);op=Begin(1,1,128,0);Accepted(&c,&op);Pump(&c);c.phase_timeout_ms=10;
    assert(!UPDATE_LifecycleResume(&c,7,1,1,c.ticket,now+10));assert(c.state==UPDATE_RECOVERY);Pump(&c);
    /* Resumed READY link also has bounded host inactivity; stage replay cannot renew it. */
    Reset(&c);op=Begin(1,1,128,0);Accepted(&c,&op);Pump(&c);Resume(&c);c.phase_timeout_ms=10;
    uint32_t since=c.since;assert(UPDATE_LifecycleStage(&c,&op,now+5,&ticket)&&c.since==since);
    op=(UPDATE_STAGED_OPERATION){.epoch=7,.token=1,.id=2,.kind=UPDATE_STAGE_BATCH};
    op.size=Rec(op.records,4,0,(uint8_t[]){0x1D,0},2);
    assert(!UPDATE_LifecycleStage(&c,&op,since+10,&ticket)&&c.state==UPDATE_RECOVERY);Pump(&c);
    /* Large multi-window image: immutable stage copy and bounded batches. */
    static uint8_t image[70000];for(size_t i=0;i<sizeof(image);i++)image[i]=(uint8_t)(i*7U+1U);
    Reset(&c);op=Begin(1,1,sizeof(image),UPDATE_Crc32Step(UINT32_MAX,image,sizeof(image))^UINT32_MAX);
    Accepted(&c,&op);Pump(&c);Resume(&c);uint32_t id=2,sequence=0;uint16_t upper=0;
    for(size_t off=0;off<sizeof(image);)
    {
        uint32_t a=START+(uint32_t)off;
        op=(UPDATE_STAGED_OPERATION){.epoch=7,.token=1,.id=id++,.kind=UPDATE_STAGE_BATCH,.sequence=sequence++};
        if(upper!=(uint16_t)(a>>16))
        {upper=(uint16_t)(a>>16);op.size=Rec(op.records,4,0,(uint8_t[]){(uint8_t)(upper>>8),(uint8_t)upper},2);}
        uint8_t count=(uint8_t)(sizeof(image)-off>128?128:sizeof(image)-off);
        op.size+=Rec(op.records+op.size,0,(uint16_t)a,image+off,count);
        uint8_t staged[UPDATE_LIFECYCLE_RECORD_MAX];memcpy(staged,op.records,op.size);
        assert(UPDATE_LifecycleStage(&c,&op,now,&ticket));memset(op.records,0xCC,sizeof(op.records));
        assert(!memcmp(c.staged.records,staged,c.staged.size));
        assert(UPDATE_LifecycleExecute(&c,7,1,c.last_id,c.ticket,now));
        UPDATE_LifecycleAck(&c,c.ticket,true,now);Pump(&c);Resume(&c);off+=count;
    }
    op=(UPDATE_STAGED_OPERATION){.epoch=7,.token=1,.id=id,.kind=UPDATE_STAGE_BATCH,.sequence=sequence};
    op.size=Rec(op.records,1,0,NULL,0);Accepted(&c,&op);Pump(&c);Resume(&c);
    assert(c.session.status==UPDATE_SESSION_COMPLETE&&!memcmp(memory,image,sizeof(image)));
    UPDATE_IMAGE_CHECK check;usb=false;
    assert(UPDATE_ImageCheckStored(&check,&backend)==UPDATE_IMAGE_CHECKING);
    while(UPDATE_ImageCheckPoll(&check)==UPDATE_IMAGE_CHECKING){}
    assert(check.status==UPDATE_IMAGE_VALID);usb=true;
    UPDATE_LIFECYCLE disabled;assert(UPDATE_LifecycleInit(&disabled,NULL,9,100,100,false));
    op.epoch=9;assert(!UPDATE_LifecycleStage(&disabled,&op,0,&ticket));
    assert(!UPDATE_LifecycleInit(&disabled,NULL,0,100,100,false));
    puts("PASS: stage validation/copies, ticket/epoch/replay, ACK/off gating, result/resume, lease wrap, timeout/quarantine, USB reenable cancellation, commit-cancel invalidation; inactive typed lifecycle only.");
    return 0;
}
