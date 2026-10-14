#ifndef UPDATE_HEX_H
#define UPDATE_HEX_H
#include "update_flash.h"
#include "update_protocol.h"

typedef enum {
    UPDATE_HEX_ACCEPTED = 0, UPDATE_HEX_FINISHED, UPDATE_HEX_BAD_RECORD,
    UPDATE_HEX_BAD_ADDRESS, UPDATE_HEX_BAD_ORDER, UPDATE_HEX_SINK_ERROR,
    UPDATE_HEX_NOT_ACTIVE
} UPDATE_HEX_STATUS;

/* The sink must copy/stage or synchronously verify a complete row BEFORE
 * returning true. It must not retain this pointer. Async hardware orchestration
 * is deliberately NOT wired to this pure row generator yet. */
typedef bool (*UPDATE_ROW_SINK)(void *context, uint32_t physical_address,
                               const uint8_t row[UPDATE_ROW_SIZE]);

typedef struct {
    uint8_t row[UPDATE_ROW_SIZE];
    uint32_t end, data_end, extension, last_end, row_address, data_bytes, rows_emitted;
    UPDATE_ROW_SINK sink;
    void *context;
    bool row_pending, have_data, start_seen, active, eof;
    UPDATE_HEX_STATUS status;
} UPDATE_HEX;

/* Begin only after caller establishes an erased writable range. Does not erase
 * or certify flash. end may restrict space reserved for future image metadata. */
bool UPDATE_HexBegin(UPDATE_HEX *hex, uint32_t end,
                     UPDATE_ROW_SINK sink, void *context);
/* Manifest-limited image: data restricted to exact length, padded final row
 * may extend to the next row boundary. Never crosses physical flash end. */
bool UPDATE_HexBeginImage(UPDATE_HEX *hex, uint32_t length,
                          UPDATE_ROW_SINK sink, void *context);
/* Nonmutating whole-batch preflight used before asynchronous record staging. */
UPDATE_HEX_STATUS UPDATE_HexCheck(const UPDATE_HEX *hex,
                                 const uint8_t *records, size_t size);
/* Whole batch structurally/address/order validated before any sink call.
 * On failure session is poisoned; abort and erase/restart before reuse.
 * EOF means row stream finished, NOT that an application is valid to run. */
UPDATE_HEX_STATUS UPDATE_HexFeed(UPDATE_HEX *hex, const uint8_t *records, size_t size);
void UPDATE_HexAbort(UPDATE_HEX *hex);
#endif
