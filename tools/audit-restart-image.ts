#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Audit linked read-only USB restart ELF/HEX spans, symbols, configuration and artifact hashes","keywords":["usb","restart","elf","hex","audit"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import {readFileSync,writeFileSync} from "node:fs";
import {resolve} from "node:path";
process.chdir(resolve(import.meta.dir,".."));
function check(ok:boolean,message:string):asserts ok{if(!ok)throw Error(message);}
function physical(v:number):number {
    check(v>=0&&v<=0xFFFFFFFF,"address outside 32-bit range");
    if(v<0x80000000)return v;
    check(v<0xC0000000,"unsupported MIPS address segment");
    return (v&0x1FFFFFFF)>>>0; // Only direct-mapped KSEG0/KSEG1.
}
function allowed(a:number,n:number):boolean {
    return (a>=0x1D000000&&a+n<=0x1D008000)||(a>=0x1FC00000&&a+n<=0x1FC004AC)||
           (a>=0x1FC00BF0&&a+n<=0x1FC00C00);
}
function hexMemory(path:string):Map<number,number>{
    const map=new Map<number,number>();let extension=0,eof=false;
    for(const raw of readFileSync(path,"utf8").split(/\r?\n/)){
        if(!raw)continue;check(!eof,"HEX after EOF");check(/^:[0-9A-Fa-f]+$/.test(raw),"HEX syntax");
        const bytes=Buffer.from(raw.slice(1),"hex");check(bytes.length===bytes[0]+5,"HEX count");
        check(bytes.reduce((a,b)=>a+b,0)%256===0,"HEX checksum");const offset=bytes.readUInt16BE(1),kind=bytes[3],n=bytes[0];
        if(kind===4){check(n===2&&offset===0,"HEX extension");extension=bytes.readUInt16BE(4)*65536;}
        else if(kind===0){const a=physical(extension+offset);check(n>0&&offset+n<=65536&&allowed(a,n),`HEX unsafe span ${a.toString(16)}+${n}`);
            for(let i=0;i<n;i++){check(!map.has(a+i),"HEX overlap");map.set(a+i,bytes[4+i]);}}
        else if(kind===1){check(n===0&&offset===0,"HEX EOF");eof=true;}
        else throw Error(`unsupported HEX record ${kind}`);
    }
    check(eof&&map.size>0,"HEX lacks EOF/data");return map;
}
try{
    const elfPath=process.argv[2]??"build/restart-check/bootloader.elf";
    const hexPath=elfPath.replace(/\.elf$/,".hex");const elf=readFileSync(elfPath);
    check(elf.length>=52&&elf.subarray(0,7).equals(Buffer.from([127,69,76,70,1,1,1])),"expected ELF32 little-endian");
    check(elf.readUInt16LE(18)===8,"expected MIPS machine");
    const memory=hexMemory(hexPath),baseline=hexMemory("build/bootloader/bootloader.hex");
    // XC32's loadable ._debug_exception is an orphaned 44-byte return stub
    // at 0x480..0x4AC. Permit its tail only if unchanged from default firmware.
    for(let a=0x1FC00490;a<0x1FC004AC;a++)
        check(memory.get(a)!==undefined&&memory.get(a)===baseline.get(a),"debug-return tail differs from baseline");
    const count=elf.readUInt16LE(44),size=elf.readUInt16LE(42),offset=elf.readUInt32LE(28);
    check(size>=32&&offset+count*size<=elf.length,"ELF program headers out of range");
    let loadBytes=0;const segments=[];const matched=new Set<number>();
    for(let i=0;i<count;i++){
        const at=offset+i*size;if(elf.readUInt32LE(at)!==1)continue;
        const file=elf.readUInt32LE(at+4),vaddr=elf.readUInt32LE(at+8),paddr=physical(elf.readUInt32LE(at+12)),n=elf.readUInt32LE(at+16);
        const memoryBytes=elf.readUInt32LE(at+20);
        check(n<=memoryBytes&&file+n<=elf.length,"ELF load file range/size invalid");
        if(vaddr>=0xA0000000&&vaddr<0xA0010000){
            check(vaddr+memoryBytes<=0xA0010000,"RAM exceeds target");
            continue; // RAM data is initialized via flash .dinit, not burned at PA0.
        }
        check(physical(vaddr)===paddr,"ELF virtual/physical Flash mapping mismatch");
        check(allowed(paddr,n)&&allowed(paddr,memoryBytes),`ELF unsafe load span ${paddr.toString(16)}+${n}`);
        if(!n)continue;
        // XC32 HEX omits ELF segment alignment holes. Every delivered byte
        // must match its load segment; missing loadable section data is checked
        // below using section-to-segment file offsets, not padding holes.
        for(let j=0;j<n;j++)if(memory.has(paddr+j)){
            check(memory.get(paddr+j)===elf[file+j],`ELF/HEX mismatch ${paddr+j}`);matched.add(paddr+j);
        }
        loadBytes+=n;segments.push({physical:`0x${paddr.toString(16).toUpperCase()}`,bytes:n});
    }
    check(loadBytes>0&&matched.size===memory.size,"HEX data not represented by ELF load segments");
    const shoff=elf.readUInt32LE(32),shsize=elf.readUInt16LE(46),shcount=elf.readUInt16LE(48);
    check(shsize>=40&&shoff+shsize*shcount<=elf.length,"ELF section headers out of range");
    for(let i=0;i<shcount;i++){
        const s=shoff+i*shsize,type=elf.readUInt32LE(s+4),flags=elf.readUInt32LE(s+8),file=elf.readUInt32LE(s+16),n=elf.readUInt32LE(s+20);
        if(type===8||!(flags&2)||!n)continue; // NOBITS/nonallocated/debug are not delivered.
        check(file+n<=elf.length,"ELF section file range invalid");
        const vaddr=elf.readUInt32LE(s+12);
        if(vaddr>=0xA0000000&&vaddr<0xA0010000){check(vaddr+n<=0xA0010000,"RAM section exceeds target");continue;} // RAM initialization lives in .dinit.
        let covered=false;
        for(let k=0;k<count;k++){
            const p=offset+k*size;if(elf.readUInt32LE(p)!==1)continue;
            const start=elf.readUInt32LE(p+4),pn=elf.readUInt32LE(p+16);
            if(file<start||file+n>start+pn)continue;
            const a=physical(elf.readUInt32LE(p+12))+file-start;
            for(let j=0;j<n;j++)check(memory.get(a+j)===elf[file+j],`missing/mismatched ELF section byte ${a+j}`);
            covered=true;break;
        }
        check(covered,"loadable section not covered by program header");
    }
    const config=[];
    for(let address=0x1FC00BF0;address<0x1FC00C00;address+=4){
        const bytes=[];
        for(let i=0;i<4;i++){const v=memory.get(address+i);check(v!==undefined&&v===baseline.get(address+i),"configuration differs from default baseline");bytes.push(v);}
        config.push({address:`0x${address.toString(16).toUpperCase()}`,value:`0x${Buffer.from(bytes).readUInt32LE().toString(16).toUpperCase().padStart(8,"0")}`});
    }
    const cfg0=Buffer.from([0,1,2,3].map(i=>memory.get(0x1FC00BFC+i)!)).readUInt32LE();
    check((cfg0&0x0107FC00)===0x0107FC00,"PWP/BWP changed");
    const xc32=process.env.XC32_ROOT??"/workspace/ChrisK/tools/xc32/v6.00";
    const nm=Bun.spawnSync([`${xc32}/bin/xc32-nm`,elfPath],{stdout:"pipe",stderr:"pipe"});check(nm.exitCode===0,"nm failed");
    const symbols=nm.stdout.toString();
    check(/^bfc00480\s+\w\s+__DbgExecReturn$/m.test(symbols),"expected XC32 debug-return stub location missing");
    for(const name of ["USB_RestartOwnerRequest","USB_RestartOwnerAck"])
        check(new RegExp(`^[0-9a-fA-F]+\\s+[Tt]\\s+${name}$`,"m").test(symbols),`required defined code symbol ${name} missing`);
    check(!/\b(?:UPDATE_(?:Session|Dispatch|LinkDispatch|Lifecycle|Flash|Nvm)\w*|NVM_(?:RowWrite|PageErase|WordWrite)|run_Application)\b/.test(symbols),"linked update/NVM write/handoff entrypoint");
    const hash=(bytes:Uint8Array)=>new Bun.CryptoHasher("sha256").update(bytes).digest("hex");
    const result={elf:elfPath,hex:hexPath,elf_sha256:hash(elf),hex_sha256:hash(readFileSync(hexPath)),hex_bytes:memory.size,segments,config,
        review_scope:"ELF load spans/HEX equality/config equality/symbol exclusion; LTO can inline code, so symbols alone do not prove arbitrary instruction safety.",
        hardware_qualified:false,board_delivery_approved:false};
    writeFileSync(elfPath.replace(/\.elf$/,"-audit.json"),JSON.stringify(result,null,2)+"\n");
    console.log(JSON.stringify(result,null,2));
}catch(error){console.error(String(error));process.exit(1);}
