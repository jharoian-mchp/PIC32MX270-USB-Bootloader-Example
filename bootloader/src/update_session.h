#ifndef UPDATE_SESSION_H
#define UPDATE_SESSION_H
#include "update_hex.h"
#include "update_image.h"
#define UPDATE_BATCH_MAX (UPDATE_PAYLOAD_MAX-1U)
/* One 255-byte record plus one pending partial row can emit at most 3 rows. */
#define UPDATE_ROW_QUEUE 3U

typedef enum {
    UPDATE_SESSION_IDLE=0, UPDATE_SESSION_ERASING_META, UPDATE_SESSION_ERASING_APP,
    UPDATE_SESSION_READY, UPDATE_SESSION_PROGRAMMING, UPDATE_SESSION_EOF,
    UPDATE_SESSION_VERIFYING, UPDATE_SESSION_COMMITTING, UPDATE_SESSION_COMPLETE,
    UPDATE_SESSION_FAILED, UPDATE_SESSION_ABORTING, UPDATE_SESSION_ABORTED
} UPDATE_SESSION_STATUS;

typedef struct {
    uint8_t bytes[UPDATE_ROW_SIZE];uint32_t address;
} UPDATE_QUEUED_ROW;
typedef struct {
    UPDATE_FLASH flash;
    UPDATE_HEX hex;
    UPDATE_IMAGE_CHECK check;
    uint8_t batch[UPDATE_BATCH_MAX];
    UPDATE_QUEUED_ROW queue[UPDATE_ROW_QUEUE];
    size_t batch_size, batch_offset;
    uint32_t length, expected_crc, erase_address, erase_end, operation_since, timeout_ms;
    uint8_t queued, queue_index;
    bool operation_active, initialized;
    UPDATE_SESSION_STATUS status;
} UPDATE_SESSION;

/* Init only new/quiescent instance. No target backend installed by firmware. */
bool UPDATE_SessionInit(UPDATE_SESSION *s,const UPDATE_FLASH_BACKEND *backend,uint32_t timeout_ms);
/* Supplied length/CRC are a manifest prerequisite, not part of old AN1388
 * command 2. Host/command integration must define their transport explicitly. */
bool UPDATE_SessionBegin(UPDATE_SESSION *s,uint32_t length,uint32_t crc,uint32_t now_ms);
bool UPDATE_SessionSubmit(UPDATE_SESSION *s,const uint8_t *records,size_t size);
UPDATE_SESSION_STATUS UPDATE_SessionPoll(UPDATE_SESSION *s,uint32_t now_ms);
/* Abort retains in-flight write buffer; waits for backend idle, then erases
 * metadata again. A prior completed image is invalidated by a requested abort. */
void UPDATE_SessionAbort(UPDATE_SESSION *s);
#endif
