#include "update_lifecycle.h"
#include <string.h>
static bool Identity(const UPDATE_LIFECYCLE *c,uint64_t epoch,uint32_t token,uint32_t id,uint32_t ticket)
{return c!=NULL&&c->initialized&&c->have_stage&&epoch==c->epoch&&token==c->token&&
        id==c->last_id&&ticket!=0U&&ticket==c->ticket;}
static bool Same(const UPDATE_STAGED_OPERATION *a,const UPDATE_STAGED_OPERATION *b)
{
    return a->epoch==b->epoch&&a->token==b->token&&a->id==b->id&&a->sequence==b->sequence&&
           a->length==b->length&&a->crc==b->crc&&a->kind==b->kind&&a->size==b->size&&
           !memcmp(a->records,b->records,a->size);
}
static bool Expired(const UPDATE_LIFECYCLE *c,uint32_t now)
{return (uint32_t)(now-c->since)>=c->phase_timeout_ms;}
static bool Overlaps(const void *a,size_t an,const void *b,size_t bn)
{
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    return x<=y?y-x<an:x-y<bn;
}
bool UPDATE_LifecycleInit(UPDATE_LIFECYCLE *c,const UPDATE_FLASH_BACKEND *backend,
                         uint64_t epoch,uint32_t flash_timeout,uint32_t phase_timeout,bool enabled)
{
    if(c==NULL||epoch==0U||phase_timeout==0U||phase_timeout>0x7FFFFFFFU)return false;
    *c=(UPDATE_LIFECYCLE){0};
    if(enabled&&!UPDATE_SessionInit(&c->session,backend,flash_timeout))return false;
    c->epoch=epoch;c->enabled=enabled;c->phase_timeout_ms=phase_timeout;c->initialized=true;return true;
}
bool UPDATE_LifecycleStage(UPDATE_LIFECYCLE *c,const UPDATE_STAGED_OPERATION *op,
                          uint32_t now,uint32_t *ticket)
{
    /* Reject output aliases BEFORE writing even a failure value. Otherwise a
     * caller can reset our counter and let a stale ticket authorise a new ACK. */
    if(ticket!=NULL&&((c!=NULL&&Overlaps(ticket,sizeof(*ticket),c,sizeof(*c)))||
       (op!=NULL&&Overlaps(ticket,sizeof(*ticket),op,sizeof(*op)))))return false;
    if(ticket!=NULL)*ticket=0;
    if(c==NULL||!c->initialized||!c->enabled||op==NULL||ticket==NULL||
       op->epoch!=c->epoch||op->token==0U||op->id==0U||op->size>UPDATE_LIFECYCLE_RECORD_MAX)return false;
    if(c->have_stage&&op->id==c->last_id)
    {
        if(!Same(op,&c->staged))return false;
        *ticket=c->ticket;return true;
    }
    if(c->state!=UPDATE_LINK||op->id<=c->last_id||c->ticket==UINT32_MAX)return false;
    UPDATE_SESSION_STATUS s=c->session.status;
    if(s==UPDATE_SESSION_READY&&Expired(c,now))
    {UPDATE_LifecycleCancel(c,now);return false;}
    if(op->kind==UPDATE_STAGE_BEGIN)
    {
        if(op->size!=0U||op->sequence!=0U||!UPDATE_ImageParameters(op->length)||op->token==c->token||
           (s!=UPDATE_SESSION_IDLE&&s!=UPDATE_SESSION_COMPLETE&&s!=UPDATE_SESSION_ABORTED))return false;
    }
    else
    {
        if(op->token!=c->token||op->length!=0U||op->crc!=0U)return false;
        if(op->kind==UPDATE_STAGE_BATCH)
        {
            if(s!=UPDATE_SESSION_READY||op->size==0U||op->sequence!=c->next_sequence||
               c->next_sequence==UINT32_MAX||
               UPDATE_HexCheck(&c->session.hex,op->records,op->size)!=UPDATE_HEX_ACCEPTED)return false;
        }
        else if(op->kind==UPDATE_STAGE_ABORT)
        {if(op->size!=0U||op->sequence!=0U||s==UPDATE_SESSION_IDLE)return false;}
        else return false;
    }
    c->staged=*op;c->token=op->token;c->last_id=op->id;++c->ticket;
    c->have_stage=true;c->execute_accepted=c->invoked=c->cleanup_started=false;
    c->outcome=UPDATE_OUTCOME_NONE;c->state=UPDATE_STAGED;c->since=now;
    *ticket=c->ticket;return true;
}
bool UPDATE_LifecycleExecute(UPDATE_LIFECYCLE *c,uint64_t epoch,uint32_t token,
                            uint32_t id,uint32_t ticket,uint32_t now)
{
    if(!Identity(c,epoch,token,id,ticket))return false;
    if(c->execute_accepted)return true; /* State query/replay, never another ACK lease. */
    if(c->state!=UPDATE_STAGED||Expired(c,now))return false;
    c->execute_accepted=true;c->state=UPDATE_ACK_WAIT;c->since=now;return true;
}
void UPDATE_LifecycleCancel(UPDATE_LIFECYCLE *c,uint32_t now)
{
    if(c==NULL||!c->initialized||!c->enabled||c->state==UPDATE_RECOVERY)return;
    if(c->session.status==UPDATE_SESSION_COMPLETE&&c->outcome==UPDATE_OUTCOME_DONE)return;
    c->outcome=UPDATE_OUTCOME_CANCELLED;c->state=UPDATE_RECOVERY;c->since=now;
    /* No flash effects. Main first needs confirmed bus exclusion for cleanup. */
}
void UPDATE_LifecycleAck(UPDATE_LIFECYCLE *c,uint32_t ticket,bool success,uint32_t now)
{
    if(c==NULL||!c->initialized||c->state!=UPDATE_ACK_WAIT||ticket!=c->ticket)return;
    if(!success||Expired(c,now)){UPDATE_LifecycleCancel(c,now);return;}
    c->state=UPDATE_OFF_WAIT;c->since=now;
}
static void Result(UPDATE_LIFECYCLE *c,UPDATE_LIFECYCLE_OUTCOME outcome,uint32_t now)
{c->outcome=outcome;c->state=UPDATE_RESULT;c->since=now;}
UPDATE_LIFECYCLE_STATE UPDATE_LifecyclePoll(UPDATE_LIFECYCLE *c,uint32_t now,bool off)
{
    if(c==NULL||!c->initialized)return UPDATE_RECOVERY;
    if(!c->enabled)return c->state;
    if(((c->state!=UPDATE_LINK&&c->state!=UPDATE_RECOVERY)||
        (c->state==UPDATE_LINK&&c->session.status==UPDATE_SESSION_READY))&&Expired(c,now))
        UPDATE_LifecycleCancel(c,now);
    if(c->state==UPDATE_RECOVERY)
    {
        if(!off)return c->state;
        UPDATE_SESSION_STATUS s=c->session.status;
        if(!c->cleanup_started)
        {
            /* Cancellation before a new BEGIN preserves prior completed/idle
             * image; a session already READY/active still requires cleanup. */
            if(s==UPDATE_SESSION_IDLE||s==UPDATE_SESSION_ABORTED||
               (s==UPDATE_SESSION_COMPLETE&&!c->invoked))
            {Result(c,c->outcome,now);return c->state;}
            UPDATE_SessionAbort(&c->session);c->cleanup_started=true;
        }
        s=UPDATE_SessionPoll(&c->session,now);
        if(s==UPDATE_SESSION_ABORTED)Result(c,c->outcome,now);
        else if(s==UPDATE_SESSION_FAILED)c->outcome=UPDATE_OUTCOME_FAILED;
        return c->state;
    }
    if(c->state==UPDATE_OFF_WAIT&&off)
    {
        bool ok=true;c->invoked=true;
        switch(c->staged.kind)
        {
            case UPDATE_STAGE_BEGIN:
                ok=UPDATE_SessionBegin(&c->session,c->staged.length,c->staged.crc,now);
                if(ok)c->next_sequence=0;
                break;
            case UPDATE_STAGE_BATCH:
                ok=UPDATE_SessionSubmit(&c->session,c->staged.records,c->staged.size);
                if(ok)++c->next_sequence;
                break;
            case UPDATE_STAGE_ABORT: UPDATE_SessionAbort(&c->session);break;
            default: ok=false;break;
        }
        c->state=UPDATE_OFF_RUNNING;c->since=now;
        if(!ok)
        {UPDATE_LifecycleCancel(c,now);c->outcome=UPDATE_OUTCOME_FAILED;return c->state;}
    }
    if(c->state==UPDATE_OFF_RUNNING)
    {
        if(!off){UPDATE_LifecycleCancel(c,now);return c->state;}
        UPDATE_SESSION_STATUS s=UPDATE_SessionPoll(&c->session,now);
        if(s==UPDATE_SESSION_READY||s==UPDATE_SESSION_COMPLETE||s==UPDATE_SESSION_ABORTED)
            Result(c,UPDATE_OUTCOME_DONE,now);
        else if(s==UPDATE_SESSION_FAILED)
        {UPDATE_LifecycleCancel(c,now);c->outcome=UPDATE_OUTCOME_FAILED;}
    }
    return c->state;
}
bool UPDATE_LifecycleResume(UPDATE_LIFECYCLE *c,uint64_t epoch,uint32_t token,
                           uint32_t id,uint32_t ticket,uint32_t now)
{
    if(!Identity(c,epoch,token,id,ticket))return false;
    if(c->state==UPDATE_LINK&&c->outcome==UPDATE_OUTCOME_DONE)return true;
    if(c->state!=UPDATE_RESULT||c->outcome!=UPDATE_OUTCOME_DONE)return false;
    if(Expired(c,now)&&c->session.status!=UPDATE_SESSION_COMPLETE)
    {UPDATE_LifecycleCancel(c,now);return false;}
    c->state=UPDATE_LINK;c->since=now;return true;
}
