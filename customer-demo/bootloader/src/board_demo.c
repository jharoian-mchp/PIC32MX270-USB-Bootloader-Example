#include "board_demo.h"

#define MCP23008_ADDRESS 0x22U
#define MCP23008_IODIR 0x00U
#define MCP23008_OLAT 0x0AU
#define MUX_MASK 0x60U
#define I2C_TIMEOUT_MS 100U
#define ADC_TIMEOUT_MS 10U
#define ADC_SAMPLE_PERIOD_MS 10U
#define ADC_CONNECT_THRESHOLD 465U
#define ADC_DISCONNECT_THRESHOLD 248U
#define ADC_QUALIFY_COUNT 3U
#define ADC_MAX_COUNT 1023U

typedef enum
{
    STATE_READ_OLAT,
    STATE_WRITE_OLAT,
    STATE_READ_IODIR,
    STATE_WRITE_IODIR,
    STATE_READBACK_OLAT,
    STATE_READBACK_IODIR,
    STATE_RUNNING,
    STATE_FAULT
} BOARD_DEMO_STATE;

static BOARD_DEMO_OPS s_ops;
static void *s_context;
/* These buffers are module-owned and remain untouched from transaction start
 * until completion. In particular, a busy timeout is terminal: no retry or
 * subsequent operation can reuse a buffer whose ownership is uncertain. */
static uint8_t s_tx[2];
static uint8_t s_rx;
static uint8_t s_saved_olat;
static uint8_t s_expected_olat;
static uint8_t s_saved_iodir;
static uint8_t s_expected_iodir;
static bool s_i2c_pending;
static bool s_adc_pending;
static uint32_t s_i2c_started_ms;
static uint32_t s_adc_started_ms;
static uint32_t s_next_sample_ms;
static uint8_t s_candidate_count;
static bool s_candidate_connected;
static BOARD_DEMO_STATE s_state;
static bool s_ready;
static bool s_cable_present;
static BOARD_DEMO_FAULT s_fault;
static bool s_initialized;

static bool InterfaceValid(const BOARD_DEMO_OPS *ops)
{
    return ops != NULL && ops->now_ms != NULL &&
           ops->i2c_write_read != NULL && ops->i2c_write != NULL &&
           ops->i2c_busy != NULL && ops->i2c_error != NULL &&
           ops->adc_start != NULL && ops->adc_ready != NULL &&
           ops->adc_result != NULL;
}

static uint32_t Now(void)
{
    return s_ops.now_ms(s_context);
}

static void Fail(BOARD_DEMO_FAULT fault)
{
    s_ready = false;
    s_cable_present = false;
    s_fault = fault;
    s_state = STATE_FAULT;
    /* Do not clear pending or touch tx/rx: a timed-out I2C operation may
     * still be using either buffer in the hardware/driver. */
}

static void StartRead(uint8_t reg, uint32_t now)
{
    s_tx[0] = reg;
    s_i2c_pending = s_ops.i2c_write_read(s_context, MCP23008_ADDRESS,
                                         s_tx, 1U, &s_rx, 1U);
    if (!s_i2c_pending)
    {
        Fail(BOARD_DEMO_FAULT_I2C);
        return;
    }
    s_i2c_started_ms = now;
}

static void StartWrite(uint8_t reg, uint8_t value, uint32_t now)
{
    s_tx[0] = reg;
    s_tx[1] = value;
    s_i2c_pending = s_ops.i2c_write(s_context, MCP23008_ADDRESS, s_tx, 2U);
    if (!s_i2c_pending)
    {
        Fail(BOARD_DEMO_FAULT_I2C);
        return;
    }
    s_i2c_started_ms = now;
}

static void StartCurrentI2cOperation(uint32_t now)
{
    switch (s_state)
    {
        case STATE_READ_OLAT:
        case STATE_READBACK_OLAT:
            StartRead(MCP23008_OLAT, now);
            break;
        case STATE_WRITE_OLAT:
            StartWrite(MCP23008_OLAT, s_expected_olat, now);
            break;
        case STATE_READ_IODIR:
        case STATE_READBACK_IODIR:
            StartRead(MCP23008_IODIR, now);
            break;
        case STATE_WRITE_IODIR:
            StartWrite(MCP23008_IODIR, s_expected_iodir, now);
            break;
        default:
            break;
    }
}

static void I2cTasks(uint32_t now)
{
    if (s_i2c_pending)
    {
        if (s_ops.i2c_busy(s_context))
        {
            if ((uint32_t)(now - s_i2c_started_ms) >= I2C_TIMEOUT_MS)
                Fail(BOARD_DEMO_FAULT_I2C);
            return;
        }
        s_i2c_pending = false;
        if (s_ops.i2c_error(s_context))
        {
            Fail(BOARD_DEMO_FAULT_I2C);
            return;
        }

        switch (s_state)
        {
            case STATE_READ_OLAT:
                s_saved_olat = s_rx;
                s_expected_olat = (uint8_t)(s_saved_olat | MUX_MASK);
                s_state = STATE_WRITE_OLAT;
                break;
            case STATE_WRITE_OLAT:
                s_state = STATE_READ_IODIR;
                break;
            case STATE_READ_IODIR:
                s_saved_iodir = s_rx;
                s_expected_iodir = (uint8_t)(s_saved_iodir & (uint8_t)~MUX_MASK);
                s_state = STATE_WRITE_IODIR;
                break;
            case STATE_WRITE_IODIR:
                s_state = STATE_READBACK_OLAT;
                break;
            case STATE_READBACK_OLAT:
                if (s_rx != s_expected_olat)
                {
                    Fail(BOARD_DEMO_FAULT_MUX_READBACK);
                    return;
                }
                s_state = STATE_READBACK_IODIR;
                break;
            case STATE_READBACK_IODIR:
                if (s_rx != s_expected_iodir)
                {
                    Fail(BOARD_DEMO_FAULT_MUX_READBACK);
                    return;
                }
                s_state = STATE_RUNNING;
                s_ready = true;
                s_next_sample_ms = now;
                break;
            default:
                Fail(BOARD_DEMO_FAULT_I2C);
                return;
        }
        return;
    }

    if (s_state != STATE_RUNNING && s_state != STATE_FAULT)
        StartCurrentI2cOperation(now);
}

static void QualifySample(uint16_t sample)
{
    bool qualifies;
    bool target;

    if (s_cable_present)
    {
        qualifies = sample <= ADC_DISCONNECT_THRESHOLD;
        target = false;
    }
    else
    {
        qualifies = sample >= ADC_CONNECT_THRESHOLD;
        target = true;
    }

    if (!qualifies)
    {
        s_candidate_count = 0U;
        return;
    }
    if (s_candidate_count == 0U || s_candidate_connected != target)
        s_candidate_connected = target;
    if (s_candidate_count < ADC_QUALIFY_COUNT)
        ++s_candidate_count;
    if (s_candidate_count >= ADC_QUALIFY_COUNT)
    {
        s_cable_present = target;
        s_candidate_count = 0U;
    }
}

static void AdcTasks(uint32_t now)
{
    if (s_adc_pending)
    {
        if (s_ops.adc_ready(s_context))
        {
            uint16_t sample = s_ops.adc_result(s_context);
            if (sample > ADC_MAX_COUNT) {
                Fail(BOARD_DEMO_FAULT_ADC_START);
                return;
            }
            s_adc_pending = false;
            QualifySample(sample);
        }
        else if ((uint32_t)(now - s_adc_started_ms) >= ADC_TIMEOUT_MS)
        {
            Fail(BOARD_DEMO_FAULT_ADC_TIMEOUT);
            return;
        }
        else
        {
            return;
        }
    }

    if ((int32_t)(now - s_next_sample_ms) >= 0)
    {
        if (!s_ops.adc_start(s_context))
        {
            Fail(BOARD_DEMO_FAULT_ADC_START);
            return;
        }
        s_adc_pending = true;
        s_adc_started_ms = now;
        s_next_sample_ms = now + ADC_SAMPLE_PERIOD_MS;
    }
}

void BOARD_DEMO_Initialize(const BOARD_DEMO_OPS *ops, void *context)
{
    /* A second initialization could reuse storage still owned by timed-out
     * hardware. This adapter is deliberately one-shot until a hardware reset. */
    if (s_initialized)
        return;
    s_initialized = true;
    s_ops = (BOARD_DEMO_OPS){0};
    s_context = context;
    s_tx[0] = s_tx[1] = s_rx = 0U;
    s_saved_olat = s_expected_olat = 0U;
    s_saved_iodir = s_expected_iodir = 0U;
    s_i2c_pending = s_adc_pending = false;
    s_i2c_started_ms = s_adc_started_ms = s_next_sample_ms = 0U;
    s_candidate_count = 0U;
    s_candidate_connected = false;
    s_ready = s_cable_present = false;
    s_fault = BOARD_DEMO_FAULT_NONE;
    s_state = STATE_READ_OLAT;

    if (!InterfaceValid(ops))
    {
        s_fault = BOARD_DEMO_FAULT_INTERFACE;
        s_state = STATE_FAULT;
        return;
    }
    s_ops = *ops;
}

void BOARD_DEMO_Tasks(void)
{
    if (s_state == STATE_FAULT || s_fault != BOARD_DEMO_FAULT_NONE)
        return;

    uint32_t now = Now();
    if (s_state != STATE_RUNNING)
    {
        I2cTasks(now);
        return;
    }
    AdcTasks(now);
}

bool BOARD_DEMO_Ready(void)
{
    return s_ready && s_fault == BOARD_DEMO_FAULT_NONE;
}

bool BOARD_DEMO_CablePresent(void)
{
    return BOARD_DEMO_Ready() && s_cable_present;
}

BOARD_DEMO_FAULT BOARD_DEMO_Fault(void)
{
    return s_fault;
}
