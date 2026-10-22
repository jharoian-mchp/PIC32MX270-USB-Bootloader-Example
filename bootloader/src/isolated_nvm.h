#ifndef ISOLATED_NVM_H
#define ISOLATED_NVM_H
#include "update_flash.h"
/* INACTIVE trusted-owner library. No startup/USB binding or target test image.
 * Init once on new/quiescent, fixed-lifetime storage. Not a mutation protocol.
 * A future target must supply a compiled, approved page (no default), uncached
 * SRAM, actual Count, USB/DMA-off/IRQ/clock checks and reviewed adapter startup.
 * Public C storage is inspectable, NOT a debugger write/authorization API. */
#define ISOLATED_NVM_TOKEN UINT32_C(0x4E564D31)
#define ISOLATED_NVM_COUNT_HZ UINT32_C(24000000)
typedef enum { ISOLATED_READY, ISOLATED_ARMED, ISOLATED_RUNNING,
    ISOLATED_PASS_SETTLED, ISOLATED_FAIL_NO_LAUNCH,
    ISOLATED_FAIL_SETTLED, ISOLATED_FAIL_UNSETTLED } ISOLATED_NVM_STATE;
typedef enum { ISOLATED_PHASE_NONE, ISOLATED_PHASE_INIT, ISOLATED_PHASE_ERASE,
    ISOLATED_PHASE_ERASE_VERIFY, ISOLATED_PHASE_ROW,
    ISOLATED_PHASE_ROW_VERIFY, ISOLATED_PHASE_FINAL_READ,
    ISOLATED_PHASE_SETTLE } ISOLATED_NVM_PHASE;
typedef enum { ISOLATED_FAULT_NONE, ISOLATED_FAULT_TOKEN, ISOLATED_FAULT_GESTURE,
    ISOLATED_FAULT_ARM_TIMEOUT, ISOLATED_FAULT_PREFLIGHT,
    ISOLATED_FAULT_TIMEOUT, ISOLATED_FAULT_BACKEND,
    ISOLATED_FAULT_READBACK, ISOLATED_FAULT_SNAPSHOT } ISOLATED_NVM_FAULT;
/* All report fields are aligned words. token is the only operator-owned word,
 * written only while READY; report is copied using the ordered snapshot API.
 * Sequence is even/stable, odd/update in progress; never wraps. */
typedef struct {
    uint32_t version,state,phase,fault,page,count_hz,arm_attempts;
    uint32_t erase_attempts,row_attempts,anchor,elapsed_ticks,unsafe_bits;
    uint32_t backend_error,verify_offset;
} ISOLATED_NVM_REPORT;
typedef struct {
    volatile uint32_t token,sequence;
    volatile ISOLATED_NVM_REPORT report;
} ISOLATED_NVM_MAILBOX;
typedef struct {
    void *context;
    uint32_t (*count)(void *context);
    /* Zero iff USBPWR=0/BUSY=0, DMA off, IRQ masked and clock preflight valid.
     * Callback must not enable/disable peripherals or mutate owner/mailbox. */
    uint32_t (*unsafe)(void *context);
    /* Invoked ONLY after valid one-shot arming. Return a complete private
     * backend for a newly initialized adapter; no launch inside this callback.
     * erase/write return acceptance only. busy MUST remain true through WR
     * and pending settlement; a false busy result means completion, required
     * settle delay and disarm are complete. error preserves hardware failures.
     * No USB, generated NVM_Initialize, reset or automatic recovery. */
    bool (*initialize)(void *context,UPDATE_FLASH_BACKEND *backend);
} ISOLATED_NVM_PORT;
typedef struct {
    uint32_t page,count_hz,debounce_ticks,arm_window_ticks,operation_ticks;
} ISOLATED_NVM_CONFIG;
typedef struct {
    ISOLATED_NVM_PORT port;
    ISOLATED_NVM_CONFIG config;
    ISOLATED_NVM_MAILBOX *mailbox;
    ISOLATED_NVM_REPORT report;
    UPDATE_FLASH_BACKEND real;
    UPDATE_FLASH flash;
    uint32_t arm_anchor,button_since,publication_sequence;
    bool initialized,spent,button_pressed,button_stable,startup_released,button_seen;
    bool fresh_press,backend_ready,failed,unsettled,snapshot_exhausted,budget_active,erase_verified;
} ISOLATED_NVM;
/* Software init publishes READY and clears token. No port.initialize or flash
 * access. Target lifecycle must prevent reinitializing this object in a boot. */
bool ISOLATED_NvmInit(ISOLATED_NVM *owner,ISOLATED_NVM_MAILBOX *mailbox,
    const ISOLATED_NVM_CONFIG *config,const ISOLATED_NVM_PORT *port);
/* Single serialized main owner, interrupts already masked, no concurrent API.
 * No reset/cancel/rearm operation. One step/chunk per call; blocking callbacks
 * are not preemptible. Count stalls/multiple wraps can defeat elapsed checks. */
void ISOLATED_NvmTasks(ISOLATED_NVM *owner,bool sw1_pressed);
/* Read-only RAM snapshot; no peripheral access. Matching even sequences are
 * consistency only, NOT authentication. Target/debugger ordering must be tested. */
bool ISOLATED_NvmSnapshot(const ISOLATED_NVM_MAILBOX *mailbox,ISOLATED_NVM_REPORT *report);
#endif
