/* Bounded AN1388-style framing/command validation. User-owned; inert until
 * a future, separately verified updater explicitly uses this layer. */
#include "update_protocol.h"
#include <string.h>

#define SOH 0x01U
#define EOT 0x04U
#define DLE 0x10U

uint16_t UPDATE_Crc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0;
    if (data == NULL && length != 0U) return 0;
    for (size_t i = 0; i < length; ++i)
    {
        crc ^= (uint16_t)((uint16_t)data[i] << 8);
        for (unsigned bit = 0; bit < 8U; ++bit)
            crc = (uint16_t)((crc << 1) ^ ((crc & 0x8000U) ? 0x1021U : 0U));
    }
    return crc;
}

void UPDATE_FrameReset(UPDATE_FRAME *frame)
{
    if (frame != NULL)
    {
        frame->length = 0;
        frame->receiving = false;
        frame->escaped = false;
        frame->ready = false;
    }
}

UPDATE_FRAME_STATUS UPDATE_FramePush(UPDATE_FRAME *frame, uint8_t byte)
{
    if (frame == NULL) return UPDATE_INVALID_ARGUMENT;
    if (frame->ready) return UPDATE_FRAME_BUSY;
    if (frame->escaped)
    {
        frame->escaped = false;
        /* Only framing bytes need escaping in the specified wire format. */
        if (byte != SOH && byte != EOT && byte != DLE)
        {
            UPDATE_FrameReset(frame);
            return UPDATE_FRAME_MALFORMED;
        }
    }
    else
    {
        if (byte == SOH)
        {
            UPDATE_FrameReset(frame);
            frame->receiving = true;
            return UPDATE_INPUT_PENDING; /* Resynchronise an unfinished frame. */
        }
        if (!frame->receiving) return UPDATE_INPUT_IDLE; /* HID padding/noise. */
        if (byte == DLE)
        {
            frame->escaped = true;
            return UPDATE_INPUT_PENDING;
        }
        if (byte == EOT)
        {
            if (frame->length < 3U) /* At least command plus CRC16. */
            {
                UPDATE_FrameReset(frame);
                return UPDATE_FRAME_MALFORMED;
            }
            size_t payload_length = frame->length - 2U;
            uint16_t expected = (uint16_t)(frame->bytes[payload_length] |
                ((uint16_t)frame->bytes[payload_length + 1U] << 8));
            if (UPDATE_Crc16(frame->bytes, payload_length) != expected)
            {
                UPDATE_FrameReset(frame);
                return UPDATE_FRAME_CRC_ERROR;
            }
            frame->length = payload_length;
            frame->ready = true;
            frame->receiving = false;
            return UPDATE_FRAME_READY;
        }
    }
    if (frame->length >= UPDATE_FRAME_CAPACITY)
    {
        UPDATE_FrameReset(frame);
        return UPDATE_FRAME_TOO_LARGE;
    }
    frame->bytes[frame->length++] = byte;
    return UPDATE_INPUT_PENDING;
}

UPDATE_FRAME_STATUS UPDATE_FrameFeed(UPDATE_FRAME *frame, const uint8_t *input,
                                    size_t length, size_t *consumed)
{
    if (consumed != NULL) *consumed = 0;
    if (frame == NULL || consumed == NULL || (input == NULL && length != 0U))
        return UPDATE_INVALID_ARGUMENT;
    if (frame->ready) return UPDATE_FRAME_BUSY;
    UPDATE_FRAME_STATUS status = frame->receiving ? UPDATE_INPUT_PENDING : UPDATE_INPUT_IDLE;
    for (size_t i = 0; i < length; ++i)
    {
        status = UPDATE_FramePush(frame, input[i]);
        *consumed = i + 1U;
        if (status >= UPDATE_FRAME_READY) return status;
    }
    return status;
}

static bool NeedsEscape(uint8_t byte)
{
    return byte == SOH || byte == EOT || byte == DLE;
}

bool UPDATE_FrameEncode(const uint8_t *payload, size_t length,
                        uint8_t *output, size_t capacity, size_t *written)
{
    if (written != NULL) *written = 0;
    if (payload == NULL || output == NULL || written == NULL ||
        length == 0U || length > UPDATE_PAYLOAD_MAX) return false;
    uint16_t crc = UPDATE_Crc16(payload, length);
    size_t needed = 2U; /* SOH/EOT. Bounded length makes arithmetic safe. */
    for (size_t i = 0; i < length + 2U; ++i)
    {
        uint8_t byte = i < length ? payload[i] :
            (i == length ? (uint8_t)crc : (uint8_t)(crc >> 8));
        needed += NeedsEscape(byte) ? 2U : 1U;
    }
    if (capacity < needed) return false;
    size_t pos = 0;
    output[pos++] = SOH;
    for (size_t i = 0; i < length + 2U; ++i)
    {
        uint8_t byte = i < length ? payload[i] :
            (i == length ? (uint8_t)crc : (uint8_t)(crc >> 8));
        if (NeedsEscape(byte)) output[pos++] = DLE;
        output[pos++] = byte;
    }
    output[pos++] = EOT;
    *written = pos;
    return true;
}

bool UPDATE_ApplicationSpan(uint32_t address, uint32_t length,
                            uint32_t *physical_address)
{
    uint32_t physical;
    if (physical_address != NULL) *physical_address = 0;
    if (address >= UINT32_C(0x1D000000) && address < UPDATE_FLASH_PHYSICAL_END)
        physical = address;
    else if (address >= UINT32_C(0x9D000000) && address < UINT32_C(0x9D040000))
        physical = address - UINT32_C(0x80000000);
    else if (address >= UINT32_C(0xBD000000) && address < UINT32_C(0xBD040000))
        physical = address - UINT32_C(0xA0000000);
    else return false;
    if (physical < UPDATE_APP_PHYSICAL_START || length == 0U ||
        length > UPDATE_FLASH_PHYSICAL_END - physical) return false;
    if (physical_address != NULL) *physical_address = physical;
    return true;
}

static uint32_t ReadLE32(const uint8_t *data)
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

static bool HexShapeValid(const uint8_t *data, size_t length)
{
    size_t pos = 0;
    bool eof = false;
    while (pos < length)
    {
        if (eof || length - pos < 5U) return false;
        const uint8_t *record = data + pos;
        size_t record_length = (size_t)record[0] + 5U;
        if (record_length > length - pos) return false;
        uint8_t sum = 0;
        for (size_t i = 0; i < record_length; ++i) sum = (uint8_t)(sum + record[i]);
        if (sum != 0U) return false;
        switch (record[3])
        {
            case 0: if (record[0] == 0U) return false; break;
            case 1: if (record[0] != 0U || record[1] || record[2]) return false; eof = true; break;
            case 2:
            case 4: if (record[0] != 2U || record[1] || record[2]) return false; break;
            case 5: if (record[0] != 4U || record[1] || record[2]) return false; break;
            default: return false;
        }
        pos += record_length;
    }
    return pos != 0U;
}

UPDATE_COMMAND_STATUS UPDATE_CommandValidate(const uint8_t *payload, size_t length,
                                            UPDATE_COMMAND *command)
{
    if (command != NULL) *command = (UPDATE_COMMAND){0};
    if (payload == NULL || command == NULL || length == 0U || length > UPDATE_PAYLOAD_MAX)
        return UPDATE_COMMAND_BAD_LENGTH;
    UPDATE_COMMAND parsed = { .id = (UPDATE_COMMAND_ID)payload[0] };
    switch (payload[0])
    {
        case UPDATE_READ_BOOT_INFO:
        case UPDATE_ERASE_FLASH:
        case UPDATE_JUMP_TO_APP:
            if (length != 1U) return UPDATE_COMMAND_BAD_LENGTH;
            break;
        case UPDATE_PROGRAM_FLASH:
            if (length < 6U) return UPDATE_COMMAND_BAD_LENGTH;
            if (!HexShapeValid(payload + 1U, length - 1U)) return UPDATE_COMMAND_BAD_HEX;
            parsed.hex_records = payload + 1U;
            parsed.hex_length = length - 1U;
            break;
        case UPDATE_READ_CRC:
            if (length != 9U) return UPDATE_COMMAND_BAD_LENGTH;
            parsed.length = ReadLE32(payload + 5U);
            if (!UPDATE_ApplicationSpan(ReadLE32(payload + 1U), parsed.length,
                                        &parsed.physical_address))
                return UPDATE_COMMAND_BAD_RANGE;
            break;
        default: return UPDATE_COMMAND_UNSUPPORTED;
    }
    *command = parsed;
    return UPDATE_COMMAND_VALID;
}
