#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "update_session.h"
#define START UPDATE_APP_PHYSICAL_START
#define END UPDATE_FLASH_PHYSICAL_END
static uint8_t memory[END-START];
static bool busy,reject,read_error,corrupt;
static uint32_t error,address;
static const uint32_t *source;
static unsigned writes,erases;
static bool Busy(void *c){(void)c;return busy;}
static uint32_t Error(void *c){(void)c;return error;}
static bool Write(void *c,const uint32_t *p,uint32_t a)
{(void)c;++writes;if(reject)return false;address=a;source=p;busy=true;return true;}
static bool Erase(void *c,uint32_t a)
{(void)c;++erases;if(reject)return false;address=a;source=NULL;busy=true;return true;}
static bool Read(void *c,uint32_t a,uint8_t *p,size_t n)
{(void)c;if(read_error)return false;assert(a>=START&&a+n<=END);memcpy(p,memory+a-START,n);return true;}
static UPDATE_FLASH_BACKEND backend={NULL,Busy,Error,Erase,Write,Read};
static void Finish(void)
{
    if(source)memcpy(memory+address-START,source,128);
    else memset(memory+address-START,0xFF,1024);
    if(corrupt)memory[address-START]^=1;
    busy=false;
}
static void Reset(UPDATE_SESSION *s)
{
    busy=reject=read_error=corrupt=false;error=0;writes=erases=0;source=NULL;
    memset(memory,0xFF,sizeof(memory));assert(UPDATE_SessionInit(s,&backend,100));
}
static UPDATE_SESSION_STATUS Pump(UPDATE_SESSION *s,uint32_t *now)
{
    for(unsigned i=0;i<10000;i++)
    {
        if(busy)Finish();
        UPDATE_SESSION_STATUS st=UPDATE_SessionPoll(s,(*now)++);
        if(st==UPDATE_SESSION_READY||st==UPDATE_SESSION_COMPLETE||st==UPDATE_SESSION_FAILED||st==UPDATE_SESSION_ABORTED)return st;
    }
    assert(!"pump timeout");return UPDATE_SESSION_FAILED;
}
static size_t Rec(uint8_t *p,uint8_t type,uint16_t off,const uint8_t *data,uint8_t n)
{
    p[0]=n;p[1]=(uint8_t)(off>>8);p[2]=(uint8_t)off;p[3]=type;
    if(n)memcpy(p+4,data,n);
    uint8_t sum=0;for(size_t i=0;i<n+4U;i++)sum=(uint8_t)(sum+p[i]);p[n+4U]=(uint8_t)(0U-sum);return n+5U;
}
static uint32_t CRC(const uint8_t *data,size_t n)
{return UPDATE_Crc32Step(UINT32_MAX,data,n)^UINT32_MAX;}
static void SubmitImage(UPDATE_SESSION *s,uint32_t *now,const uint8_t *image,size_t len)
{
    uint8_t batch[509];size_t n=Rec(batch,4,0,(uint8_t[]){0x1D,0},2);
    assert(UPDATE_SessionSubmit(s,batch,n));assert(Pump(s,now)==UPDATE_SESSION_READY);
    for(size_t off=0;off<len;)
    {
        uint8_t count=(uint8_t)((len-off)>255?255:len-off);
        n=Rec(batch,0,(uint16_t)(0x8000+off),image+off,count);
        assert(UPDATE_SessionSubmit(s,batch,n));memset(batch,0xCC,sizeof(batch)); /* Session owns copy. */
        assert(Pump(s,now)==UPDATE_SESSION_READY);off+=count;
    }
    n=Rec(batch,1,0,NULL,0);assert(UPDATE_SessionSubmit(s,batch,n));
}
static void StoredValid(void)
{
    UPDATE_IMAGE_CHECK c;assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_CHECKING);
    while(UPDATE_ImageCheckPoll(&c)==UPDATE_IMAGE_CHECKING){}
    assert(c.status==UPDATE_IMAGE_VALID);
}
int main(void)
{
    UPDATE_SESSION s;uint32_t now=0;uint8_t image[766];
    for(size_t i=0;i<sizeof(image);i++)image[i]=(uint8_t)(i*7U);
    assert(CRC((uint8_t*)"123456789",9)==0xCBF43926U);
    Reset(&s);assert(UPDATE_SessionBegin(&s,sizeof(image),CRC(image,sizeof(image)),now));
    assert(address==UPDATE_IMAGE_META_ADDRESS&&writes==0);
    assert(!UPDATE_SessionSubmit(&s,image,4));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    assert(erases==2);SubmitImage(&s,&now,image,sizeof(image));
    assert(Pump(&s,&now)==UPDATE_SESSION_COMPLETE);assert(!memcmp(image,memory,sizeof(image)));
    assert(writes==7);StoredValid();
    /* Stored CRC catches changed image, metadata tears and erased reset word. */
    memory[10]^=1;UPDATE_IMAGE_CHECK c;assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_CHECKING);
    while(UPDATE_ImageCheckPoll(&c)==UPDATE_IMAGE_CHECKING){}assert(c.status==UPDATE_IMAGE_INVALID);memory[10]^=1;
    uint8_t original[128];memcpy(original,memory+UPDATE_IMAGE_META_ADDRESS-START,128);
    for(unsigned n=0;n<32;n++)
    {memset(memory+UPDATE_IMAGE_META_ADDRESS-START,0xFF,128);memcpy(memory+UPDATE_IMAGE_META_ADDRESS-START,original,n);
     assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_INVALID);}
    memcpy(memory+UPDATE_IMAGE_META_ADDRESS-START,original,128);StoredValid();
    /* A new update invalidates old metadata before touching old app bytes. */
    assert(UPDATE_SessionBegin(&s,4,CRC(image,4),now));assert(address==UPDATE_IMAGE_META_ADDRESS);
    assert(!memcmp(memory,image,sizeof(image)));Finish();
    assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_INVALID);
    assert(!memcmp(memory,image,sizeof(image))); /* Metadata erase alone doesn't erase app. */
    UPDATE_SessionAbort(&s);assert(Pump(&s,&now)==UPDATE_SESSION_ABORTED);

    /* Abort during metadata commit waits, erases record after operation settles. */
    Reset(&s);now=0;assert(UPDATE_SessionBegin(&s,4,CRC(image,4),now));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    SubmitImage(&s,&now,image,4);
    while(s.status!=UPDATE_SESSION_COMMITTING){if(busy)Finish();UPDATE_SessionPoll(&s,now++);}
    assert(busy);UPDATE_SessionAbort(&s);assert(UPDATE_SessionPoll(&s,now)==UPDATE_SESSION_ABORTING);
    assert(Pump(&s,&now)==UPDATE_SESSION_ABORTED);
    assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_INVALID);

    Reset(&s);now=0;assert(UPDATE_SessionBegin(&s,4,CRC(image,4)^1,now));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    SubmitImage(&s,&now,image,4);assert(Pump(&s,&now)==UPDATE_SESSION_FAILED);
    assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_INVALID);
    assert(!UPDATE_SessionBegin(&s,4,0,now));UPDATE_SessionAbort(&s);assert(Pump(&s,&now)==UPDATE_SESSION_ABORTED);

    Reset(&s);now=0;assert(UPDATE_SessionBegin(&s,4,CRC(image,4),now));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    uint8_t batch[20];size_t n=Rec(batch,4,0,(uint8_t[]){0x1D,0},2);
    assert(UPDATE_SessionSubmit(&s,batch,n));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    n=Rec(batch,0,0x8003,image,2);assert(!UPDATE_SessionSubmit(&s,batch,n));assert(s.status==UPDATE_SESSION_FAILED&&writes==0);

    Reset(&s);now=0;assert(UPDATE_SessionBegin(&s,128,CRC(image,128),now));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    n=Rec(batch,4,0,(uint8_t[]){0x1D,0},2);assert(UPDATE_SessionSubmit(&s,batch,n));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    uint8_t row_record[133];n=Rec(row_record,0,0x8000,image,128);assert(UPDATE_SessionSubmit(&s,row_record,n));
    UPDATE_SessionPoll(&s,now++);UPDATE_SessionPoll(&s,now++);assert(busy);
    UPDATE_SessionAbort(&s);assert(UPDATE_SessionPoll(&s,now)==UPDATE_SESSION_ABORTING);
    Finish();assert(Pump(&s,&now)==UPDATE_SESSION_ABORTED&&writes==1);

    Reset(&s);now=UINT32_MAX-50;assert(UPDATE_SessionBegin(&s,4,0,now));
    UPDATE_SessionPoll(&s,60);assert(s.status==UPDATE_SESSION_FAILED&&busy);
    UPDATE_SessionAbort(&s);assert(UPDATE_SessionPoll(&s,61)==UPDATE_SESSION_ABORTING);
    Finish();assert(Pump(&s,&now)==UPDATE_SESSION_ABORTED);

    Reset(&s);reject=true;assert(!UPDATE_SessionBegin(&s,4,0,0)&&s.status==UPDATE_SESSION_FAILED);
    Reset(&s);now=0;assert(UPDATE_SessionBegin(&s,4,CRC(image,4),now));Finish();error=1;
    assert(UPDATE_SessionPoll(&s,1)==UPDATE_SESSION_FAILED);
    Reset(&s);now=0;assert(UPDATE_SessionBegin(&s,4,CRC(image,4),now));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    uint8_t erased[4]={255,255,255,255};SubmitImage(&s,&now,erased,4);
    assert(Pump(&s,&now)==UPDATE_SESSION_FAILED); /* reset word cannot be erased */
    assert(!UPDATE_ImageParameters(3)&&!UPDATE_ImageParameters(UPDATE_IMAGE_MAX_LENGTH+1));
    assert(!UPDATE_SessionBegin(NULL,4,0,0));

    /* Larger than one physical-address window, with bounded verification progress. */
    static uint8_t large_image[70000];
    for(size_t i=0;i<sizeof(large_image);i++)large_image[i]=(uint8_t)(i*11U+3U);
    Reset(&s);now=0;assert(UPDATE_SessionBegin(&s,sizeof(large_image),CRC(large_image,sizeof(large_image)),now));
    assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    uint16_t upper=0;
    for(size_t off=0;off<sizeof(large_image);)
    {
        uint32_t a=START+(uint32_t)off;
        if((uint16_t)(a>>16)!=upper)
        {
            upper=(uint16_t)(a>>16);n=Rec(batch,4,0,(uint8_t[]){(uint8_t)(upper>>8),(uint8_t)upper},2);
            assert(UPDATE_SessionSubmit(&s,batch,n));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
        }
        uint8_t record[133];uint8_t count=(uint8_t)((sizeof(large_image)-off)>128?128:sizeof(large_image)-off);
        n=Rec(record,0,(uint16_t)a,large_image+off,count);assert(UPDATE_SessionSubmit(&s,record,n));
        assert(Pump(&s,&now)==UPDATE_SESSION_READY);off+=count;
    }
    n=Rec(batch,1,0,NULL,0);assert(UPDATE_SessionSubmit(&s,batch,n));
    assert(Pump(&s,&now)==UPDATE_SESSION_COMPLETE);StoredValid();
    assert(!memcmp(memory,large_image,sizeof(large_image)));

    /* No EOF/power-cut between rows: absent metadata prevents boot acceptance. */
    Reset(&s);now=0;assert(UPDATE_SessionBegin(&s,128,CRC(image,128),now));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    n=Rec(batch,4,0,(uint8_t[]){0x1D,0},2);assert(UPDATE_SessionSubmit(&s,batch,n));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    n=Rec(row_record,0,0x8000,image,128);assert(UPDATE_SessionSubmit(&s,row_record,n));
    assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_INVALID);
    assert(!UPDATE_SessionSubmit(&s,batch,sizeof(batch))); /* Malformed batch poisons; no marker. */
    assert(s.status==UPDATE_SESSION_FAILED);

    /* Worst per-record queue: old partial row + two full rows = three copies. */
    Reset(&s);now=0;uint8_t sparse[640];memset(sparse,0xFF,sizeof(sparse));
    memcpy(sparse,image,4);memcpy(sparse+129,image,255);
    assert(UPDATE_SessionBegin(&s,sizeof(sparse),CRC(sparse,sizeof(sparse)),now));
    assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    n=Rec(batch,4,0,(uint8_t[]){0x1D,0},2);assert(UPDATE_SessionSubmit(&s,batch,n));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    n=Rec(batch,0,0x8000,image,4);assert(UPDATE_SessionSubmit(&s,batch,n));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    uint8_t max_record[260];n=Rec(max_record,0,0x8081,image,255);
    assert(UPDATE_SessionSubmit(&s,max_record,n));UPDATE_SessionPoll(&s,now++);
    assert(s.queued==3);assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    n=Rec(batch,1,0,NULL,0);assert(UPDATE_SessionSubmit(&s,batch,n));
    assert(Pump(&s,&now)==UPDATE_SESSION_COMPLETE);StoredValid();

    /* Metadata corruption / read failure and non-progress verification fail closed. */
    Reset(&s);uint8_t metadata[128];UPDATE_ImageRecord(metadata,4,CRC(image,4));
    memcpy(memory,image,4);memcpy(memory+UPDATE_IMAGE_META_ADDRESS-START,metadata,128);
    for(unsigned i=0;i<32;i++)
    { memory[UPDATE_IMAGE_META_ADDRESS-START+i]^=1;
      assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_INVALID);
      memory[UPDATE_IMAGE_META_ADDRESS-START+i]^=1; }
    memory[UPDATE_IMAGE_META_ADDRESS-START+100]=0;
    assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_INVALID);
    read_error=true;assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_READ_ERROR);
    read_error=false;
    assert(UPDATE_ImageCheckExpected(&c,&backend,4,CRC(image,4)));read_error=true;
    assert(UPDATE_ImageCheckPoll(&c)==UPDATE_IMAGE_READ_ERROR);

    Reset(&s);now=0;assert(UPDATE_SessionBegin(&s,4,CRC(image,4),now));assert(Pump(&s,&now)==UPDATE_SESSION_READY);
    SubmitImage(&s,&now,image,4);
    while(s.status!=UPDATE_SESSION_VERIFYING){if(busy)Finish();UPDATE_SessionPoll(&s,now++);}
    busy=true;assert(UPDATE_SessionPoll(&s,now+101)==UPDATE_SESSION_FAILED);
    busy=false;assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_INVALID);

    puts("PASS: invalidate-before-erase, bounded async rows/copies/readback, exact manifest range/CRC, metadata-last commit, torn-record rejection, abort/timeout quarantine and no target backend.");
    return 0;
}
