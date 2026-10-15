#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "update_link_dispatch.h"
#define START UPDATE_APP_PHYSICAL_START
#define END UPDATE_FLASH_PHYSICAL_END
static uint8_t memory[END-START];
static bool busy,usb=true,reject;
static uint32_t address,error,now;
static const uint32_t *source;
static unsigned writes,erases;
static bool Busy(void *c){(void)c;return busy;}
static uint32_t Error(void *c){(void)c;return error;}
static bool Erase(void *c,uint32_t a)
{(void)c;assert(!usb);++erases;if(reject)return false;address=a;source=NULL;busy=true;return true;}
static bool Write(void *c,const uint32_t *p,uint32_t a)
{(void)c;assert(!usb);++writes;if(reject)return false;address=a;source=p;busy=true;return true;}
static bool Read(void *c,uint32_t a,uint8_t *p,size_t n)
{(void)c;assert(!usb&&!busy);assert(a>=START&&n<=END-a);memcpy(p,memory+a-START,n);return true;}
static const UPDATE_FLASH_BACKEND backend={NULL,Busy,Error,Erase,Write,Read};
static void Finish(void)
{assert(busy);if(source)memcpy(memory+address-START,source,128);else memset(memory+address-START,255,1024);busy=false;}
static void Reset(UPDATE_LINK_DISPATCH *d,bool enabled)
{
    busy=reject=false;usb=true;error=now=writes=erases=0;source=NULL;
    memset(memory,255,sizeof(memory));assert(UPDATE_LinkDispatchInit(d,&backend,0x100000007ULL,100,10000,enabled));
}
static void Put32(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8U*i));}
static uint32_t Get32(const uint8_t *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static size_t Request(uint8_t *p,uint8_t op,uint32_t rpc,uint32_t token,uint32_t id,uint32_t ticket)
{
    memset(p,0,510);p[0]=op;p[1]=2;Put32(p+4,rpc);
    if(op!=UPDATE_LINK_HELLO){Put32(p+8,7);Put32(p+12,1);Put32(p+16,token);Put32(p+20,id);Put32(p+24,ticket);}
    return op==UPDATE_LINK_HELLO?24:28;
}
static size_t Begin(uint8_t *p,uint32_t rpc,uint32_t id,uint32_t token,uint32_t length,uint32_t crc)
{Request(p,UPDATE_LINK_STAGE,rpc,token,id,0);p[24]=UPDATE_STAGE_BEGIN;Put32(p+28,length);Put32(p+32,crc);return 36;}
static uint8_t Run(UPDATE_LINK_DISPATCH *d,const uint8_t *p,size_t n,uint32_t *cookie,uint8_t *out)
{
    size_t rn;assert(UPDATE_LinkDispatchRequest(d,p,n,now,out,56,&rn,cookie));
    assert(rn==56&&out[0]==(p[0]|0x80U)&&Get32(out+4)==Get32(p+4));return out[2];
}
static void Pump(UPDATE_LINK_DISPATCH *d)
{
    usb=false;
    for(unsigned i=0;i<50000&&d->core.state!=UPDATE_RESULT;i++)
    {if(busy)Finish();UPDATE_LinkDispatchPoll(d,now++,true);}
    assert(d->core.state==UPDATE_RESULT&&!busy);usb=true;
}
static bool Stored(void)
{
    UPDATE_IMAGE_CHECK check;usb=false;
    UPDATE_IMAGE_STATUS s=UPDATE_ImageCheckStored(&check,&backend);
    while(s==UPDATE_IMAGE_CHECKING)s=UPDATE_ImageCheckPoll(&check);
    usb=true;return s==UPDATE_IMAGE_VALID;
}
int main(int argc,char **argv)
{
    UPDATE_LINK_DISPATCH d;uint8_t p[510],out[56];uint32_t cookie;size_t n,rn;
    Reset(&d,true);
    if(argc==2&&strcmp(argv[1],"--host")==0)
    {
        char line[1200];setvbuf(stdout,NULL,_IOLBF,0);
        while(fgets(line,sizeof(line),stdin))
        {
            if(strncmp(line,"send ",5)==0)
            {
                char *hex=line+5;size_t len=strcspn(hex,"\r\n");assert(len%2==0&&len/2<=510);n=len/2;
                for(size_t i=0;i<n;i++){unsigned v;assert(sscanf(hex+i*2,"%2x",&v)==1);p[i]=(uint8_t)v;}
                if(UPDATE_LinkDispatchRequest(&d,p,n,now,out,56,&rn,&cookie))
                {
                    for(size_t i=0;i<rn;i++)printf("%02x",out[i]);
                    printf(" %u %u %u\n",cookie,erases,writes);
                }
                else puts("NONE");
            }
            else if(strncmp(line,"ack ",4)==0)
            {unsigned value,ok;assert(sscanf(line+4,"%u %u",&value,&ok)==2);UPDATE_LinkDispatchSent(&d,value,ok!=0,now++);puts("OK");}
            else if(strcmp(line,"pump\n")==0){Pump(&d);printf("DONE %u %u\n",erases,writes);}
            else if(strcmp(line,"verify\n")==0){printf("VALID %u\n",Stored());}
            else if(strcmp(line,"cancel\n")==0){UPDATE_LinkDispatchCancel(&d,now++);puts("OK");}
            else assert(!"unknown host command");
        }
        return 0;
    }
    n=Request(p,UPDATE_LINK_HELLO,1,0,0,0);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK&&cookie==0&&out[13]==7);
    n=Begin(p,2,1,1,4,0);UPDATE_LINK_DISPATCH previous=d;
    assert(!UPDATE_LinkDispatchRequest(&d,p,n,0,out,55,&rn,&cookie));assert(!memcmp(&d,&previous,sizeof(d)));
    assert(!UPDATE_LinkDispatchRequest(&d,p,n,0,out,56,&rn,&d.pending_cookie));assert(!memcmp(&d,&previous,sizeof(d)));
    assert(!UPDATE_LinkDispatchRequest(&d,p,n,0,p,sizeof(p),&rn,&cookie));assert(!memcmp(&d,&previous,sizeof(d)));
    assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK&&Get32(out+32)==1&&cookie==0&&erases==0);
    Put32(p+4,3);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK&&Get32(out+32)==1); /* changed RPC, same mutation */
    p[32]^=1;assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_IDENTITY);p[32]^=1;
    n=Request(p,UPDATE_LINK_EXECUTE,4,1,1,1);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK&&cookie==1);
    uint32_t first=cookie;previous=d;assert(!UPDATE_LinkDispatchRequest(&d,p,n,0,out,56,&rn,&cookie));assert(!memcmp(&d,&previous,sizeof(d)));
    UPDATE_LinkDispatchSent(&d,first+1,true,now);assert(d.core.state==UPDATE_ACK_WAIT&&erases==0);
    UPDATE_LinkDispatchPoll(&d,now++,false);assert(erases==0);
    UPDATE_LinkDispatchSent(&d,first,true,now);assert(d.core.state==UPDATE_OFF_WAIT);
    UPDATE_LinkDispatchSent(&d,first,true,now);assert(d.core.state==UPDATE_OFF_WAIT);
    assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK&&cookie==0&&erases==0); /* duplicate execute, no cookie */
    UPDATE_LinkDispatchPoll(&d,now++,false);assert(erases==0);Pump(&d);
    n=Request(p,UPDATE_LINK_QUERY,5,1,1,1);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK&&out[48]==UPDATE_RESULT&&out[49]==UPDATE_OUTCOME_DONE);
    p[8]^=1;assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_IDENTITY);p[8]^=1;
    n=Request(p,UPDATE_LINK_RESUME,6,1,1,1);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK);
    /* New reply cookie cannot be authorised by old response completion. */
    n=Request(p,UPDATE_LINK_STAGE,7,1,2,0);p[24]=UPDATE_STAGE_ABORT;assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK);
    n=Request(p,UPDATE_LINK_EXECUTE,8,1,2,2);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK&&cookie==2);
    UPDATE_LinkDispatchSent(&d,first,true,now);assert(d.core.state==UPDATE_ACK_WAIT);
    UPDATE_LinkDispatchCancel(&d,now);UPDATE_LinkDispatchSent(&d,2,true,now);assert(d.core.state==UPDATE_RECOVERY);Pump(&d);
    assert(d.core.session.status==UPDATE_SESSION_ABORTED);
    /* Disabled schema remains queryable but never stages/executes. */
    Reset(&d,false);n=Request(p,UPDATE_LINK_HELLO,1,0,0,0);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK&&out[13]==3);
    n=Begin(p,2,1,1,4,0);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_DISABLED&&erases==0);
    /* Cookie exhaustion fails before acceptance; no wrapping. */
    Reset(&d,true);n=Begin(p,1,1,1,4,0);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK);
    d.send_generation=UINT32_MAX;n=Request(p,UPDATE_LINK_EXECUTE,2,1,1,1);
    assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_FAILED&&d.core.state==UPDATE_STAGED);
    /* Failed send cannot execute, even after a late success callback. */
    d.send_generation=0;assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK);
    UPDATE_LinkDispatchSent(&d,cookie,false,now);UPDATE_LinkDispatchSent(&d,cookie,true,now);
    assert(d.core.state==UPDATE_RECOVERY&&erases==0);Pump(&d);assert(erases==0);
    /* ACK lease expiry retains pending completion ownership but cannot execute. */
    Reset(&d,true);n=Begin(p,1,1,1,4,0);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK);
    n=Request(p,UPDATE_LINK_RESUME,2,1,1,1);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_BUSY);
    n=Request(p,UPDATE_LINK_EXECUTE,3,1,1,1);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK);
    uint32_t late=cookie;d.core.phase_timeout_ms=10;
    UPDATE_LinkDispatchPoll(&d,now+10,false);assert(d.core.state==UPDATE_RECOVERY&&d.pause_pending);
    UPDATE_LinkDispatchSent(&d,late,true,now+11);assert(!d.pause_pending&&d.core.state==UPDATE_RECOVERY&&erases==0);
    Pump(&d);n=Request(p,UPDATE_LINK_QUERY,4,1,1,1);
    assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_OK&&out[49]==UPDATE_OUTCOME_CANCELLED);
    n=Request(p,UPDATE_LINK_RESUME,5,1,1,1);assert(Run(&d,p,n,&cookie,out)==UPDATE_LINK_FAILED);
    /* Bad CRC envelope cannot change staged/controller data or reply bytes. */
    Reset(&d,true);n=Begin(p,1,1,1,4,0);p[1]=1;previous=d;memset(out,0xA5,sizeof(out));
    assert(!UPDATE_LinkDispatchRequest(&d,p,n,now,out,56,&rn,&cookie));
    assert(rn==0&&cookie==0&&out[0]==0xA5&&!memcmp(&d,&previous,sizeof(d)));
    puts("PASS: v2 dispatch capacity/aliases, stage no-write, dynamic tickets/RPC, execute cookie/ACK gating, query/resume replay, disabled features/cookie exhaustion/cancellation; no live HID binding.");return 0;
}
