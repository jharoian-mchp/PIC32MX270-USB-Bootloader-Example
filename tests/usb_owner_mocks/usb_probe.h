#include "definitions.h"
void USB_PROBE_Initialize(USB_DEVICE_HANDLE);
void USB_PROBE_Configure(bool);
void USB_PROBE_Event(USB_DEVICE_HID_EVENT,void*);
void USB_PROBE_Tasks(uint32_t);
