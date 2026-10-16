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
};
const hooks: Record<string,string>={
"usb/src/usb_device_hid.c": `
#if USB_RETIREMENT_HOOKS
bool USB_RetirementHidDrained(void)
{
    if(!USB_RetirementBlocked()||!USB_RetirementCallbacksIdle())return false;
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
    memset(gUSBDeviceHIDIRP,0,sizeof(gUSBDeviceHIDIRP));
    memset(gUsbDeviceHidInstance,0,sizeof(gUsbDeviceHidInstance));
    __builtin_mtc0(12,0,irq);return true;
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
    __builtin_mtc0(12,0,irq);return true;
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
    __builtin_mtc0(12,0,irq);return true;
}
#endif
`,
};
function functionOpening(text:string,name:string):number {
    const regex=new RegExp(`^.*\\b${name}\\s*\\([^;]*?\\)\\s*\\{`,"gm");
    const matches=Array.from(text.matchAll(regex));
    if(matches.length!==1)throw Error(`expected one function ${name}`);
    return matches[0].index!+matches[0][0].length;
}
const guards:Record<string,[string,string][]>={
"usb/src/usb_device_hid.c":[["USB_DEVICE_HID_ReportSend","USB_DEVICE_HID_RESULT_ERROR"],["USB_DEVICE_HID_ReportReceive","USB_DEVICE_HID_RESULT_ERROR"],["USB_DEVICE_HID_EventHandlerSet","USB_DEVICE_HID_RESULT_ERROR"],["F_USB_DEVICE_HID_InitializeByDescriptorType",""]],
"usb/src/usb_device.c":[["USB_DEVICE_ControlSend","USB_DEVICE_CONTROL_TRANSFER_RESULT_FAILED"],["USB_DEVICE_ControlReceive","USB_DEVICE_CONTROL_TRANSFER_RESULT_FAILED"],["USB_DEVICE_ControlStatus","USB_DEVICE_CONTROL_TRANSFER_RESULT_FAILED"]],
"driver/usb/usbfs/src/drv_usbfs_device.c":[["DRV_USBFS_DEVICE_IRPSubmit","USB_ERROR_IRP_QUEUE_FULL"]],
};
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
            text=text.replace(/USB_RETIREMENT_NOTIFY\(([^;]+)\);/g,(_,call)=>`${call};`);
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
