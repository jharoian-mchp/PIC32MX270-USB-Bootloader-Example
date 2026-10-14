#include "update_hex.h"
#include <string.h>

static uint32_t BE32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

bool UPDATE_HexBegin(UPDATE_HEX *h, uint32_t end, UPDATE_ROW_SINK sink, void *context)
{
    if (h == NULL || sink == NULL || end <= UPDATE_APP_PHYSICAL_START ||
        end > UPDATE_FLASH_PHYSICAL_END || end % UPDATE_ROW_SIZE) return false;
    *h = (UPDATE_HEX){ .end = end, .sink = sink, .context = context,
                      .status = UPDATE_HEX_ACCEPTED, .active = true };
    memset(h->row, 0xFF, sizeof(h->row));
    return true;
}

static bool Emit(UPDATE_HEX *h)
{
    if (!h->row_pending) return true;
    if (!h->sink(h->context, h->row_address, h->row)) return false;
    ++h->rows_emitted;
    h->row_pending = false;
    memset(h->row, 0xFF, sizeof(h->row));
    return true;
}

static UPDATE_HEX_STATUS Fail(UPDATE_HEX *h, UPDATE_HEX_STATUS error)
{
    h->active = false;
    h->status = error;
    h->row_pending = false; /* Do not later flush stale data on EOF. */
    memset(h->row, 0xFF, sizeof(h->row));
    return error;
}

static UPDATE_HEX_STATUS ValidateBatch(const UPDATE_HEX *h, const uint8_t *data, size_t size)
{
    uint32_t extension = h->extension, last_end = h->last_end;
    bool have_data = h->have_data, start_seen = h->start_seen, eof = h->eof;
    size_t pos = 0;
    if (data == NULL || size == 0U || size > UPDATE_PAYLOAD_MAX - 1U)
        return UPDATE_HEX_BAD_RECORD;
    while (pos < size)
    {
        if (eof || size - pos < 5U) return UPDATE_HEX_BAD_RECORD;
        const uint8_t *r = data + pos;
        size_t total = (size_t)r[0] + 5U;
        if (total > size - pos) return UPDATE_HEX_BAD_RECORD;
        uint8_t sum = 0;
        for (size_t i = 0; i < total; ++i) sum = (uint8_t)(sum + r[i]);
        if (sum) return UPDATE_HEX_BAD_RECORD;
        uint32_t offset = ((uint32_t)r[1] << 8) | r[2];
        switch (r[3])
        {
            case 0:
            {
                if (!r[0]) return UPDATE_HEX_BAD_RECORD;
                if (start_seen) return UPDATE_HEX_BAD_ORDER;
                /* Reject overflowing record offsets/extension before conversion. */
                if (offset + r[0] > UINT32_C(0x10000) || extension > UINT32_MAX - offset)
                    return UPDATE_HEX_BAD_ADDRESS;
                uint32_t physical;
                if (!UPDATE_ApplicationSpan(extension + offset, r[0], &physical) ||
                    physical >= h->end || r[0] > h->end - physical)
                    return UPDATE_HEX_BAD_ADDRESS;
                if (have_data && physical < last_end) return UPDATE_HEX_BAD_ORDER;
                last_end = physical + r[0];
                have_data = true;
                break;
            }
            case 1:
                if (r[0] || offset || !have_data) return UPDATE_HEX_BAD_RECORD;
                eof = true;
                break;
            case 2:
            case 4:
                if (r[0] != 2U || offset || start_seen) return UPDATE_HEX_BAD_RECORD;
                extension = ((uint32_t)r[4] << 8) | r[5];
                extension <<= r[3] == 2U ? 4 : 16;
                break;
            case 5:
            {
                if (r[0] != 4U || offset || !have_data || start_seen)
                    return UPDATE_HEX_BAD_RECORD;
                uint32_t physical;
                if (!UPDATE_ApplicationSpan(BE32(r + 4), 1U, &physical) ||
                    physical != UPDATE_APP_PHYSICAL_START) return UPDATE_HEX_BAD_ADDRESS;
                start_seen = true; /* Optional entry record last before EOF. */
                break;
            }
            default: return UPDATE_HEX_BAD_RECORD;
        }
        pos += total;
    }
    return UPDATE_HEX_ACCEPTED;
}

UPDATE_HEX_STATUS UPDATE_HexFeed(UPDATE_HEX *h, const uint8_t *records, size_t size)
{
    if (h == NULL) return UPDATE_HEX_NOT_ACTIVE;
    if (!h->active)
        return h->status == UPDATE_HEX_ACCEPTED || h->status == UPDATE_HEX_FINISHED ?
            UPDATE_HEX_NOT_ACTIVE : h->status;
    UPDATE_HEX_STATUS validation = ValidateBatch(h, records, size);
    if (validation != UPDATE_HEX_ACCEPTED) return Fail(h, validation);
    size_t pos = 0;
    while (pos < size)
    {
        const uint8_t *r = records + pos;
        uint32_t offset = ((uint32_t)r[1] << 8) | r[2];
        if (r[3] == 0U)
        {
            uint32_t physical = 0;
            (void)UPDATE_ApplicationSpan(h->extension + offset, r[0], &physical);
            size_t left = r[0], from = 0;
            while (left)
            {
                uint32_t row_address = physical - physical % UPDATE_ROW_SIZE;
                if (h->row_pending && row_address != h->row_address && !Emit(h))
                    return Fail(h, UPDATE_HEX_SINK_ERROR);
                if (!h->row_pending)
                {
                    h->row_address = row_address;
                    h->row_pending = true;
                }
                size_t row_offset = physical % UPDATE_ROW_SIZE;
                size_t count = UPDATE_ROW_SIZE - row_offset;
                if (count > left) count = left;
                memcpy(h->row + row_offset, r + 4U + from, count);
                physical += (uint32_t)count;
                from += count;
                left -= count;
                if (row_offset + count == UPDATE_ROW_SIZE && !Emit(h))
                    return Fail(h, UPDATE_HEX_SINK_ERROR);
            }
            h->last_end = physical;
            h->data_bytes += r[0];
            h->have_data = true;
        }
        else if (r[3] == 2U || r[3] == 4U)
        {
            h->extension = ((uint32_t)r[4] << 8) | r[5];
            h->extension <<= r[3] == 2U ? 4 : 16;
        }
        else if (r[3] == 5U) h->start_seen = true;
        else if (r[3] == 1U)
        {
            if (!Emit(h)) return Fail(h, UPDATE_HEX_SINK_ERROR);
            h->eof = true;
            h->active = false;
            h->status = UPDATE_HEX_FINISHED;
        }
        pos += (size_t)r[0] + 5U;
    }
    return h->status;
}

void UPDATE_HexAbort(UPDATE_HEX *h)
{
    if (h != NULL)
    {
        h->active = false;
        h->row_pending = false;
        h->extension = h->last_end = 0;
        h->status = UPDATE_HEX_NOT_ACTIVE;
        memset(h->row, 0xFF, sizeof(h->row));
    }
}
