#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Test exact overlaid HID send/receive submission and callback bookkeeping with mocked lower driver","keywords":["usb","hid","submission","ledger"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import{mkdirSync,readFileSync,writeFileSync}from"node:fs";import{resolve}from"node:path";
process.chdir(resolve(import.meta.dir,".."));const dir="build/hid-submission";mkdirSync(dir,{recursive:true});
try{
    const text=readFileSync("bootloader/src/config/default/usb/src/usb_device_hid.c","utf8");const parts=[];
    for(const name of ["USB_DEVICE_HID_ReportSend","USB_DEVICE_HID_ReportReceive","F_USB_DEVICE_HID_ReportSendCallBack","F_USB_DEVICE_HID_ReportReceiveCallBack"]){
        const match=Array.from(text.matchAll(new RegExp(`^(?:USB_DEVICE_HID_RESULT|void)\\s+${name}\\s*\\([^;]*?\\)\\s*\\{`,"gm")));
        if(match.length!==1)throw Error(name);const start=match[0].index!;let at=start+match[0][0].length,depth=1;
        for(;at<text.length&&depth;at++){if(text[at]==="{")depth++;else if(text[at]==="}")depth--;}
        if(depth)throw Error("unclosed "+name);parts.push(text.slice(start,at));
    }
    const start=text.lastIndexOf('#if USB_RETIREMENT_HOOKS\n#include "usb_irp_ledger.h"');const end=text.indexOf('bool USB_RetirementHidDrained',start);
    if(start<0||end<0)throw Error("ledgerhook");parts.unshift(text.slice(start,end)+"\n#endif\n");
    writeFileSync(dir+"/submission.inc",parts.join("\n"));
    const cc=(process.env.HOST_CC??"cc").trim().split(/\s+/);const result=Bun.spawnSync([...cc,"-std=c11","-O2","-Wall","-Wextra","-Werror","-DUSB_RETIREMENT_HOOKS=1","-Ibootloader/src","-Itests/retirement_mocks","-I"+dir,"bootloader/src/usb_retirement.c","bootloader/src/usb_irp_ledger.c","tests/usb_hid_submission_test.c","-o",dir+"/test"],{stdout:"inherit",stderr:"inherit"});if(result.exitCode)throw Error("compilefailed");
    const test=Bun.spawnSync([dir+"/test"],{stdout:"inherit",stderr:"inherit"});if(test.exitCode)throw Error("testfailed");
}catch(error){console.error(String(error));process.exit(1);}
