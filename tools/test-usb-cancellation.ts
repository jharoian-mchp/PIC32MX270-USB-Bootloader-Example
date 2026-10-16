#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Test exact generated USB queue flush and HID callback bodies against fenced retirement hooks","keywords":["usb","cancellation","callback","retirement"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import {mkdirSync,readFileSync,writeFileSync}from"node:fs";
import{resolve}from"node:path";
process.chdir(resolve(import.meta.dir,".."));
const dir="build/usb-cancellation";mkdirSync(dir,{recursive:true});
const selections=[
["bootloader/src/config/default/driver/usb/usbfs/src/drv_usbfs_device.c","F_DRV_USBFS_DEVICE_IRPQueueFlush"],
["bootloader/src/config/default/usb/src/usb_device_hid.c","F_USB_DEVICE_HID_ReportReceiveCallBack"],
["bootloader/src/config/default/usb/src/usb_device_hid.c","F_USB_DEVICE_HID_ReportSendCallBack"],
];
try{
    const chunks=[];const manifest=[];
    for(const[path,name]of selections){
        const text=readFileSync(path,"utf8");const matches=Array.from(text.matchAll(new RegExp(`^void\\s+${name}\\s*\\([^;]*?\\)\\s*\\{`,"gm")));
        if(matches.length!==1)throw Error("expected exact function "+name);
        const start=matches[0].index!;let cursor=start+matches[0][0].length,depth=1;
        for(;cursor<text.length&&depth;cursor++){if(text[cursor]==="{")depth++;else if(text[cursor]==="}")depth--;}
        if(depth)throw Error("unclosed "+name);const body=text.slice(start,cursor);
        if(/"[^"\n]*[{}][^"\n]*"/.test(body))throw Error("brace-bearing string "+name);
        chunks.push(body);manifest.push({path,name,sha256:new Bun.CryptoHasher("sha256").update(body).digest("hex")});
    }
    const hid=readFileSync("bootloader/src/config/default/usb/src/usb_device_hid.c","utf8");
    const start=hid.lastIndexOf("#if USB_RETIREMENT_HOOKS\n#include \"usb_irp_ledger.h\"");
    const end=hid.indexOf("bool USB_RetirementHidDrained",start);
    if(start<0||end<0)throw Error("completion ledger hooks missing");
    chunks.unshift(hid.slice(start,end)+"\n#endif\n");
    writeFileSync(dir+"/callbacks.inc",chunks.join("\n"));writeFileSync(dir+"/manifest.json",JSON.stringify(manifest,null,2)+"\n");
    const cc=(process.env.HOST_CC??"cc").trim().split(/\s+/);
    const compile=Bun.spawnSync([...cc,"-std=c11","-O2","-Wall","-Wextra","-Werror","-DUSB_RETIREMENT_HOOKS=1","-Ibootloader/src","-Itests/retirement_mocks","-I"+dir,"bootloader/src/usb_retirement.c","bootloader/src/usb_irp_ledger.c","tests/usb_cancellation_test.c","-o",dir+"/test"],{stdout:"inherit",stderr:"inherit"});
    if(compile.exitCode!==0)throw Error("compilefailed");
    const test=Bun.spawnSync([dir+"/test"],{stdout:"inherit",stderr:"inherit"});if(test.exitCode!==0)throw Error("testfailed");
}catch(error){console.error(String(error));process.exit(1);}
