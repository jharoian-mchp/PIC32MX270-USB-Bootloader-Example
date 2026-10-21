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
    function extract(path:string,name:string):string {
        const text=readFileSync("bootloader/src/config/default/"+path,"utf8");
        const code=text.replace(/\/\*[\s\S]*?\*\/|\/\/[^\n]*/g,comment=>comment.replace(/[^\n]/g," "));
        const start=code.indexOf("void "+name);let at=code.indexOf("{",start)+1,depth=1;
        for(;at<code.length&&depth;at++){if(code[at]==="{")depth++;else if(code[at]==="}")depth--;}
        if(start<0||depth)throw Error("missing exact function "+name);return text.slice(start,at);
    }
    writeFileSync(dir+"/binding.inc",extract("driver/usb/usbfs/src/drv_usbfs.c","DRV_USBFS_ClientEventCallBackSet")+"\n"+
        extract("driver/usb/usbfs/src/drv_usbfs_device.c","DRV_USBFS_DEVICE_Attach"));
    const drv=readFileSync("bootloader/src/config/default/driver/usb/usbfs/src/drv_usbfs.c","utf8");
    const begin=drv.indexOf("#if USB_RESTART_DIAGNOSTIC\n                USB_RetirementPowerEnableSample(false);");
    const finish=drv.indexOf("#endif",drv.indexOf("USB_RetirementPowerEnableSample(true);",begin))+6;
    if(begin<0||finish<begin)throw Error("power sites missing");
    writeFileSync(dir+"/power-enable.inc",drv.slice(begin,finish));
    // Test the production NVM exclusion predicate, not a hand-written duplicate.
    const nvm=readFileSync("bootloader/src/update_nvm_port.h","utf8");
    const nvmStart=nvm.indexOf("static inline bool UPDATE_NvmPortUsbActive(void)");
    const nvmEnd=nvm.indexOf("\n}",nvmStart)+2;
    if(nvmStart<0||nvmEnd<nvmStart)throw Error("NVM USB gate missing");
    writeFileSync(dir+"/nvm-usb-gate.inc",nvm.slice(nvmStart,nvmEnd));
    const cc=(process.env.HOST_CC??"cc").trim().split(/\s+/);
    const args=[...cc,"-std=c11","-O2","-Wall","-Wextra","-Werror","-DUSB_RETIREMENT_HOOKS=1","-DUSB_RESTART_DIAGNOSTIC=1","-Itests/retirement_mocks","-Ibootloader/src","-I"+dir,"bootloader/src/usb_retirement.c","bootloader/src/usb_irp_ledger.c","tests/usb_retirement_test.c","-o",dir+"/test"];
    const compile=Bun.spawnSync(args,{stdout:"inherit",stderr:"inherit"});if(compile.exitCode!==0)throw Error("compilefailed");
    const test=Bun.spawnSync([dir+"/test"],{stdout:"inherit",stderr:"inherit"});if(test.exitCode!==0)throw Error("testsfailed");
    for(const enabled of [0,1]) {
        const binary=dir+"/binding-"+enabled;
        const build=Bun.spawnSync([...cc,"-std=c11","-O2","-Wall","-Wextra","-Werror","-DUSB_RETIREMENT_HOOKS="+enabled,
            "-Ibootloader/src","-I"+dir,"tests/usb_callback_binding_test.c","-o",binary],{stdout:"inherit",stderr:"inherit"});
        if(build.exitCode!==0)throw Error("binding compilefailed");
        const run=Bun.spawnSync([binary],{stdout:"inherit",stderr:"inherit"});if(run.exitCode!==0)throw Error("binding testsfailed");
    }
}catch(error){console.error(String(error));process.exit(1);}
