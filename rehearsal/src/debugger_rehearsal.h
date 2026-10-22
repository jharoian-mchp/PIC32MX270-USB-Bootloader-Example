#ifndef DEBUGGER_REHEARSAL_H
#define DEBUGGER_REHEARSAL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* INACTIVE RAM_REHEARSAL_DEBUG_V1 library. No main, fuse TU, page, owner,
 * Flash/NVM backend, GPIO, interrupt mutation, image or board permission.
 * Trusted serialized caller masks IRQ once and initializes new/quiescent
 * fixed-lifetime disjoint storage ONCE per boot; this is not a reset API.
 * Future final link must prove uncached KSEG1/runtime/executive placement. */
#define RAM_REHEARSAL_CHALLENGE UINT32_C(0x52414D31)
#define RAM_REHEARSAL_GUARD_LOW UINT32_C(0x13579BDF)
#define RAM_REHEARSAL_GUARD_HIGH UINT32_C(0x2468ACE0)
#define RAM_REHEARSAL_EXHAUSTED (UINT32_MAX-1U)
#define RAM_REHEARSAL_OBSERVATIONS 256U
typedef enum { RAM_CHECKING=1,RAM_WAIT_EDIT,RAM_OBSERVING,RAM_COMPLETE,RAM_FAILED } RAM_REHEARSAL_STATE;
typedef enum { RAM_FAULT_NONE,RAM_FAULT_PROFILE,RAM_FAULT_COUNT,
    RAM_FAULT_CHALLENGE,RAM_FAULT_GUARDS,RAM_FAULT_SEQUENCE } RAM_REHEARSAL_FAULT;
typedef struct {
    uint32_t osccon,count,status,cause,debug,pwrc,dma,nvm,cfg0,cfg1,cfg2,cfg3;
} RAM_REHEARSAL_RAW;
typedef struct {
    uint32_t version,mode,state,fault,edit_attempts,observations,challenge_observed;
    uint32_t resume_count_first,resume_count_delta;
    RAM_REHEARSAL_RAW before,after;
} RAM_REHEARSAL_REPORT;
typedef struct {
    volatile uint32_t guard_low,edit_word,guard_high,sequence;
    volatile RAM_REHEARSAL_REPORT report;
} RAM_REHEARSAL_MAILBOX;
typedef struct {
    RAM_REHEARSAL_MAILBOX *mailbox;
    RAM_REHEARSAL_REPORT report;
    RAM_REHEARSAL_RAW denied_raw;
    uint32_t expected_sequence,readout_reason;
    bool initialized,spent,terminal,sequence_invalid,preedit_called;
    bool readout_attempted,readout_denied;
} RAM_REHEARSAL;
_Static_assert(sizeof(RAM_REHEARSAL_RAW)==48U,"raw word layout");
_Static_assert(sizeof(RAM_REHEARSAL_REPORT)==132U,"report word layout");
_Static_assert(sizeof(RAM_REHEARSAL_MAILBOX)==148U,"37-word mailbox layout");
_Static_assert(offsetof(RAM_REHEARSAL_MAILBOX,edit_word)==4U&&
    offsetof(RAM_REHEARSAL_MAILBOX,sequence)==12U&&
    offsetof(RAM_REHEARSAL_MAILBOX,report)==16U,"mailbox offsets");
_Static_assert(offsetof(RAM_REHEARSAL_REPORT,before)==36U&&
    offsetof(RAM_REHEARSAL_REPORT,after)==84U,"raw offsets");
/* Finite read-only preflight, then one pre-edit rendezvous. Caller already
 * masked IRQ; no interrupt writes here. Returns true only for WAIT_EDIT.
 * Does not consume a debugger edit made at the rendezvous; Tasks does that
 * after return, spending before integrity/profile/challenge validation. */
bool RAM_RehearsalInit(RAM_REHEARSAL *rehearsal,RAM_REHEARSAL_MAILBOX *mailbox);
/* Serialized RAM-only polling. Zero edit inert; first nonzero spent once.
 * Post-consumption Count anchor is fresh, observation loop <=256 iterations.
 * Failures terminal; readout is one-shot and conservative full-profile idle.
 * After terminal, only detected sequence corruption may change its validity
 * marker (once). Payload immutable; no rearm/retry/reset or further edit clear. */
void RAM_RehearsalTasks(RAM_REHEARSAL *rehearsal);
/* Guarded ordered read. Invalid/unaligned/aliased/zero/odd/exhausted/changed
 * sequence or guard damage fails without writing output. Evidence only. */
bool RAM_RehearsalSnapshot(const RAM_REHEARSAL_MAILBOX *mailbox,RAM_REHEARSAL_REPORT *report);
/* Inert noinline nop locations; NOT traps or permission to set a breakpoint.
 * Final linked instruction/delay-slot/debugger semantics need separate audit.
 * Trusted caller must not call these directly to bypass private eligibility. */
void RAM_RehearsalPreEdit(void);
void RAM_RehearsalReadout(void);
#endif
