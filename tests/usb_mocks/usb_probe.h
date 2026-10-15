#ifndef USB_PROBE_MOCK_H
#define USB_PROBE_MOCK_H
#include "definitions.h"
/* Board state-machine unit tests mock the separately tested HID endpoint. */
void USB_PROBE_Initialize(USB_DEVICE_HANDLE handle);
void USB_PROBE_Configure(bool active);
void USB_PROBE_Event(USB_DEVICE_HID_EVENT event,void *data);
void USB_PROBE_Tasks(uint32_t now);
#endif
