#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "app_led.h"

static TMR_CALLBACK timer;
static uintptr_t timer_context;
static bool busy, reject, timer_started, interrupts_enabled = true, reading;
static uint8_t *pending_tx, *pending_rx;
static size_t pending_tx_size;
static uint8_t submitted[2];
static uint8_t registers[11];
static I2C_ERROR result;
static unsigned aborts, writes, transfers;

void TMR2_CallbackRegister(TMR_CALLBACK callback, uintptr_t context)
{ timer = callback; timer_context = context; }
void TMR2_Start(void) { timer_started = true; }
bool EVIC_INT_Disable(void) { bool old = interrupts_enabled; interrupts_enabled = false; return old; }
void EVIC_INT_Restore(bool old) { interrupts_enabled = old; }
void I2C1_TransferAbort(void) { assert(!interrupts_enabled); busy = false; ++aborts; }
bool I2C1_IsBusy(void) { return busy; }
I2C_ERROR I2C1_ErrorGet(void) { I2C_ERROR error = result; result = I2C_ERROR_NONE; return error; }

static bool Submit(uint16_t address, uint8_t *tx, size_t size, uint8_t *rx)
{
    assert(address == 0x22U);
    if (reject || busy) return false;
    assert(size == (rx ? 1U : 2U));
    busy = true;
    reading = rx != NULL;
    pending_tx = tx;
    pending_tx_size = size;
    pending_rx = rx;
    memcpy(submitted, tx, size);
    ++transfers;
    return true;
}
bool I2C1_WriteRead(uint16_t address, uint8_t *tx, size_t size, uint8_t *rx, size_t rx_size)
{ assert(rx_size == 1U); return Submit(address, tx, size, rx); }
bool I2C1_Write(uint16_t address, uint8_t *tx, size_t size)
{ return Submit(address, tx, size, NULL); }

static void Complete(I2C_ERROR error)
{
    assert(busy);
    assert(memcmp(pending_tx, submitted, pending_tx_size) == 0); /* Async buffer lifetime. */
    if (error == I2C_ERROR_NONE)
    {
        if (reading) *pending_rx = registers[submitted[0]];
        else { registers[submitted[0]] = submitted[1]; ++writes; }
    }
    result = error;
    busy = false;
    APP_LED_Tasks();
}
static void Tick(unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
    {
        unsigned before = transfers;
        timer(1U, timer_context);
        assert(transfers == before); /* Timer ISR must never submit I2C. */
        APP_LED_Tasks();
    }
}
static void Reset(uint8_t latch, uint8_t direction)
{
    busy = reject = timer_started = false;
    result = I2C_ERROR_NONE;
    aborts = writes = transfers = 0;
    memset(registers, 0, sizeof(registers));
    registers[0x0A] = latch;
    registers[0] = direction;
    APP_LED_Initialize();
    assert(timer_started && interrupts_enabled);
    APP_LED_Tasks();
}
static void InitialiseSuccessfully(void)
{
    assert(reading && submitted[0] == 0x0A);
    Complete(I2C_ERROR_NONE);
    assert(!reading && submitted[0] == 0x0A && (submitted[1] & 1U));
    Complete(I2C_ERROR_NONE);
    assert(reading && submitted[0] == 0);
    Complete(I2C_ERROR_NONE);
    assert(!reading && submitted[0] == 0 && !(submitted[1] & 1U));
    Complete(I2C_ERROR_NONE);
    assert(app_led_diagnostics.ready && !app_led_diagnostics.led_on && !busy);
}
int main(void)
{
    Reset(0xAA, 0xFF);
    InitialiseSuccessfully();
    assert(registers[0x0A] == 0xAB && registers[0] == 0xFE);
    Tick(49);
    assert(!busy);
    Tick(1);
    assert(busy && submitted[0] == 0x0A && submitted[1] == 0xAA);
    Complete(I2C_ERROR_NONE);
    assert(app_led_diagnostics.led_on && app_led_diagnostics.transitions == 1);
    Tick(50);
    Complete(I2C_ERROR_NONE);
    assert(!app_led_diagnostics.led_on && app_led_diagnostics.transitions == 2);
    assert((registers[0x0A] & 0xFEU) == 0xAAU);

    Reset(0x54, 0xA5); /* Preserve already-configured non-LED directions. */
    InitialiseSuccessfully();
    assert(registers[0] == 0xA4 && registers[0x0A] == 0x55);

    Reset(0x00, 0xFF);
    Complete(I2C_ERROR_NACK);
    assert(app_led_diagnostics.last_fault == APP_LED_FAULT_NACK && aborts == 1);
    assert(!app_led_diagnostics.ready && writes == 0);
    Tick(99); assert(!busy);
    Tick(1); InitialiseSuccessfully();
    assert(app_led_diagnostics.errors == 1 && app_led_diagnostics.last_fault == APP_LED_FAULT_NONE);

    Reset(0x00, 0xFF);
    Tick(10);
    assert(app_led_diagnostics.last_fault == APP_LED_FAULT_TIMEOUT && aborts == 1);
    Tick(100); InitialiseSuccessfully();

    Reset(0x00, 0xFF);
    Complete(I2C_ERROR_BUS_COLLISION);
    assert(app_led_diagnostics.last_fault == APP_LED_FAULT_BUS_COLLISION);

    Reset(0x62, 0xFF);
    InitialiseSuccessfully();
    Tick(50);
    Complete(I2C_ERROR_NACK); /* A failed blink must not count as a transition. */
    assert(app_led_diagnostics.transitions == 0 && !app_led_diagnostics.ready);
    Tick(100); InitialiseSuccessfully();
    assert(registers[0x0A] == 0x63 && app_led_diagnostics.errors == 1);

    Reset(0x00, 0xFF);
    Complete(I2C_ERROR_NONE); /* Preload is now pending. */
    Complete(I2C_ERROR_NACK);
    assert(registers[0] == 0xFF); /* Never enable LED after failed preload. */

    Reset(0x00, 0xFF);
    Complete(I2C_ERROR_NACK);
    reject = true;
    Tick(100); /* Restart init, bus rejects transfer. */
    Tick(10);
    assert(app_led_diagnostics.errors == 2 && app_led_diagnostics.last_fault == APP_LED_FAULT_TIMEOUT);

    puts("PASS: safe init, GP1-GP7 preservation, 500 ms cadence, async buffers, NACK, collision, timeout, retries, rejected bus starts; no I2C in ISR.");
    return 0;
}
