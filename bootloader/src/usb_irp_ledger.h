#ifndef USB_IRP_LEDGER_H
#define USB_IRP_LEDGER_H
#include <stdbool.h>
#include <stddef.h>
/* Pure ownership bookkeeping. Caller serialises mutations (target IRQ guard).
 * No pointer-generation magic: old callback after slot reuse remains ambiguous.
 * FREE only when API submission and claimed callback have BOTH returned. */
typedef struct {bool armed,submitting,callback,transmit;} USB_IRP_SLOT;
bool USB_IrpAvailable(const USB_IRP_SLOT *slot);
bool USB_IrpReserve(USB_IRP_SLOT *slot,bool transmit);
bool USB_IrpSubmitResult(USB_IRP_SLOT *slot,bool driver_owned);
bool USB_IrpClaim(USB_IRP_SLOT *slot,bool transmit);
bool USB_IrpRelease(USB_IRP_SLOT *slot);
bool USB_IrpDrained(const USB_IRP_SLOT *slots,size_t count);
#endif
