#ifndef USB_REBUILD_H
#define USB_REBUILD_H
#include "usb_retirement.h"
#if USB_RETIREMENT_HOOKS
#include "usb/usb_device.h"
#include "usb/usb_device_hid.h"
/* Default-off owning initialization entrypoint. Rebuilds only USB, detached,
 * masked; no board tasks/attach/NVM/session reset. Caller already retired all
 * old software/hardware refs and must bind fresh generation callback context.
 * Partial failure permanently poisons retirement gate; no automatic retry. */
bool USB_RetirementBindDevice(USB_DEVICE_EVENT_HANDLER handler,uintptr_t context,USB_DEVICE_HANDLE *handle);
bool USB_RetirementBindHid(USB_DEVICE_HID_EVENT_HANDLER handler,uintptr_t context);
bool USB_RebuildDetached(uint32_t generation,USB_DEVICE_EVENT_HANDLER device_callback,
                         USB_DEVICE_HID_EVENT_HANDLER hid_callback,uintptr_t context,
                         USB_DEVICE_HANDLE *handle);
#endif
#endif
