#ifndef USB_RESTART_H
#define USB_RESTART_H
#include <stdbool.h>
#include <stdint.h>
/* Inactive READ-ONLY lifecycle coordinator: no NVM or generated USB binding.
 * Port owner retains all IRP/buffer/mutex storage through shutdown/drain. */
typedef enum {
    USB_RESTART_READY=0,USB_RESTART_ACK_WAIT,USB_RESTART_DRAIN,
    USB_RESTART_OFF_WAIT,USB_RESTART_RETIRE,USB_RESTART_INIT,
    USB_RESTART_CONFIG_WAIT,USB_RESTART_DONE,USB_RESTART_FAILED
} USB_RESTART_STATE;
typedef struct {
    void *context;
    /* Atomic owner fence: block ALL submission paths (HID/EP0/tasks/ISR) and
     * retain it through drain/off/retirement. initialize_usb_only may replace
     * the old fenced generation only after no old submissions can occur. */
    bool (*fence_submissions)(void *context);
    bool (*cancel_and_detach)(void *context); /* Request only; not drain proof. */
    bool (*transfers_drained)(void *context); /* All IRPs including EP0+HID. */
    bool (*mask_and_power_off)(void *context); /* Must not retire buffers/mutex. */
    bool (*module_off_idle)(void *context); /* U1PWRC USBPWR=USBBUSY=0 only. */
    bool (*retire_resources)(void *context); /* Only off idle and drained. */
    bool (*initialize_usb_only)(void *context,uint32_t generation); /* Never SYS_Initialize. */
    bool (*configured)(void *context,uint32_t generation);
} USB_RESTART_PORT;
typedef struct {
    USB_RESTART_PORT port;
    uint32_t generation,ticket,since,timeout_ms;
    USB_RESTART_STATE state;
    bool initialized,offline_confirmed;
} USB_RESTART;
/* New/quiescent storage only. No active integration calls this coordinator. */
bool USB_RestartInit(USB_RESTART *s,const USB_RESTART_PORT *port,uint32_t timeout_ms);
bool USB_RestartRequest(USB_RESTART *s,uint32_t now,uint32_t *ticket);
/* Full LOCAL pause-response completion; wrong/late/failed callbacks cannot
 * authorise teardown. Host receipt/reconnect protocol is outside this module. */
void USB_RestartAck(USB_RESTART *s,uint32_t ticket,bool success,uint32_t now);
USB_RESTART_STATE USB_RestartPoll(USB_RESTART *s,uint32_t now);
/* Unexpected cable/reset/task fault -> FAILED; keep storage, no hidden retry.
 * FAILED is quarantine, not proof that USB is off or transfers are released. */
void USB_RestartCancel(USB_RESTART *s);
#endif
