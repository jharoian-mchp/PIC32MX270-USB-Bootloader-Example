#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Regression-test restart image audit against corrupted HEX/configuration/ELF artifacts","keywords":["usb","restart","audit","negative"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import{mkdirSync,readFileSync,writeFileSync,rmSync}from"node:fs";import{resolve}from"node:path";
process.chdir(resolve(import.meta.dir,".."));const dir="build/restart-audit-negative";mkdirSync(dir,{recursive:true});
const source="build/restart-check/bootloader",elf=readFileSync(source+".elf"),hex=readFileSync(source+".hex","utf8");
function run(e:Buffer,h:string,success:boolean){writeFileSync(dir+"/case.elf",e);writeFileSync(dir+"/case.hex",h);const r=Bun.spawnSync(["bun","tools/audit-restart-image.ts",dir+"/case.elf"],{stdout:"pipe",stderr:"pipe"});if((r.exitCode===0)!==success)throw Error("unexpectedaudit result "+r.stderr.toString());}
function corruptData():string{let changed=false;return hex.split("\n").map(line=>{if(changed||!line.startsWith(":"))return line;const b=Buffer.from(line.slice(1),"hex");if(b[3]!==0||b[0]===0)return line;changed=true;b[4]^=1;b[b.length-1]=0;let sum=0;for(const v of b)sum+=v;b[b.length-1]=(-sum)&255;return ":"+b.toString("hex").toUpperCase();}).join("\n");}
try{
    run(elf,hex,true);run(elf,corruptData(),false);
    const bad=Buffer.from(elf);bad[5]=2;run(bad,hex,false);
    run(elf,hex.replace(/\n:[^\n]+(?=\n:00000001FF)/,""),false);
    const phoff=elf.readUInt32LE(28),phsize=elf.readUInt16LE(42),phcount=elf.readUInt16LE(44);
    // Repurpose unused MIPS_ABIFLAGS/non-load header as rogue PT_LOAD. No HEX
    // byte maps to it: bounds must reject it independently of HEX matching.
    let spare=-1;
    for(let i=0;i<phcount;i++)if(elf.readUInt32LE(phoff+i*phsize)!==1){spare=phoff+i*phsize;break;}
    if(spare<0)throw Error("no non-load regression header");
    const rogue=Buffer.from(elf);rogue.writeUInt32LE(1,spare);rogue.writeUInt32LE(0,spare+4);
    rogue.writeUInt32LE(0x9D008000,spare+8);rogue.writeUInt32LE(0x9D008000,spare+12);
    rogue.writeUInt32LE(1,spare+16);rogue.writeUInt32LE(1,spare+20);run(rogue,hex,false);
    rogue.writeUInt32LE(0,spare+16);run(rogue,hex,false); /* zero-file, nonzero memory still unsafe */
    rogue.writeUInt32LE(1,spare+16);
    rogue.writeUInt32LE(0x9D000000,spare+8);rogue.writeUInt32LE(0x9D000000,spare+12);
    rogue.writeUInt32LE(elf.length+1,spare+4);run(rogue,hex,false);
    rogue.writeUInt32LE(0,spare+4);rogue.writeUInt32LE(0xDD000000,spare+8);rogue.writeUInt32LE(0xDD000000,spare+12);
    run(rogue,hex,false); // KSEG2 alias previously masked to Program Flash.
    run(elf,hex.replace(/:020000041d00dd/i,":02000004dd001d"),false);
    // Change the required Ack symbol to SHN_UNDEF while keeping its name.
    const missing=Buffer.from(elf),shoff=elf.readUInt32LE(32),shsize=elf.readUInt16LE(46),shcount=elf.readUInt16LE(48);
    let changed=false;
    for(let i=0;i<shcount;i++){
        const s=shoff+i*shsize;if(elf.readUInt32LE(s+4)!==2)continue;
        const table=elf.readUInt32LE(s+16),bytes=elf.readUInt32LE(s+20),entry=elf.readUInt32LE(s+36);
        const strings=shoff+elf.readUInt32LE(s+24)*shsize,stroff=elf.readUInt32LE(strings+16);
        for(let j=0;j<bytes;j+=entry){const at=table+j,n=stroff+elf.readUInt32LE(at),end=elf.indexOf(0,n);
            if(elf.toString("utf8",n,end)==="USB_RestartOwnerAck"){
                missing.writeUInt32LE(0,at+4);missing.writeUInt16LE(0,at+14);changed=true;
            }
        }
    }
    if(!changed)throw Error("required symbol regression unavailable");run(missing,hex,false);
    // Change a real startup clear destination to retained RAM. Mirror the
    // modified .dinit byte in HEX so ELF/HEX equality is not the rejection.
    const namesHeader=shoff+elf.readUInt16LE(50)*shsize,baseNames=elf.readUInt32LE(namesHeader+16);
    let dinit=-1,virtual=0,dinitHeader=-1;
    for(let i=0;i<shcount;i++){
        const s=shoff+i*shsize,start=baseNames+elf.readUInt32LE(s),end=elf.indexOf(0,start);
        if(elf.toString("utf8",start,end)===".dinit"){dinit=elf.readUInt32LE(s+16);virtual=elf.readUInt32LE(s+12);dinitHeader=s;}
    }
    if(dinit<0)throw Error(".dinit regression section missing");
    const badNames=Buffer.from(elf);badNames.writeUInt16LE(shcount,50);run(badNames,hex,false);
    const fakeDinit=Buffer.from(elf);fakeDinit.writeUInt32LE(8,dinitHeader+4);run(fakeDinit,hex,false);
    fakeDinit.writeUInt32LE(1,dinitHeader+4);fakeDinit.writeUInt32LE(0,dinitHeader+8);run(fakeDinit,hex,false);
    const overlap=Buffer.from(elf);overlap.writeUInt32LE(0xA0000000,dinit);
    let extension=0;
    const mirrored=hex.split("\n").map(line=>{
        if(!line.startsWith(":"))return line;const b=Buffer.from(line.slice(1),"hex");
        if(b[3]===4){extension=b.readUInt16BE(4)*65536;return line;}
        if(b[3]!==0)return line;const address=extension+b.readUInt16BE(1),target=virtual&0x1FFFFFFF;let touched=false;
        for(let i=0;i<b[0];i++)if(address+i>=target&&address+i<target+4){b[4+i]=overlap[dinit+address+i-target];touched=true;}
        if(!touched)return line;b[b.length-1]=0;b[b.length-1]=(-b.reduce((a,v)=>a+v,0))&255;
        return ":"+b.toString("hex");
    }).join("\n");
    run(overlap,mirrored,false);
    console.log("PASS: audit rejects corrupted/missing bytes, rogue/out-of-file PT_LOAD and undefined required code symbol");
}
finally{rmSync(dir,{recursive:true,force:true});}
