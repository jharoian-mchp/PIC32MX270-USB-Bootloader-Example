#ifndef USB_RESTART_JOURNAL_H
#define USB_RESTART_JOURNAL_H
#include "usb_restart.h"
/* Retained SRAM evidence ONLY, never used to resume or authorise operations.
 * Two checksummed slots committed last. Preservation across MCLR is subject
 * to the target startup/runtime; power loss, programming or random SRAM can
 * invalidate it. No flash/NVM I/O. One owner serializes BOTH reads and writes
 * under IRQ exclusion; validation does not replace concurrent-access locking.
 * Input/output objects must not overlap journal storage. */
#define USB_RESTART_JOURNAL_WORDS 12U
typedef struct {volatile uint32_t slot[2][USB_RESTART_JOURNAL_WORDS];} USB_RESTART_JOURNAL;
typedef struct {
    uint32_t sequence,ticket,generation,time_ms;
    USB_RESTART_STATE state,phase;
    USB_RESTART_FAULT fault;
    uint8_t evidence;
} USB_RESTART_RECORD;
bool USB_RestartJournalRead(const USB_RESTART_JOURNAL *journal,USB_RESTART_RECORD *out);
bool USB_RestartJournalWrite(USB_RESTART_JOURNAL *journal,const USB_RESTART_RECORD *record);
#endif
