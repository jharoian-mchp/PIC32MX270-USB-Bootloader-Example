#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Compile/test opt-in USB owning-TU retirement hook bodies with host mocks","keywords":["usb","retirement","hooks","tests"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import {mkdirSync,readFileSync,writeFileSync} from "node:fs";
import {resolve} from "node:path";
process.chdir(resolve(import.meta.dir,".."));
const dir="build/retirement-hooks";mkdirSync(dir,{recursive:true});
const paths=["usb/src/usb_device_hid.c","usb/src/usb_device.c","driver/usb/usbfs/src/drv_usbfs.c"];
try {
    const chunks=[];
    for(const rel of paths){const text=readFileSync("bootloader/src/config/default/"+rel,"utf8");const start=text.lastIndexOf("#if USB_RETIREMENT_HOOKS");if(start<0)throw Error("missinghooks "+rel);const end=text.indexOf("/* USB_RETIREMENT_OVERLAY_END */",start);chunks.push(text.slice(start,end));}
    writeFileSync(dir+"/hooks.inc",chunks.join("\n"));
    const cc=(process.env.HOST_CC??"cc").trim().split(/\s+/);
    const args=[...cc,"-std=c11","-O2","-Wall","-Wextra","-Werror","-DUSB_RETIREMENT_HOOKS=1","-Itests/retirement_mocks","-Ibootloader/src","-I"+dir,"bootloader/src/usb_retirement.c","bootloader/src/usb_irp_ledger.c","tests/usb_retirement_test.c","-o",dir+"/test"];
    const compile=Bun.spawnSync(args,{stdout:"inherit",stderr:"inherit"});if(compile.exitCode!==0)throw Error("compilefailed");
    const test=Bun.spawnSync([dir+"/test"],{stdout:"inherit",stderr:"inherit"});if(test.exitCode!==0)throw Error("testsfailed");
}catch(error){console.error(String(error));process.exit(1);}
