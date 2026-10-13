/* MCP23008 LED1 bring-up. User-owned; generated PLIBs remain unchanged. */
#include "app_led.h"
#include "definitions.h"

#define MCP23008_ADDRESS       0x22U /* Seven-bit address, PIM J1 pins 1-2. */
#define MCP23008_IODIR         0x00U
#define MCP23008_OLAT          0x0AU
#define LED1_MASK              0x01U /* GP0, active low. */
#define BLINK_TICKS            50U  /* 50 x 10 ms = 500 ms. */
#define TRANSFER_TIMEOUT_TICKS 10U  /* 100 ms, including bus-start rejection. */
#define RETRY_TICKS            100U /* One second between recovery attempts. */

typedef enum
{
    READ_LATCH,
    PRELOAD_OFF,
    READ_DIRECTION,
    ENABLE_LED,
    READY,
    WRITE_LED,
    RETRY
} LED_STATE;

volatile APP_LED_DIAGNOSTICS app_led_diagnostics;
static volatile uint32_t timer_ticks;
static LED_STATE state;
static uint32_t deadline_start;
static uint32_t last_toggle;
static bool transfer_pending;
static uint8_t tx[2]; /* Persistent until the asynchronous PLIB completes. */
static uint8_t rx;
static uint8_t latch;
static uint8_t direction;

static void TimerCallback(uint32_t status, uintptr_t context)
{
    (void)status;
    (void)context;
    ++timer_ticks; /* No I2C operations in the interrupt. */
}

static uint32_t TicksGet(void)
{
    /* PIC32 aligned 32-bit load is atomic; unsigned subtraction handles wrap. */
    return timer_ticks;
}

static void Fail(APP_LED_FAULT fault, uint32_t now)
{
    /* Keep PLIB reset atomic with respect to its interrupt handlers. */
    bool interrupts_were_enabled = EVIC_INT_Disable();
    I2C1_TransferAbort();
    EVIC_INT_Restore(interrupts_were_enabled);
    transfer_pending = false;
    state = RETRY;
    deadline_start = now;
    app_led_diagnostics.ready = false;
    app_led_diagnostics.last_fault = fault;
    ++app_led_diagnostics.errors;
}

void APP_LED_Initialize(void)
{
    app_led_diagnostics.ready = false;
    app_led_diagnostics.led_on = false;
    app_led_diagnostics.last_fault = APP_LED_FAULT_NONE;
    app_led_diagnostics.errors = 0;
    app_led_diagnostics.transitions = 0;
    timer_ticks = 0;
    deadline_start = 0;
    last_toggle = 0;
    transfer_pending = false;
    state = READ_LATCH;
    TMR2_CallbackRegister(TimerCallback, 0);
    TMR2_Start(); /* MCC initialises Timer2 but leaves it stopped. */
}

void APP_LED_Tasks(void)
{
    uint32_t now = TicksGet();

    if (state == RETRY)
    {
        if ((uint32_t)(now - deadline_start) < RETRY_TICKS)
        {
            return;
        }
        state = READ_LATCH;
        deadline_start = now;
    }

    if (transfer_pending)
    {
        if (I2C1_IsBusy())
        {
            if ((uint32_t)(now - deadline_start) >= TRANSFER_TIMEOUT_TICKS)
            {
                Fail(APP_LED_FAULT_TIMEOUT, now);
            }
            return;
        }

        I2C_ERROR error = I2C1_ErrorGet();
        transfer_pending = false;
        if (error != I2C_ERROR_NONE)
        {
            Fail(error == I2C_ERROR_NACK ? APP_LED_FAULT_NACK :
                 APP_LED_FAULT_BUS_COLLISION, now);
            return;
        }

        switch (state)
        {
            case READ_LATCH:
                latch = rx | LED1_MASK; /* Preserve GP1-GP7; LED1 OFF first. */
                state = PRELOAD_OFF;
                break;
            case PRELOAD_OFF:
                app_led_diagnostics.led_on = false;
                state = READ_DIRECTION;
                break;
            case READ_DIRECTION:
                direction = rx & (uint8_t)~LED1_MASK; /* Only GP0 becomes output. */
                state = ENABLE_LED;
                break;
            case ENABLE_LED:
                app_led_diagnostics.ready = true;
                app_led_diagnostics.last_fault = APP_LED_FAULT_NONE;
                last_toggle = now;
                state = READY;
                break;
            case WRITE_LED:
                app_led_diagnostics.led_on = (latch & LED1_MASK) == 0U;
                ++app_led_diagnostics.transitions;
                last_toggle = now;
                state = READY;
                break;
            default:
                break;
        }
        deadline_start = now;
    }

    if (state == READY)
    {
        if ((uint32_t)(now - last_toggle) < BLINK_TICKS)
        {
            return;
        }
        latch ^= LED1_MASK;
        state = WRITE_LED;
        deadline_start = now;
    }

    bool accepted = false;
    switch (state)
    {
        case READ_LATCH:
            tx[0] = MCP23008_OLAT;
            accepted = I2C1_WriteRead(MCP23008_ADDRESS, tx, 1U, &rx, 1U);
            break;
        case READ_DIRECTION:
            tx[0] = MCP23008_IODIR;
            accepted = I2C1_WriteRead(MCP23008_ADDRESS, tx, 1U, &rx, 1U);
            break;
        case PRELOAD_OFF:
        case WRITE_LED:
            tx[0] = MCP23008_OLAT;
            tx[1] = latch;
            accepted = I2C1_Write(MCP23008_ADDRESS, tx, 2U);
            break;
        case ENABLE_LED:
            tx[0] = MCP23008_IODIR;
            tx[1] = direction;
            accepted = I2C1_Write(MCP23008_ADDRESS, tx, 2U);
            break;
        default:
            return;
    }

    if (accepted)
    {
        transfer_pending = true;
        deadline_start = now;
    }
    else if ((uint32_t)(now - deadline_start) >= TRANSFER_TIMEOUT_TICKS)
    {
        Fail(APP_LED_FAULT_TIMEOUT, now);
    }
}
