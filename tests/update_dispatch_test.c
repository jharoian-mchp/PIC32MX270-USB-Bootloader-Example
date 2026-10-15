#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "update_dispatch.h"
#define START UPDATE_APP_PHYSICAL_START
#define END UPDATE_FLASH_PHYSICAL_END
static uint8_t memory[END-START];
static bool busy,reject;
static uint32_t address,error;
static const uint32_t *source;
static unsigned writes,erases;
static uint32_t now;
static bool Busy(void *c){(void)c;return busy;}
static uint32_t Error(void *c){(void)c;return error;}
static bool Erase(void *c,uint32_t a)
{(void)c;++erases;if(reject)return false;address=a;source=NULL;busy=true;return true;}
static bool Write(void *c,const uint32_t *p,uint32_t a)
{(void)c;++writes;if(reject)return false;address=a;source=p;busy=true;return true;}
static bool Read(void *c,uint32_t a,uint8_t *p,size_t n)
{(void)c;assert(a>=START&&n<=END-a);memcpy(p,memory+a-START,n);return true;}
static const UPDATE_FLASH_BACKEND backend={NULL,Busy,Error,Erase,Write,Read};
static void Finish(void)
{
    if(source)memcpy(memory+address-START,source,UPDATE_ROW_SIZE);
    else memset(memory+address-START,255,UPDATE_ERASE_SIZE);
    busy=false;
}
static void Reset(UPDATE_DISPATCH *d,bool enabled)
{
    busy=reject=false;error=0;writes=erases=0;source=NULL;now=0;
    memset(memory,255,sizeof(memory));assert(UPDATE_DispatchInit(d,&backend,100,enabled));
}
static uint32_t Get32(const uint8_t *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void Put32(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8U*i));}
static size_t Request(uint8_t *p,uint8_t op,uint32_t id,uint32_t token,uint32_t a,uint32_t b)
{
    p[0]=op;p[1]=1;Put32(p+2,id);Put32(p+6,token);Put32(p+10,a);Put32(p+14,b);
    return op==UPDATE_WIRE_BEGIN?18U:op==UPDATE_WIRE_ABORT?10U:6U;
}
static UPDATE_REPLY Run(UPDATE_DISPATCH *d,const uint8_t *p,size_t n)
{
    /* Exercise escaped frame split over actual 64-byte zero-padded reports. */
    uint8_t wire[2U*UPDATE_FRAME_CAPACITY+2U],reply[28],response[64];size_t wn,rn,used;
    assert(UPDATE_FrameEncode(p,n,wire,sizeof(wire),&wn));UPDATE_FRAME f;UPDATE_FrameReset(&f);
    for(size_t off=0;off<wn;off+=64)
    {
        uint8_t report[64]={0};size_t count=wn-off>64?64:wn-off;
        memcpy(report,wire+off,count);
        UPDATE_FRAME_STATUS s=UPDATE_FrameFeed(&f,report,64,&used);
        if(off+count==wn)assert(s==UPDATE_FRAME_READY);
        else assert(s==UPDATE_INPUT_PENDING);
    }
    assert(f.length==n&&!memcmp(f.bytes,p,n));
    assert(UPDATE_DispatchRequest(d,f.bytes,f.length,now,reply,sizeof(reply),&rn));
    assert(rn==28&&reply[0]==(p[0]|0x80U)&&Get32(reply+4)==Get32(p+2));
    assert(UPDATE_FrameEncode(reply,rn,response,sizeof(response),&wn));
    UPDATE_FrameReset(&f);assert(UPDATE_FrameFeed(&f,response,wn,&used)==UPDATE_FRAME_READY);
    return (UPDATE_REPLY)reply[2];
}
static void Pump(UPDATE_DISPATCH *d,UPDATE_SESSION_STATUS target)
{
    for(unsigned i=0;i<30000&&d->session.status!=target;i++)
    {
        if(busy)
        {
            unsigned e=erases,w=writes;uint8_t retained[128];
            if(source)memcpy(retained,source,128);
            assert(Run(d,d->last_request,d->last_size)==d->last_result);
            assert(e==erases&&w==writes);
            if(source)assert(!memcmp(retained,source,128));
            Finish();
        }
        UPDATE_DispatchPoll(d,now++);assert(d->session.status!=UPDATE_SESSION_FAILED);
    }
    assert(d->session.status==target);
}
static void StoredValid(void)
{
    UPDATE_IMAGE_CHECK c;assert(UPDATE_ImageCheckStored(&c,&backend)==UPDATE_IMAGE_CHECKING);
    while(UPDATE_ImageCheckPoll(&c)==UPDATE_IMAGE_CHECKING){}
    assert(c.status==UPDATE_IMAGE_VALID);
}
static size_t Rec(uint8_t *p,uint8_t type,uint16_t offset,const uint8_t *data,uint8_t n)
{
    p[0]=n;p[1]=(uint8_t)(offset>>8);p[2]=(uint8_t)offset;p[3]=type;
    if(n)memcpy(p+4,data,n);
    uint8_t sum=0;
    for(size_t i=0;i<n+4U;i++)sum=(uint8_t)(sum+p[i]);
    p[n+4U]=(uint8_t)(0U-sum);return n+5U;
}
int main(int argc,char **argv)
{
    UPDATE_DISPATCH d;uint8_t p[510],q[510],out[28];size_t n,rn;
    if(argc==2)
    {
        /* Python's offline planner supplies complete image payload vectors. */
        FILE *file=fopen(argv[1],"r");assert(file);char line[1100];Reset(&d,true);
        unsigned count=0;
        while(fgets(line,sizeof(line),file))
        {
            char *space=strchr(line,' ');assert(space);*space=0;
            int target=atoi(space+1);n=strlen(line)/2;assert(n<=sizeof(p));
            for(size_t i=0;i<n;i++){unsigned byte;assert(sscanf(line+2*i,"%2x",&byte)==1);p[i]=(uint8_t)byte;}
            assert(Run(&d,p,n)==UPDATE_REPLY_OK);
            unsigned e=erases,w=writes;
            assert(Run(&d,p,n)==UPDATE_REPLY_OK);assert(e==erases&&w==writes); /* lost ACK */
            Request(q,UPDATE_WIRE_STATUS,100000U+count,0,0,0);assert(Run(&d,q,6)==UPDATE_REPLY_OK);
            assert(Run(&d,p,n)==UPDATE_REPLY_OK);assert(e==erases&&w==writes);
            Pump(&d,(UPDATE_SESSION_STATUS)target);e=erases;w=writes;
            assert(Run(&d,p,n)==UPDATE_REPLY_OK);assert(e==erases&&w==writes);++count;
        }
        fclose(file);assert(count>=2);StoredValid();
        UPDATE_DispatchDisconnect(&d);assert(d.session.status==UPDATE_SESSION_COMPLETE);StoredValid();
        printf("PASS: Python plan -> framed reports -> dispatcher -> verified mock flash/commit (%u mutations).\n",count);
        return 0;
    }
    Reset(&d,false);n=Request(p,UPDATE_WIRE_BEGIN,1,7,4,0);
    assert(Run(&d,p,n)==UPDATE_REPLY_DISABLED&&erases==0);
    Request(p,UPDATE_WIRE_STATUS,1,0,0,0);assert(Run(&d,p,6)==UPDATE_REPLY_OK);
    assert(UPDATE_DispatchInit(&d,NULL,100,false));assert(!UPDATE_DispatchInit(&d,NULL,100,true));
    Reset(&d,true);n=Request(p,UPDATE_WIRE_BEGIN,1,7,4,0);
    assert(!UPDATE_DispatchRequest(&d,p,n,0,out,27,&rn)&&rn==0&&erases==0);
    assert(Run(&d,p,n)==UPDATE_REPLY_OK&&erases==1);
    assert(Run(&d,p,n)==UPDATE_REPLY_OK&&erases==1);
    memcpy(q,p,n);q[14]^=1;assert(Run(&d,q,n)==UPDATE_REPLY_CONFLICT&&erases==1);
    Request(q,UPDATE_WIRE_BEGIN,2,8,4,0);assert(Run(&d,q,18)==UPDATE_REPLY_BUSY);
    Request(q,UPDATE_WIRE_BATCH,2,8,0,0);q[14]=0;
    assert(Run(&d,q,15)==UPDATE_REPLY_TOKEN);
    Request(q,UPDATE_WIRE_BATCH,2,7,1,0);q[14]=0;
    assert(Run(&d,q,15)==UPDATE_REPLY_SEQUENCE);
    Request(q,UPDATE_WIRE_BATCH,2,7,0,0);q[14]=0;
    assert(Run(&d,q,15)==UPDATE_REPLY_BUSY&&d.last_id==1);
    Pump(&d,UPDATE_SESSION_READY);
    /* A malformed accepted batch fails the session once; retry cannot poison twice. */
    assert(Run(&d,q,15)==UPDATE_REPLY_FAILED&&d.last_id==2);
    assert(Run(&d,q,15)==UPDATE_REPLY_FAILED&&d.session.status==UPDATE_SESSION_FAILED);
    assert(Run(&d,p,n)==UPDATE_REPLY_STALE);
    Request(q,UPDATE_WIRE_ABORT,3,7,0,0);assert(Run(&d,q,10)==UPDATE_REPLY_OK);
    assert(Run(&d,q,10)==UPDATE_REPLY_OK);Pump(&d,UPDATE_SESSION_ABORTED);
    unsigned e=erases;assert(Run(&d,q,10)==UPDATE_REPLY_OK&&erases==e);
    Request(p,UPDATE_WIRE_BEGIN,4,7,4,0);assert(Run(&d,p,18)==UPDATE_REPLY_TOKEN);
    Request(p,UPDATE_WIRE_BEGIN,4,8,3,0);assert(Run(&d,p,18)==UPDATE_REPLY_BAD_REQUEST);
    Request(p,UPDATE_WIRE_BEGIN,4,8,4,0);assert(Run(&d,p,18)==UPDATE_REPLY_OK);
    UPDATE_DispatchDisconnect(&d);assert(busy);assert(UPDATE_DispatchPoll(&d,now)==UPDATE_SESSION_ABORTING);
    Finish();Pump(&d,UPDATE_SESSION_ABORTED);
    Request(p,UPDATE_WIRE_BEGIN,5,9,4,0);reject=true;
    assert(Run(&d,p,18)==UPDATE_REPLY_FAILED);e=erases;
    assert(Run(&d,p,18)==UPDATE_REPLY_FAILED&&erases==e);
    /* Write-buffer retention after lost link, including a timed-out operation. */
    Reset(&d,true);Request(p,UPDATE_WIRE_BEGIN,1,1,128,0);assert(Run(&d,p,18)==UPDATE_REPLY_OK);
    Pump(&d,UPDATE_SESSION_READY);Request(p,UPDATE_WIRE_BATCH,2,1,0,0);
    n=14+Rec(p+14,4,0,(uint8_t[]){0x1D,0},2);assert(Run(&d,p,n)==UPDATE_REPLY_OK);Pump(&d,UPDATE_SESSION_READY);
    Request(p,UPDATE_WIRE_BATCH,3,1,1,0);uint8_t image[128];memset(image,0x10,sizeof(image));
    n=14+Rec(p+14,0,0x8000,image,128);assert(Run(&d,p,n)==UPDATE_REPLY_OK);
    UPDATE_DispatchPoll(&d,now++);UPDATE_DispatchPoll(&d,now++);assert(busy&&source);
    uint8_t retained[128];memcpy(retained,source,128);
    UPDATE_DispatchPoll(&d,now+101);assert(d.session.status==UPDATE_SESSION_FAILED&&busy);
    UPDATE_DispatchDisconnect(&d);assert(UPDATE_DispatchPoll(&d,now+102)==UPDATE_SESSION_ABORTING);
    assert(!memcmp(retained,source,128));Finish();Pump(&d,UPDATE_SESSION_ABORTED);
    /* Envelopes and unsupported RUN/legacy commands have no effects. */
    e=erases;Request(p,0x65,4,1,0,0);
    assert(!UPDATE_DispatchRequest(&d,p,10,now,out,28,&rn));
    p[0]=UPDATE_WIRE_BEGIN;p[1]=2;assert(!UPDATE_DispatchRequest(&d,p,18,now,out,28,&rn));
    p[1]=1;Put32(p+2,0);assert(!UPDATE_DispatchRequest(&d,p,18,now,out,28,&rn));
    assert(erases==e);
    /* CRC failure reported through STATUS, never mistaken for a successful ACK. */
    Reset(&d,true);uint8_t four[4]={1,2,3,4};
    uint32_t crc=UPDATE_Crc32Step(UINT32_MAX,four,4)^UINT32_MAX;
    Request(p,UPDATE_WIRE_BEGIN,1,1,4,crc^1U);assert(Run(&d,p,18)==UPDATE_REPLY_OK);
    Pump(&d,UPDATE_SESSION_READY);Request(p,UPDATE_WIRE_BATCH,2,1,0,0);
    n=14+Rec(p+14,4,0,(uint8_t[]){0x1D,0},2);
    n+=Rec(p+n,0,0x8000,four,4);n+=Rec(p+n,1,0,NULL,0);
    assert(Run(&d,p,n)==UPDATE_REPLY_OK);
    while(d.session.status!=UPDATE_SESSION_FAILED)
    {if(busy)Finish();UPDATE_DispatchPoll(&d,now++);}
    Request(q,UPDATE_WIRE_STATUS,50,0,0,0);
    assert(UPDATE_DispatchRequest(&d,q,6,now,out,28,&rn)&&out[24]==UPDATE_SESSION_FAILED);
    assert(Run(&d,p,n)==UPDATE_REPLY_OK); /* accepted earlier, current state FAILED */
    assert(memory[UPDATE_IMAGE_META_ADDRESS-START]==255);
    /* Link loss at commit waits for active write, then invalidates the record. */
    Reset(&d,true);Request(p,UPDATE_WIRE_BEGIN,1,1,4,crc);assert(Run(&d,p,18)==UPDATE_REPLY_OK);
    Pump(&d,UPDATE_SESSION_READY);Request(p,UPDATE_WIRE_BATCH,2,1,0,0);
    n=14+Rec(p+14,4,0,(uint8_t[]){0x1D,0},2);
    n+=Rec(p+n,0,0x8000,four,4);n+=Rec(p+n,1,0,NULL,0);
    assert(Run(&d,p,n)==UPDATE_REPLY_OK);
    while(d.session.status!=UPDATE_SESSION_COMMITTING)
    {if(busy)Finish();UPDATE_DispatchPoll(&d,now++);}
    assert(busy);UPDATE_DispatchDisconnect(&d);Pump(&d,UPDATE_SESSION_ABORTED);
    assert(memory[UPDATE_IMAGE_META_ADDRESS-START]==255);
    Request(q,UPDATE_WIRE_BEGIN,UINT32_MAX,2,4,crc);assert(Run(&d,q,18)==UPDATE_REPLY_OK);
    e=erases;assert(Run(&d,q,18)==UPDATE_REPLY_OK&&erases==e);
    assert(Run(&d,p,n)==UPDATE_REPLY_STALE);
    UPDATE_DispatchDisconnect(&d);Pump(&d,UPDATE_SESSION_ABORTED);
    puts("PASS: disabled-by-permission, exact replay, conflicts/stale/token/sequence rejection, failed replay, abort/disconnect/timeout retention; no target binding.");
    return 0;
}
