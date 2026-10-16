#ifndef USB_PROBE_H
#define USB_PROBE_H
#include "definitions.h"

typedef struct {
    uint32_t queries, responses, rejected, frame_errors, rx_errors, tx_errors;
    uint32_t timeouts, cancel_errors;
    bool active, rx_pending, tx_pending;
} USB_PROBE_DIAGNOSTICS;
extern volatile USB_PROBE_DIAGNOSTICS usb_probe_diagnostics;
/* One client, one RX and one TX buffer. Read-only CAPABILITIES endpoint only. */
void USB_PROBE_Initialize(USB_DEVICE_HANDLE handle);
void USB_PROBE_Configure(bool configured); /* Event context, no submissions. */
void USB_PROBE_Event(USB_DEVICE_HID_EVENT event, void *data);
void USB_PROBE_Tasks(uint32_t now_ms);
/* No USB register/API calls: consume recorded completions and confirm buffer
 * ownership released before retirement/reinitialization. Main owner only. */
bool USB_PROBE_RetireReady(void);
#endif
