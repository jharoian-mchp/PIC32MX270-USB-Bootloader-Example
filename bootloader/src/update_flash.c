#include "update_flash.h"
#include "update_protocol.h"
#include <string.h>

static bool SpanValid(const UPDATE_FLASH *f, uint32_t address, uint32_t size)
{
    return f != NULL && f->initialized && address >= f->start &&
           address < f->end && size <= f->end - address;
}

bool UPDATE_FlashInit(UPDATE_FLASH *f, const UPDATE_FLASH_BACKEND *backend,
                      uint32_t start, uint32_t end, uint32_t timeout_ms)
{
    if (f == NULL || backend == NULL || backend->busy == NULL || backend->error == NULL ||
        backend->erase == NULL || backend->write == NULL || backend->read == NULL ||
        start < UPDATE_APP_PHYSICAL_START || end > UPDATE_FLASH_PHYSICAL_END ||
        end <= start || start % UPDATE_ERASE_SIZE || end % UPDATE_ERASE_SIZE ||
        timeout_ms == 0U || timeout_ms > UINT32_C(0x7FFFFFFF)) return false;
    *f = (UPDATE_FLASH){ .backend = *backend, .start = start, .end = end,
                        .timeout_ms = timeout_ms, .status = UPDATE_FLASH_IDLE,
                        .initialized = true };
    return true;
}

static bool CanStart(UPDATE_FLASH *f, uint32_t address, uint32_t size)
{
    if (!SpanValid(f, address, size) || f->status != UPDATE_FLASH_IDLE ||
        f->backend.busy(f->backend.context)) return false;
    if (f->backend.error(f->backend.context) != 0U)
    {
        f->status = UPDATE_FLASH_ERROR;
        return false;
    }
    return true;
}

bool UPDATE_FlashErase(UPDATE_FLASH *f, uint32_t address, uint32_t now)
{
    if (address % UPDATE_ERASE_SIZE || !CanStart(f, address, UPDATE_ERASE_SIZE)) return false;
    f->address = address;
    f->since = now;
    f->verify_offset = 0;
    f->erasing = true;
    bool accepted = f->backend.erase(f->backend.context, address);
    f->status = accepted ? UPDATE_FLASH_BUSY : UPDATE_FLASH_REJECTED;
    return accepted;
}

bool UPDATE_FlashWrite(UPDATE_FLASH *f, uint32_t address, const uint8_t *row, uint32_t now)
{
    if (row == NULL || address % UPDATE_ROW_SIZE || !CanStart(f, address, UPDATE_ROW_SIZE))
        return false;
    memcpy(f->row, row, UPDATE_ROW_SIZE);
    f->address = address;
    f->since = now;
    f->verify_offset = 0;
    f->erasing = false;
    bool accepted = f->backend.write(f->backend.context, f->row, address);
    f->status = accepted ? UPDATE_FLASH_BUSY : UPDATE_FLASH_REJECTED;
    return accepted;
}

UPDATE_FLASH_STATUS UPDATE_FlashPoll(UPDATE_FLASH *f, uint32_t now)
{
    if (f == NULL || !f->initialized) return UPDATE_FLASH_ERROR;
    if (f->status != UPDATE_FLASH_BUSY && f->status != UPDATE_FLASH_VERIFY) return f->status;
    if (f->backend.busy(f->backend.context))
    {
        if ((uint32_t)(now - f->since) >= f->timeout_ms) f->status = UPDATE_FLASH_TIMEOUT;
        return f->status;
    }
    if (f->backend.error(f->backend.context) != 0U)
    {
        f->status = UPDATE_FLASH_ERROR;
        return f->status;
    }
    f->status = UPDATE_FLASH_VERIFY;
    uint8_t actual[UPDATE_ROW_SIZE];
    if (!f->backend.read(f->backend.context, f->address + f->verify_offset,
                         actual, sizeof(actual)))
    {
        f->status = UPDATE_FLASH_ERROR;
        return f->status;
    }
    if (f->erasing)
    {
        for (size_t i = 0; i < sizeof(actual); ++i)
            if (actual[i] != 0xFFU) { f->status = UPDATE_FLASH_ERROR; return f->status; }
    }
    else if (memcmp(actual, f->row, sizeof(actual)) != 0)
    {
        f->status = UPDATE_FLASH_ERROR;
        return f->status;
    }
    f->verify_offset += UPDATE_ROW_SIZE;
    if (!f->erasing || f->verify_offset == UPDATE_ERASE_SIZE) f->status = UPDATE_FLASH_OK;
    return f->status;
}

bool UPDATE_FlashReset(UPDATE_FLASH *f)
{
    if (f == NULL || !f->initialized || f->status == UPDATE_FLASH_BUSY ||
        f->status == UPDATE_FLASH_VERIFY || f->backend.busy(f->backend.context)) return false;
    f->status = UPDATE_FLASH_IDLE;
    return true;
}
