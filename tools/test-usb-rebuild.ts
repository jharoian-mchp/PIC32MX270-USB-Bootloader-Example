#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Test exact opt-in USB-only rebuild entrypoint ordering, preflight and partial failure with host mocks","keywords":["usb","rebuild","initializer","tests"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import{mkdirSync,readFileSync,writeFileSync}from"node:fs";import{resolve}from"node:path";
process.chdir(resolve(import.meta.dir,".."));const dir="build/usb-rebuild";mkdirSync(dir,{recursive:true});
try{
    const text=readFileSync("bootloader/src/config/default/initialization.c","utf8");const start=text.indexOf("bool USB_RebuildDetached");let at=text.indexOf("{",start)+1,depth=1;
    for(;at<text.length&&depth;at++){if(text[at]==="{")depth++;else if(text[at]==="}")depth--;}
    if(start<0||depth)throw Error("rebuildmissing");writeFileSync(dir+"/rebuild.inc",text.slice(start,at));
    const cc=(process.env.HOST_CC??"cc").trim().split(/\s+/);
    const compile=Bun.spawnSync([...cc,"-std=c11","-O2","-Wall","-Wextra","-Werror","-I"+dir,"tests/usb_rebuild_test.c","-o",dir+"/test"],{stdout:"inherit",stderr:"inherit"});if(compile.exitCode)throw Error("compile");
    const run=Bun.spawnSync([dir+"/test"],{stdout:"inherit",stderr:"inherit"});if(run.exitCode)throw Error("test");
}catch(error){console.error(String(error));process.exit(1);}
