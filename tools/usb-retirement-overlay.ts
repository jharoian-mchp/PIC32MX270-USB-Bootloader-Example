#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Apply/check opt-in owning-translation-unit USB retirement hooks after MCC regeneration","keywords":["usb","retirement","overlay","mcc"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import { readFileSync, writeFileSync } from "node:fs";
import { resolve } from "node:path";
const root = resolve(import.meta.dir,"..");process.chdir(root);
const write = process.argv.includes("--apply");
const folder="bootloader/src/config/default/";
const bases:Record<string,string>={
"usb/src/usb_device_hid.c":"827231ed7615048a8fb5a77f2184518f1f7b0e9e9f98cd04330890ca12b8e09a",
"usb/src/usb_device.c":"31a06bf9e6de7e73f89b9a774669ee4cbf8c8eff5df55eea1ce47803ec6247cc",
"driver/usb/usbfs/src/drv_usbfs.c":"fa8330d4d08dd7d50721bfb41e848929381faf60ffde449341921578d7c4280f",
"driver/usb/usbfs/src/drv_usbfs_device.c":"940b5a1e65b6dab7481f759115b1ea25d1e25a8d83a2c2fb1eb4a4a443a42be3",
"initialization.c":"fda6bfe92c2348379234bde5f15184162c999e1f81b5964a992715f76cf5f15a",
};
const hooks: Record<string,string>={
"usb/src/usb_device_hid.c": `
#if USB_RETIREMENT_HOOKS
#include "usb_irp_ledger.h"
static USB_IRP_SLOT usb_retirement_slots[USB_DEVICE_HID_QUEUE_DEPTH_COMBINED];
static unsigned USB_RetirementHidIndex(const void *pointer)
{
    uintptr_t p=(uintptr_t)pointer,base=(uintptr_t)&gUSBDeviceHIDIRP[0];
    if(p<base||p-base>=sizeof(gUSBDeviceHIDIRP)||(p-base)%sizeof(gUSBDeviceHIDIRP[0])!=0U)return USB_DEVICE_HID_QUEUE_DEPTH_COMBINED;
    return (unsigned)((p-base)/sizeof(gUSBDeviceHIDIRP[0]));
}
bool USB_RetirementHidSlotAvailable(unsigned i)
{return i<USB_DEVICE_HID_QUEUE_DEPTH_COMBINED&&USB_IrpAvailable(&usb_retirement_slots[i]);}
bool USB_RetirementHidReserve(void *irp,bool transmit)
{
    uint32_t irq=__builtin_disable_interrupts();unsigned i=USB_RetirementHidIndex(irp);
    bool ok=i<USB_DEVICE_HID_QUEUE_DEPTH_COMBINED&&gUSBDeviceHIDIRP[i].status<=USB_DEVICE_IRP_STATUS_COMPLETED_SHORT&&USB_IrpReserve(&usb_retirement_slots[i],transmit);
    __builtin_mtc0(12,0,irq);return ok;
}
bool USB_RetirementHidSubmitResult(void *pointer,bool success)
{
    uint32_t irq=__builtin_disable_interrupts();unsigned i=USB_RetirementHidIndex(pointer);
    if(i==USB_DEVICE_HID_QUEUE_DEPTH_COMBINED){USB_RetirementFault();__builtin_mtc0(12,0,irq);return false;}
    USB_DEVICE_IRP *irp=(USB_DEVICE_IRP *)pointer;USB_IRP_SLOT *slot=&usb_retirement_slots[i];
    bool rollback=!success&&slot->armed&&irp->status<=USB_DEVICE_IRP_STATUS_SETUP;
    bool owned=success||irp->status>USB_DEVICE_IRP_STATUS_SETUP;
    if(!USB_IrpSubmitResult(slot,owned))USB_RetirementFault();
    __builtin_mtc0(12,0,irq);return rollback;
}
bool USB_RetirementHidClaim(void *pointer,bool transmit)
{
    uint32_t irq=__builtin_disable_interrupts();unsigned i=USB_RetirementHidIndex(pointer);
    bool ok=USB_RetirementHidCompletionValid(pointer,transmit)&&i<USB_DEVICE_HID_QUEUE_DEPTH_COMBINED&&USB_IrpClaim(&usb_retirement_slots[i],transmit);
    __builtin_mtc0(12,0,irq);return ok;
}
void USB_RetirementHidRelease(void *pointer)
{
    uint32_t irq=__builtin_disable_interrupts();unsigned i=USB_RetirementHidIndex(pointer);
    if(i==USB_DEVICE_HID_QUEUE_DEPTH_COMBINED||!USB_IrpRelease(&usb_retirement_slots[i]))USB_RetirementFault();
    __builtin_mtc0(12,0,irq);
}
bool USB_RetirementHidCompletionValid(const void *pointer,bool transmit)
{
    uintptr_t p=(uintptr_t)pointer,base=(uintptr_t)&gUSBDeviceHIDIRP[0];
    if(p<base||p-base>=sizeof(gUSBDeviceHIDIRP)||(p-base)%sizeof(gUSBDeviceHIDIRP[0])!=0U)return false;
    const USB_DEVICE_IRP *irp=(const USB_DEVICE_IRP *)pointer;
    if(irp->userData>=USB_DEVICE_HID_INSTANCES_NUMBER||irp->status>USB_DEVICE_IRP_STATUS_COMPLETED_SHORT)return false;
    const USB_DEVICE_HID_INSTANCE *hid=&gUsbDeviceHidInstance[irp->userData];
    return transmit?hid->currentTxQueueSize!=0U:hid->currentRxQueueSize!=0U;
}
bool USB_RetirementHidDrained(void)
{
    if(!USB_RetirementBlocked()||!USB_RetirementCallbacksIdle()||!USB_IrpDrained(usb_retirement_slots,USB_DEVICE_HID_QUEUE_DEPTH_COMBINED))return false;
    for(unsigned i=0;i<USB_DEVICE_HID_QUEUE_DEPTH_COMBINED;i++)
        if(gUSBDeviceHIDIRP[i].status>USB_DEVICE_IRP_STATUS_COMPLETED_SHORT)return false;
    for(unsigned i=0;i<USB_DEVICE_HID_INSTANCES_NUMBER;i++)
        if(gUsbDeviceHidInstance[i].currentTxQueueSize||gUsbDeviceHidInstance[i].currentRxQueueSize)return false;
    return true;
}
bool USB_RetirementHidRetire(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    if(!USB_RetirementRetirementAuthorized()||!USB_RetirementHidDrained())
    {__builtin_mtc0(12,0,irq);return false;}
    /* Preserve the shared basic-OSAL mutex/one-time guard; reset only retired
     * IRP/instance storage after all references and callbacks are released. */
    memset(usb_retirement_slots,0,sizeof(usb_retirement_slots));
    memset(gUSBDeviceHIDIRP,0,sizeof(gUSBDeviceHIDIRP));
    memset(gUsbDeviceHidInstance,0,sizeof(gUsbDeviceHidInstance));
    USB_RetirementLayerRetired(USB_RETIREMENT_LAYER_HID);
    __builtin_mtc0(12,0,irq);return true;
}
bool USB_RetirementBindHid(USB_DEVICE_HID_EVENT_HANDLER callback,uintptr_t context)
{
    if(!USB_RetirementPreparing()||callback==NULL||!USB_IrpDrained(usb_retirement_slots,USB_DEVICE_HID_QUEUE_DEPTH_COMBINED))return false;
    gUsbDeviceHidInstance[0].appCallBack=callback;gUsbDeviceHidInstance[0].userData=context;return true;
}
bool USB_RetirementHidReplacementReady(void)
{
    uint16_t rejected=0;
    if(!USB_RetirementPreparing())rejected=1U;
    else if(!gUSBDeviceHidCommonDataObj.isMutexHidIrpInitialized)rejected=2U;
    else if(!USB_IrpDrained(usb_retirement_slots,USB_DEVICE_HID_QUEUE_DEPTH_COMBINED))rejected=4U;
    else if(gUsbDeviceHidInstance[0].appCallBack==NULL)rejected=8U;
    else if(gUsbDeviceHidInstance[0].currentTxQueueSize!=0U)rejected=16U;
    else if(gUsbDeviceHidInstance[0].currentRxQueueSize!=0U)rejected=32U;
    USB_RetirementDetail(USB_REBUILD_DETAIL_HID_READY,rejected,0);return rejected==0U;
}
#endif
`,
"usb/src/usb_device.c": `
#if USB_RETIREMENT_HOOKS
bool USB_RetirementDeviceDrained(void)
{
    if(!USB_RetirementBlocked()||!USB_RetirementCallbacksIdle())return false;
    for(unsigned i=0;i<USB_DEVICE_INSTANCES_NUMBER;i++)
    {
        USB_DEVICE_OBJ *d=&usbDeviceInstance[i];
        if(d->irpEp0Rx.status>USB_DEVICE_IRP_STATUS_SETUP||d->irpEp0Tx.status>USB_DEVICE_IRP_STATUS_SETUP)return false;
    }
    return true;
}
bool USB_RetirementDeviceRetire(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    if(!USB_RetirementRetirementAuthorized()||!USB_RetirementDeviceDrained())
    {__builtin_mtc0(12,0,irq);return false;}
    memset(usbDeviceInstance,0,sizeof(usbDeviceInstance));
    USB_RetirementLayerRetired(USB_RETIREMENT_LAYER_DEVICE);
    __builtin_mtc0(12,0,irq);return true;
}
bool USB_RetirementBindDevice(USB_DEVICE_EVENT_HANDLER callback,uintptr_t context,USB_DEVICE_HANDLE *handle)
{
    if(!USB_RetirementPreparing()||callback==NULL||handle==NULL)return false;
    USB_DEVICE_OBJ *d=&usbDeviceInstance[0];
    if(d->usbDeviceInstanceState!=SYS_STATUS_READY||d->usbCDHandle==DRV_HANDLE_INVALID||
       d->driverInterface==NULL||d->driverInterface->eventHandlerSet==NULL)return false;
    d->inUse=true;d->clientState=USB_DEVICE_CLIENT_STATUS_READY;d->callBackFunc=callback;d->context=context;
    d->driverInterface->eventHandlerSet(d->usbCDHandle,(uintptr_t)d,&F_USB_DEVICE_EventHandler);
    *handle=(USB_DEVICE_HANDLE)d;return true;
}
bool USB_RetirementDeviceReplacementReady(void)
{
    USB_DEVICE_OBJ *d=&usbDeviceInstance[0];
    uint16_t rejected=0;
    if(!USB_RetirementPreparing())rejected=1U;
    else if(d->usbDeviceInstanceState!=SYS_STATUS_READY)rejected=2U;
    else if(d->taskState!=USB_DEVICE_TASK_STATE_RUNNING)rejected=4U;
    else if(!d->inUse)rejected=8U;
    else if(d->callBackFunc==NULL)rejected=16U;
    else if(d->ptrMasterDescTable==NULL)rejected=32U;
    else if(d->driverInterface==NULL)rejected=64U;
    else if(d->usbDeviceStatusStruct.usbDeviceState!=USB_DEVICE_STATE_DETACHED)rejected=128U;
    else if(d->irpEp0Rx.status>USB_DEVICE_IRP_STATUS_SETUP)rejected=256U;
    else if(d->irpEp0Tx.status>USB_DEVICE_IRP_STATUS_SETUP)rejected=512U;
    USB_RetirementDetail(USB_REBUILD_DETAIL_DEVICE_READY,rejected,0);return rejected==0U;
}
#endif
`,
"driver/usb/usbfs/src/drv_usbfs.c": `
#if USB_RETIREMENT_HOOKS
bool USB_RetirementDriverDrained(void)
{
    if(!USB_RetirementBlocked()||!USB_RetirementCallbacksIdle())return false;
    for(unsigned i=0;i<DRV_USBFS_INSTANCES_NUMBER;i++)
    {
        DRV_USBFS_OBJ *d=&gDrvUSBGroup[i].gDrvUSBObj;
        if(!d->inUse)continue;
        if(d->inInterruptContext||d->endpointTable==NULL)return false;
        for(unsigned e=0;e<DRV_USBFS_ENDPOINTS_NUMBER*2U;e++)if(d->endpointTable[e].irpQueue!=NULL)return false;
        if(d->pBDT==NULL)return false;
        for(unsigned e=0;e<DRV_USBFS_ENDPOINTS_NUMBER*4U;e++)
        {
            uint8_t status=d->pBDT[e].byte[0];
            /* UOWN|STALL may deliberately persist without an IRP. Ordinary
             * owned buffers must finish/cancel before shutdown. */
            if((status&USBFS_UOWN)!=0U&&(status&USBFS_STALL_SET)==0U)return false;
        }
    }
    return true;
}
bool USB_RetirementPowerOff(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    bool ok=USB_RetirementHidDrained()&&USB_RetirementDeviceDrained()&&USB_RetirementDriverDrained();
    if(ok)for(unsigned i=0;i<DRV_USBFS_INSTANCES_NUMBER;i++)
    {
        DRV_USBFS_OBJ *d=&gDrvUSBGroup[i].gDrvUSBObj;
        if(d->inUse){(void)M_DRV_USBFS_InterruptSourceDisable(d->interruptSource);PLIB_USB_Disable(d->usbID);}
    }
    if(ok)USB_RetirementPowerOffAuthorized();
    __builtin_mtc0(12,0,irq);return ok;
}
bool USB_RetirementDriverRetire(void)
{
    uint32_t irq=__builtin_disable_interrupts();
    if(!USB_RetirementRetirementAuthorized()||!USB_RetirementDriverDrained())
    {__builtin_mtc0(12,0,irq);return false;}
    /* BDT can still contain UOWN|STALL with empty queue before shutdown. It
     * may be cleared ONLY now, off idle. Preserve globally allocated BDT. */
    for(unsigned i=0;i<DRV_USBFS_INSTANCES_NUMBER;i++)
    {
        DRV_USBFS_OBJ *d=&gDrvUSBGroup[i].gDrvUSBObj;
        DRV_USBFS_BDT_ENTRY *bdt=d->pBDT;
        if(d->inUse)DRV_USBFS_Deinitialize(i);
        if(bdt!=NULL)memset((void *)bdt,0,DRV_USBFS_ENDPOINTS_NUMBER*4U*sizeof(DRV_USBFS_BDT_ENTRY));
        memset(gDrvUSBGroup[i].gDrvUSBEndpoints,0,sizeof(gDrvUSBGroup[i].gDrvUSBEndpoints));
        memset(d,0,sizeof(*d));
    }
    USB_RetirementLayerRetired(USB_RETIREMENT_LAYER_DRIVER);
    __builtin_mtc0(12,0,irq);return true;
}
bool USB_RetirementDriverReplacementReady(void)
{
    DRV_USBFS_OBJ *d=&gDrvUSBGroup[0].gDrvUSBObj;
    uint16_t rejected=0;uint8_t observation=0;
    if(!USB_RetirementPreparing())rejected=1U;
    else if(!d->inUse)rejected=2U;
    else if(!d->isOpened)rejected=4U;
    else if(d->status!=SYS_STATUS_READY)rejected=8U;
    else if(d->pEventCallBack==NULL)rejected=16U;
    else if(d->isAttached)rejected=32U;
    else if(d->inInterruptContext)rejected=64U;
    else if(d->endpointTable==NULL)rejected=128U;
    else if(d->pBDT==NULL)rejected=256U;
    else if(EVIC_SourceIsEnabled((INT_SOURCE)d->interruptSource))rejected=512U;
    else
    {
        uint32_t power=U1PWRC;
#if USB_RESTART_DIAGNOSTIC
        USB_RetirementPowerReadySample(power);
#endif
        observation=((power&_U1PWRC_USBPWR_MASK)!=0U?1U:0U)|((power&_U1PWRC_USBBUSY_MASK)!=0U?2U:0U);
        if((power&_U1PWRC_USBBUSY_MASK)!=0U)rejected=1024U;
        else
        {
            if((U1CON&_U1CON_USBEN_MASK)!=0U){rejected=2048U;observation|=4U;}
            else
            {
                for(unsigned e=0;e<DRV_USBFS_ENDPOINTS_NUMBER*2U;e++)if(d->endpointTable[e].irpQueue!=NULL){rejected=4096U;break;}
                if(rejected==0U)for(unsigned e=0;e<DRV_USBFS_ENDPOINTS_NUMBER*4U;e++)if((d->pBDT[e].byte[0]&USBFS_UOWN)!=0U){rejected=8192U;break;}
            }
        }
    }
    USB_RetirementDetail(USB_REBUILD_DETAIL_DRIVER_READY,rejected,observation);return rejected==0U;
}
#endif
`,
"initialization.c": `
#if USB_RETIREMENT_HOOKS
#include "usb_rebuild.h"
bool USB_RebuildDetached(uint32_t generation,USB_DEVICE_EVENT_HANDLER device_callback,
                         USB_DEVICE_HID_EVENT_HANDLER hid_callback,uintptr_t context,USB_DEVICE_HANDLE *handle)
{
    uint32_t irq=__builtin_disable_interrupts();
    USB_RetirementDetail(USB_REBUILD_DETAIL_INPUT,0,0);
    if(handle==NULL||device_callback==NULL||hid_callback==NULL)
    {USB_RetirementDetail(USB_REBUILD_DETAIL_INPUT,1,0);__builtin_mtc0(12,0,irq);return false;}
    USB_RetirementDetail(USB_REBUILD_DETAIL_PREPARE,0,0);
    if(!USB_RetirementPrepare(generation)){USB_RetirementDetail(USB_REBUILD_DETAIL_PREPARE,1,0);__builtin_mtc0(12,0,irq);return false;}
    bool ok=true;USB_DEVICE_HANDLE rebuilt=USB_DEVICE_HANDLE_INVALID;
    if(ok)
    {
        USB_RetirementDetail(USB_REBUILD_DETAIL_DRIVER,0,0);
        sysObj.drvUSBFSObject=DRV_USBFS_Initialize(DRV_USBFS_INDEX_0,(SYS_MODULE_INIT *)&drvUSBFSInit);
        ok=sysObj.drvUSBFSObject!=SYS_MODULE_OBJ_INVALID;
        if(!ok)USB_RetirementDetail(USB_REBUILD_DETAIL_DRIVER,1,0);
    }
    if(ok)
    {
        USB_RetirementDetail(USB_REBUILD_DETAIL_DEVICE,0,0);
        sysObj.usbDevObject0=USB_DEVICE_Initialize(USB_DEVICE_INDEX_0,(SYS_MODULE_INIT *)&usbDevInitData);
        ok=sysObj.usbDevObject0!=SYS_MODULE_OBJ_INVALID;
        if(!ok)USB_RetirementDetail(USB_REBUILD_DETAIL_DEVICE,1,0);
    }
    if(ok)
    {
        USB_RetirementDetail(USB_REBUILD_DETAIL_OPEN,0,0);
        USB_DEVICE_Tasks(sysObj.usbDevObject0); /* Opens driver only; no normal driver task/attach. */
        USB_RetirementDetail(USB_REBUILD_DETAIL_BIND_DEVICE,0,0);
        ok=USB_RetirementBindDevice(device_callback,context,&rebuilt);
        if(!ok)USB_RetirementDetail(USB_REBUILD_DETAIL_BIND_DEVICE,1,0);
    }
    if(ok)
    {
        USB_RetirementDetail(USB_REBUILD_DETAIL_BIND_HID,0,0);
        ok=USB_RetirementBindHid(hid_callback,context);
        if(!ok)USB_RetirementDetail(USB_REBUILD_DETAIL_BIND_HID,1,0);
    }
    if(ok)
    {
        USB_RetirementDetail(USB_REBUILD_DETAIL_REOPEN,0,0);
        ok=USB_RetirementReopen(generation); /* Readiness predicates record exact rejected bit. */
    }
    if(!ok)USB_RetirementFault();else {*handle=rebuilt;USB_RetirementDetail(USB_REBUILD_DETAIL_COMPLETE,0,0);}
    __builtin_mtc0(12,0,irq);return ok;
}
#endif
`,
};
// Inline generated edits are stripped before the unchanged MCC base hash is checked.
const callbackEnable = `            /* Enable the session valid interrupt */
            PLIB_USB_OTG_InterruptEnable(pUSBDrvObj->usbID, USB_OTG_INT_SESSION_VALID);`;
const callbackMasked = `#if USB_RETIREMENT_HOOKS
            /* Binding during detached rebuild stores callbacks only. Keep both
             * peripheral and CPU-source interrupts masked until admitted attach. */
            if(USB_RetirementBlocked())return;
#endif
` + callbackEnable;
const attachEnable = `        PLIB_USB_AllInterruptEnable(((DRV_USBFS_OBJ *)handle)->usbID, (USB_INTERRUPTS)usbIntResume, USB_ERR_INT_ALL, (USB_OTG_INTERRUPTS)usbOtg);`;
const attachUnmask = attachEnable + `
#if USB_RETIREMENT_HOOKS
        /* The owner reaches attach only after detached readiness and publication.
         * Callback binding deliberately deferred this source enable while fenced. */
        M_DRV_USBFS_InterruptSourceEnable(((DRV_USBFS_OBJ *)handle)->interruptSource);
#endif`;
const inlinePatches:Record<string,[string,string][]>= {
    "driver/usb/usbfs/src/drv_usbfs.c": [[callbackEnable,callbackMasked],["                PLIB_USB_Enable(usbID);","#if USB_RESTART_DIAGNOSTIC\n                USB_RetirementPowerEnableSample(false);\n#endif\n                PLIB_USB_Enable(usbID);\n#if USB_RESTART_DIAGNOSTIC\n                USB_RetirementPowerEnableSample(true);\n#endif"]],
    "driver/usb/usbfs/src/drv_usbfs_device.c": [[attachEnable,attachUnmask]],
};
function functionOpening(text:string,name:string):number {
    const regex=new RegExp(`^.*\\b${name}\\s*\\([^;]*?\\)\\s*\\{`,"gm");
    // Ignore documentation prototypes without changing source offsets.
    const code=text.replace(/\/\*[\s\S]*?\*\/|\/\/[^\n]*/g,comment=>comment.replace(/[^\n]/g," "));
    const matches=Array.from(code.matchAll(regex));
    if(matches.length!==1)throw Error(`expected one function ${name}`);
    return matches[0].index!+matches[0][0].length;
}
const guards:Record<string,[string,string][]>={
"usb/src/usb_device_hid.c":[["USB_DEVICE_HID_ReportSend","USB_DEVICE_HID_RESULT_ERROR"],["USB_DEVICE_HID_ReportReceive","USB_DEVICE_HID_RESULT_ERROR"],["USB_DEVICE_HID_EventHandlerSet","USB_DEVICE_HID_RESULT_ERROR"],["F_USB_DEVICE_HID_InitializeByDescriptorType",""]],
"usb/src/usb_device.c":[["USB_DEVICE_ControlSend","USB_DEVICE_CONTROL_TRANSFER_RESULT_FAILED"],["USB_DEVICE_ControlReceive","USB_DEVICE_CONTROL_TRANSFER_RESULT_FAILED"],["USB_DEVICE_ControlStatus","USB_DEVICE_CONTROL_TRANSFER_RESULT_FAILED"]],
"driver/usb/usbfs/src/drv_usbfs_device.c":[["DRV_USBFS_DEVICE_IRPSubmit","USB_ERROR_IRP_QUEUE_FULL"],["DRV_USBFS_DEVICE_Attach",""]],
};
const completions:[string,string,boolean][]=[
["F_USB_DEVICE_HID_ReportReceiveCallBack","irpRx",false],
["F_USB_DEVICE_HID_ReportSendCallBack","irpTx",true],
];
function completionGuard(pointer:string,transmit:boolean):string {
    return `\n#if USB_RETIREMENT_HOOKS\n    if(!USB_RetirementHidClaim(${pointer},${transmit?"true":"false"}))\n    {USB_RetirementFault();return;}\n    USB_RETIREMENT_HID_CALLBACK_SCOPE retirement_callback __attribute__((cleanup(USB_RetirementHidCallbackCleanup)))={${pointer}};\n#endif\n`;
}
function allocationGuard(tx:boolean):[string,string] {
    const base=tx?"if(gUSBDeviceHIDIRP[count].status\n                 <= USB_DEVICE_IRP_STATUS_COMPLETED_SHORT)":"if( gUSBDeviceHIDIRP[count].status <=\n                 USB_DEVICE_IRP_STATUS_COMPLETED_SHORT)";
    const patched=base.slice(0,-1)+"\n#if USB_RETIREMENT_HOOKS\n                 && USB_RetirementHidReserve(&gUSBDeviceHIDIRP[count],"+(tx?"true":"false")+")\n#endif\n         )";
    return [base,patched];
}
function submitParts(tx:boolean):[string,string,string,string] {
    const call=tx?"hidSendError = USB_DEVICE_IRPSubmit( thisHIDInstance->devLayerHandle,\n                                  thisHIDInstance->endpointTx,\n                                  irp);":"hidReceiveError = USB_DEVICE_IRPSubmit( thisHIDInstance->devLayerHandle,\n                                           thisHIDInstance->endpointRx,\n                                           irp);";
    const error=tx?"hidSendError":"hidReceiveError",queue=tx?"currentTxQueueSize":"currentRxQueueSize";
    const patched=call+"\n#if USB_RETIREMENT_HOOKS\n             bool retirement_rollback=USB_RetirementHidSubmitResult(irp,"+error+"==USB_ERROR_NONE);\n#endif";
    const rollback="status = OSAL_CRIT_Enter(OSAL_CRIT_TYPE_HIGH);\n                thisHIDInstance->"+queue+" --;\n                OSAL_CRIT_Leave(OSAL_CRIT_TYPE_HIGH, status);\n                *handle = USB_DEVICE_HID_TRANSFER_HANDLE_INVALID;";
    const guarded="#if USB_RETIREMENT_HOOKS\n                if(retirement_rollback) {\n#endif\n                "+rollback+"\n#if USB_RETIREMENT_HOOKS\n                }\n#endif";
    return [call,patched,rollback,guarded];
}
function stripSubmissions(text:string):string {
    for(const tx of [true,false]) {
        const[base,patched]=allocationGuard(tx);text=text.replace(patched,base);
        const[call,patchedCall,rollback,guarded]=submitParts(tx);text=text.replace(patchedCall,call).replace(guarded,rollback);
    }
    return text;
}
function patchSubmissions(text:string):string {
    for(const tx of [true,false]) {
        const[base,patched]=allocationGuard(tx);const[call,patchedCall,rollback,guarded]=submitParts(tx);
        for(const old of [base,call,rollback])if(text.split(old).length!==2)throw Error("HID submit shape changed");
        text=text.replace(base,patched).replace(call,patchedCall).replace(rollback,guarded);
    }
    return text;
}
try {
    const changes:{path:string,text:string}[]=[];
    for(const rel of new Set([...Object.keys(hooks),...Object.keys(guards)])) {
        const path=folder+rel;const original=readFileSync(path,"utf8");let text=original;
        const present=text.includes("USB_RETIREMENT_OVERLAY_BEGIN");
        if(present) {
            const tail='\n'+(hooks[rel]??'')+'\n/* USB_RETIREMENT_OVERLAY_END */\n';
            if(!text.endsWith(tail))throw Error(`overlay tail differs: ${rel}`);
            text=text.slice(0,-tail.length);
            text=text.replace('\n/* USB_RETIREMENT_OVERLAY_BEGIN: tools/usb-retirement-overlay.ts */\n#include "usb_retirement.h"','');
            for(const [,result] of guards[rel]??[])text=text.replace(`\n#if USB_RETIREMENT_HOOKS\n    USB_RETIREMENT_ADMISSION(${result});\n#endif\n`,'');
            for(const [base,patched] of inlinePatches[rel]??[])text=text.replace(patched,base);
            text=text.replace(/USB_RETIREMENT_NOTIFY\(([^;]+)\);/g,(_,call)=>`${call};`);
            if(rel.endsWith("usb_device_hid.c")){
                for(const[,pointer,transmit]of completions)text=text.replace(completionGuard(pointer,transmit),"");
                text=stripSubmissions(text);
            }
        }
        const hash=new Bun.CryptoHasher("sha256").update(text).digest("hex");
        if(hash!==bases[rel])throw Error(`generated base changed: ${rel}; review/update overlay hashes before applying`);
        const includes=Array.from(text.matchAll(/^#include[^\n]*$/gm));
        if(!includes.length)throw Error(`no includes ${rel}`);
        const last=includes.at(-1)!;const at=last.index!+last[0].length;
        text=text.slice(0,at)+'\n/* USB_RETIREMENT_OVERLAY_BEGIN: tools/usb-retirement-overlay.ts */\n#include "usb_retirement.h"'+text.slice(at);
        for(const [name,result] of guards[rel]??[]) {
            const at=functionOpening(text,name);
            const guard=`\n#if USB_RETIREMENT_HOOKS\n    USB_RETIREMENT_ADMISSION(${result});\n#endif\n`;
            text=text.slice(0,at)+guard+text.slice(at);
        }
        for(const [base,patched] of inlinePatches[rel]??[]) {
            if(text.split(base).length!==2)throw Error(`inline generated shape changed: ${rel}`);
            text=text.replace(base,patched);
        }
        if(rel.endsWith("usb_device_hid.c")){
            text=patchSubmissions(text);
            for(const[name,pointer,transmit]of completions){
                const at=functionOpening(text,name);text=text.slice(0,at)+completionGuard(pointer,transmit)+text.slice(at);
            }
        }
        // Count all lowest-layer callback/event dispatches. Call expressions
        // span lines but have no nested function argument parentheses today.
        if(rel.endsWith("drv_usbfs_device.c")) {
            let count=0;
            text=text.replace(/\b((?:irp|irp_t|irp_temp|irpIterator|iterator|irpToCancel)->callback\s*\([^;]+?\)|hDriver->pEventCallBack\s*\([^;]+?\));/g,(match,call)=>{
                count++;return `USB_RETIREMENT_NOTIFY(${call});`;
            });
            if(count!==13)throw Error(`callback shape/count changed (${count}); review ${rel}`);
        }
        text+='\n'+(hooks[rel]??'')+'\n/* USB_RETIREMENT_OVERLAY_END */\n';
        if(present) {
            if(text!==original)throw Error(`overlay guard/callback content differs: ${rel}`);
            console.log(`VERIFIED: ${rel}`);
        } else if(write){changes.push({path,text});}else throw Error(`overlay missing: ${rel}; run --apply after review`);
    }
    // Validate every file/callback shape before any generated file is changed.
    for(const {path,text} of changes){writeFileSync(path,text);console.log(`APPLIED: ${path}`);}
} catch(error){console.error(String(error));process.exit(1);}
