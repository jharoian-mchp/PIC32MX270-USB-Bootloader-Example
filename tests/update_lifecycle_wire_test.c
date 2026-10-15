#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "update_lifecycle_wire.h"
static void Put32(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8U*i));}
int main(int argc,char **argv)
{
    UPDATE_LINK_REQUEST r;uint8_t p[510]={UPDATE_LINK_STAGE,2,0,0};
    Put32(p+4,1);Put32(p+8,7);Put32(p+16,1);Put32(p+20,1);p[24]=UPDATE_STAGE_BEGIN;
    Put32(p+28,128);Put32(p+32,0xCBF43926);
    assert(UPDATE_LinkDecode(p,36,&r)&&r.operation.length==128&&r.operation.crc==0xCBF43926);
    UPDATE_LINK_STATUS s={.epoch=7,.token=1,.operation_id=1,.ticket=1,.length=128,
                          .crc=0xCBF43926,.features=3,.lifecycle=UPDATE_STAGED};
    uint8_t out[56];size_t n=0;assert(UPDATE_LinkEncode(&r,0,&s,out,sizeof(out),&n)&&n==56);
    assert(out[0]==0xF1&&out[1]==2&&!memcmp(out+8,"PKL2",4)&&out[16]==7&&out[48]==UPDATE_STAGED);
    if(argc==2&&strcmp(argv[1],"--vectors")==0)
    {
        for(size_t i=0;i<36;i++)printf("%02x",p[i]);
        puts("");
        for(size_t i=0;i<n;i++)printf("%02x",out[i]);
        puts("");return 0;
    }
    memset(out,0xA5,sizeof(out));assert(!UPDATE_LinkEncode(&r,0,&s,out,55,&n)&&n==0&&out[0]==0xA5);
    UPDATE_LINK_REQUEST previous=r;p[25]=1;assert(!UPDATE_LinkDecode(p,36,&r));assert(!memcmp(&previous,&r,sizeof(r)));p[25]=0;
    assert(!UPDATE_LinkDecode(p,35,&r));p[1]=1;assert(!UPDATE_LinkDecode(p,36,&r));p[1]=2;
    Put32(p+4,0);assert(!UPDATE_LinkDecode(p,36,&r));Put32(p+4,1);
    p[24]=UPDATE_STAGE_BATCH;Put32(p+28,0);memcpy(p+32,(uint8_t[]){0,0,0,1,255},5);
    assert(UPDATE_LinkDecode(p,37,&r)&&r.operation.size==5);
    p[36]^=1;assert(!UPDATE_LinkDecode(p,37,&r));p[36]^=1;
    assert(!UPDATE_LinkDecode(p,36,&r));
    /* Exactly478 record bytes (260+218) fill the510 payload boundary. */
    memset(p+32,0,478);p[32]=255;p[35]=0;p[291]=1; /* checksum -255 */
    p[292]=213;p[295]=0;p[509]=43; /* checksum -213 */
    assert(UPDATE_LinkDecode(p,510,&r)&&r.operation.size==478);
    p[509]^=1;assert(!UPDATE_LinkDecode(p,510,&r));p[509]^=1;
    assert(!UPDATE_LinkDecode(p,511,&r));
    p[0]=UPDATE_LINK_EXECUTE;Put32(p+24,1);assert(UPDATE_LinkDecode(p,28,&r)&&r.ticket==1);
    Put32(p+24,0);assert(!UPDATE_LinkDecode(p,28,&r));
    p[0]=UPDATE_LINK_HELLO;memset(p+8,0,16);assert(UPDATE_LinkDecode(p,24,&r));
    p[8]=1;assert(!UPDATE_LinkDecode(p,24,&r));
    assert(!UPDATE_LinkDecode(NULL,24,&r)&&!UPDATE_LinkDecode(p,24,NULL));
    puts("PASS: pure v2 lifecycle envelopes/identity/stage bodies/HEX structure/reserved/capacity and C/Python golden vector; no dispatch or HID binding.");return 0;
}
