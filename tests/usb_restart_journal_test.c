#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "usb_restart_journal.h"
int main(void)
{
    USB_RESTART_JOURNAL j={0};USB_RESTART_RECORD read;
    assert(!USB_RestartJournalRead(&j,&read));
    USB_RESTART_RECORD a={.ticket=1,.generation=1,.time_ms=20,.state=USB_RESTART_DRAIN,.phase=USB_RESTART_DRAIN,.checkpoint=USB_RESTART_PORT_DRAIN};
    assert(USB_RestartJournalWrite(&j,&a)&&USB_RestartJournalRead(&j,&read));
    assert(read.sequence==1&&read.state==USB_RESTART_DRAIN&&read.checkpoint==USB_RESTART_PORT_DRAIN);
    USB_RESTART_JOURNAL first=j;
    USB_RESTART_RECORD b={.ticket=1,.generation=1,.time_ms=10020,.state=USB_RESTART_FAILED,
        .phase=USB_RESTART_DRAIN,.fault=USB_RESTART_FAULT_TIMEOUT,.checkpoint=USB_RESTART_PORT_DRAIN,.evidence=4};
    assert(USB_RestartJournalWrite(&j,&b)&&USB_RestartJournalRead(&j,&read));
    assert(read.sequence==2&&read.state==USB_RESTART_FAILED&&read.phase==USB_RESTART_DRAIN&&read.fault==USB_RESTART_FAULT_TIMEOUT);
    /* Simulated reset preserves validated RAM but has no operation authority. */
    USB_RESTART_JOURNAL preserved=j;assert(USB_RestartJournalRead(&preserved,&read)&&read.ticket==1);
    /* Every prefix of the new slot remains uncommitted, older slot survives. */
    for(unsigned n=0;n<USB_RESTART_JOURNAL_WORDS;n++)
    {
        USB_RESTART_JOURNAL cut=first;cut.slot[1][11]=0;
        for(unsigned i=0;i<n;i++)cut.slot[1][i]=j.slot[1][i];
        assert(USB_RestartJournalRead(&cut,&read)&&read.sequence==1);
    }
    for(unsigned i=0;i<USB_RESTART_JOURNAL_WORDS;i++)
    {
        USB_RESTART_JOURNAL bad=j;bad.slot[1][i]^=1;
        assert(USB_RestartJournalRead(&bad,&read)&&read.sequence==1);
    }
    memset(&j,0xA5,sizeof(j));assert(!USB_RestartJournalRead(&j,&read));
    assert(USB_RestartJournalWrite(&j,&a));a.state=(USB_RESTART_STATE)-1;
    assert(!USB_RestartJournalWrite(&j,&a));
    assert(!USB_RestartJournalRead(NULL,&read)&&!USB_RestartJournalRead(&j,NULL));
    puts("PASS: retained SRAM journal slot CRC/commit/complement, torn/corrupt record fallback, reset simulation and invalid states; no flash or resume authority.");return 0;
}
