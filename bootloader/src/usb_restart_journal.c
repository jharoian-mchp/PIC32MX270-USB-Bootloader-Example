#include "usb_restart_journal.h"
#include <stddef.h>
#define MAGIC 0x55534A31U
#define COMMIT 0x434D5431U
static uint32_t Checksum(const uint32_t *words)
{
    uint32_t crc=UINT32_MAX;
    for(unsigned i=0;i<10;i++)for(unsigned byte=0;byte<4;byte++)
    {
        crc^=(words[i]>>(8U*byte))&255U;
        for(unsigned bit=0;bit<8;bit++)crc=(crc>>1)^((crc&1U)?0xEDB88320U:0U);
    }
    return crc^UINT32_MAX;
}
static bool Slot(const USB_RESTART_JOURNAL *j,unsigned index,USB_RESTART_RECORD *out)
{
    uint32_t w[USB_RESTART_JOURNAL_WORDS];
    for(unsigned i=0;i<USB_RESTART_JOURNAL_WORDS;i++)w[i]=j->slot[index][i];
    if(w[11]!=COMMIT||w[0]!=MAGIC||w[1]!=1U||w[2]==0U||w[3]!=~w[2]||
       w[10]!=Checksum(w)||w[8]!=0U||w[9]!=0U||w[4]==0U||w[5]==0U)return false;
    uint32_t status=w[7];
    if((status&255U)>USB_RESTART_FAILED||((status>>8)&255U)>USB_RESTART_FAILED||
       ((status>>16)&255U)>USB_RESTART_FAULT_EXTERNAL)return false;
    *out=(USB_RESTART_RECORD){.sequence=w[2],.ticket=w[4],.generation=w[5],.time_ms=w[6],
        .state=(USB_RESTART_STATE)(status&255U),.phase=(USB_RESTART_STATE)((status>>8)&255U),
        .fault=(USB_RESTART_FAULT)((status>>16)&255U),.evidence=(uint8_t)(status>>24)};
    return true;
}
static int Latest(const USB_RESTART_JOURNAL *j,USB_RESTART_RECORD *out)
{
    USB_RESTART_RECORD a,b;bool av=Slot(j,0,&a),bv=Slot(j,1,&b);
    if(!av&&!bv)return -1;
    if(av&&(!bv||a.sequence>b.sequence)){*out=a;return 0;}
    *out=b;return 1;
}
bool USB_RestartJournalRead(const USB_RESTART_JOURNAL *j,USB_RESTART_RECORD *out)
{return j!=NULL&&out!=NULL&&Latest(j,out)>=0;}
bool USB_RestartJournalWrite(USB_RESTART_JOURNAL *j,const USB_RESTART_RECORD *r)
{
    if(j==NULL||r==NULL||(unsigned)r->state>USB_RESTART_FAILED||(unsigned)r->phase>USB_RESTART_FAILED||
       (unsigned)r->fault>USB_RESTART_FAULT_EXTERNAL||r->generation==0U||r->ticket==0U)return false;
    USB_RESTART_RECORD old;int latest=Latest(j,&old);
    if(latest>=0&&old.sequence==UINT32_MAX)return false; /* No sequence wrap. */
    unsigned slot=latest==0?1U:0U;uint32_t sequence=latest<0?1U:old.sequence+1U;
    uint32_t w[USB_RESTART_JOURNAL_WORDS]={MAGIC,1U,sequence,~sequence,r->ticket,r->generation,r->time_ms,
        (uint32_t)r->state|((uint32_t)r->phase<<8)|((uint32_t)r->fault<<16)|((uint32_t)r->evidence<<24),0,0,0,COMMIT};
    w[10]=Checksum(w);j->slot[slot][11]=0;
    for(unsigned i=0;i<11;i++)j->slot[slot][i]=w[i];
    /* Compiler ordering protects the volatile transaction. On target, caller
     * uses uncached retained SRAM; this is not nonvolatile persistence. */
    __asm__ volatile("":::"memory");j->slot[slot][11]=COMMIT;return true;
}
