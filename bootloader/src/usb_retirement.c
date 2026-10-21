#include "usb_retirement.h"
#if USB_RETIREMENT_HOOKS
#include <xc.h>
#include <limits.h>
#if USB_RESTART_DIAGNOSTIC
#if defined(__XC32)
#include "definitions.h"
_Static_assert(CPU_CLOCK_FREQUENCY==48000000U,"power evidence requires nominal 24MHz Count");
_Static_assert(DRV_USBFS_INSTANCES_NUMBER==1U,"power evidence is single-instance");
#endif
static USB_POWER_EVIDENCE power_evidence;
static void (*power_observer)(const USB_POWER_EVIDENCE *,bool);
void USB_RetirementPowerObserver(void (*observer)(const USB_POWER_EVIDENCE *,bool))
{power_observer=observer;}
void USB_RetirementPowerReset(void){power_evidence=(USB_POWER_EVIDENCE){0};}
static void PowerSample(unsigned site,uint32_t count,uint32_t power,bool commit)
{
    uint16_t bits=(uint16_t)(((power&_U1PWRC_USBPWR_MASK)!=0U?1U:0U)|((power&_U1PWRC_USBBUSY_MASK)!=0U?2U:0U));
    if(site==0U)power_evidence.pre_count=count;
    else if(site==1U)power_evidence.post_delta=count-power_evidence.pre_count;
    else power_evidence.ready_delta=count-power_evidence.pre_count;
    power_evidence.meta|=(uint16_t)((1U<<site)|(bits<<(3U+2U*site)));
    if(power_observer!=NULL)power_observer(&power_evidence,commit);
}
void USB_RetirementPowerEnableSample(bool after)
{
    unsigned site=after?1U:0U;
    if(!USB_RetirementPreparing()||(power_evidence.meta&(1U<<site))!=0U||
       (after&&(power_evidence.meta&1U)==0U))return;
    uint32_t count=_CP0_GET_COUNT();uint32_t power=U1PWRC;
    PowerSample(site,count,power,true);
}
void USB_RetirementPowerReadySample(uint32_t power)
{
    if(!USB_RetirementPreparing()||(power_evidence.meta&4U)!=0U||(power_evidence.meta&3U)!=3U)return;
    PowerSample(2U,_CP0_GET_COUNT(),power,false); /* Original predicate power sample, then Count. */
}
#endif
static volatile bool fenced,poisoned,power_off_authorized;
static volatile uint32_t callbacks,admissions;
static uint32_t retired_layers,generation=1,preparing_generation;
static void (*detail_observer)(uint32_t);
void USB_RetirementDetailObserver(void (*observer)(uint32_t))
{detail_observer=observer;}
void USB_RetirementDetail(USB_REBUILD_DETAIL_STAGE stage,uint16_t rejected,uint8_t observation)
{
    if(detail_observer!=NULL)detail_observer((uint32_t)stage|((uint32_t)rejected<<8)|((uint32_t)(observation&7U)<<24));
}
bool USB_RetirementBlocked(void){return fenced||poisoned;}
void USB_RetirementFault(void)
{
    uint32_t irq=__builtin_disable_interrupts();poisoned=true;__builtin_mtc0(12,0,irq);
}
bool USB_RetirementFence(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    bool ok=!fenced&&!poisoned&&callbacks==0U&&admissions==0U;
    if(ok){fenced=true;retired_layers=0;preparing_generation=0;power_off_authorized=false;}
    __builtin_mtc0(12,0,irq);return ok;
}
bool USB_RetirementCallbacksIdle(void){return !poisoned&&callbacks==0U&&admissions==0U;}
bool USB_RetirementAdmissionBegin(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    bool ok=!fenced&&!poisoned&&admissions!=UINT32_MAX;
    if(ok)++admissions;
    __builtin_mtc0(12,0,irq);return ok;
}
void USB_RetirementAdmissionEnd(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    if(admissions==0U)poisoned=true;else --admissions;
    __builtin_mtc0(12,0,irq);
}
void USB_RetirementPowerOffAuthorized(void){power_off_authorized=true;}
bool USB_RetirementRetirementAuthorized(void)
{return fenced&&power_off_authorized&&preparing_generation==0U&&USB_RetirementCallbacksIdle()&&USB_RetirementOffIdle();}
void USB_RetirementLayerRetired(unsigned layer)
{
    if(layer!=USB_RETIREMENT_LAYER_HID&&layer!=USB_RETIREMENT_LAYER_DEVICE&&layer!=USB_RETIREMENT_LAYER_DRIVER)
    {USB_RetirementFault();return;}
    if(!USB_RetirementRetirementAuthorized()){USB_RetirementFault();return;}
    retired_layers|=layer;
}
bool USB_RetirementPrepare(uint32_t next)
{
    uint32_t irq=__builtin_disable_interrupts();
    bool ok=USB_RetirementRetirementAuthorized()&&retired_layers==7U&&next!=0U&&next>generation;
    if(ok)preparing_generation=next;
    USB_RetirementDetail(USB_REBUILD_DETAIL_PREPARE,ok?0U:1U,0);
    __builtin_mtc0(12,0,irq);return ok;
}
bool USB_RetirementPreparing(void)
{return fenced&&!poisoned&&retired_layers==7U&&preparing_generation!=0U&&USB_RetirementCallbacksIdle();}
bool USB_RetirementReopen(uint32_t next)
{
    uint32_t irq=__builtin_disable_interrupts();
    bool ok=false;
    if(!USB_RetirementPreparing())USB_RetirementDetail(USB_REBUILD_DETAIL_REOPEN,1U,0);
    else if(next!=preparing_generation)USB_RetirementDetail(USB_REBUILD_DETAIL_REOPEN,2U,0);
    else ok=USB_RetirementHidReplacementReady()&&USB_RetirementDeviceReplacementReady()&&USB_RetirementDriverReplacementReady();
    if(ok){generation=next;preparing_generation=0;retired_layers=0;power_off_authorized=false;fenced=false;}
    __builtin_mtc0(12,0,irq);return ok;
}
void USB_RetirementCallbackEnter(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    if(callbacks==UINT32_MAX)poisoned=true;else ++callbacks;
    __builtin_mtc0(12,0,irq);
}
void USB_RetirementCallbackLeave(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    if(callbacks==0U)poisoned=true;else --callbacks;
    __builtin_mtc0(12,0,irq);
}
bool USB_RetirementOffIdle(void)
{return (U1PWRC&(_U1PWRC_USBPWR_MASK|_U1PWRC_USBBUSY_MASK))==0U;}
#endif
