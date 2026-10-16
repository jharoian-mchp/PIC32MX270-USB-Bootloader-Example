#include "usb_irp_ledger.h"
bool USB_IrpAvailable(const USB_IRP_SLOT *s)
{return s!=NULL&&!s->armed&&!s->submitting&&!s->callback;}
bool USB_IrpReserve(USB_IRP_SLOT *s,bool tx)
{
    if(!USB_IrpAvailable(s))return false;
    *s=(USB_IRP_SLOT){.armed=true,.submitting=true,.transmit=tx};return true;
}
bool USB_IrpSubmitResult(USB_IRP_SLOT *s,bool owned)
{
    if(s==NULL||!s->submitting)return false;
    s->submitting=false;
    /* If no callback has claimed the reservation, failed non-owned submit
     * releases it. Error alone never releases an IRP still owned by driver. */
    if(!owned&&s->armed)s->armed=false;
    return true;
}
bool USB_IrpClaim(USB_IRP_SLOT *s,bool tx)
{
    if(s==NULL||!s->armed||s->callback||s->transmit!=tx)return false;
    s->armed=false;s->callback=true;return true;
}
bool USB_IrpRelease(USB_IRP_SLOT *s)
{
    if(s==NULL||!s->callback)return false;
    s->callback=false;return true;
}
bool USB_IrpDrained(const USB_IRP_SLOT *slots,size_t count)
{
    if(slots==NULL)return false;
    for(size_t i=0;i<count;i++)if(!USB_IrpAvailable(&slots[i]))return false;
    return true;
}
