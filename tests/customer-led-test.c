#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "app_led.h"

static TMR_CALLBACK timer_callback;
static uintptr_t timer_context;
static bool busy, reject_start;
static bool is_read;
static uint8_t *async_tx, *async_rx, submitted[2];
static size_t submitted_size;
static I2C_ERROR result;
static uint8_t registers[11];
static unsigned transfer_count, write_count, timer_starts;

void TMR2_CallbackRegister(TMR_CALLBACK callback, uintptr_t context)
{ timer_callback = callback; timer_context = context; }
void TMR2_Start(void) { ++timer_starts; }
bool I2C1_IsBusy(void) { return busy; }
I2C_ERROR I2C1_ErrorGet(void) { I2C_ERROR e = result; result = I2C_ERROR_NONE; return e; }
static bool Submit(uint16_t address, uint8_t *tx, size_t n, uint8_t *rx)
{
    assert(address == 0x22U);
    if (busy || reject_start) return false;
    assert(n == (rx ? 1U : 2U));
    busy = true; is_read = rx != NULL; async_tx = tx; async_rx = rx;
    submitted_size = n; memcpy(submitted, tx, n); ++transfer_count;
    return true;
}
bool I2C1_WriteRead(uint16_t a, uint8_t *t, size_t n, uint8_t *r, size_t rn)
{ assert(rn == 1U); return Submit(a, t, n, r); }
bool I2C1_Write(uint16_t a, uint8_t *t, size_t n) { return Submit(a, t, n, NULL); }
static void Complete(I2C_ERROR error)
{
    assert(busy);
    assert(memcmp(async_tx, submitted, submitted_size) == 0); /* async lifetime */
    if (error == I2C_ERROR_NONE)
    {
        if (is_read) *async_rx = registers[submitted[0]];
        else { registers[submitted[0]] = submitted[1]; ++write_count; }
    }
    result = error;
    busy = false;
    APP_LED_Tasks();
}
static void Tick(unsigned count)
{
    while (count--)
    {
        unsigned before = transfer_count;
        timer_callback(1U, timer_context);
        assert(transfer_count == before); /* callback does timing only */
        APP_LED_Tasks();
    }
}
static void Reset(uint8_t olat, uint8_t iodir)
{
    busy = reject_start = false; is_read = false; result = I2C_ERROR_NONE;
    transfer_count = write_count = timer_starts = 0;
    memset(registers, 0, sizeof(registers));
    registers[0x0A] = olat; registers[0] = iodir;
    APP_LED_Initialize();
    assert(timer_starts == 1U && timer_callback != NULL);
    APP_LED_Tasks();
}
static void Initialize(void)
{
    assert(busy && is_read && submitted[0] == 0x0A);
    Complete(I2C_ERROR_NONE);
    assert(busy && !is_read && submitted[0] == 0x0A && (submitted[1] & 1U));
    Complete(I2C_ERROR_NONE);
    assert(busy && is_read && submitted[0] == 0x00);
    Complete(I2C_ERROR_NONE);
    assert(busy && !is_read && submitted[0] == 0x00 && !(submitted[1] & 1U));
    Complete(I2C_ERROR_NONE);
    assert(busy && is_read && submitted[0] == 0x00); /* Direction readback. */
    Complete(I2C_ERROR_NONE);
    assert(app_led_diagnostics.ready && !app_led_diagnostics.led_on);
}
static void TestPreservationAndBlink(void)
{
    Reset(0xAA, 0xA5); Initialize();
    assert(registers[0x0A] == 0xAB); /* preload inactive level */
    assert(registers[0] == 0xA4); /* all GP1-GP7 directions retained */
    Tick(49); assert(!busy);
    Tick(1); assert(busy && submitted[0] == 0x0A && submitted[1] == 0xAA);
    Complete(I2C_ERROR_NONE);
    assert(app_led_diagnostics.led_on && app_led_diagnostics.transitions == 1U);
    assert((registers[0x0A] & 0xFEU) == 0xAAU);
    Tick(50); assert(busy && submitted[1] == 0xAB);
    Complete(I2C_ERROR_NONE);
    assert(!app_led_diagnostics.led_on && app_led_diagnostics.transitions == 2U);
}
static void TestStartupReadbackMismatch(void)
{
    Reset(0x80, 0xFF);
    assert(busy && is_read && submitted[0] == 0x0A);
    Complete(I2C_ERROR_NONE); /* OLAT read */
    Complete(I2C_ERROR_NONE); /* preload OFF */
    Complete(I2C_ERROR_NONE); /* IODIR read */
    Complete(I2C_ERROR_NONE); /* write GP0 output */
    assert(busy && is_read && submitted[0] == 0x00);
    registers[0] ^= 0x02U; /* Simulate unexpected change on a preserved pin. */
    Complete(I2C_ERROR_NONE);
    assert(app_led_diagnostics.terminal && !app_led_diagnostics.ready);
    assert(app_led_diagnostics.last_fault == APP_LED_FAULT_READBACK_MISMATCH);
    unsigned count = transfer_count;
    Tick(100); assert(transfer_count == count);
}
static void TestNackTerminal(void)
{
    Reset(0x00, 0xFF); Complete(I2C_ERROR_NACK);
    assert(app_led_diagnostics.terminal && !app_led_diagnostics.ready);
    assert(app_led_diagnostics.last_fault == APP_LED_FAULT_NACK);
    unsigned count = transfer_count;
    Tick(200); APP_LED_Tasks();
    assert(transfer_count == count && !busy);
}
static void TestTimeoutTerminal(void)
{
    Reset(0x00, 0xFF);
    unsigned count = transfer_count;
    Tick(9); assert(!app_led_diagnostics.terminal);
    Tick(1);
    assert(app_led_diagnostics.terminal && app_led_diagnostics.last_fault == APP_LED_FAULT_TIMEOUT);
    assert(busy); /* still owned by unresolved PLIB: don't abort or reuse buffers */
    Tick(200); APP_LED_Tasks();
    assert(transfer_count == count && busy);
}
static void TestRejectedStartTerminal(void)
{
    busy = false; reject_start = true; result = I2C_ERROR_NONE;
    transfer_count = write_count = timer_starts = 0;
    APP_LED_Initialize();
    APP_LED_Tasks(); /* First startup read is rejected by the PLIB. */
    assert(app_led_diagnostics.terminal && app_led_diagnostics.last_fault == APP_LED_FAULT_START_REJECTED);
    assert(transfer_count == 0U);
}
int main(void)
{
    TestPreservationAndBlink();
    TestStartupReadbackMismatch();
    TestNackTerminal();
    TestTimeoutTerminal();
    TestRejectedStartTerminal();
    puts("PASS: LED init/readback mismatch, GP1-GP7 preservation, 500ms toggle, async buffers, NACK, timeout terminal ownership, rejected start, Timer2-only tick");
    return 0;
}
