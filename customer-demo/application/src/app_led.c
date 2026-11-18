/* Minimal asynchronous MCP23008 LED1 demo. No recovery after transfer failure. */
#include "app_led.h"
#include "definitions.h"

#define MCP23008_ADDRESS       0x22U
#define MCP23008_IODIR         0x00U
#define MCP23008_OLAT          0x0AU
#define LED1_MASK              0x01U /* GP0, active-low. */
#define BLINK_TICKS            50U   /* Timer2 runs at 10 ms. */
#define TRANSFER_TIMEOUT_TICKS 10U

typedef enum
{
    READ_OLAT,
    WRITE_LED_OFF,
    READ_IODIR,
    WRITE_IODIR,
    READBACK_IODIR,
    IDLE,
    WRITE_LED_TOGGLE,
    TERMINAL
} APP_LED_STATE;

volatile APP_LED_DIAGNOSTICS app_led_diagnostics;
static volatile uint32_t timer_ticks;
static APP_LED_STATE state;
static uint32_t transfer_started;
static uint32_t last_toggle;
static bool transfer_pending;
/* PLIB keeps these pointers until its asynchronous transfer completes. */
static uint8_t tx[2];
static uint8_t rx;
static uint8_t olat;
static uint8_t iodir;

static void Timer2Callback(uint32_t status, uintptr_t context)
{
    (void)status;
    (void)context;
    ++timer_ticks; /* Timing only: never call I2C from the interrupt. */
}

static uint32_t TicksGet(void)
{
    return timer_ticks; /* Aligned PIC32 32-bit read; subtraction tolerates wrap. */
}

static void Fail(APP_LED_FAULT fault)
{
    app_led_diagnostics.ready = false;
    app_led_diagnostics.terminal = true;
    app_led_diagnostics.last_fault = fault;
    state = TERMINAL;
    /* Do not abort/restart: PLIB buffer ownership may be uncertain on timeout. */
}

void APP_LED_Initialize(void)
{
    app_led_diagnostics.ready = false;
    app_led_diagnostics.led_on = false;
    app_led_diagnostics.terminal = false;
    app_led_diagnostics.last_fault = APP_LED_FAULT_NONE;
    app_led_diagnostics.transitions = 0U;
    timer_ticks = 0U;
    transfer_started = 0U;
    last_toggle = 0U;
    transfer_pending = false;
    state = READ_OLAT;
    TMR2_CallbackRegister(Timer2Callback, 0U);
    TMR2_Start();
}

void APP_LED_Tasks(void)
{
    uint32_t now = TicksGet();
    if (state == TERMINAL)
    {
        return;
    }

    if (transfer_pending)
    {
        if (I2C1_IsBusy())
        {
            if ((uint32_t)(now - transfer_started) >= TRANSFER_TIMEOUT_TICKS)
            {
                Fail(APP_LED_FAULT_TIMEOUT);
            }
            return; /* In particular, never reuse either buffer after timeout. */
        }

        I2C_ERROR error = I2C1_ErrorGet();
        transfer_pending = false;
        if (error != I2C_ERROR_NONE)
        {
            Fail(error == I2C_ERROR_NACK ? APP_LED_FAULT_NACK : APP_LED_FAULT_BUS_COLLISION);
            return;
        }

        switch (state)
        {
            case READ_OLAT:
                olat = (uint8_t)(rx | LED1_MASK); /* Turn off GP0; retain GP1-7. */
                state = WRITE_LED_OFF;
                break;
            case WRITE_LED_OFF:
                app_led_diagnostics.led_on = false;
                state = READ_IODIR;
                break;
            case READ_IODIR:
                iodir = (uint8_t)(rx & (uint8_t)~LED1_MASK); /* Only GP0 output. */
                state = WRITE_IODIR;
                break;
            case WRITE_IODIR:
                state = READBACK_IODIR;
                break;
            case READBACK_IODIR:
                if (rx != iodir)
                {
                    Fail(APP_LED_FAULT_READBACK_MISMATCH);
                    return;
                }
                app_led_diagnostics.ready = true;
                last_toggle = now;
                state = IDLE;
                break;
            case WRITE_LED_TOGGLE:
                app_led_diagnostics.led_on = (olat & LED1_MASK) == 0U;
                ++app_led_diagnostics.transitions;
                last_toggle = now;
                state = IDLE;
                break;
            default:
                Fail(APP_LED_FAULT_BUS_COLLISION);
                return;
        }
    }

    if (state == IDLE)
    {
        if ((uint32_t)(now - last_toggle) < BLINK_TICKS)
        {
            return;
        }
        olat ^= LED1_MASK;
        state = WRITE_LED_TOGGLE;
    }

    bool accepted;
    switch (state)
    {
        case READ_OLAT:
            tx[0] = MCP23008_OLAT;
            accepted = I2C1_WriteRead(MCP23008_ADDRESS, tx, 1U, &rx, 1U);
            break;
        case READ_IODIR:
        case READBACK_IODIR:
            tx[0] = MCP23008_IODIR;
            accepted = I2C1_WriteRead(MCP23008_ADDRESS, tx, 1U, &rx, 1U);
            break;
        case WRITE_LED_OFF:
        case WRITE_LED_TOGGLE:
            tx[0] = MCP23008_OLAT;
            tx[1] = olat;
            accepted = I2C1_Write(MCP23008_ADDRESS, tx, 2U);
            break;
        case WRITE_IODIR:
            tx[0] = MCP23008_IODIR;
            tx[1] = iodir;
            accepted = I2C1_Write(MCP23008_ADDRESS, tx, 2U);
            break;
        default:
            return;
    }

    if (!accepted)
    {
        Fail(APP_LED_FAULT_START_REJECTED);
        return;
    }
    transfer_pending = true;
    transfer_started = now;
}
