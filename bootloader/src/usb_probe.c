/* Read-only query receiver. Never dispatches legacy/update/session commands. */
#include "usb_probe.h"
#include "update_wire.h"
#include <string.h>
#define REPORT_SIZE 64U
#define FRAME_TIMEOUT_MS 1000U
#define SEND_TIMEOUT_MS 1000U
#define RETRY_MS 10U

volatile USB_PROBE_DIAGNOSTICS usb_probe_diagnostics;
static USB_DEVICE_HANDLE usb_handle;
static uint8_t __attribute__((aligned(16))) rx_buffer[REPORT_SIZE];
static uint8_t __attribute__((aligned(16))) tx_buffer[REPORT_SIZE];
static UPDATE_FRAME frame;
static USB_DEVICE_HID_TRANSFER_HANDLE rx_handle, tx_handle;
static volatile bool rx_complete, tx_complete, reset_requested;
static volatile size_t rx_length;
static volatile USB_DEVICE_HID_RESULT rx_status, tx_status;
static bool response_ready, frame_timed, send_timed, cancel_attempted;
static uint32_t frame_since, send_since, retry_since;

void USB_PROBE_Initialize(USB_DEVICE_HANDLE handle)
{
    usb_handle=handle;
    usb_probe_diagnostics=(USB_PROBE_DIAGNOSTICS){0};
    rx_handle=tx_handle=USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;
    rx_complete=tx_complete=false;reset_requested=true;
    response_ready=frame_timed=send_timed=cancel_attempted=false;
    retry_since=0;UPDATE_FrameReset(&frame);
}
void USB_PROBE_Configure(bool configured)
{
    usb_probe_diagnostics.active=configured;
    reset_requested=true; /* Main owns frame/buffer mutation. */
}
void USB_PROBE_Event(USB_DEVICE_HID_EVENT event,void *data)
{
    if(event==USB_DEVICE_HID_EVENT_REPORT_RECEIVED&&data!=NULL)
    {
        USB_DEVICE_HID_EVENT_DATA_REPORT_RECEIVED *r=data;
        if(usb_probe_diagnostics.rx_pending&&r->handle==rx_handle)
        {rx_length=r->length;rx_status=r->status;rx_complete=true;}
    }
    else if(event==USB_DEVICE_HID_EVENT_REPORT_SENT&&data!=NULL)
    {
        USB_DEVICE_HID_EVENT_DATA_REPORT_SENT *r=data;
        if(usb_probe_diagnostics.tx_pending&&r->handle==tx_handle)
        {tx_status=r->status;tx_complete=true;}
    }
}

static void CancelTransfers(void)
{
    /* Cancellation callback may be synchronous; interrupts remain excluded by
     * caller. Never reuse a buffer unless its pending transfer completed. */
    /* EP1 OUT/IN are fixed by generated descriptors. CancelAll flushes even
     * the queue head synchronously; TransferCancel can wait for an ISR which
     * may not run after cable loss. Never mutate the buffers during callbacks. */
    if(usb_probe_diagnostics.rx_pending&&!rx_complete)
        if(USB_DEVICE_IRPCancelAll(usb_handle,0x01U)!=USB_ERROR_NONE)
            ++usb_probe_diagnostics.cancel_errors;
    if(usb_probe_diagnostics.tx_pending&&!tx_complete)
        if(USB_DEVICE_IRPCancelAll(usb_handle,0x81U)!=USB_ERROR_NONE)
            ++usb_probe_diagnostics.cancel_errors;
}
static void CompletionTasks(void)
{
    if(tx_complete)
    {
        tx_complete=false;usb_probe_diagnostics.tx_pending=false;
        if(tx_status==USB_DEVICE_HID_RESULT_OK)++usb_probe_diagnostics.responses;
        else ++usb_probe_diagnostics.tx_errors;
        response_ready=send_timed=cancel_attempted=false;
    }
    if(rx_complete&&(reset_requested||!usb_probe_diagnostics.active))
    {rx_complete=false;usb_probe_diagnostics.rx_pending=false;}
}

void USB_PROBE_Tasks(uint32_t now)
{
    bool enabled=EVIC_INT_Disable();
    CompletionTasks();
    if(reset_requested)
    {
        CancelTransfers();CompletionTasks();
        UPDATE_FrameReset(&frame);frame_timed=false;
        if(usb_probe_diagnostics.rx_pending||usb_probe_diagnostics.tx_pending)
        {EVIC_INT_Restore(enabled);return;}
        response_ready=send_timed=cancel_attempted=false;
        reset_requested=false;retry_since=now;
    }
    if(!usb_probe_diagnostics.active)
    {EVIC_INT_Restore(enabled);return;}
    if(rx_complete)
    {
        rx_complete=false;usb_probe_diagnostics.rx_pending=false;
        if(rx_status!=USB_DEVICE_HID_RESULT_OK||rx_length>REPORT_SIZE)
        {++usb_probe_diagnostics.rx_errors;UPDATE_FrameReset(&frame);frame_timed=false;retry_since=now;}
        else
        {
            size_t consumed=0;
            UPDATE_FRAME_STATUS status=UPDATE_FrameFeed(&frame,rx_buffer,rx_length,&consumed);
            if(status==UPDATE_FRAME_READY)
            {
                bool trailing=false;for(size_t i=consumed;i<rx_length;i++)if(rx_buffer[i])trailing=true;
                UPDATE_WIRE_QUERY query;uint8_t payload[28];size_t length=0,wire_length=0;
                if(trailing||!UPDATE_WireQuery(frame.bytes,frame.length,&query)||
                   !UPDATE_WireCapabilities(&query,payload,sizeof(payload),&length)||
                   !UPDATE_FrameEncode(payload,length,tx_buffer,sizeof(tx_buffer),&wire_length))
                    ++usb_probe_diagnostics.rejected;
                else
                {
                    memset(tx_buffer+wire_length,0,sizeof(tx_buffer)-wire_length);
                    response_ready=true;send_timed=false;cancel_attempted=false;
                    retry_since=now-RETRY_MS;
                    ++usb_probe_diagnostics.queries;
                }
                UPDATE_FrameReset(&frame);frame_timed=false;
            }
            else if(status>=UPDATE_FRAME_CRC_ERROR)
            {++usb_probe_diagnostics.frame_errors;UPDATE_FrameReset(&frame);frame_timed=false;}
            else if(frame.receiving&&!frame_timed){frame_since=now;frame_timed=true;}
        }
    }
    if(frame_timed&&(uint32_t)(now-frame_since)>=FRAME_TIMEOUT_MS)
    {++usb_probe_diagnostics.timeouts;UPDATE_FrameReset(&frame);frame_timed=false;}
    if(response_ready)
    {
        if(!send_timed){send_since=now;send_timed=true;}
        if((uint32_t)(now-send_since)>=SEND_TIMEOUT_MS)
        {
            if(!cancel_attempted)
            {
                ++usb_probe_diagnostics.timeouts;cancel_attempted=true;
                if(usb_probe_diagnostics.tx_pending)
                {
                    if(USB_DEVICE_IRPCancelAll(usb_handle,0x81U)!=USB_ERROR_NONE)
                        ++usb_probe_diagnostics.cancel_errors;
                }
                else response_ready=send_timed=false;
            }
        }
        else if(!usb_probe_diagnostics.tx_pending&&(uint32_t)(now-retry_since)>=RETRY_MS)
        {
            tx_complete=false;usb_probe_diagnostics.tx_pending=true;
            tx_handle=USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;
            USB_DEVICE_HID_RESULT status=USB_DEVICE_HID_ReportSend(USB_DEVICE_HID_INDEX_0,&tx_handle,tx_buffer,sizeof(tx_buffer));
            if(status!=USB_DEVICE_HID_RESULT_OK)
            {
                /* A post-submit OSAL error may still leave a live IRP handle.
                 * Quarantine that buffer until completion/cancellation. */
                usb_probe_diagnostics.tx_pending=tx_handle!=USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;
                ++usb_probe_diagnostics.tx_errors;retry_since=now;
            }
        }
    }
    else if(!usb_probe_diagnostics.rx_pending&&(uint32_t)(now-retry_since)>=RETRY_MS)
    {
        rx_complete=false;usb_probe_diagnostics.rx_pending=true;
        rx_handle=USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;
        USB_DEVICE_HID_RESULT status=USB_DEVICE_HID_ReportReceive(USB_DEVICE_HID_INDEX_0,&rx_handle,rx_buffer,sizeof(rx_buffer));
        if(status!=USB_DEVICE_HID_RESULT_OK)
        {
            usb_probe_diagnostics.rx_pending=rx_handle!=USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;
            ++usb_probe_diagnostics.rx_errors;retry_since=now;
        }
    }
    EVIC_INT_Restore(enabled);
}
