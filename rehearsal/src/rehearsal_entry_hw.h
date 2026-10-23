#ifndef REHEARSAL_ENTRY_HW_H
#define REHEARSAL_ENTRY_HW_H
#include <xc.h>
#include <stdbool.h>
#if !defined(__32MX270F256D__)
#error "rehearsal entry requires PIC32MX270F256D"
#endif
/* One explicit main-entry disable/hazard; no restore API. */
static inline void RAM_RehearsalEntryMask(void)
{
    (void)__builtin_disable_interrupts();
    __asm__ volatile("ehb\n\tsync":::"memory");
}
static inline bool RAM_RehearsalEntryMasked(void){return (__builtin_mfc0(12,0)&1U)==0U;}
#endif
