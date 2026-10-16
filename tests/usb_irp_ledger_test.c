#include <assert.h>
#include <stdio.h>
#include "usb_irp_ledger.h"
int main(void)
{
    USB_IRP_SLOT s[2]={{0}};assert(USB_IrpDrained(s,2));
    assert(USB_IrpReserve(&s[0],false)&&!USB_IrpAvailable(&s[0]));
    assert(!USB_IrpReserve(&s[0],true));assert(USB_IrpSubmitResult(&s[0],true));
    assert(!USB_IrpClaim(&s[0],true));assert(USB_IrpClaim(&s[0],false));
    assert(!USB_IrpClaim(&s[0],false)&&!USB_IrpAvailable(&s[0]));
    assert(USB_IrpReserve(&s[1],false)&&USB_IrpSubmitResult(&s[1],true));
    assert(USB_IrpRelease(&s[0])&&!USB_IrpClaim(&s[0],false)); /* other RX still pending */
    assert(!USB_IrpDrained(s,2));assert(USB_IrpClaim(&s[1],false)&&USB_IrpRelease(&s[1]));
    /* Completion before submission API returns holds reuse until API settles. */
    assert(USB_IrpReserve(&s[0],true));assert(USB_IrpClaim(&s[0],true));assert(USB_IrpRelease(&s[0]));
    assert(!USB_IrpAvailable(&s[0]));assert(USB_IrpSubmitResult(&s[0],true)&&USB_IrpAvailable(&s[0]));
    /* Failed unowned submit releases; failure with pending driver ownership doesn't. */
    assert(USB_IrpReserve(&s[0],false));assert(USB_IrpSubmitResult(&s[0],false)&&USB_IrpAvailable(&s[0]));
    assert(USB_IrpReserve(&s[0],false));assert(USB_IrpSubmitResult(&s[0],true)&&!USB_IrpAvailable(&s[0]));
    assert(USB_IrpClaim(&s[0],false)&&USB_IrpRelease(&s[0]));
    assert(!USB_IrpRelease(&s[0])&&!USB_IrpSubmitResult(&s[0],false));
    assert(!USB_IrpReserve(NULL,false)&&!USB_IrpClaim(NULL,false)&&!USB_IrpDrained(NULL,1));
    puts("PASS: per-slot reserve/claim/release, duplicate with other pending IRP, callback-before-submit-return, unowned failure rollback and owned failure retention; no pointer-reuse generation guarantee.");return 0;
}
