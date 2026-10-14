#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "update_hex.h"

#define START UPDATE_APP_PHYSICAL_START
#define END UPDATE_FLASH_PHYSICAL_END
static uint8_t flash_image[END-START];
static uint32_t rng=0xFCAB2134;
static uint32_t Random(void) { rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng; }
static unsigned row_count,sink_fail_at;
static uint32_t addresses[8];
static uint8_t rows[8][UPDATE_ROW_SIZE];
static bool Sink(void *context,uint32_t address,const uint8_t *data)
{
    (void)context;assert(address>=START&&address+UPDATE_ROW_SIZE<=END&&!(address%128));
    if(sink_fail_at&&row_count+1==sink_fail_at)return false;
    if(row_count<8){addresses[row_count]=address;memcpy(rows[row_count],data,128);}
    memcpy(flash_image+address-START,data,128);++row_count;return true;
}
static void Begin(UPDATE_HEX *h,uint32_t end)
{
    memset(flash_image,0xFF,sizeof(flash_image));row_count=sink_fail_at=0;
    assert(UPDATE_HexBegin(h,end,Sink,NULL));
}
static size_t Record(uint8_t *dst,uint8_t type,uint16_t offset,const uint8_t *data,uint8_t size)
{
    dst[0]=size;dst[1]=(uint8_t)(offset>>8);dst[2]=(uint8_t)offset;dst[3]=type;
    if(size)memcpy(dst+4,data,size);
    uint8_t sum=0;for(size_t i=0;i<(size_t)size+4;i++)sum=(uint8_t)(sum+dst[i]);
    dst[size+4]=(uint8_t)(0U-sum);return size+5U;
}
static void Upper(UPDATE_HEX *h,uint16_t upper)
{
    uint8_t r[7],d[2]={(uint8_t)(upper>>8),(uint8_t)upper};Record(r,4,0,d,2);
    assert(UPDATE_HexFeed(h,r,7)==UPDATE_HEX_ACCEPTED);
}
static UPDATE_HEX_STATUS End(UPDATE_HEX *h)
{ uint8_t r[5];Record(r,1,0,NULL,0);return UPDATE_HexFeed(h,r,5); }
static void HexTests(void)
{
    UPDATE_HEX h;uint8_t record[520],data[255];for(size_t i=0;i<255;i++)data[i]=(uint8_t)i;
    Begin(&h,END);Upper(&h,0x1D00);
    size_t n=Record(record,0,0x8003,data,5);assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_ACCEPTED);
    n=Record(record,0,0x8010,data+5,3);assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_ACCEPTED);
    assert(!row_count);assert(End(&h)==UPDATE_HEX_FINISHED&&row_count==1);
    assert(addresses[0]==START&&!memcmp(rows[0]+3,data,5)&&rows[0][8]==0xFF&&rows[0][15]==0xFF);
    assert(!memcmp(rows[0]+16,data+5,3)&&h.data_bytes==8);
    assert(End(&h)==UPDATE_HEX_NOT_ACTIVE); /* EOF cannot acknowledge another session. */

    Begin(&h,END);Upper(&h,0x9D00);n=Record(record,0,0x807F,data,255);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_ACCEPTED);
    assert(End(&h)==UPDATE_HEX_FINISHED&&row_count==3);
    assert(!memcmp(flash_image+127,data,255));
    for(size_t i=0;i<127;i++)assert(flash_image[i]==0xFF);
    assert(flash_image[382]==0xFF);

    Begin(&h,END);Upper(&h,0xBD03);n=Record(record,0,0xFFF0,data,16);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_ACCEPTED&&row_count==1);
    assert(End(&h)==UPDATE_HEX_FINISHED&&addresses[0]==END-128);

    const uint16_t bad_upper[]={0x1D04,0x1FC0,0xA000,0xFFFF};
    for(size_t i=0;i<4;i++)
    {
        Begin(&h,END);Upper(&h,bad_upper[i]);n=Record(record,0,0,data,1);
        assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_ADDRESS&&!row_count);
    }
    Begin(&h,END);Upper(&h,0x1D03);n=Record(record,0,0xFFFF,data,4);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_ADDRESS&&!row_count);
    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x7FFF,data,2);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_ADDRESS&&!row_count);
    Begin(&h,END-1024);Upper(&h,0x1D03);n=Record(record,0,0xFC00,data,1);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_ADDRESS);

    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,128);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_ACCEPTED&&row_count==1);
    n=Record(record,0,0x8000,data,1);assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_ORDER&&row_count==1);
    assert(End(&h)==UPDATE_HEX_BAD_ORDER); /* Poisoned session cannot flush again. */

    /* Later invalid record preflight prevents the first valid row from emitting. */
    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,128);
    n+=Record(record+n,0,0x7FFF,data,1);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_ADDRESS&&!row_count);
    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,128);record[n]=0x42;
    assert(UPDATE_HexFeed(&h,record,n+1)==UPDATE_HEX_BAD_RECORD&&!row_count);
    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,128);record[n-1]^=1;
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_RECORD&&!row_count);
    Begin(&h,END);uint8_t short_record[]={255,0,0,0,0};
    assert(UPDATE_HexFeed(&h,short_record,5)==UPDATE_HEX_BAD_RECORD);
    Begin(&h,END);assert(End(&h)==UPDATE_HEX_BAD_RECORD);
    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,4);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_ACCEPTED);UPDATE_HexAbort(&h);
    assert(End(&h)==UPDATE_HEX_NOT_ACTIVE&&!row_count);
    Begin(&h,END);n=Record(record,0,0x8000,data,1); /* Extension cleared after abort/restart. */
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_ADDRESS);

    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,255);sink_fail_at=2;
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_ACCEPTED&&row_count==1);
    assert(End(&h)==UPDATE_HEX_SINK_ERROR&&row_count==1&&!h.active);
    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,128);sink_fail_at=1;
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_SINK_ERROR&&!row_count);
    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x807F,data,255);sink_fail_at=2;
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_SINK_ERROR&&row_count==1);
    assert(End(&h)==UPDATE_HEX_SINK_ERROR&&row_count==1);

    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,1);assert(UPDATE_HexFeed(&h,record,n)==0);
    n=Record(record,5,0,(uint8_t[]){0x9D,0,0x80,0},4);assert(UPDATE_HexFeed(&h,record,n)==0);
    n=Record(record,0,0x8001,data,1);assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_ORDER);
    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,1);assert(UPDATE_HexFeed(&h,record,n)==0);
    n=Record(record,5,0,(uint8_t[]){0x9D,0,0x80,4},4);assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_ADDRESS);

    /* Deterministic sequential/sparse streams versus an independent byte map. */
    static uint8_t expected[END-START];
    for(unsigned trial=0;trial<100;trial++)
    {
        Begin(&h,END);memset(expected,0xFF,sizeof(expected));uint32_t address=START;uint16_t last_upper=0;
        for(unsigned rec=0;rec<250;rec++)
        {
            address+=Random()%17U;uint8_t count=(uint8_t)(Random()%127U+1U);
            if((address&0xFFFFU)+count>0x10000U)address=(address&0xFFFF0000U)+0x10000U;
            if(address+count>END)break;
            uint16_t upper=(uint16_t)(address>>16);if(upper!=last_upper){Upper(&h,upper);last_upper=upper;}
            for(unsigned i=0;i<count;i++)data[i]=(uint8_t)Random();
            memcpy(expected+address-START,data,count);n=Record(record,0,(uint16_t)address,data,count);
            assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_ACCEPTED);address+=count;
        }
        assert(End(&h)==UPDATE_HEX_FINISHED);assert(!memcmp(expected,flash_image,sizeof(expected)));
    }
    /* Uninitialized, null/empty/oversized batches and unsupported shapes. */
    h=(UPDATE_HEX){0};assert(End(&h)==UPDATE_HEX_NOT_ACTIVE);
    /* Malformed extension lengths/checksum are rejected before changing state. */
    Begin(&h,END);n=Record(record,4,0,data,1);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_RECORD&&h.extension==0);
    Begin(&h,END);n=Record(record,4,1,data,2);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_RECORD&&h.extension==0);
    Begin(&h,END);assert(UPDATE_HexFeed(&h,NULL,1)==UPDATE_HEX_BAD_RECORD);
    Begin(&h,END);assert(UPDATE_HexFeed(&h,record,0)==UPDATE_HEX_BAD_RECORD);
    Begin(&h,END);assert(UPDATE_HexFeed(&h,record,510)==UPDATE_HEX_BAD_RECORD);
    for(uint8_t type=0;type<7;type++)for(uint8_t len=0;len<=4;len++)
    {
        if((type==0&&len>0)||((type==2||type==4)&&len==2))continue;
        Begin(&h,END);n=Record(record,type,0,data,len);
        assert(UPDATE_HexFeed(&h,record,n)!=UPDATE_HEX_ACCEPTED&&!row_count);
    }
    /* Out-of-order aliases cannot write a previously programmed row twice. */
    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,128);
    assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_ACCEPTED);Upper(&h,0x9D00);
    n=Record(record,0,0x807F,data,1);assert(UPDATE_HexFeed(&h,record,n)==UPDATE_HEX_BAD_ORDER);
    /* A valid optional entry record and EOF flush only once. */
    Begin(&h,END);Upper(&h,0x1D00);n=Record(record,0,0x8000,data,1);assert(UPDATE_HexFeed(&h,record,n)==0);
    n=Record(record,5,0,(uint8_t[]){0x9D,0,0x80,0},4);assert(UPDATE_HexFeed(&h,record,n)==0);
    assert(End(&h)==UPDATE_HEX_FINISHED&&row_count==1);
    assert(!UPDATE_HexBegin(NULL,END,Sink,NULL));assert(!UPDATE_HexBegin(&h,END,NULL,NULL));
    assert(!UPDATE_HexBegin(&h,END+128,Sink,NULL));assert(!UPDATE_HexBegin(&h,END-1,Sink,NULL));
}

static bool hardware_busy,reject_hw,read_failure,corrupt;
static uint32_t hw_error,hw_addr;
static const uint32_t *hw_source;
static unsigned hw_writes,hw_erases,reads;
static bool Busy(void *c){(void)c;return hardware_busy;}
static uint32_t Error(void *c){(void)c;return hw_error;}
static bool Write(void *c,const uint32_t *r,uint32_t a)
{(void)c;assert(!((uintptr_t)r%16));++hw_writes;if(reject_hw)return false;hw_source=r;hw_addr=a;hardware_busy=true;return true;}
static bool Erase(void *c,uint32_t a)
{(void)c;++hw_erases;if(reject_hw)return false;hw_source=NULL;hw_addr=a;hardware_busy=true;return true;}
static bool Read(void *c,uint32_t a,uint8_t *out,size_t n)
{(void)c;++reads;if(read_failure)return false;memcpy(out,flash_image+a-START,n);if(corrupt)out[0]^=1;return true;}
static void Finish(void)
{
    if(hw_source)memcpy(flash_image+hw_addr-START,hw_source,128);
    else memset(flash_image+hw_addr-START,0xFF,1024);
    hardware_busy=false;
}
static void FlashBegin(UPDATE_FLASH *f)
{
    hardware_busy=reject_hw=read_failure=corrupt=false;hw_error=0;hw_source=NULL;
    hw_writes=hw_erases=reads=0;UPDATE_FLASH_BACKEND b={NULL,Busy,Error,Erase,Write,Read};
    assert(UPDATE_FlashInit(f,&b,START,END-1024,100));
}
static void FlashTests(void)
{
    UPDATE_FLASH f;uint8_t row[128];memset(row,0xAA,sizeof(row));FlashBegin(&f);
    assert(!UPDATE_FlashWrite(&f,START-128,row,0));assert(!UPDATE_FlashWrite(&f,START+1,row,0));
    assert(!UPDATE_FlashWrite(&f,END-1024,row,0));assert(!UPDATE_FlashWrite(&f,START,NULL,0));
    assert(!UPDATE_FlashErase(&f,START+128,0));assert(!UPDATE_FlashErase(&f,END-1024,0));
    assert(!hw_writes&&!hw_erases);
    assert(UPDATE_FlashWrite(&f,START,row,0));memset(row,0x55,sizeof(row));
    assert(!UPDATE_FlashWrite(&f,START+128,row,0));assert(!UPDATE_FlashReset(&f));
    assert(UPDATE_FlashPoll(&f,20)==UPDATE_FLASH_BUSY);Finish();
    assert(UPDATE_FlashPoll(&f,21)==UPDATE_FLASH_OK);assert(flash_image[0]==0xAA&&reads==1);
    assert(UPDATE_FlashReset(&f));
    memset(flash_image,0,sizeof(flash_image));assert(UPDATE_FlashErase(&f,START,30));Finish();
    for(unsigned i=0;i<7;i++)assert(UPDATE_FlashPoll(&f,31+i)==UPDATE_FLASH_VERIFY);
    assert(UPDATE_FlashPoll(&f,38)==UPDATE_FLASH_OK);assert(reads==9);assert(UPDATE_FlashReset(&f));

    FlashBegin(&f);reject_hw=true;assert(!UPDATE_FlashWrite(&f,START,row,0));
    assert(UPDATE_FlashPoll(&f,1)==UPDATE_FLASH_REJECTED);assert(UPDATE_FlashReset(&f));
    FlashBegin(&f);hw_error=1;assert(!UPDATE_FlashErase(&f,START,0));assert(!hw_erases&&f.status==UPDATE_FLASH_ERROR);
    FlashBegin(&f);assert(UPDATE_FlashWrite(&f,START,row,0));Finish();hw_error=1;
    assert(UPDATE_FlashPoll(&f,1)==UPDATE_FLASH_ERROR);
    FlashBegin(&f);assert(UPDATE_FlashWrite(&f,START,row,0));Finish();corrupt=true;
    assert(UPDATE_FlashPoll(&f,1)==UPDATE_FLASH_ERROR);
    FlashBegin(&f);assert(UPDATE_FlashErase(&f,START,0));Finish();read_failure=true;
    assert(UPDATE_FlashPoll(&f,1)==UPDATE_FLASH_ERROR);
    FlashBegin(&f);assert(UPDATE_FlashWrite(&f,START,row,UINT32_MAX-50));
    assert(UPDATE_FlashPoll(&f,40)==UPDATE_FLASH_BUSY);
    assert(UPDATE_FlashPoll(&f,50)==UPDATE_FLASH_TIMEOUT);assert(!UPDATE_FlashReset(&f));
    assert(!UPDATE_FlashWrite(&f,START+128,row,51)); /* Buffer never reused while hardware busy. */
    Finish();assert(UPDATE_FlashReset(&f));
    FlashBegin(&f);assert(UPDATE_FlashErase(&f,START,0));Finish();corrupt=true;
    assert(UPDATE_FlashPoll(&f,1)==UPDATE_FLASH_ERROR); /* Erase readback detects retained bit. */
    FlashBegin(&f);hardware_busy=true;
    assert(!UPDATE_FlashWrite(&f,START,row,0)&&!hw_writes);hardware_busy=false;
    UPDATE_FLASH uninitialized={0};assert(UPDATE_FlashPoll(&uninitialized,0)==UPDATE_FLASH_ERROR);
    UPDATE_FLASH_BACKEND b={NULL,Busy,Error,Erase,Write,Read};
    assert(!UPDATE_FlashInit(&f,&b,START+1,END,100));assert(!UPDATE_FlashInit(&f,&b,START,END+1024,100));
    assert(!UPDATE_FlashInit(&f,&b,START,END,0));assert(!UPDATE_FlashInit(&f,&b,START,END,UINT32_MAX));
    b.read=NULL;assert(!UPDATE_FlashInit(&f,&b,START,END,100));
    assert(UPDATE_FlashPoll(NULL,0)==UPDATE_FLASH_ERROR);assert(!UPDATE_FlashReset(NULL));
}
int main(void)
{
    HexTests();FlashTests();
    puts("PASS: HEX full-span/order/checksum/preflight/session/row padding; flash bounds, copied buffers, rejection, error, readback, nonblocking erase verification and timeout quarantine. No target flash backend bound.");
    return 0;
}
