#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Regression-test restart image audit against corrupted HEX/configuration/ELF artifacts","keywords":["usb","restart","audit","negative"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import{mkdirSync,readFileSync,writeFileSync,rmSync}from"node:fs";import{resolve}from"node:path";
process.chdir(resolve(import.meta.dir,".."));const dir="build/restart-audit-negative";mkdirSync(dir,{recursive:true});
const source="build/restart-check/bootloader",elf=readFileSync(source+".elf"),hex=readFileSync(source+".hex","utf8");
function run(e:Buffer,h:string,success:boolean){writeFileSync(dir+"/case.elf",e);writeFileSync(dir+"/case.hex",h);const r=Bun.spawnSync(["bun","tools/audit-restart-image.ts",dir+"/case.elf"],{stdout:"pipe",stderr:"pipe"});if((r.exitCode===0)!==success)throw Error("unexpectedaudit result "+r.stderr.toString());}
function corruptData():string{let changed=false;return hex.split("\n").map(line=>{if(changed||!line.startsWith(":"))return line;const b=Buffer.from(line.slice(1),"hex");if(b[3]!==0||b[0]===0)return line;changed=true;b[4]^=1;b[b.length-1]=0;let sum=0;for(const v of b)sum+=v;b[b.length-1]=(-sum)&255;return ":"+b.toString("hex").toUpperCase();}).join("\n");}
try{run(elf,hex,true);run(elf,corruptData(),false);const bad=Buffer.from(elf);bad[5]=2;run(bad,hex,false);run(elf,hex.replace(/\n:[^\n]+(?=\n:00000001FF)/,""),false);console.log("PASS: restart audit rejects data mismatch, ELF format corruption and missing delivered section record");}
finally{rmSync(dir,{recursive:true,force:true});}
