#ifndef UPDATE_LINK_DISPATCH_H
#define UPDATE_LINK_DISPATCH_H
#include "update_lifecycle_wire.h"
typedef enum {
    UPDATE_LINK_OK=0,UPDATE_LINK_BAD_REQUEST,UPDATE_LINK_DISABLED,
    UPDATE_LINK_BUSY,UPDATE_LINK_IDENTITY,UPDATE_LINK_FAILED
} UPDATE_LINK_RESULT;
typedef struct {
    UPDATE_LIFECYCLE core;
    uint32_t send_generation,pending_cookie,pending_ticket;
    bool initialized,pause_pending;
} UPDATE_LINK_DISPATCH;
/* Inactive adapter. Caller supplies fresh boot epoch; enabled means permission
 * in THIS instance (mocks only today). USB transport feature stays false. */
bool UPDATE_LinkDispatchInit(UPDATE_LINK_DISPATCH *d,const UPDATE_FLASH_BACKEND *backend,
                             uint64_t epoch,uint32_t flash_timeout,uint32_t phase_timeout,bool enabled);
/* CRC-validated payload, no CRC bytes. False -> no reply, no mutation.
 * All input/output storage must be distinct; overlaps rejected BEFORE writes.
 * Entire56byte output capacity is required before lifecycle changes.
 * Only an EXECUTE reply entering ACK_WAIT receives nonzero completion cookie;
 * serialise all requests until that report has completed/cancelled. */
bool UPDATE_LinkDispatchRequest(UPDATE_LINK_DISPATCH *d,const uint8_t *payload,size_t size,
                                uint32_t now,uint8_t *reply,size_t capacity,size_t *length,uint32_t *cookie);
/* Only the matching pending full-response completion may authorise off phase.
 * Transport owns actual IRP generation, buffer retention and hardware-off gate. */
void UPDATE_LinkDispatchSent(UPDATE_LINK_DISPATCH *d,uint32_t cookie,bool success,uint32_t now);
UPDATE_LIFECYCLE_STATE UPDATE_LinkDispatchPoll(UPDATE_LINK_DISPATCH *d,uint32_t now,bool buses_inactive);
void UPDATE_LinkDispatchCancel(UPDATE_LINK_DISPATCH *d,uint32_t now);
#endif
