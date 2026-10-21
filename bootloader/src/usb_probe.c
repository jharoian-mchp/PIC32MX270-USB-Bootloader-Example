/* Read-only query receiver. Never dispatches legacy/update/session commands. */
#include "usb_probe.h"
#include "update_wire.h"
#include "usb_restart_owner.h"
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
#if USB_RESTART_DIAGNOSTIC
static uint32_t pause_completion_ticket,task_now;
static bool tx_full;
static uint32_t Get32(const uint8_t *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void Put32(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8U*i));}
static bool DiagnosticReply(const uint8_t *p,size_t n,uint8_t *out,size_t *length)
{
    if(n<6U||p[1]!=1U||Get32(p+2)==0U)return false;
    uint32_t ticket=0,generation=0;
    if(p[0]==0x66U&&n==10U)
    {
        unsigned status=USB_RestartOwnerPause(Get32(p+2),Get32(p+6),task_now,&ticket);
        if(status==2U)return false;
        if(status==0U)pause_completion_ticket=ticket;
    }
    else if(p[0]==0x68U&&n==6U)
    {
        USB_RESTART_RECORD r={0};bool valid=USB_RestartOwnerPrevious(&r);
        out[0]=0xE8;out[1]=1;out[2]=0;out[3]=0;Put32(out+4,Get32(p+2));
        Put32(out+8,r.ticket);Put32(out+12,r.generation);Put32(out+16,r.time_ms);
        out[20]=(uint8_t)r.state;out[21]=(uint8_t)r.phase;out[22]=(uint8_t)r.fault;out[23]=r.evidence;
        out[24]=valid?1U:0U;out[25]=(uint8_t)r.checkpoint;Put32(out+26,r.detail);*length=30;return true;
    }
    else if(p[0]!=0x67U||n!=6U)return false;
    USB_RestartOwnerStatus(&ticket,&generation);
    out[0]=(uint8_t)(p[0]|0x80U);out[1]=1;out[2]=0;out[3]=0;
    Put32(out+4,Get32(p+2));Put32(out+8,ticket);Put32(out+12,generation);
    Put32(out+16,usb_restart_owner_diagnostics.completed_cycles);
    out[20]=(uint8_t)usb_restart_owner_diagnostics.state;*length=21U;return true;
}
#endif

void USB_PROBE_Initialize(USB_DEVICE_HANDLE handle)
{
    usb_handle=handle;
    usb_probe_diagnostics=(USB_PROBE_DIAGNOSTICS){0};
    rx_handle=tx_handle=USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;
    rx_complete=tx_complete=false;reset_requested=true;
    response_ready=frame_timed=send_timed=cancel_attempted=false;
    retry_since=0;UPDATE_FrameReset(&frame);
#if USB_RESTART_DIAGNOSTIC
    pause_completion_ticket=0;tx_full=false;
#endif
}
void USB_PROBE_Configure(bool configured)
{
    usb_probe_diagnostics.active=configured;
    reset_requested=true; /* Main owns frame/buffer mutation and ACK decisions. */
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
        {tx_status=r->status;tx_complete=true;
#if USB_RESTART_DIAGNOSTIC
         tx_full=r->length==REPORT_SIZE;
#endif
        }
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
#if USB_RESTART_DIAGNOSTIC
        if(pause_completion_ticket!=0U)
        {
            uint32_t ticket=pause_completion_ticket;pause_completion_ticket=0;
            /* CompletionTasks runs with GLOBAL interrupts excluded. OwnerAck
             * checks queued reset/error in that same nested critical section,
             * then fences/cancels; no ISR gap between decision and fencing. */
            USB_RestartOwnerAck(ticket,tx_status==USB_DEVICE_HID_RESULT_OK&&tx_full&&!cancel_attempted&&!reset_requested&&
                                send_timed&&(uint32_t)(task_now-send_since)<SEND_TIMEOUT_MS,task_now);
        }
#endif
        response_ready=send_timed=cancel_attempted=false;
    }
    if(rx_complete&&(reset_requested||!usb_probe_diagnostics.active))
    {rx_complete=false;usb_probe_diagnostics.rx_pending=false;}
}

bool USB_PROBE_RetireReady(void)
{
    bool irq=EVIC_INT_Disable();CompletionTasks();
    bool ready=!usb_probe_diagnostics.rx_pending&&!usb_probe_diagnostics.tx_pending;
    EVIC_INT_Restore(irq);return ready;
}

void USB_PROBE_Tasks(uint32_t now)
{
    bool enabled=EVIC_INT_Disable();
#if USB_RESTART_DIAGNOSTIC
    task_now=now;
#endif
    CompletionTasks();
    if(reset_requested)
    {
        CancelTransfers();CompletionTasks();
        UPDATE_FrameReset(&frame);frame_timed=false;
        if(usb_probe_diagnostics.rx_pending||usb_probe_diagnostics.tx_pending)
        {EVIC_INT_Restore(enabled);return;}
#if USB_RESTART_DIAGNOSTIC
        if(pause_completion_ticket!=0U)
        {uint32_t ticket=pause_completion_ticket;pause_completion_ticket=0;USB_RestartOwnerAck(ticket,false,task_now);}
#endif
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
                UPDATE_WIRE_QUERY query;
#if USB_RESTART_DIAGNOSTIC
                uint8_t payload[30];
#else
                uint8_t payload[28];
#endif
                size_t length=0,wire_length=0;
#if USB_RESTART_DIAGNOSTIC
                bool valid=!trailing&&UPDATE_WireQuery(frame.bytes,frame.length,&query)&&
                           UPDATE_WireCapabilities(&query,payload,sizeof(payload),&length);
                if(!valid&&!trailing)valid=DiagnosticReply(frame.bytes,frame.length,payload,&length);
                if(!valid||!UPDATE_FrameEncode(payload,length,tx_buffer,sizeof(tx_buffer),&wire_length))
#else
                if(trailing||!UPDATE_WireQuery(frame.bytes,frame.length,&query)||
                   !UPDATE_WireCapabilities(&query,payload,sizeof(payload),&length)||
                   !UPDATE_FrameEncode(payload,length,tx_buffer,sizeof(tx_buffer),&wire_length))
#endif
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
                else
                {
#if USB_RESTART_DIAGNOSTIC
                    if(pause_completion_ticket!=0U)
                    {uint32_t ticket=pause_completion_ticket;pause_completion_ticket=0;USB_RestartOwnerAck(ticket,false,task_now);}
#endif
                    response_ready=send_timed=false;
                }
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
    else if(
#if USB_RESTART_DIAGNOSTIC
        usb_probe_diagnostics.active&&!reset_requested&&
#endif
        !usb_probe_diagnostics.rx_pending&&(uint32_t)(now-retry_since)>=RETRY_MS)
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
