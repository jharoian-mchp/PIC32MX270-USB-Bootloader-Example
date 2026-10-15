#include "update_lifecycle_wire.h"
#include <string.h>
static uint32_t Get32(const uint8_t *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint64_t Get64(const uint8_t *p){return Get32(p)|((uint64_t)Get32(p+4)<<32);}
static void Put32(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8U*i));}
static void Put64(uint8_t *p,uint64_t v){Put32(p,(uint32_t)v);Put32(p+4,(uint32_t)(v>>32));}
bool UPDATE_LinkDecode(const uint8_t *p,size_t n,UPDATE_LINK_REQUEST *out)
{
    if(p==NULL||out==NULL||n<UPDATE_LINK_HEADER_SIZE||n>UPDATE_PAYLOAD_MAX||
       p[1]!=UPDATE_LINK_VERSION||p[2]!=0U||p[3]!=0U||Get32(p+4)==0U)return false;
    UPDATE_LINK_REQUEST r={.opcode=p[0],.rpc_id=Get32(p+4),.epoch=Get64(p+8),
                           .token=Get32(p+16),.operation_id=Get32(p+20)};
    if(r.opcode==UPDATE_LINK_HELLO)
    {if(n!=24U||r.epoch!=0U||r.token!=0U||r.operation_id!=0U)return false;}
    else if(r.opcode==UPDATE_LINK_STAGE)
    {
        if(n<28U||r.epoch==0U||r.token==0U||r.operation_id==0U||p[25]!=0U||p[26]!=0U||p[27]!=0U)return false;
        r.operation=(UPDATE_STAGED_OPERATION){.epoch=r.epoch,.token=r.token,.id=r.operation_id,
                                             .kind=(UPDATE_STAGE_KIND)p[24]};
        if(r.operation.kind==UPDATE_STAGE_BEGIN)
        {
            if(n!=36U||!UPDATE_ImageParameters(Get32(p+28)))return false;
            r.operation.length=Get32(p+28);r.operation.crc=Get32(p+32);
        }
        else if(r.operation.kind==UPDATE_STAGE_BATCH)
        {
            if(n<=32U)return false;
            r.operation.sequence=Get32(p+28);r.operation.size=n-32U;
            if(r.operation.size>UPDATE_LINK_STAGE_RECORD_MAX)return false;
            /* Syntax/checksum before session-specific address/order preflight. */
            size_t off=32U;
            while(off<n)
            {
                size_t count=(size_t)p[off]+5U;
                if(count>n-off)return false;
                uint8_t sum=0;for(size_t i=0;i<count;i++)sum=(uint8_t)(sum+p[off+i]);
                if(sum!=0U)return false;
                off+=count;
            }
            memcpy(r.operation.records,p+32,r.operation.size);
        }
        else if(r.operation.kind==UPDATE_STAGE_ABORT)
        {if(n!=28U)return false;}
        else return false;
    }
    else if(r.opcode==UPDATE_LINK_EXECUTE||r.opcode==UPDATE_LINK_QUERY||r.opcode==UPDATE_LINK_RESUME)
    {
        if(n!=28U||r.epoch==0U||r.token==0U||r.operation_id==0U||Get32(p+24)==0U)return false;
        r.ticket=Get32(p+24);
    }
    else return false;
    *out=r;return true;
}
bool UPDATE_LinkEncode(const UPDATE_LINK_REQUEST *r,uint8_t result,const UPDATE_LINK_STATUS *s,
                       uint8_t *out,size_t cap,size_t *length)
{
    if(length!=NULL)*length=0;
    if(r==NULL||s==NULL||out==NULL||length==NULL||cap<UPDATE_LINK_REPLY_SIZE||
       r->rpc_id==0U||s->epoch==0U||r->opcode<UPDATE_LINK_HELLO||r->opcode>UPDATE_LINK_RESUME||
       result>5U||s->lifecycle>UPDATE_RECOVERY||s->outcome>UPDATE_OUTCOME_FAILED||
       s->session>UPDATE_SESSION_ABORTED||s->flash>UPDATE_FLASH_TIMEOUT||
       s->hex>UPDATE_HEX_NOT_ACTIVE||(s->features&0xF0U)!=0U)return false;
    uint8_t p[UPDATE_LINK_REPLY_SIZE]={0};p[0]=(uint8_t)(r->opcode|0x80U);p[1]=UPDATE_LINK_VERSION;p[2]=result;
    Put32(p+4,r->rpc_id);memcpy(p+8,"PKL2",4);p[12]=UPDATE_LINK_VERSION;p[13]=s->features;
    p[14]=(uint8_t)UPDATE_PAYLOAD_MAX;p[15]=(uint8_t)(UPDATE_PAYLOAD_MAX>>8);
    Put64(p+16,s->epoch);Put32(p+24,s->token);Put32(p+28,s->operation_id);Put32(p+32,s->ticket);
    Put32(p+36,s->next_sequence);Put32(p+40,s->length);Put32(p+44,s->crc);
    p[48]=s->lifecycle;p[49]=s->outcome;p[50]=s->session;p[51]=s->flash;p[52]=s->hex;
    memcpy(out,p,sizeof(p));*length=sizeof(p);return true;
}
