#ifndef UPDATE_IMAGE_H
#define UPDATE_IMAGE_H
#include "update_flash.h"
#include "update_protocol.h"
#define UPDATE_IMAGE_META_ADDRESS (UPDATE_FLASH_PHYSICAL_END - UPDATE_ERASE_SIZE)
#define UPDATE_IMAGE_MAX_LENGTH (UPDATE_IMAGE_META_ADDRESS - UPDATE_APP_PHYSICAL_START)

typedef enum { UPDATE_IMAGE_CHECKING=0, UPDATE_IMAGE_VALID,
    UPDATE_IMAGE_INVALID, UPDATE_IMAGE_READ_ERROR } UPDATE_IMAGE_STATUS;
typedef struct {
    UPDATE_FLASH_BACKEND backend;
    uint32_t length, expected_crc, offset, crc;
    UPDATE_IMAGE_STATUS status;
} UPDATE_IMAGE_CHECK;
/* Standard reflected CRC32: init=FFFFFFFF, final XOR=FFFFFFFF. */
uint32_t UPDATE_Crc32Step(uint32_t state, const uint8_t *data, size_t size);
bool UPDATE_ImageParameters(uint32_t length);
void UPDATE_ImageRecord(uint8_t row[UPDATE_ROW_SIZE], uint32_t length, uint32_t crc);
/* Precommit check against host-declared length/CRC; does not write metadata. */
bool UPDATE_ImageCheckExpected(UPDATE_IMAGE_CHECK *check,
                               const UPDATE_FLASH_BACKEND *backend,
                               uint32_t length, uint32_t crc);
/* Boot-time acceptance starts by validating stored metadata. No jump performed. */
UPDATE_IMAGE_STATUS UPDATE_ImageCheckStored(UPDATE_IMAGE_CHECK *check,
                                           const UPDATE_FLASH_BACKEND *backend);
/* At most one 128-byte read per call; retry after backend busy, no busy loop. */
UPDATE_IMAGE_STATUS UPDATE_ImageCheckPoll(UPDATE_IMAGE_CHECK *check);
#endif
