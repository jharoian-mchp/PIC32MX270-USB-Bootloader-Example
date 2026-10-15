#ifndef UPDATE_DISPATCH_H
#define UPDATE_DISPATCH_H
#include "update_session.h"
#include "update_wire.h"

#define UPDATE_WIRE_BEGIN 0x62U
#define UPDATE_WIRE_BATCH 0x63U
#define UPDATE_WIRE_ABORT 0x64U
#define UPDATE_WIRE_BATCH_MAX (UPDATE_PAYLOAD_MAX-14U)
#define UPDATE_WIRE_STATE_SIZE 20U
#define UPDATE_WIRE_REPLY_SIZE (8U+UPDATE_WIRE_STATE_SIZE)

typedef enum {
    UPDATE_REPLY_OK=0, UPDATE_REPLY_BAD_REQUEST, UPDATE_REPLY_DISABLED,
    UPDATE_REPLY_BUSY, UPDATE_REPLY_TOKEN, UPDATE_REPLY_STALE,
    UPDATE_REPLY_CONFLICT, UPDATE_REPLY_SEQUENCE, UPDATE_REPLY_FAILED
} UPDATE_REPLY;
typedef struct {
    UPDATE_SESSION session;
    uint8_t last_request[UPDATE_PAYLOAD_MAX];
    size_t last_size;
    uint32_t token, last_id, next_sequence;
    UPDATE_REPLY last_result;
    bool initialized, enabled;
} UPDATE_DISPATCH;

/* Library only: no caller in diagnostic firmware; permission is explicit.
 * Initialise only new/quiescent storage, never over a live backend buffer. */
bool UPDATE_DispatchInit(UPDATE_DISPATCH *d,const UPDATE_FLASH_BACKEND *backend,
                         uint32_t flash_timeout_ms,bool enabled);
/* Input is a CRC-validated payload (CRC removed), not an arbitrary HID report.
 * False means no reply: malformed envelope, unknown opcode or short output.
 * Output capacity is checked BEFORE any side effect. Stop-and-wait; cache only
 * the latest accepted mutation, compare all bytes for exact retry. */
bool UPDATE_DispatchRequest(UPDATE_DISPATCH *d,const uint8_t *request,size_t size,
                            uint32_t now_ms,uint8_t *reply,size_t capacity,size_t *length);
UPDATE_SESSION_STATUS UPDATE_DispatchPoll(UPDATE_DISPATCH *d,uint32_t now_ms);
/* Transport owner must call this on link loss, then keep polling until settled.
 * Completed commits survive link loss; incomplete sessions invalidate metadata. */
void UPDATE_DispatchDisconnect(UPDATE_DISPATCH *d);
#endif
