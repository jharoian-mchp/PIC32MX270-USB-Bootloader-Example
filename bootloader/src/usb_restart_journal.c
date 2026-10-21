#include "usb_restart_journal.h"
#include <stddef.h>
#define MAGIC 0x55534A31U
#define COMMIT 0x434D5431U
bool USB_RestartPowerValid(const USB_POWER_EVIDENCE *p)
{
    uint16_t m=p->meta;
    return (m&~511U)==0U&&((m&2U)==0U||(m&1U)!=0U)&&((m&4U)==0U||(m&3U)==3U)&&
        ((m&1U)!=0U||(p->pre_count==0U&&(m&24U)==0U))&&
        ((m&2U)!=0U||(p->post_delta==0U&&(m&96U)==0U))&&
        ((m&4U)!=0U||(p->ready_delta==0U&&(m&384U)==0U));
}
static uint32_t Checksum(const uint32_t *words)
{
    uint32_t crc=UINT32_MAX;
    for(unsigned i=0;i<14;i++)for(unsigned byte=0;byte<4;byte++)
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
    USB_POWER_EVIDENCE power={w[10],w[11],w[12],(uint16_t)w[13]};
    if(w[15]!=COMMIT||w[0]!=MAGIC||w[1]!=4U||w[2]==0U||w[3]!=~w[2]||
       w[14]!=Checksum(w)||(w[7]&0xE0000000U)!=0U||w[13]>65535U||!USB_RestartPowerValid(&power)||w[8]>USB_RESTART_PORT_CONFIGURED||(w[9]&0xF8000000U)!=0U||(w[9]&255U)>12U||w[4]==0U||w[5]==0U)return false;
    uint32_t status=w[7];
    if((status&255U)>USB_RESTART_FAILED||((status>>8)&255U)>USB_RESTART_FAILED||
       ((status>>16)&255U)>USB_RESTART_FAULT_EXTERNAL)return false;
    *out=(USB_RESTART_RECORD){.sequence=w[2],.ticket=w[4],.generation=w[5],.time_ms=w[6],.detail=w[9],.power=power,
        .state=(USB_RESTART_STATE)(status&255U),.phase=(USB_RESTART_STATE)((status>>8)&255U),
        .fault=(USB_RESTART_FAULT)((status>>16)&255U),.checkpoint=(USB_RESTART_CHECKPOINT)w[8],
        .evidence=(uint8_t)(status>>24)};
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
       (unsigned)r->fault>USB_RESTART_FAULT_EXTERNAL||(unsigned)r->checkpoint>USB_RESTART_PORT_CONFIGURED||(r->detail&0xF8000000U)!=0U||(r->detail&255U)>12U||(r->evidence&~31U)!=0U||r->generation==0U||r->ticket==0U||!USB_RestartPowerValid(&r->power))return false;
    USB_RESTART_RECORD old;int latest=Latest(j,&old);
    if(latest>=0&&old.sequence==UINT32_MAX)return false; /* No sequence wrap. */
    unsigned slot=latest==0?1U:0U;uint32_t sequence=latest<0?1U:old.sequence+1U;
    uint32_t w[USB_RESTART_JOURNAL_WORDS]={MAGIC,4U,sequence,~sequence,r->ticket,r->generation,r->time_ms,
        (uint32_t)r->state|((uint32_t)r->phase<<8)|((uint32_t)r->fault<<16)|((uint32_t)r->evidence<<24),(uint32_t)r->checkpoint,r->detail,r->power.pre_count,r->power.post_delta,r->power.ready_delta,r->power.meta,0,COMMIT};
    w[14]=Checksum(w);j->slot[slot][15]=0;
    __asm__ volatile("":::"memory"); /* Invalidate old commit before rewriting its volatile words. */
    for(unsigned i=0;i<15;i++)j->slot[slot][i]=w[i];
    /* Compiler ordering protects the volatile transaction. On target, caller
     * uses uncached retained SRAM; this is not nonvolatile persistence. */
    __asm__ volatile("":::"memory");j->slot[slot][15]=COMMIT;return true;
}
