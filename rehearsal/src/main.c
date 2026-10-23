#include "debugger_rehearsal.h"
#include <rehearsal_entry_hw.h>
/* Source/compile-only entry. No project/startup binding or linked image.
 * Final runtime/map must prove these initialized objects are uncached,
 * disjoint from stack/heap/executive, and never reinitialized in a boot. */
static RAM_REHEARSAL rehearsal_context __attribute__((used,aligned(4)));
static RAM_REHEARSAL_MAILBOX rehearsal_mailbox __attribute__((used,aligned(4)));
int main(void)
{
    RAM_RehearsalEntryMask(); /* Once, immediately after reviewed C startup. */
    if(!RAM_RehearsalEntryMasked())for(;;) { /* No fix/re-mask/retry. */ }
    (void)RAM_RehearsalInit(&rehearsal_context,&rehearsal_mailbox);
    for(;;)RAM_RehearsalTasks(&rehearsal_context);
}
