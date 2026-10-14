#ifndef UPDATE_FLASH_H
#define UPDATE_FLASH_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UPDATE_ROW_SIZE 128U
#define UPDATE_ERASE_SIZE 1024U

typedef enum {
    UPDATE_FLASH_IDLE = 0, UPDATE_FLASH_BUSY, UPDATE_FLASH_VERIFY,
    UPDATE_FLASH_OK, UPDATE_FLASH_REJECTED, UPDATE_FLASH_ERROR,
    UPDATE_FLASH_TIMEOUT
} UPDATE_FLASH_STATUS;

typedef struct {
    void *context;
    bool (*busy)(void *context);
    uint32_t (*error)(void *context);
    bool (*erase)(void *context, uint32_t physical_address);
    bool (*write)(void *context, const uint32_t *row, uint32_t physical_address);
    bool (*read)(void *context, uint32_t physical_address, uint8_t *out, size_t size);
} UPDATE_FLASH_BACKEND;

typedef struct {
    /* Private copy must outlive pending hardware operations, even on timeout. */
    _Alignas(16) uint32_t row[UPDATE_ROW_SIZE / sizeof(uint32_t)];
    UPDATE_FLASH_BACKEND backend;
    uint32_t start, end; /* Physical writable interval, end exclusive. */
    uint32_t address, since, timeout_ms, verify_offset;
    UPDATE_FLASH_STATUS status;
    bool erasing;
    bool initialized;
} UPDATE_FLASH;

/* Inert backend adapter, not bound to target PLIBs by current firmware.
 * Call Init only on a new/quiescent instance, never over an active operation. */
bool UPDATE_FlashInit(UPDATE_FLASH *flash, const UPDATE_FLASH_BACKEND *backend,
                      uint32_t start, uint32_t end, uint32_t timeout_ms);
bool UPDATE_FlashErase(UPDATE_FLASH *flash, uint32_t physical_address, uint32_t now_ms);
bool UPDATE_FlashWrite(UPDATE_FLASH *flash, uint32_t physical_address,
                       const uint8_t *row, uint32_t now_ms);
UPDATE_FLASH_STATUS UPDATE_FlashPoll(UPDATE_FLASH *flash, uint32_t now_ms);
/* Reset never cancels hardware: refused while BUSY/VERIFY or backend still busy.
 * Timed-out/aborted operations retain their source buffer until hardware settles. */
bool UPDATE_FlashReset(UPDATE_FLASH *flash);
#endif
