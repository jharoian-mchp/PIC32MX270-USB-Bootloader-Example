#ifndef ISOLATED_NVM_TARGET_H
#define ISOLATED_NVM_TARGET_H
#include "isolated_nvm.h"
#include "update_nvm.h"
/* INACTIVE port library: never called by current firmware. No target main,
 * page default, image or board permission. New/quiescent static storage only;
 * caller establishes IRQ exclusion and calls preflight once per boot. */
#define ISOLATED_TARGET_OBSERVATIONS 256U
typedef enum { ISOLATED_TARGET_PRE_INIT,ISOLATED_TARGET_INITIALIZED,
    ISOLATED_TARGET_INIT_FAILED } ISOLATED_TARGET_LIFECYCLE;
enum {
    ISOLATED_TARGET_USB=1U,ISOLATED_TARGET_DMA=2U,ISOLATED_TARGET_IRQ=4U,
    ISOLATED_TARGET_CLOCK=8U,ISOLATED_TARGET_COUNT=16U,ISOLATED_TARGET_DEBUG=32U,
    ISOLATED_TARGET_NVM=64U,ISOLATED_TARGET_GPIO=128U,ISOLATED_TARGET_STATE=256U
};
typedef struct {
    uint32_t osccon,count,status,cause,debug,pwrc,dma,nvm,cfg0,cfg1,cfg2,cfg3;
    uint32_t anselb,trisb,pullup,pulldown;
} ISOLATED_TARGET_RAW;
typedef struct {
    uint32_t version,state,unsafe_bits,observations,count_first,count_last;
    ISOLATED_TARGET_RAW raw;
} ISOLATED_TARGET_REPORT;
typedef struct {
    volatile uint32_t sequence;
    volatile ISOLATED_TARGET_REPORT report;
} ISOLATED_TARGET_MAILBOX;
typedef struct {
    UPDATE_NVM adapter;
    ISOLATED_TARGET_MAILBOX *mailbox;
    ISOLATED_TARGET_REPORT report;
    uint32_t publication_sequence,unsafe_bits,initialize_attempts;
    ISOLATED_TARGET_LIFECYCLE lifecycle;
    bool prepared;
} ISOLATED_NVM_TARGET;
/* Read-only status observations plus minimal ANSELB2/TRISB2 input setup.
 * No NVM initialization/writes. Failures terminal; report states 1=checking,
 * 2=ready, 3=failed. Every payload update has odd/even sync publication.
 * Reject aliasing; caller prevents live-object reinitialization. */
bool ISOLATED_TargetPreflight(ISOLATED_NVM_TARGET *target,ISOLATED_TARGET_MAILBOX *mailbox);
/* Construct private owner callbacks only after successful preflight. No page
 * or owner initialization: the future entrypoint must supply an approved page.
 * Port callbacks do not reenter owner, force-clear hardware or restore IRQs. */
bool ISOLATED_TargetPort(ISOLATED_NVM_TARGET *target,ISOLATED_NVM_PORT *port);
bool ISOLATED_TargetSnapshot(const ISOLATED_TARGET_MAILBOX *mailbox,ISOLATED_TARGET_REPORT *report);
/* Raw input sample only; future main is not supplied. */
bool ISOLATED_TargetPressed(void);
#endif
