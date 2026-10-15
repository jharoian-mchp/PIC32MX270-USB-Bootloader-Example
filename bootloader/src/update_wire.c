#include "update_wire.h"
#include <string.h>
static uint32_t LE32(const uint8_t *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void Put32(uint8_t *p,uint32_t n)
{for(unsigned i=0;i<4U;i++)p[i]=(uint8_t)(n>>(8U*i));}
static void Put16(uint8_t *p,uint16_t n)
{p[0]=(uint8_t)n;p[1]=(uint8_t)(n>>8);}
bool UPDATE_WireQuery(const uint8_t *p,size_t n,UPDATE_WIRE_QUERY *q)
{
    if(q!=NULL)*q=(UPDATE_WIRE_QUERY){0};
    if(p==NULL||q==NULL||n!=UPDATE_WIRE_REQUEST_SIZE||p[1]!=UPDATE_WIRE_VERSION||
       (p[0]!=UPDATE_WIRE_CAPABILITIES&&p[0]!=UPDATE_WIRE_STATUS)||LE32(p+2)==0U)return false;
    *q=(UPDATE_WIRE_QUERY){.opcode=p[0],.request_id=LE32(p+2)};return true;
}
bool UPDATE_WireCapabilities(const UPDATE_WIRE_QUERY *q,uint8_t *out,size_t capacity,size_t *written)
{
    if(written!=NULL)*written=0;
    if(q==NULL||out==NULL||written==NULL||q->opcode!=UPDATE_WIRE_CAPABILITIES||
       q->request_id==0U||capacity<UPDATE_WIRE_RESPONSE_SIZE+UPDATE_WIRE_CAP_BODY_SIZE)return false;
    out[0]=q->opcode|0x80U;out[1]=UPDATE_WIRE_VERSION;out[2]=out[3]=0;
    Put32(out+4,q->request_id);memcpy(out+8,"PKBL",4);
    Put16(out+12,UPDATE_WIRE_VERSION);Put16(out+14,UPDATE_PAYLOAD_MAX);
    Put32(out+16,UPDATE_APP_PHYSICAL_START);Put32(out+20,UPDATE_IMAGE_META_ADDRESS);
    Put16(out+24,UPDATE_ROW_SIZE);Put16(out+26,UPDATE_ERASE_SIZE);
    *written=28U;return true;
}
