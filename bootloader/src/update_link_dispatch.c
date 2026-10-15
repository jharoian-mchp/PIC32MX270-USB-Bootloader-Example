#include "update_link_dispatch.h"
static bool Overlap(const void *a,size_t an,const void *b,size_t bn)
{
    uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
    return x<=y?y-x<an:x-y<bn;
}
static bool Identity(const UPDATE_LIFECYCLE *c,const UPDATE_LINK_REQUEST *r)
{return c->have_stage&&r->epoch==c->epoch&&r->token==c->token&&
        r->operation_id==c->last_id&&r->ticket==c->ticket;}
static UPDATE_LINK_STATUS Status(const UPDATE_LINK_DISPATCH *d)
{
    const UPDATE_LIFECYCLE *c=&d->core;
    return (UPDATE_LINK_STATUS){.epoch=c->epoch,.token=c->token,.operation_id=c->last_id,
        .ticket=c->ticket,.next_sequence=c->next_sequence,.length=c->session.length,
        .crc=c->session.expected_crc,.lifecycle=(uint8_t)c->state,.outcome=(uint8_t)c->outcome,
        .session=(uint8_t)c->session.status,.flash=(uint8_t)c->session.flash.status,
        .hex=(uint8_t)c->session.hex.status,.features=UPDATE_LINK_FEATURE_SCHEMA|
        UPDATE_LINK_FEATURE_TYPED_CORE|(c->enabled?UPDATE_LINK_FEATURE_WRITES:0U)};
}
bool UPDATE_LinkDispatchInit(UPDATE_LINK_DISPATCH *d,const UPDATE_FLASH_BACKEND *backend,
                             uint64_t epoch,uint32_t flash_timeout,uint32_t phase_timeout,bool enabled)
{
    if(d==NULL)return false;
    *d=(UPDATE_LINK_DISPATCH){0};
    if(!UPDATE_LifecycleInit(&d->core,backend,epoch,flash_timeout,phase_timeout,enabled))return false;
    d->initialized=true;return true;
}
bool UPDATE_LinkDispatchRequest(UPDATE_LINK_DISPATCH *d,const uint8_t *p,size_t n,uint32_t now,
                                uint8_t *reply,size_t capacity,size_t *length,uint32_t *cookie)
{
    if(d==NULL||p==NULL||reply==NULL||length==NULL||cookie==NULL||
       n<UPDATE_LINK_HEADER_SIZE||n>UPDATE_PAYLOAD_MAX||capacity<UPDATE_LINK_REPLY_SIZE)return false;
    /* Treat the entire caller-declared output capacity as owned storage. */
    if(Overlap(d,sizeof(*d),p,n)||Overlap(d,sizeof(*d),reply,capacity)||
       Overlap(d,sizeof(*d),length,sizeof(*length))||Overlap(d,sizeof(*d),cookie,sizeof(*cookie))||
       Overlap(p,n,reply,capacity)||Overlap(p,n,length,sizeof(*length))||Overlap(p,n,cookie,sizeof(*cookie))||
       Overlap(reply,capacity,length,sizeof(*length))||Overlap(reply,capacity,cookie,sizeof(*cookie))||
       Overlap(length,sizeof(*length),cookie,sizeof(*cookie)))return false;
    *length=0;*cookie=0;
    if(!d->initialized||d->pause_pending)return false;
    UPDATE_LINK_REQUEST r;
    if(!UPDATE_LinkDecode(p,n,&r))return false;
    /* Validate encoder capacity/status before any lifecycle side effects. */
    UPDATE_LINK_STATUS status=Status(d);uint8_t check[UPDATE_LINK_REPLY_SIZE];size_t checked;
    if(!UPDATE_LinkEncode(&r,UPDATE_LINK_OK,&status,check,sizeof(check),&checked))return false;
    UPDATE_LIFECYCLE *c=&d->core;UPDATE_LINK_RESULT result=UPDATE_LINK_OK;
    if(r.opcode==UPDATE_LINK_HELLO){}
    else if(r.epoch!=c->epoch)result=UPDATE_LINK_IDENTITY;
    else if(r.opcode==UPDATE_LINK_QUERY)
    {if(!Identity(c,&r))result=UPDATE_LINK_IDENTITY;}
    else if(!c->enabled)result=UPDATE_LINK_DISABLED;
    else if(r.opcode==UPDATE_LINK_STAGE)
    {
        uint32_t ticket;
        if(!UPDATE_LifecycleStage(c,&r.operation,now,&ticket))
        {
            if(r.operation_id<c->last_id||(r.operation_id==c->last_id&&c->have_stage)||
               (r.operation.kind!=UPDATE_STAGE_BEGIN&&r.token!=c->token))result=UPDATE_LINK_IDENTITY;
            else if(c->state!=UPDATE_LINK)result=UPDATE_LINK_BUSY;
            else result=UPDATE_LINK_BAD_REQUEST;
        }
    }
    else if(!Identity(c,&r))result=UPDATE_LINK_IDENTITY;
    else if(r.opcode==UPDATE_LINK_EXECUTE)
    {
        if(c->state==UPDATE_STAGED&&d->send_generation==UINT32_MAX)result=UPDATE_LINK_FAILED;
        else if(!UPDATE_LifecycleExecute(c,r.epoch,r.token,r.operation_id,r.ticket,now))result=UPDATE_LINK_BUSY;
        else if(c->state==UPDATE_ACK_WAIT)
        {
            ++d->send_generation;d->pending_cookie=d->send_generation;d->pending_ticket=c->ticket;
            d->pause_pending=true;*cookie=d->pending_cookie;
        }
    }
    else if(r.opcode==UPDATE_LINK_RESUME)
    {
        if(!UPDATE_LifecycleResume(c,r.epoch,r.token,r.operation_id,r.ticket,now))
            result=c->outcome==UPDATE_OUTCOME_FAILED||c->outcome==UPDATE_OUTCOME_CANCELLED?
                   UPDATE_LINK_FAILED:UPDATE_LINK_BUSY;
    }
    status=Status(d);
    return UPDATE_LinkEncode(&r,(uint8_t)result,&status,reply,capacity,length);
}
void UPDATE_LinkDispatchSent(UPDATE_LINK_DISPATCH *d,uint32_t cookie,bool success,uint32_t now)
{
    if(d==NULL||!d->initialized||!d->pause_pending||cookie==0U||cookie!=d->pending_cookie)return;
    uint32_t ticket=d->pending_ticket;d->pause_pending=false;d->pending_cookie=d->pending_ticket=0;
    UPDATE_LifecycleAck(&d->core,ticket,success,now);
}
UPDATE_LIFECYCLE_STATE UPDATE_LinkDispatchPoll(UPDATE_LINK_DISPATCH *d,uint32_t now,bool off)
{
    if(d==NULL||!d->initialized)return UPDATE_RECOVERY;
    return UPDATE_LifecyclePoll(&d->core,now,off);
}
void UPDATE_LinkDispatchCancel(UPDATE_LINK_DISPATCH *d,uint32_t now)
{
    if(d==NULL||!d->initialized)return;
    /* Late transport completion cannot authorise cancelled work; actual USB
     * buffers still belong to transport until completion/cancellation. */
    d->pause_pending=false;d->pending_cookie=d->pending_ticket=0;
    UPDATE_LifecycleCancel(&d->core,now);
}
