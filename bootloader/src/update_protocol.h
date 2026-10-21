#ifndef UPDATE_PROTOCOL_H
#define UPDATE_PROTOCOL_H

/* User-owned pure validation layer. No USB, NVM or application-jump operations.
 * Validation is NOT authorisation to execute a command. Not yet integrated. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UPDATE_FRAME_CAPACITY 512U /* Decoded command plus two CRC bytes. */
#define UPDATE_PAYLOAD_MAX (UPDATE_FRAME_CAPACITY - 2U)
#if USB_RESTART_DEBUG_LAYOUT
#include "usb_restart_debug_layout.h"
#define UPDATE_APP_PHYSICAL_START USB_RESTART_DEBUG_APP_ORIGIN
#else
#define UPDATE_APP_PHYSICAL_START UINT32_C(0x1D008000)
#endif
#define UPDATE_FLASH_PHYSICAL_END UINT32_C(0x1D040000) /* Exclusive. */

typedef enum
{
    UPDATE_INPUT_IDLE = 0,
    UPDATE_INPUT_PENDING,
    UPDATE_FRAME_READY,
    UPDATE_FRAME_CRC_ERROR,
    UPDATE_FRAME_TOO_LARGE,
    UPDATE_FRAME_MALFORMED,
    UPDATE_FRAME_BUSY,
    UPDATE_INVALID_ARGUMENT
} UPDATE_FRAME_STATUS;

typedef struct
{
    uint8_t bytes[UPDATE_FRAME_CAPACITY];
    size_t length; /* Includes CRC while receiving; excludes CRC when ready. */
    bool receiving;
    bool escaped;
    bool ready;
} UPDATE_FRAME;

void UPDATE_FrameReset(UPDATE_FRAME *frame);
UPDATE_FRAME_STATUS UPDATE_FramePush(UPDATE_FRAME *frame, uint8_t byte);
/* Stops at first ready/error. consumed includes the offending byte for errors,
 * but is zero if a completed frame has not been reset by the caller. */
UPDATE_FRAME_STATUS UPDATE_FrameFeed(UPDATE_FRAME *frame, const uint8_t *input,
                                    size_t length, size_t *consumed);
/* Separate input/output buffers required. Exact capacity is checked first;
 * insufficient capacity writes nothing and returns false with written=0. */
bool UPDATE_FrameEncode(const uint8_t *payload, size_t length,
                        uint8_t *output, size_t capacity, size_t *written);
uint16_t UPDATE_Crc16(const uint8_t *data, size_t length);

/* Accept physical, KSEG0 or KSEG1 aliases of application program flash only.
 * Reject empty spans and any end crossing; never dereference the address. */
bool UPDATE_ApplicationSpan(uint32_t address, uint32_t length,
                            uint32_t *physical_address);

typedef enum
{
    UPDATE_READ_BOOT_INFO = 1,
    UPDATE_ERASE_FLASH = 2,
    UPDATE_PROGRAM_FLASH = 3,
    UPDATE_READ_CRC = 4,
    UPDATE_JUMP_TO_APP = 5
} UPDATE_COMMAND_ID;

typedef enum
{
    UPDATE_COMMAND_VALID = 0,
    UPDATE_COMMAND_BAD_LENGTH,
    UPDATE_COMMAND_UNSUPPORTED,
    UPDATE_COMMAND_BAD_RANGE,
    UPDATE_COMMAND_BAD_HEX
} UPDATE_COMMAND_STATUS;

typedef struct
{
    UPDATE_COMMAND_ID id;
    const uint8_t *hex_records; /* Borrowed from frame until caller resets it. */
    size_t hex_length;
    uint32_t physical_address;
    uint32_t length;
} UPDATE_COMMAND;

/* CRC already stripped/verified by frame decoder. Checks command length and
 * READ_CRC range; PROGRAM checks HEX shape/checksum only, NOT final addresses,
 * row ordering, session state or image validity. No command is executed. */
UPDATE_COMMAND_STATUS UPDATE_CommandValidate(const uint8_t *payload, size_t length,
                                            UPDATE_COMMAND *command);
#endif
