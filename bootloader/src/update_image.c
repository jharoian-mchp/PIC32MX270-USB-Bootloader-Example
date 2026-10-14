#include "update_image.h"
#include <string.h>
#define IMAGE_MAGIC UINT32_C(0x314D4932)
#define IMAGE_VERSION 1U

static uint32_t LE32(const uint8_t *p)
{ return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }
static void Put32(uint8_t *p,uint32_t n)
{ for(unsigned i=0;i<4U;++i)p[i]=(uint8_t)(n>>(8U*i)); }

uint32_t UPDATE_Crc32Step(uint32_t state,const uint8_t *data,size_t size)
{
    for(size_t i=0;i<size;++i)
    {
        state^=data[i];
        for(unsigned b=0;b<8U;++b)state=(state>>1)^((state&1U)?UINT32_C(0xEDB88320):0U);
    }
    return state;
}
bool UPDATE_ImageParameters(uint32_t length)
{ return length>=4U && length<=UPDATE_IMAGE_MAX_LENGTH; }

void UPDATE_ImageRecord(uint8_t row[UPDATE_ROW_SIZE],uint32_t length,uint32_t crc)
{
    memset(row,0xFF,UPDATE_ROW_SIZE);
    Put32(row,IMAGE_MAGIC);Put32(row+4,IMAGE_VERSION);
    Put32(row+8,UPDATE_APP_PHYSICAL_START);Put32(row+12,length);Put32(row+16,crc);
    Put32(row+20,~length);Put32(row+24,~crc);
    Put32(row+28,UPDATE_Crc32Step(UINT32_MAX,row,28)^UINT32_MAX);
}

bool UPDATE_ImageCheckExpected(UPDATE_IMAGE_CHECK *c,const UPDATE_FLASH_BACKEND *b,
                               uint32_t length,uint32_t crc)
{
    if(c==NULL||b==NULL||b->read==NULL||b->busy==NULL||!UPDATE_ImageParameters(length))return false;
    *c=(UPDATE_IMAGE_CHECK){.backend=*b,.length=length,.expected_crc=crc,
                           .crc=UINT32_MAX,.status=UPDATE_IMAGE_CHECKING};
    return true;
}

UPDATE_IMAGE_STATUS UPDATE_ImageCheckStored(UPDATE_IMAGE_CHECK *c,const UPDATE_FLASH_BACKEND *b)
{
    if(c==NULL)return UPDATE_IMAGE_INVALID;
    *c=(UPDATE_IMAGE_CHECK){.status=UPDATE_IMAGE_INVALID};
    if(b==NULL||b->read==NULL||b->busy==NULL)return c->status;
    if(b->busy(b->context))return c->status; /* Boot caller must establish NVM idle first. */
    uint8_t row[UPDATE_ROW_SIZE];
    if(!b->read(b->context,UPDATE_IMAGE_META_ADDRESS,row,sizeof(row)))
        return c->status=UPDATE_IMAGE_READ_ERROR;
    uint32_t length=LE32(row+12),crc=LE32(row+16);
    if(LE32(row)!=IMAGE_MAGIC||LE32(row+4)!=IMAGE_VERSION||LE32(row+8)!=UPDATE_APP_PHYSICAL_START||
       !UPDATE_ImageParameters(length)||LE32(row+20)!=~length||LE32(row+24)!=~crc||
       LE32(row+28)!=(UPDATE_Crc32Step(UINT32_MAX,row,28)^UINT32_MAX))return c->status;
    /* Canonical erased tail; no other records share the commit row. */
    for(size_t i=32;i<sizeof(row);++i)if(row[i]!=0xFFU)return c->status;
    (void)UPDATE_ImageCheckExpected(c,b,length,crc);
    return c->status;
}

UPDATE_IMAGE_STATUS UPDATE_ImageCheckPoll(UPDATE_IMAGE_CHECK *c)
{
    if(c==NULL)return UPDATE_IMAGE_INVALID;
    if(c->status!=UPDATE_IMAGE_CHECKING)return c->status;
    if(c->backend.busy(c->backend.context))return c->status;
    uint8_t data[UPDATE_ROW_SIZE];
    uint32_t size=c->length-c->offset;
    if(size>sizeof(data))size=sizeof(data);
    if(!c->backend.read(c->backend.context,UPDATE_APP_PHYSICAL_START+c->offset,data,size))
        return c->status=UPDATE_IMAGE_READ_ERROR;
    if(c->offset==0U && LE32(data)==UINT32_MAX)return c->status=UPDATE_IMAGE_INVALID;
    c->crc=UPDATE_Crc32Step(c->crc,data,size);c->offset+=size;
    if(c->offset==c->length)c->status=(c->crc^UINT32_MAX)==c->expected_crc?
        UPDATE_IMAGE_VALID:UPDATE_IMAGE_INVALID;
    return c->status;
}
