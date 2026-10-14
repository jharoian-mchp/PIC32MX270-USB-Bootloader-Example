/* User-owned, enumeration-only bring-up. No update protocol or flash writes. */
#include "board_usb_diag.h"
#include "definitions.h"

#define EXPANDER_ADDRESS 0x22U
#define REG_IODIR 0x00U
#define REG_OLAT 0x0AU
#define MUX_MASK 0x60U
#define CONNECT_COUNT 465U /* Approx. 1.5 V at AVDD=3.3 V. Bring-up only. */
#define DISCONNECT_COUNT 248U /* Approx. 0.8 V. */
#define SAMPLE_MS 10U
#define ADC_TIMEOUT_MS 10U
#define MUX_TIMEOUT_MS 100U
#define RETRY_MS 1000U
#define QUALIFY_SAMPLES 3U
#define COUNTS_PER_MS (CPU_CLOCK_FREQUENCY / 2U / 1000U)

typedef enum
{
    MUX_READ_LATCH,
    MUX_READ_DIRECTION,
    MUX_DISCONNECT,
    MUX_OUTPUTS,
    MUX_SELECT,
    MUX_CHECK_LATCH,
    MUX_CHECK_DIRECTION,
    MUX_READY,
    MUX_RETRY
} MUX_STATE;

volatile BOARD_USB_DIAGNOSTICS board_usb_diagnostics;
static MUX_STATE mux_state;
static uint8_t tx[2], rx, saved_latch, saved_direction;
static bool mux_pending, adc_pending;
static uint32_t last_count, milliseconds, mux_since, adc_since, last_sample;
static uint8_t connect_streak;
static USB_DEVICE_HANDLE usb_handle;
static uint8_t hid_idle;

/* This build always stays in diagnostic boot mode, including with an old app
 * still in flash. Deliberately overrides the generated weak hook. */
bool bootloader_Trigger(void)
{
    return true;
}

static uint32_t TimeGet(void)
{
    uint32_t count = _CP0_GET_COUNT();
    uint32_t elapsed_ms = (uint32_t)(count - last_count) / COUNTS_PER_MS;
    last_count += elapsed_ms * COUNTS_PER_MS;
    milliseconds += elapsed_ms;
    return milliseconds;
}

static void Detach(void)
{
    if (board_usb_diagnostics.attached)
    {
        bool enabled = EVIC_INT_Disable();
        USB_DEVICE_Detach(usb_handle);
        board_usb_diagnostics.attached = false;
        board_usb_diagnostics.configured = false;
        board_usb_diagnostics.suspended = false;
        ++board_usb_diagnostics.detach_count;
        EVIC_INT_Restore(enabled);
    }
}

static USB_DEVICE_HID_EVENT_RESPONSE HIDEvent(
    USB_DEVICE_HID_INDEX index, USB_DEVICE_HID_EVENT event,
    void *data, uintptr_t context)
{
    (void)index;
    (void)context;
    switch (event)
    {
        case USB_DEVICE_HID_EVENT_SET_IDLE:
            hid_idle = ((USB_DEVICE_HID_EVENT_DATA_SET_IDLE *)data)->duration;
            (void)USB_DEVICE_ControlStatus(usb_handle, USB_DEVICE_CONTROL_STATUS_OK);
            break;
        case USB_DEVICE_HID_EVENT_GET_IDLE:
            (void)USB_DEVICE_ControlSend(usb_handle, &hid_idle, 1U);
            break;
        default:
            /* No report queues or update command handler in this image. */
            break;
    }
    return USB_DEVICE_HID_EVENT_RESPONSE_NONE;
}

static void USBEvent(USB_DEVICE_EVENT event, void *data, uintptr_t context)
{
    (void)data;
    (void)context;
    switch (event)
    {
        case USB_DEVICE_EVENT_CONFIGURED:
            board_usb_diagnostics.configured = true;
            (void)USB_DEVICE_HID_EventHandlerSet(USB_DEVICE_HID_INDEX_0, HIDEvent, 0);
            break;
        case USB_DEVICE_EVENT_RESET:
            ++board_usb_diagnostics.usb_resets;
            board_usb_diagnostics.configured = false;
            board_usb_diagnostics.suspended = false;
            break;
        case USB_DEVICE_EVENT_DECONFIGURED:
            board_usb_diagnostics.configured = false;
            break;
        case USB_DEVICE_EVENT_SUSPENDED:
            board_usb_diagnostics.suspended = true;
            break;
        case USB_DEVICE_EVENT_RESUMED:
            board_usb_diagnostics.suspended = false;
            break;
        case USB_DEVICE_EVENT_ERROR:
            ++board_usb_diagnostics.usb_errors;
            break;
        default:
            /* Dedicated VBUS is board-powered. POWER_DETECTED must NOT attach
             * here: only the AN1-qualified main loop controls the pull-up. */
            break;
    }
}

static void MuxFail(BOARD_USB_FAULT fault, uint32_t now)
{
    bool enabled = EVIC_INT_Disable();
    I2C1_TransferAbort();
    EVIC_INT_Restore(enabled);
    mux_pending = false;
    mux_state = MUX_RETRY;
    mux_since = now;
    board_usb_diagnostics.mux_ready = false;
    board_usb_diagnostics.last_fault = fault;
    ++board_usb_diagnostics.mux_errors;
    Detach();
}

static void MuxTasks(uint32_t now)
{
    if (mux_state == MUX_READY) return;
    if (mux_state == MUX_RETRY)
    {
        if ((uint32_t)(now - mux_since) < RETRY_MS) return;
        mux_state = MUX_READ_LATCH;
        mux_since = now;
    }
    if (mux_pending)
    {
        if (I2C1_IsBusy())
        {
            if ((uint32_t)(now - mux_since) >= MUX_TIMEOUT_MS)
                MuxFail(BOARD_USB_FAULT_I2C, now);
            return;
        }
        mux_pending = false;
        if (I2C1_ErrorGet() != I2C_ERROR_NONE)
        {
            MuxFail(BOARD_USB_FAULT_I2C, now);
            return;
        }
        switch (mux_state)
        {
            case MUX_READ_LATCH: saved_latch = rx; break;
            case MUX_READ_DIRECTION: saved_direction = rx; break;
            case MUX_CHECK_LATCH:
                board_usb_diagnostics.mux_latch = rx;
                if (rx != (uint8_t)(saved_latch | MUX_MASK))
                {
                    MuxFail(BOARD_USB_FAULT_MUX_READBACK, now);
                    return;
                }
                break;
            case MUX_CHECK_DIRECTION:
                board_usb_diagnostics.mux_direction = rx;
                if (rx != (uint8_t)(saved_direction & (uint8_t)~MUX_MASK))
                {
                    MuxFail(BOARD_USB_FAULT_MUX_READBACK, now);
                    return;
                }
                board_usb_diagnostics.mux_ready = true;
                break;
            default: break;
        }
        mux_state = (MUX_STATE)(mux_state + 1);
        mux_since = now;
    }
    if (mux_state == MUX_READY) return;
    bool accepted;
    switch (mux_state)
    {
        case MUX_READ_LATCH:
        case MUX_CHECK_LATCH:
            tx[0] = REG_OLAT;
            accepted = I2C1_WriteRead(EXPANDER_ADDRESS, tx, 1U, &rx, 1U);
            break;
        case MUX_READ_DIRECTION:
        case MUX_CHECK_DIRECTION:
            tx[0] = REG_IODIR;
            accepted = I2C1_WriteRead(EXPANDER_ADDRESS, tx, 1U, &rx, 1U);
            break;
        case MUX_DISCONNECT:
            tx[0] = REG_OLAT;
            tx[1] = saved_latch & (uint8_t)~MUX_MASK;
            accepted = I2C1_Write(EXPANDER_ADDRESS, tx, 2U);
            break;
        case MUX_OUTPUTS:
            tx[0] = REG_IODIR;
            tx[1] = saved_direction & (uint8_t)~MUX_MASK;
            accepted = I2C1_Write(EXPANDER_ADDRESS, tx, 2U);
            break;
        case MUX_SELECT:
            tx[0] = REG_OLAT;
            tx[1] = saved_latch | MUX_MASK;
            accepted = I2C1_Write(EXPANDER_ADDRESS, tx, 2U);
            break;
        default: return;
    }
    if (accepted) { mux_pending = true; mux_since = now; }
    else if ((uint32_t)(now - mux_since) >= MUX_TIMEOUT_MS)
        MuxFail(BOARD_USB_FAULT_I2C, now);
}

static void SenseTasks(uint32_t now)
{
    if (adc_pending)
    {
        if (ADC_ResultIsReady())
        {
            uint16_t sample = (uint16_t)(ADC_ResultGet(ADC_RESULT_BUFFER_0) & 1023U);
            adc_pending = false;
            board_usb_diagnostics.adc_raw = sample;
            ++board_usb_diagnostics.adc_samples;
            if (sample < board_usb_diagnostics.adc_min) board_usb_diagnostics.adc_min = sample;
            if (sample > board_usb_diagnostics.adc_max) board_usb_diagnostics.adc_max = sample;
            if (sample >= CONNECT_COUNT)
            {
                if (connect_streak < QUALIFY_SAMPLES) ++connect_streak;
                if (connect_streak >= QUALIFY_SAMPLES)
                    board_usb_diagnostics.cable_present = true;
            }
            else
            {
                connect_streak = 0;
                if (sample <= DISCONNECT_COUNT)
                    board_usb_diagnostics.cable_present = false;
            }
        }
        else if ((uint32_t)(now - adc_since) >= ADC_TIMEOUT_MS)
        {
            adc_pending = false;
            connect_streak = 0;
            board_usb_diagnostics.cable_present = false;
            board_usb_diagnostics.last_fault = BOARD_USB_FAULT_ADC_TIMEOUT;
            ++board_usb_diagnostics.adc_timeouts;
            ADC_Initialize();
            last_sample = now; /* Allow analogue startup before the next sample. */
        }
    }
    if (!adc_pending && (uint32_t)(now - last_sample) >= SAMPLE_MS)
    {
        last_sample = adc_since = now;
        adc_pending = true;
        ADC_SamplingStart(); /* Counter ends 16 TAD acquisition automatically. */
    }
}

void BOARD_USB_DIAG_Initialize(void)
{
    board_usb_diagnostics = (BOARD_USB_DIAGNOSTICS){0};
    board_usb_diagnostics.adc_min = 1023U;
    last_count = _CP0_GET_COUNT();
    milliseconds = mux_since = last_sample = adc_since = 0;
    mux_state = MUX_READ_LATCH;
    mux_pending = adc_pending = false;
    connect_streak = hid_idle = 0;
    /* Explicit ADC reset reference/mux/pin assumptions; no external VREF- on RA1. */
    ANSELASET = 1U << 1;
    TRISASET = 1U << 1;
    CNPUACLR = 1U << 1;
    CNPDACLR = 1U << 1;
    AD1CON2 = 0;
    usb_handle = USB_DEVICE_HANDLE_INVALID;
}

void BOARD_USB_DIAG_Tasks(void)
{
    uint32_t now = TimeGet();
    if (usb_handle == USB_DEVICE_HANDLE_INVALID)
    {
        usb_handle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);
        if (usb_handle != USB_DEVICE_HANDLE_INVALID)
        {
            USB_DEVICE_EventHandlerSet(usb_handle, USBEvent, 0);
            USB_DEVICE_Detach(usb_handle); /* Explicitly start with no D+ pull-up. */
            board_usb_diagnostics.usb_open = true;
        }
        else board_usb_diagnostics.last_fault = BOARD_USB_FAULT_USB_OPEN;
    }
    SenseTasks(now);
    MuxTasks(now);
    if (!board_usb_diagnostics.cable_present || !board_usb_diagnostics.mux_ready)
        Detach();
    else if (!board_usb_diagnostics.attached && board_usb_diagnostics.usb_open)
    {
        bool enabled = EVIC_INT_Disable();
        USB_DEVICE_Attach(usb_handle);
        board_usb_diagnostics.attached = true;
        ++board_usb_diagnostics.attach_count;
        EVIC_INT_Restore(enabled);
    }
    /* Deliberately bypass generated SYS_Tasks: no bootloader protocol parser,
     * report receive queue, NVM erase/write or application handoff runs. */
    USB_DEVICE_Tasks(sysObj.usbDevObject0);
}
