#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "update_protocol.h"

#define ENCODED_MAX (2U * UPDATE_FRAME_CAPACITY + 2U)
static const uint16_t crc_table[16] = {
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
    0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF
};
static uint16_t ReferenceCRC(const uint8_t *p, size_t n)
{
    uint16_t c=0;
    while(n--) {
        unsigned i=(c>>12)^(*p>>4);c=(uint16_t)(crc_table[i&15]^(c<<4));
        i=(c>>12)^(*p++);c=(uint16_t)(crc_table[i&15]^(c<<4));
    }
    return c;
}
static uint32_t rng=0xABCD1234U;
static uint32_t Random(void) { rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng; }
static size_t Record(uint8_t *dst,uint8_t type,uint16_t address,const uint8_t *data,uint8_t n)
{
    dst[0]=n;dst[1]=(uint8_t)(address>>8);dst[2]=(uint8_t)address;dst[3]=type;
    if(n)memcpy(dst+4,data,n);
    uint8_t sum=0;
    for(size_t i=0;i<(size_t)n+4U;i++)sum=(uint8_t)(sum+dst[i]);
    dst[n+4U]=(uint8_t)(0U-sum);return (size_t)n+5U;
}
static void LE32(uint8_t *p,uint32_t n)
{ for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(n>>(i*8)); }

static void FrameTests(void)
{
    UPDATE_FRAME f;
    uint8_t wire[ENCODED_MAX],payload[UPDATE_PAYLOAD_MAX];
    size_t n,used;
    assert(UPDATE_Crc16((const uint8_t*)"123456789",9)==0x31C3U);
    for(size_t i=0;i<sizeof(payload);i++)payload[i]=(uint8_t)i;
    assert(UPDATE_Crc16(payload,sizeof(payload))==ReferenceCRC(payload,sizeof(payload)));
    assert(UPDATE_Crc16(NULL,0)==0);
    for(size_t size=1;size<=UPDATE_PAYLOAD_MAX;size+=size<20?1:37)
    {
        assert(UPDATE_FrameEncode(payload,size,wire,sizeof(wire),&n));
        for(size_t split=0;split<=n;split++)
        {
            UPDATE_FrameReset(&f);
            UPDATE_FRAME_STATUS s=UPDATE_FrameFeed(&f,wire,split,&used);
            assert(used==split);
            if(split<n){assert(s!=UPDATE_FRAME_READY);s=UPDATE_FrameFeed(&f,wire+split,n-split,&used);assert(used==n-split);}
            assert(s==UPDATE_FRAME_READY&&f.length==size&&!memcmp(f.bytes,payload,size));
            used=99;assert(UPDATE_FrameFeed(&f,wire,n,&used)==UPDATE_FRAME_BUSY&&used==0);
            assert(UPDATE_FramePush(&f,1)==UPDATE_FRAME_BUSY);
        }
    }
    memset(payload,0x10,sizeof(payload)); /* Max decoded frame and escape expansion. */
    assert(UPDATE_FrameEncode(payload,sizeof(payload),wire,sizeof(wire),&n));
    UPDATE_FrameReset(&f);
    assert(UPDATE_FrameFeed(&f,wire,n,&used)==UPDATE_FRAME_READY&&f.length==UPDATE_PAYLOAD_MAX);
    uint8_t small[ENCODED_MAX];memset(small,0xAA,sizeof(small));used=123;
    assert(!UPDATE_FrameEncode(payload,sizeof(payload),small,n-1,&used)&&used==0);
    for(size_t i=0;i<sizeof(small);i++)assert(small[i]==0xAA);
    assert(UPDATE_FrameEncode(payload,sizeof(payload),small,n,&used)&&used==n);
    assert(!UPDATE_FrameEncode(payload,0,wire,sizeof(wire),&n));
    assert(!UPDATE_FrameEncode(payload,UPDATE_PAYLOAD_MAX+1,wire,sizeof(wire),&n));
    assert(!UPDATE_FrameEncode(NULL,1,wire,sizeof(wire),&n));
    assert(!UPDATE_FrameEncode(payload,1,wire,sizeof(wire),NULL));

    UPDATE_FrameReset(&f);
    assert(UPDATE_FramePush(&f,0x55)==UPDATE_INPUT_IDLE);
    assert(UPDATE_FramePush(&f,1)==UPDATE_INPUT_PENDING);
    for(size_t i=0;i<UPDATE_FRAME_CAPACITY;i++)assert(UPDATE_FramePush(&f,0x55)==UPDATE_INPUT_PENDING);
    assert(UPDATE_FramePush(&f,0x55)==UPDATE_FRAME_TOO_LARGE&&f.length==0&&!f.receiving);
    payload[0]=UPDATE_READ_BOOT_INFO;
    assert(UPDATE_FrameEncode(payload,1,wire,sizeof(wire),&n));
    assert(UPDATE_FrameFeed(&f,wire,n,&used)==UPDATE_FRAME_READY); /* Fresh SOH resync. */

    UPDATE_FrameReset(&f);UPDATE_FramePush(&f,1);UPDATE_FramePush(&f,0x55);
    assert(UPDATE_FrameFeed(&f,wire,n,&used)==UPDATE_FRAME_READY); /* Nested SOH resync. */
    UPDATE_FrameReset(&f);UPDATE_FramePush(&f,1);UPDATE_FramePush(&f,0x10);
    assert(UPDATE_FramePush(&f,0x55)==UPDATE_FRAME_MALFORMED&&!f.receiving);
    assert(UPDATE_FrameFeed(&f,wire,n,&used)==UPDATE_FRAME_READY);
    UPDATE_FrameReset(&f);UPDATE_FramePush(&f,1);assert(UPDATE_FramePush(&f,4)==UPDATE_FRAME_MALFORMED);
    assert(UPDATE_FrameFeed(&f,(uint8_t[]){1,2,0x55,0x55,4},5,&used)==UPDATE_FRAME_CRC_ERROR);
    assert(!f.receiving&&!f.ready&&f.length==0);
    /* Every bit in an ordinary payload must be protected by the CRC. */
    uint8_t original_crc_payload[]={2,0x55};
    uint16_t original_crc=ReferenceCRC(original_crc_payload,2);
    for(unsigned byte=0;byte<2;byte++)for(unsigned bit=0;bit<8;bit++)
    {
        uint8_t changed[2]={2,0x55};changed[byte]^=(uint8_t)(1U<<bit);
        uint8_t corrupt_wire[12];size_t pos=0;corrupt_wire[pos++]=1;
        uint8_t body[4]={changed[0],changed[1],(uint8_t)original_crc,(uint8_t)(original_crc>>8)};
        for(unsigned i=0;i<4;i++){if(body[i]==1||body[i]==4||body[i]==0x10)corrupt_wire[pos++]=0x10;corrupt_wire[pos++]=body[i];}
        corrupt_wire[pos++]=4;UPDATE_FrameReset(&f);
        assert(UPDATE_FrameFeed(&f,corrupt_wire,pos,&used)==UPDATE_FRAME_CRC_ERROR);
    }
    /* Reset cancels partial escape across disconnect/timeout; stale bytes ignored. */
    UPDATE_FramePush(&f,1);UPDATE_FramePush(&f,0x10);UPDATE_FrameReset(&f);
    assert(UPDATE_FramePush(&f,4)==UPDATE_INPUT_IDLE);
    assert(UPDATE_FrameFeed(NULL,wire,n,&used)==UPDATE_INVALID_ARGUMENT);
    assert(UPDATE_FrameFeed(&f,NULL,1,&used)==UPDATE_INVALID_ARGUMENT);
    assert(UPDATE_FrameFeed(&f,wire,n,NULL)==UPDATE_INVALID_ARGUMENT);
    assert(UPDATE_FramePush(NULL,1)==UPDATE_INVALID_ARGUMENT);
    UPDATE_FrameReset(NULL);
    assert(UPDATE_FrameFeed(&f,NULL,0,&used)==UPDATE_INPUT_IDLE&&used==0);

    /* Caller must retain unconsumed frames; two commands are not silently dropped. */
    size_t one=n;memcpy(wire+one,wire,one);
    UPDATE_FrameReset(&f);assert(UPDATE_FrameFeed(&f,wire,one*2,&used)==UPDATE_FRAME_READY&&used==one);
    UPDATE_FrameReset(&f);assert(UPDATE_FrameFeed(&f,wire+used,one,&used)==UPDATE_FRAME_READY&&used==one);
}

static void RangeTests(void)
{
    uint32_t p;
    for(unsigned alias=0;alias<3;alias++)
    {
        uint32_t offset=alias==0?0:alias==1?0x80000000U:0xA0000000U;
        assert(UPDATE_ApplicationSpan(UPDATE_APP_PHYSICAL_START+offset,1,&p)&&p==UPDATE_APP_PHYSICAL_START);
        assert(UPDATE_ApplicationSpan(UPDATE_APP_PHYSICAL_START+offset,0x38000,&p));
        assert(UPDATE_ApplicationSpan(UPDATE_FLASH_PHYSICAL_END-1+offset,1,&p));
        assert(!UPDATE_ApplicationSpan(UPDATE_FLASH_PHYSICAL_END-1+offset,2,&p)&&p==0);
        assert(!UPDATE_ApplicationSpan(UPDATE_FLASH_PHYSICAL_END+offset,1,&p));
        assert(!UPDATE_ApplicationSpan(UPDATE_APP_PHYSICAL_START-1+offset,1,&p));
        assert(!UPDATE_ApplicationSpan(UPDATE_APP_PHYSICAL_START+offset,UINT32_MAX,&p));
        assert(!UPDATE_ApplicationSpan(UPDATE_APP_PHYSICAL_START+offset,0,&p));
    }
    for(unsigned i=0;i<10000;i++)
    {
        uint32_t a=Random(),len=Random(),base=a;
        if(a>=0x9D000000U&&a<0x9D040000U)base=a-0x80000000U;
        else if(a>=0xBD000000U&&a<0xBD040000U)base=a-0xA0000000U;
        bool expected=base>=UPDATE_APP_PHYSICAL_START&&base<UPDATE_FLASH_PHYSICAL_END&&
            len>0U&&(uint64_t)base+len<=UPDATE_FLASH_PHYSICAL_END;
        assert(UPDATE_ApplicationSpan(a,len,NULL)==expected);
    }
    assert(!UPDATE_ApplicationSpan(0xBF800000,4,&p));
    assert(!UPDATE_ApplicationSpan(0x1FC00BF0,4,&p));
    assert(!UPDATE_ApplicationSpan(0xA0000000,4,&p));
    assert(!UPDATE_ApplicationSpan(UINT32_MAX,1,&p));
}

static void CommandTests(void)
{
    uint8_t payload[UPDATE_PAYLOAD_MAX]={0};UPDATE_COMMAND cmd;
    for(unsigned c=1;c<=5;c++)
    {
        payload[0]=(uint8_t)c;
        if(c==1||c==2||c==5){assert(UPDATE_CommandValidate(payload,1,&cmd)==UPDATE_COMMAND_VALID&&cmd.id==(UPDATE_COMMAND_ID)c);assert(UPDATE_CommandValidate(payload,2,&cmd)==UPDATE_COMMAND_BAD_LENGTH);}
    }
    payload[0]=UPDATE_READ_CRC;LE32(payload+1,0x9D008000);LE32(payload+5,0x38000);
    assert(UPDATE_CommandValidate(payload,9,&cmd)==UPDATE_COMMAND_VALID&&cmd.physical_address==0x1D008000&&cmd.length==0x38000);
    for(size_t i=1;i<9;i++)assert(UPDATE_CommandValidate(payload,i,&cmd)==UPDATE_COMMAND_BAD_LENGTH);
    assert(UPDATE_CommandValidate(payload,10,&cmd)==UPDATE_COMMAND_BAD_LENGTH);
    LE32(payload+1,0xFFFFFFFF);LE32(payload+5,1);
    assert(UPDATE_CommandValidate(payload,9,&cmd)==UPDATE_COMMAND_BAD_RANGE);
    assert(cmd.id==0&&cmd.length==0); /* Failure leaves no executable command. */
    LE32(payload+1,0x1D03FFFF);LE32(payload+5,2);
    assert(UPDATE_CommandValidate(payload,9,&cmd)==UPDATE_COMMAND_BAD_RANGE);
    LE32(payload+1,0x1D008000);LE32(payload+5,0);
    assert(UPDATE_CommandValidate(payload,9,&cmd)==UPDATE_COMMAND_BAD_RANGE);
    payload[0]=99;assert(UPDATE_CommandValidate(payload,1,&cmd)==UPDATE_COMMAND_UNSUPPORTED);
    assert(UPDATE_CommandValidate(NULL,1,&cmd)==UPDATE_COMMAND_BAD_LENGTH);
    assert(UPDATE_CommandValidate(payload,1,NULL)==UPDATE_COMMAND_BAD_LENGTH);
    assert(UPDATE_CommandValidate(payload,0,&cmd)==UPDATE_COMMAND_BAD_LENGTH);
    assert(UPDATE_CommandValidate(payload,UPDATE_PAYLOAD_MAX+1,&cmd)==UPDATE_COMMAND_BAD_LENGTH);

    payload[0]=UPDATE_PROGRAM_FLASH;uint8_t data[4]={0x11,0x22,0x33,0x44};
    size_t n=Record(payload+1,4,0,(uint8_t[]){0x1D,0},2);
    n+=Record(payload+1+n,0,0x8000,data,4);
    n+=Record(payload+1+n,1,0,NULL,0);
    assert(UPDATE_CommandValidate(payload,n+1,&cmd)==UPDATE_COMMAND_VALID&&cmd.hex_length==n&&cmd.hex_records==payload+1);
    for(size_t cut=1;cut<7;cut++)assert(UPDATE_CommandValidate(payload,cut+1,&cmd)!=UPDATE_COMMAND_VALID);
    payload[n+1]=0x42;assert(UPDATE_CommandValidate(payload,n+2,&cmd)==UPDATE_COMMAND_BAD_HEX);
    payload[n]^=1;assert(UPDATE_CommandValidate(payload,n+1,&cmd)==UPDATE_COMMAND_BAD_HEX);payload[n]^=1;
    /* Type-specific lengths, unknown record and EOF trailing another complete record. */
    for(uint8_t type=0;type<7;type++)for(uint8_t len=0;len<6;len++)
    {
        n=Record(payload+1,type,0,data,len>4?4:len);
        uint8_t actual=len>4?4:len;
        bool good=(type==0&&actual>0)||(type==1&&actual==0)||((type==2||type==4)&&actual==2)||(type==5&&actual==4);
        assert((UPDATE_CommandValidate(payload,n+1,&cmd)==UPDATE_COMMAND_VALID)==good);
    }
    n=Record(payload+1,1,0,NULL,0);n+=Record(payload+n+1,0,0x8000,data,1);
    assert(UPDATE_CommandValidate(payload,n+1,&cmd)==UPDATE_COMMAND_BAD_HEX);
    n=Record(payload+1,4,1,data,2);assert(UPDATE_CommandValidate(payload,n+1,&cmd)==UPDATE_COMMAND_BAD_HEX);
    /* Full 255-byte record fits; short allocation claiming 255 must reject safely. */
    uint8_t many[255];memset(many,0xAA,sizeof(many));n=Record(payload+1,0,0x8000,many,255);
    assert(UPDATE_CommandValidate(payload,n+1,&cmd)==UPDATE_COMMAND_VALID);
    uint8_t truncated[]={3,255,0,0,0,0};assert(UPDATE_CommandValidate(truncated,sizeof(truncated),&cmd)==UPDATE_COMMAND_BAD_HEX);
    /* Valid shape is not final address acceptance: HEX writer must check extension/data address. */
    n=Record(payload+1,4,0,(uint8_t[]){0xFF,0xFF},2);
    assert(UPDATE_CommandValidate(payload,n+1,&cmd)==UPDATE_COMMAND_VALID);
}

static void RandomizedTests(void)
{
    UPDATE_FRAME frame;uint8_t payload[UPDATE_PAYLOAD_MAX],wire[ENCODED_MAX];size_t n,used;
    for(unsigned t=0;t<2000;t++)
    {
        size_t len=Random()%UPDATE_PAYLOAD_MAX+1;
        for(size_t i=0;i<len;i++)payload[i]=(uint8_t)Random();
        assert(UPDATE_Crc16(payload,len)==ReferenceCRC(payload,len));
        assert(UPDATE_FrameEncode(payload,len,wire,sizeof(wire),&n));
        UPDATE_FrameReset(&frame);size_t pos=0;
        while(pos<n){size_t chunk=Random()%64+1;if(chunk>n-pos)chunk=n-pos;UPDATE_FRAME_STATUS s=UPDATE_FrameFeed(&frame,wire+pos,chunk,&used);assert(used==chunk);pos+=used;assert(s==(pos==n?UPDATE_FRAME_READY:UPDATE_INPUT_PENDING));}
        assert(frame.length==len&&!memcmp(frame.bytes,payload,len));
    }
    UPDATE_FrameReset(&frame);
    for(unsigned t=0;t<100000;t++)
    {
        UPDATE_FRAME_STATUS s=UPDATE_FramePush(&frame,(uint8_t)Random());
        assert(frame.length<=UPDATE_FRAME_CAPACITY);
        if(s>=UPDATE_FRAME_READY)UPDATE_FrameReset(&frame);
        if((t%97)==0)UPDATE_FrameReset(&frame);
    }
    for(unsigned t=0;t<5000;t++)
    {
        size_t n=Random()%sizeof(payload)+1;for(size_t i=0;i<n;i++)payload[i]=(uint8_t)Random();
        UPDATE_COMMAND c;(void)UPDATE_CommandValidate(payload,n,&c);
    }
}
int main(void)
{
    FrameTests();RangeTests();CommandTests();RandomizedTests();
    puts("PASS: bounded frame/escape/CRC/resync, exact command lengths, flash-alias spans, HEX shape checks and deterministic randomized tests; no hardware operations.");
    return 0;
}
