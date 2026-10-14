#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "definitions.h"
#include "board_usb_diag.h"

uint32_t mock_count, ANSELASET, TRISASET, CNPUACLR, CNPDACLR, AD1CON2;
struct SYSTEM_OBJECTS sysObj;
static bool enabled = true, bus_busy, reject_bus, fail_open, adc_pending, adc_ready;
static uint8_t registers[11], *pending_tx, *pending_rx, submitted[2];
static size_t tx_size;
static unsigned aborts, submits, attaches, detaches, usb_tasks, adc_resets;
static I2C_ERROR error;
static uint16_t sample_value;
static USB_DEVICE_EVENT_HANDLER usb_event;
static USB_DEVICE_HID_EVENT_HANDLER hid_event;
bool bootloader_Trigger(void);

bool EVIC_INT_Disable(void) { bool old = enabled; enabled = false; return old; }
void EVIC_INT_Restore(bool old) { enabled = old; }
bool I2C1_IsBusy(void) { return bus_busy; }
I2C_ERROR I2C1_ErrorGet(void) { I2C_ERROR e = error; error = I2C_ERROR_NONE; return e; }
void I2C1_TransferAbort(void) { assert(!enabled); bus_busy = false; ++aborts; }
static bool Submit(uint16_t addr, uint8_t *data, size_t size, uint8_t *rx)
{
    assert(addr == 0x22U);
    if (bus_busy || reject_bus) return false;
    assert(size == (rx ? 1U : 2U));
    bus_busy = true; pending_tx = data; pending_rx = rx; tx_size = size;
    memcpy(submitted, data, size); ++submits;
    return true;
}
bool I2C1_Write(uint16_t a, uint8_t *t, size_t s) { return Submit(a,t,s,NULL); }
bool I2C1_WriteRead(uint16_t a, uint8_t *t, size_t s, uint8_t *r, size_t n)
{ assert(n == 1U); return Submit(a,t,s,r); }
void ADC_SamplingStart(void) { assert(!adc_pending); adc_pending = true; adc_ready = false; }
bool ADC_ResultIsReady(void) { return adc_ready; }
uint32_t ADC_ResultGet(int b) { assert(b == 0 && adc_pending && adc_ready); adc_pending = adc_ready = false; return sample_value; }
void ADC_Initialize(void) { adc_pending = adc_ready = false; ++adc_resets; }
USB_DEVICE_HANDLE USB_DEVICE_Open(int i, int intent)
{ assert(i==0 && intent==3); return fail_open ? USB_DEVICE_HANDLE_INVALID : 1U; }
void USB_DEVICE_EventHandlerSet(USB_DEVICE_HANDLE h, USB_DEVICE_EVENT_HANDLER cb, uintptr_t c)
{ assert(h==1 && c==0); usb_event=cb; }
void USB_DEVICE_Attach(USB_DEVICE_HANDLE h)
{ assert(h==1 && !enabled); ++attaches; }
void USB_DEVICE_Detach(USB_DEVICE_HANDLE h) { assert(h==1); ++detaches; }
void USB_DEVICE_Tasks(uintptr_t o) { (void)o; ++usb_tasks; }
int USB_DEVICE_HID_EventHandlerSet(int i, USB_DEVICE_HID_EVENT_HANDLER cb, uintptr_t c)
{ assert(i==0 && c==0); hid_event=cb; return 0; }
int USB_DEVICE_ControlStatus(USB_DEVICE_HANDLE h, int s) { assert(h==1 && s==0); return 0; }
int USB_DEVICE_ControlSend(USB_DEVICE_HANDLE h, void *p, size_t n) { assert(h==1 && p && n==1); return 0; }

static void Reset(uint8_t latch, uint8_t dir, uint32_t initial_count)
{
    mock_count=initial_count; bus_busy=reject_bus=fail_open=adc_pending=adc_ready=false;
    error=I2C_ERROR_NONE; aborts=submits=attaches=detaches=usb_tasks=adc_resets=0;
    usb_event=NULL; hid_event=NULL; memset(registers,0,sizeof(registers));
    registers[0x0A]=latch; registers[0]=dir;
    BOARD_USB_DIAG_Initialize(); BOARD_USB_DIAG_Tasks();
    assert(ANSELASET==2 && TRISASET==2 && CNPUACLR==2 && CNPDACLR==2 && AD1CON2==0);
    assert(usb_event && detaches==1 && attaches==0 && bootloader_Trigger());
}
static void Complete(I2C_ERROR e)
{
    assert(bus_busy && !memcmp(pending_tx,submitted,tx_size));
    if (e==I2C_ERROR_NONE)
    { if(pending_rx) *pending_rx=registers[submitted[0]]; else registers[submitted[0]]=submitted[1]; }
    error=e; bus_busy=false; BOARD_USB_DIAG_Tasks();
}
static void MuxReady(void)
{
    assert(pending_rx && submitted[0]==0x0A); Complete(I2C_ERROR_NONE);
    assert(pending_rx && submitted[0]==0); Complete(I2C_ERROR_NONE);
    assert(!pending_rx && submitted[0]==0x0A && !(submitted[1]&0x60U)); Complete(I2C_ERROR_NONE);
    assert(!pending_rx && submitted[0]==0 && !(submitted[1]&0x60U)); Complete(I2C_ERROR_NONE);
    assert(!pending_rx && submitted[0]==0x0A && (submitted[1]&0x60U)==0x60U); Complete(I2C_ERROR_NONE);
    assert(pending_rx && submitted[0]==0x0A); Complete(I2C_ERROR_NONE);
    assert(pending_rx && submitted[0]==0); Complete(I2C_ERROR_NONE);
    assert(board_usb_diagnostics.mux_ready && !bus_busy);
}
static void Advance(unsigned ms) { mock_count += ms*24000U; BOARD_USB_DIAG_Tasks(); }
static void Sample(uint16_t v)
{ Advance(10); assert(adc_pending); sample_value=v; adc_ready=true; BOARD_USB_DIAG_Tasks(); }
int main(void)
{
    Reset(0xA5,0xB7,0); MuxReady();
    assert(registers[0x0A]==0xE5 && registers[0]==0x97);
    assert((registers[0x0A]&0x9FU)==(0xA5&0x9FU));
    usb_event(USB_DEVICE_EVENT_POWER_DETECTED,NULL,0); assert(attaches==0);
    Sample(624); Sample(624); assert(attaches==0); Sample(624); assert(attaches==1);
    usb_event(USB_DEVICE_EVENT_CONFIGURED,NULL,0); assert(board_usb_diagnostics.configured && hid_event);
    USB_DEVICE_HID_EVENT_DATA_SET_IDLE idle={4,0};
    hid_event(0,USB_DEVICE_HID_EVENT_SET_IDLE,&idle,0);
    hid_event(0,USB_DEVICE_HID_EVENT_GET_IDLE,NULL,0);
    Sample(350); assert(board_usb_diagnostics.cable_present && attaches==1);
    Sample(0); assert(!board_usb_diagnostics.cable_present && board_usb_diagnostics.detach_count==1);
    Sample(624); Sample(624); Sample(624); assert(attaches==2);
    Advance(10); assert(adc_pending); Advance(10);
    assert(board_usb_diagnostics.adc_timeouts==1 && adc_resets==1 && !adc_pending);
    assert(!board_usb_diagnostics.attached); Advance(1); assert(!adc_pending); Advance(9); assert(adc_pending);

    Reset(0x00,0xFF,0); Complete(I2C_ERROR_NACK);
    assert(!board_usb_diagnostics.mux_ready && board_usb_diagnostics.mux_errors==1 && aborts==1);
    for(unsigned i=0;i<3;i++) Sample(624);
    assert(board_usb_diagnostics.cable_present && attaches==0);
    Advance(970); MuxReady(); assert(attaches==1);

    Reset(0x00,0xFF,0); Advance(100);
    assert(aborts==1 && board_usb_diagnostics.mux_errors==1 && attaches==0);

    Reset(0x80,0xFF,0); Complete(I2C_ERROR_NONE); Complete(I2C_ERROR_NONE);
    Complete(I2C_ERROR_NONE); Complete(I2C_ERROR_NONE); Complete(I2C_ERROR_NONE);
    registers[0x0A]=0; Complete(I2C_ERROR_NONE);
    assert(board_usb_diagnostics.last_fault==BOARD_USB_FAULT_MUX_READBACK && !board_usb_diagnostics.mux_ready);

    Reset(0x00,0xFF,UINT32_MAX-5U*24000U); MuxReady();
    Sample(624); Sample(624); Sample(624); assert(attaches==1); /* CP0 wrap. */
    assert(board_usb_diagnostics.adc_raw==624 && board_usb_diagnostics.adc_samples==3);
    assert(usb_tasks>0);

    Reset(0x80,0xFF,0); MuxReady();
    Sample(624); Sample(350); Sample(624); Sample(624);
    assert(attaches==0); Sample(624); assert(attaches==1); /* Three consecutive highs. */
    usb_event(USB_DEVICE_EVENT_POWER_REMOVED,NULL,0);
    assert(board_usb_diagnostics.attached); /* Dedicated power events ignored; ADC decides. */
    usb_event(USB_DEVICE_EVENT_RESET,NULL,0);
    assert(board_usb_diagnostics.usb_resets==1 && !board_usb_diagnostics.configured);
    Sample(0); assert(!board_usb_diagnostics.attached);

    Reset(0,0xFF,0); Complete(I2C_ERROR_BUS_COLLISION);
    assert(board_usb_diagnostics.mux_errors==1 && attaches==0);
    reject_bus=true; Advance(1000); Advance(100);
    assert(board_usb_diagnostics.mux_errors==2 && aborts==2);

    puts("PASS: mux disconnect/preload/readback, unrelated pins preserved, ADC cadence/hysteresis/timeout, CP0 wrap, no attach from dedicated VBUS, qualified attach/detach and no flash/update APIs.");
    return 0;
}
