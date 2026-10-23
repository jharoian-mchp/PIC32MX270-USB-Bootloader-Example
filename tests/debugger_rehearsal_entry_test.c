#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include "debugger_rehearsal.h"
#define main RehearsalTestMain
#include "../rehearsal/src/main.c"
#undef main
static unsigned masks,inits,tasks;
static bool init_ok;
static jmp_buf done;
void RAM_RehearsalEntryMask(void){assert(!masks&&!inits&&!tasks);++masks;}
bool RAM_RehearsalEntryMasked(void){assert(masks==1&&!inits);return true;}
bool RAM_RehearsalInit(RAM_REHEARSAL *r,RAM_REHEARSAL_MAILBOX *m)
{assert(masks==1&&!inits&&!tasks&&r==&rehearsal_context&&m==&rehearsal_mailbox);++inits;return init_ok;}
void RAM_RehearsalTasks(RAM_REHEARSAL *r)
{assert(r==&rehearsal_context&&masks==1&&inits==1);if(++tasks==3)longjmp(done,1);}
int main(void)
{
    for(volatile unsigned mode=0;mode<2;mode++){
        masks=inits=tasks=0;init_ok=mode!=0;
        if(!setjmp(done)){(void)RehearsalTestMain();assert(0);}
        assert(masks==1&&inits==1&&tasks==3);
    }
    puts("PASS: exact standalone entry masks once, initializes once and keeps serialized polling after success/failure; hardware mocked, no target link.");
}
