#ifndef USB_RESTART_OWNER_H
#define USB_RESTART_OWNER_H
#include "usb_retirement.h"
/* Explicit replacement of board USB ownership; OFF in production/default. */
#ifndef USB_RESTART_DIAGNOSTIC
#define USB_RESTART_DIAGNOSTIC 0
#endif
#if USB_RESTART_DIAGNOSTIC && !USB_RETIREMENT_HOOKS
#error "USB_RESTART_DIAGNOSTIC requires retirement hooks"
#endif
#if USB_RETIREMENT_HOOKS
#include "definitions.h"
#include "usb_restart.h"
#include "usb_restart_journal.h"
typedef struct {
    USB_RESTART_STATE state;
    uint32_t generation,completed_cycles,stale_events,cancel_errors;
    bool configured,exclusive,attached;
} USB_RESTART_OWNER_DIAGNOSTICS;
extern volatile USB_RESTART_OWNER_DIAGNOSTICS usb_restart_owner_diagnostics;
/* Uncalled default-off target binding. Caller must stop the old board USB
 * tasks/attach/event paths permanently before Init. Owner handles USB+probe,
 * but ADC/mux remain caller-owned; pass qualified cable/mux state to Tasks.
 * Initial handle must be valid/new with no pending probe report transfers. */
bool USB_RestartOwnerInit(USB_DEVICE_HANDLE handle,uint32_t timeout_ms);
bool USB_RestartOwnerRequest(uint32_t now,uint32_t *ticket);
/* Caller confirms full LOCAL read-only pause response completed. No on-wire
 * restart opcode or report delivery is implemented by this owner. */
void USB_RestartOwnerAck(uint32_t ticket,bool success,uint32_t now);
/* Diagnostic opcode66 adapter: 0=new pause, 1=exact latest retry (no new
 * completion authority), 2=rejected. IDs increase, no wrap. Cache survives
 * USB-only rebuild, not MCU reset. Output is separate caller storage. */
unsigned USB_RestartOwnerPause(uint32_t id,uint32_t generation,uint32_t now,uint32_t *ticket);
void USB_RestartOwnerStatus(uint32_t *ticket,uint32_t *generation);
/* Previous validated retained-RAM record captured at owner initialization;
 * not updated by current operation, and never consumed as authorization. */
bool USB_RestartOwnerPrevious(USB_RESTART_RECORD *record);
void USB_RestartOwnerTasks(uint32_t now,bool cable_present,bool mux_ready);
#endif
#endif
