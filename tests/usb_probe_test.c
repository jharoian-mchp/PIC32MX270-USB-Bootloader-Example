#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "usb_probe.h"
#include "update_wire.h"
static bool interrupts=true,rx_owned,tx_owned,reject_rx,reject_tx,cancel_fail;
static bool live_rx_error,live_tx_error;
static uint8_t *rx,*tx;
static uint8_t sent[64];
static USB_DEVICE_HID_TRANSFER_HANDLE rh,th;
static unsigned receives,sends,cancels;
bool EVIC_INT_Disable(void){bool e=interrupts;interrupts=false;return e;}
void EVIC_INT_Restore(bool e){interrupts=e;}
USB_DEVICE_HID_RESULT USB_DEVICE_HID_ReportReceive(int i,USB_DEVICE_HID_TRANSFER_HANDLE *h,void *b,size_t n)
{
    assert(i==0&&n==64&&!interrupts&&!rx_owned);++receives;
    if(reject_rx){*h=USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;return -2;}
    rh=*h=100;rx=b;rx_owned=true;return live_rx_error?-1:0;
}
USB_DEVICE_HID_RESULT USB_DEVICE_HID_ReportSend(int i,USB_DEVICE_HID_TRANSFER_HANDLE *h,void *b,size_t n)
{
    assert(i==0&&n==64&&!interrupts&&!tx_owned);++sends;
    if(reject_tx){*h=USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;return -2;}
    th=*h=200;tx=b;memcpy(sent,b,64);tx_owned=true;return live_tx_error?-1:0;
}
USB_DEVICE_HID_RESULT USB_DEVICE_HID_TransferCancel(int i,USB_DEVICE_HID_TRANSFER_HANDLE h)
{
    assert(i==0&&!interrupts);++cancels;if(cancel_fail)return -1;
    USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED d={0,h,-1};
    if(rx_owned&&h==rh){rx_owned=false;USB_PROBE_Event(USB_DEVICE_HID_EVENT_REPORT_RECEIVED,&d);return 0;}
    if(tx_owned&&h==th){assert(!memcmp(tx,sent,64));tx_owned=false;USB_PROBE_Event(USB_DEVICE_HID_EVENT_REPORT_SENT,&d);return 0;}
    return -1;
}
int USB_DEVICE_IRPCancelAll(USB_DEVICE_HANDLE handle,uint8_t endpoint)
{
    assert(handle==1);
    return USB_DEVICE_HID_TransferCancel(0,endpoint==0x01?rh:th);
}
static void Reset(void)
{
    rx_owned=tx_owned=reject_rx=reject_tx=cancel_fail=false;
    live_rx_error=live_tx_error=false;receives=sends=cancels=0;
    USB_PROBE_Initialize(1);USB_PROBE_Configure(true);USB_PROBE_Tasks(0);USB_PROBE_Tasks(10);
    assert(rx_owned&&receives==1);
}
static void Deliver(const uint8_t *bytes,size_t length,uint32_t now)
{
    assert(rx_owned&&length<=64);memcpy(rx,bytes,length);rx_owned=false;
    USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED d={length,rh,0};
    USB_PROBE_Event(USB_DEVICE_HID_EVENT_REPORT_RECEIVED,&d);USB_PROBE_Tasks(now);
}
static size_t Query(uint8_t *out,uint8_t opcode,uint8_t version,uint32_t id)
{
    uint8_t p[]={opcode,version,(uint8_t)id,(uint8_t)(id>>8),(uint8_t)(id>>16),(uint8_t)(id>>24)};
    size_t n=0;assert(UPDATE_FrameEncode(p,sizeof(p),out,64,&n));return n;
}
static void Sent(uint32_t now)
{
    assert(tx_owned&&!memcmp(tx,sent,64));tx_owned=false;
    USB_DEVICE_HID_EVENT_DATA_REPORT_SENT d={64,th,0};USB_PROBE_Event(USB_DEVICE_HID_EVENT_REPORT_SENT,&d);
    USB_PROBE_Tasks(now);
}
int main(void)
{
    uint8_t wire[64];size_t n;
    Reset();assert(!USB_PROBE_RetireReady());USB_PROBE_Configure(false);
    bool irq=EVIC_INT_Disable();assert(USB_DEVICE_IRPCancelAll(1,0x01)==0);EVIC_INT_Restore(irq);
    assert(USB_PROBE_RetireReady()&&!usb_probe_diagnostics.rx_pending);
    Reset();n=Query(wire,0x60,1,0x01041021);
    /* A trailing DLE at first chunk survives into the next report. */
    size_t split=0;for(size_t i=0;i<n;i++)if(wire[i]==16){split=i+1;break;}
    assert(split);Deliver(wire,split,11);assert(rx_owned&&sends==0);
    Deliver(wire+split,n-split,12);assert(tx_owned&&sends==1&&usb_probe_diagnostics.queries==1);
    UPDATE_FRAME frame;UPDATE_FrameReset(&frame);size_t used;
    assert(UPDATE_FrameFeed(&frame,sent,64,&used)==UPDATE_FRAME_READY);
    assert(frame.length==28&&frame.bytes[0]==0xE0&&frame.bytes[1]==1&&frame.bytes[4]==0x21);
    assert(!memcmp(frame.bytes+8,"PKBL",4));for(size_t i=used;i<64;i++)assert(sent[i]==0);
    assert(!rx_owned);Sent(13);assert(rx_owned&&usb_probe_diagnostics.responses==1);

    /* Every destructive/legacy request is rejected, no response or operations. */
    for(unsigned op=0;op<256;op++)if(op!=0x60)
    {
        Reset();n=Query(wire,(uint8_t)op,1,1);Deliver(wire,n,11);
        assert(!tx_owned&&!sends&&usb_probe_diagnostics.rejected==1);
    }
    Reset();n=Query(wire,0x60,2,1);Deliver(wire,n,11);assert(usb_probe_diagnostics.rejected==1);
    Reset();n=Query(wire,0x60,1,0);Deliver(wire,n,11);assert(usb_probe_diagnostics.rejected==1);
    Reset();Deliver((uint8_t[]){1,2,0x55,0x55,4},5,11);assert(usb_probe_diagnostics.frame_errors==1&&!tx_owned);
    Reset();n=Query(wire,0x60,1,1);wire[n]=0x42;Deliver(wire,n+1,11);assert(usb_probe_diagnostics.rejected==1);
    Reset();n=Query(wire,0x60,1,1);memset(wire+n,0,64-n);Deliver(wire,64,11);assert(tx_owned);Sent(12);

    /* Partial frame timeout resets escaped state; later fresh valid frame works. */
    Reset();Deliver((uint8_t[]){1,16},2,11);USB_PROBE_Tasks(1011);
    assert(usb_probe_diagnostics.timeouts==1&&rx_owned);
    n=Query(wire,0x60,1,7);Deliver(wire,n,1012);assert(tx_owned);Sent(1013);
    Reset();reject_tx=true;n=Query(wire,0x60,1,1);Deliver(wire,n,11);
    assert(!tx_owned&&sends==1);USB_PROBE_Tasks(12);assert(sends==1);
    reject_tx=false;USB_PROBE_Tasks(21);assert(tx_owned);Sent(22);
    Reset();n=Query(wire,0x60,1,1);Deliver(wire,n,11);assert(tx_owned);
    USB_PROBE_Tasks(1011);assert(cancels==1&&!tx_owned);USB_PROBE_Tasks(1012);
    assert(!usb_probe_diagnostics.tx_pending&&rx_owned);

    /* Buffer ownership is preserved if cancellation cannot complete. */
    Reset();n=Query(wire,0x60,1,1);Deliver(wire,n,11);cancel_fail=true;
    USB_PROBE_Configure(false);USB_PROBE_Tasks(12);assert(tx_owned&&cancels==1&&!memcmp(tx,sent,64));
    USB_PROBE_Configure(true);USB_PROBE_Tasks(13);assert(tx_owned&&sends==1);
    cancel_fail=false;USB_PROBE_Tasks(14);assert(!tx_owned);USB_PROBE_Tasks(24);assert(rx_owned);
    Reset();USB_PROBE_Configure(false);USB_PROBE_Tasks(11);assert(!rx_owned&&!usb_probe_diagnostics.rx_pending);
    USB_PROBE_Configure(true);USB_PROBE_Tasks(12);USB_PROBE_Tasks(22);assert(rx_owned);
    /* Wrong handle completion must not release current report buffer. */
    USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED wrong={64,999,0};USB_PROBE_Event(USB_DEVICE_HID_EVENT_REPORT_RECEIVED,&wrong);
    USB_PROBE_Tasks(23);assert(rx_owned);
    /* Transient RX queue failure retries only after backoff. */
    Reset();USB_PROBE_Configure(false);USB_PROBE_Tasks(11);reject_rx=true;
    USB_PROBE_Configure(true);USB_PROBE_Tasks(12);USB_PROBE_Tasks(22);
    unsigned attempts=receives;USB_PROBE_Tasks(23);assert(receives==attempts);
    reject_rx=false;USB_PROBE_Tasks(32);assert(rx_owned);
    /* A completion error/oversized report is discarded without inspecting bytes. */
    rx_owned=false;USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED bad={65,rh,0};
    USB_PROBE_Event(USB_DEVICE_HID_EVENT_REPORT_RECEIVED,&bad);USB_PROBE_Tasks(33);
    assert(!tx_owned&&usb_probe_diagnostics.rx_errors==2);USB_PROBE_Tasks(43);assert(rx_owned);
    /* Post-submit error with a live handle must NOT cause buffer reuse. */
    Reset();live_tx_error=true;n=Query(wire,0x60,1,9);Deliver(wire,n,11);
    assert(tx_owned&&usb_probe_diagnostics.tx_pending);
    USB_PROBE_Tasks(21);assert(sends==1&&!memcmp(tx,sent,64));Sent(22);
    assert(usb_probe_diagnostics.responses==1&&rx_owned);
    Reset();USB_PROBE_Configure(false);USB_PROBE_Tasks(11);
    live_rx_error=true;USB_PROBE_Configure(true);USB_PROBE_Tasks(12);USB_PROBE_Tasks(22);
    assert(rx_owned&&usb_probe_diagnostics.rx_pending);attempts=receives;
    USB_PROBE_Tasks(32);assert(receives==attempts);
    USB_PROBE_Configure(false);USB_PROBE_Tasks(33);assert(!rx_owned);
    puts("PASS: read-only capabilities, split frames/padding/CRC, legacy/write rejection, queue retries/completion/timeout, disconnect cancellation and buffer quarantine; no flash API.");
    return 0;
}
