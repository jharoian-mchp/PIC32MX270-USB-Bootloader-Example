#include "update_dispatch.h"
#include <string.h>

static uint32_t Get32(const uint8_t *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void Put32(uint8_t *p,uint32_t value)
{for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(value>>(8U*i));}
static void State(const UPDATE_DISPATCH *d,uint8_t *p)
{
    Put32(p,d->token);Put32(p+4,d->last_id);Put32(p+8,d->next_sequence);
    Put32(p+12,d->session.length);
    p[16]=(uint8_t)d->session.status;p[17]=(uint8_t)d->session.flash.status;
    p[18]=(uint8_t)d->session.hex.status;p[19]=(uint8_t)d->enabled;
}
static void Remember(UPDATE_DISPATCH *d,const uint8_t *p,size_t n,UPDATE_REPLY result)
{
    memcpy(d->last_request,p,n);d->last_size=n;d->last_id=Get32(p+2);
    d->last_result=result;
}
bool UPDATE_DispatchInit(UPDATE_DISPATCH *d,const UPDATE_FLASH_BACKEND *b,
                         uint32_t timeout,bool enabled)
{
    if(d==NULL)return false;
    *d=(UPDATE_DISPATCH){0};
    if(enabled&&!UPDATE_SessionInit(&d->session,b,timeout))return false;
    d->enabled=enabled;d->initialized=true;return true;
}
UPDATE_SESSION_STATUS UPDATE_DispatchPoll(UPDATE_DISPATCH *d,uint32_t now)
{
    if(d==NULL||!d->initialized)return UPDATE_SESSION_FAILED;
    return d->enabled?UPDATE_SessionPoll(&d->session,now):UPDATE_SESSION_IDLE;
}
void UPDATE_DispatchDisconnect(UPDATE_DISPATCH *d)
{
    if(d==NULL||!d->initialized||!d->enabled)return;
    if(d->session.status!=UPDATE_SESSION_IDLE&&d->session.status!=UPDATE_SESSION_COMPLETE&&
       d->session.status!=UPDATE_SESSION_ABORTED)
        UPDATE_SessionAbort(&d->session);
}
bool UPDATE_DispatchRequest(UPDATE_DISPATCH *d,const uint8_t *p,size_t n,uint32_t now,
                            uint8_t *out,size_t cap,size_t *length)
{
    if(length!=NULL)*length=0;
    if(d==NULL||!d->initialized||p==NULL||out==NULL||length==NULL||
       n<6U||n>UPDATE_PAYLOAD_MAX||p[1]!=UPDATE_WIRE_VERSION||Get32(p+2)==0U)
        return false;
    uint8_t op=p[0];uint32_t id=Get32(p+2);
    if(op==UPDATE_WIRE_CAPABILITIES)
    {
        UPDATE_WIRE_QUERY q;
        return UPDATE_WireQuery(p,n,&q)&&UPDATE_WireCapabilities(&q,out,cap,length);
    }
    if(op!=UPDATE_WIRE_STATUS&&op!=UPDATE_WIRE_BEGIN&&op!=UPDATE_WIRE_BATCH&&
       op!=UPDATE_WIRE_ABORT)return false;
    if(cap<UPDATE_WIRE_REPLY_SIZE)return false;
    UPDATE_REPLY result=UPDATE_REPLY_OK;
    bool shape=(op==UPDATE_WIRE_STATUS&&n==6U)||(op==UPDATE_WIRE_BEGIN&&n==18U)||
               (op==UPDATE_WIRE_BATCH&&n>14U)||(op==UPDATE_WIRE_ABORT&&n==10U);
    if(!shape)result=UPDATE_REPLY_BAD_REQUEST;
    else if(op==UPDATE_WIRE_STATUS){} /* Queries never evict the mutation cache. */
    else if(!d->enabled)result=UPDATE_REPLY_DISABLED;
    else if(id==d->last_id)
    {
        result=(n==d->last_size&&!memcmp(p,d->last_request,n))?
               d->last_result:UPDATE_REPLY_CONFLICT;
    }
    else if(id<d->last_id)result=UPDATE_REPLY_STALE;
    else if(Get32(p+6)==0U)result=UPDATE_REPLY_TOKEN;
    else if(op==UPDATE_WIRE_BEGIN)
    {
        UPDATE_SESSION_STATUS s=d->session.status;
        if(!UPDATE_ImageParameters(Get32(p+10)))result=UPDATE_REPLY_BAD_REQUEST;
        else if(s!=UPDATE_SESSION_IDLE&&s!=UPDATE_SESSION_COMPLETE&&s!=UPDATE_SESSION_ABORTED)
            result=UPDATE_REPLY_BUSY;
        else if(Get32(p+6)==d->token)result=UPDATE_REPLY_TOKEN;
        else
        {
            d->token=Get32(p+6);d->next_sequence=0;
            if(!UPDATE_SessionBegin(&d->session,Get32(p+10),Get32(p+14),now))
                result=UPDATE_REPLY_FAILED;
            Remember(d,p,n,result);
        }
    }
    else if(Get32(p+6)!=d->token||d->token==0U)result=UPDATE_REPLY_TOKEN;
    else if(op==UPDATE_WIRE_BATCH)
    {
        if(Get32(p+10)!=d->next_sequence||d->next_sequence==UINT32_MAX)
            result=UPDATE_REPLY_SEQUENCE;
        else if(d->session.status!=UPDATE_SESSION_READY)result=UPDATE_REPLY_BUSY;
        else
        {
            if(UPDATE_SessionSubmit(&d->session,p+14,n-14U))++d->next_sequence;
            else result=UPDATE_REPLY_FAILED;
            Remember(d,p,n,result);
        }
    }
    else
    {
        UPDATE_SessionAbort(&d->session);Remember(d,p,n,result);
    }
    out[0]=(uint8_t)(op|0x80U);out[1]=UPDATE_WIRE_VERSION;
    out[2]=(uint8_t)result;out[3]=0;Put32(out+4,id);State(d,out+8);
    *length=UPDATE_WIRE_REPLY_SIZE;return true;
}
