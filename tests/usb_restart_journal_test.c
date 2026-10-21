#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "usb_restart_journal.h"
static uint32_t Crc(const volatile uint32_t *words)
{
    uint32_t crc=UINT32_MAX;for(unsigned i=0;i<14;i++)for(unsigned byte=0;byte<4;byte++) {
        crc^=(words[i]>>(8U*byte))&255U;for(unsigned bit=0;bit<8;bit++)crc=(crc>>1)^((crc&1U)?0xEDB88320U:0U);
    }return crc^UINT32_MAX;
}
int main(void)
{
    USB_RESTART_JOURNAL j={0};USB_RESTART_RECORD read;
    assert(!USB_RestartJournalRead(&j,&read));
    USB_RESTART_RECORD a={.ticket=1,.generation=1,.time_ms=20,.state=USB_RESTART_DRAIN,.phase=USB_RESTART_DRAIN,.checkpoint=USB_RESTART_PORT_DRAIN};
    assert(USB_RestartJournalWrite(&j,&a)&&USB_RestartJournalRead(&j,&read));
    assert(read.sequence==1&&read.state==USB_RESTART_DRAIN&&read.checkpoint==USB_RESTART_PORT_DRAIN);
    USB_RESTART_JOURNAL first=j;
    USB_RESTART_RECORD b={.ticket=1,.generation=1,.time_ms=10020,.state=USB_RESTART_FAILED,
        .phase=USB_RESTART_DRAIN,.fault=USB_RESTART_FAULT_TIMEOUT,.checkpoint=USB_RESTART_PORT_DRAIN,.detail=11U|(1024U<<8)|(3U<<24),.evidence=4,.power={.pre_count=UINT32_MAX-5,.post_delta=10,.ready_delta=20,.meta=487}};
    assert(USB_RestartJournalWrite(&j,&b)&&USB_RestartJournalRead(&j,&read));
    assert(read.sequence==2&&read.state==USB_RESTART_FAILED&&read.phase==USB_RESTART_DRAIN&&read.fault==USB_RESTART_FAULT_TIMEOUT&&read.detail==b.detail&&read.power.meta==487&&read.power.post_delta==10&&read.power.ready_delta==20);
    /* Simulated reset preserves validated RAM but has no operation authority. */
    USB_RESTART_JOURNAL preserved=j;assert(USB_RestartJournalRead(&preserved,&read)&&read.ticket==1);
    /* Every prefix of the new slot remains uncommitted, older slot survives. */
    for(unsigned n=0;n<USB_RESTART_JOURNAL_WORDS;n++)
    {
        USB_RESTART_JOURNAL cut=first;cut.slot[1][15]=0;
        for(unsigned i=0;i<n;i++)cut.slot[1][i]=j.slot[1][i];
        assert(USB_RestartJournalRead(&cut,&read)&&read.sequence==1);
    }
    for(unsigned i=0;i<USB_RESTART_JOURNAL_WORDS;i++)
    {
        USB_RESTART_JOURNAL bad=j;bad.slot[1][i]^=1;
        assert(USB_RestartJournalRead(&bad,&read)&&read.sequence==1);
    }
    /* CRC-valid older schema, reserved metadata/evidence, and exhausted sequence. */
    for(unsigned version=1;version<=3;version++) {
        USB_RESTART_JOURNAL bad=first;bad.slot[0][1]=version;bad.slot[0][14]=Crc(bad.slot[0]);
        assert(!USB_RestartJournalRead(&bad,&read));
    }
    for(unsigned value=0;value<65536;value++) {
        USB_POWER_EVIDENCE p={.meta=(uint16_t)value};
        bool expected=(value&~511U)==0U&&(!(value&2U)||(value&1U))&&(!(value&4U)||((value&3U)==3U))&&
            ((value&1U)||!(value&24U))&&((value&2U)||!(value&96U))&&((value&4U)||!(value&384U));
        assert(USB_RestartPowerValid(&p)==expected);
    }
    USB_RESTART_JOURNAL bad=first;bad.slot[0][13]=512;bad.slot[0][14]=Crc(bad.slot[0]);assert(!USB_RestartJournalRead(&bad,&read));
    bad=first;bad.slot[0][7]|=0x20000000U;bad.slot[0][14]=Crc(bad.slot[0]);assert(!USB_RestartJournalRead(&bad,&read));
    bad=first;bad.slot[0][2]=UINT32_MAX;bad.slot[0][3]=0;bad.slot[0][14]=Crc(bad.slot[0]);assert(!USB_RestartJournalWrite(&bad,&a));
    /* Missing samples differ from valid sampled zeros; time0 is valid identity. */
    USB_RESTART_RECORD zero=a;zero.time_ms=0;zero.power=(USB_POWER_EVIDENCE){.meta=7};
    assert(USB_RestartJournalWrite(&j,&zero)&&USB_RestartJournalRead(&j,&read)&&read.power.meta==7&&read.time_ms==0);
    zero.power=(USB_POWER_EVIDENCE){.post_delta=1};assert(!USB_RestartJournalWrite(&j,&zero));
    zero.power=(USB_POWER_EVIDENCE){0};zero.evidence=32;assert(!USB_RestartJournalWrite(&j,&zero));
    memset(&j,0xA5,sizeof(j));assert(!USB_RestartJournalRead(&j,&read));
    assert(USB_RestartJournalWrite(&j,&a));a.state=(USB_RESTART_STATE)-1;
    assert(!USB_RestartJournalWrite(&j,&a));
    a.state=USB_RESTART_DRAIN;a.detail=13U;assert(!USB_RestartJournalWrite(&j,&a));
    a.detail=0xF8000000U;assert(!USB_RestartJournalWrite(&j,&a));
    assert(!USB_RestartJournalRead(NULL,&read)&&!USB_RestartJournalRead(&j,NULL));
    puts("PASS: retained SRAM journal slot CRC/commit/complement, torn/corrupt record fallback, reset simulation and invalid states; no flash or resume authority.");return 0;
}
