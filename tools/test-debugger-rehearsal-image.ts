#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Run synthetic positive and negative tests for the structural-only PIC32MX270 rehearsal image auditor","keywords":["PIC32MX270","rehearsal","auditor","tests"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import { createHash } from "node:crypto";
import { auditRehearsalImage, runAuditCli, type AuditInput } from "./rehearsal-image";
import {mkdtempSync,writeFileSync,rmSync,symlinkSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';

const POLICY_SCHEMA="pic32mx270-debugger-rehearsal-policy-v1";
const MAP_SCHEMA="pic32mx270-debugger-rehearsal-normalized-map-v1";
const MANIFEST_SCHEMA="pic32mx270-debugger-rehearsal-source-manifest-v1";
const h=(b:Uint8Array)=>createHash("sha256").update(b).digest("hex");
const U=(n:number)=>n>>>0;
const put16=(b:Buffer,o:number,v:number)=>b.writeUInt16LE(v&0xffff,o);
const put32=(b:Buffer,o:number,v:number)=>b.writeUInt32LE(U(v),o);
const str=(s:string)=>Buffer.from(s+"\0");
const config=Buffer.alloc(16); [0xffffffff,0xfff97afa,0xfd74ce5b,0x7fffffe0].forEach((v,i)=>config.writeUInt32LE(v>>>0,i*4));

function fixture(opts:Record<string,any>={}):{input:AuditInput; loc:Record<string,number>; policy:any; map:any; manifest:any} {
  const textAddr=0x9d000000, dinitAddr=0x9d000100, contextAddr=0xa0001000, mailboxAddr=0xa0001020, configAddr=0xbfc00bf0;
  const dinit=Buffer.alloc(32);
  // clear context (32), clear mailbox (148), terminate; remaining tail zero.
  dinit.writeUInt32LE(opts.forbiddenDinit?0xa000f000:contextAddr,0); dinit.writeUInt32LE(32,4); dinit.writeUInt32LE(opts.badOp?7:0,8);
  dinit.writeUInt32LE(opts.pairDinit?contextAddr:mailboxAddr,12); dinit.writeUInt32LE(opts.badDinitLength?0:148,16); dinit.writeUInt32LE(0,20);
  // Zero termination at 24.
  const sections=[
    {name:"",type:0,flags:0,addr:0,off:0,size:0,link:0,info:0,align:0,entsize:0},
    {name:".text",type:1,flags:6,addr:textAddr,off:0x100,size:0x100,link:0,info:0,align:4,entsize:0},
    {name:".dinit",type:1,flags:2,addr:dinitAddr,off:0x200,size:dinit.length,link:0,info:0,align:4,entsize:0},
    {name:".context",type:8,flags:3,addr:contextAddr,off:0,size:32,link:0,info:0,align:4,entsize:0},
    {name:".mailbox",type:8,flags:3,addr:mailboxAddr,off:0,size:opts.mailboxSectionSize??148,link:0,info:0,align:4,entsize:0},
    {name:".config",type:1,flags:2,addr:configAddr,off:0x240,size:16,link:0,info:0,align:4,entsize:0},
    {name:".shstrtab",type:3,flags:0,addr:0,off:0x260,size:0,link:0,info:0,align:1,entsize:0},
    {name:".symtab",type:2,flags:0,addr:0,off:0x2a0,size:0,link:8,info:1,align:4,entsize:16},
    {name:".strtab",type:3,flags:0,addr:0,off:0x360,size:0,link:0,info:0,align:1,entsize:0},
    {name:".reset",type:1,flags:6,addr:0xbfc00000,off:0x620,size:8,link:0,info:0,align:4,entsize:0},
  ];
  const shstrings=Buffer.from("\0.text\0.dinit\0.context\0.mailbox\0.config\0.shstrtab\0.symtab\0.strtab\0.reset\0");
  const secNames=["",".text",".dinit",".context",".mailbox",".config",".shstrtab",".symtab",".strtab"];
  const no=(name:string)=>name?shstrings.indexOf(Buffer.from(name+"\0")):0;
  sections.forEach((s,i)=>{(s as any).nameoff=no(s.name);});
  sections[6].size=shstrings.length;
  const symstr=Buffer.from("\0main\0_reset\0RAM_RehearsalPreEdit\0RAM_RehearsalReadout\0_dinit_addr\0rehearsal_context\0rehearsal_mailbox\0");
  const symNames=["","main","_reset","RAM_RehearsalPreEdit","RAM_RehearsalReadout","_dinit_addr","rehearsal_context","rehearsal_mailbox"];
  const symOff=(n:string)=>n?symstr.indexOf(Buffer.from(n+"\0")):0;
  sections[8].size=symstr.length; sections[7].size=symNames.length*16;
  const data=Buffer.alloc(0x500);
  for(let i=0;i<0x100;i++)data[0x100+i]=i&255;
  dinit.copy(data,0x200); config.copy(data,0x240); shstrings.copy(data,0x260); symstr.copy(data,0x360);
  const syms=[
    {name:"",value:0,size:0,info:0,shndx:0},
    {name:"main",value:textAddr+0x10,size:8,info:0x12,shndx:1},
    {name:"_reset",value:0xbfc00000,size:8,info:0x12,shndx:9},
    {name:"RAM_RehearsalPreEdit",value:textAddr+0x30,size:4,info:0x12,shndx:1},
    {name:"RAM_RehearsalReadout",value:textAddr+0x40,size:4,info:0x12,shndx:1},
    {name:"_dinit_addr",value:dinitAddr,size:0,info:0x10,shndx:0xfff1},
    {name:"rehearsal_context",value:contextAddr,size:opts.contextSymbolSize??32,info:0x11,shndx:3},
    {name:"rehearsal_mailbox",value:mailboxAddr,size:opts.mailboxSymbolSize??148,info:0x11,shndx:4},
  ];
  for(let i=0;i<syms.length;i++){
    const x=0x2a0+i*16,s=syms[i]; put32(data,x,s.name? symOff(s.name):0); put32(data,x+4,s.value); put32(data,x+8,s.size); data[x+12]=s.info; put16(data,x+14,s.shndx);
  }
  const b=Buffer.alloc(0x900); data.copy(b,0x100);
  // ELF32 little-endian ET_EXEC MIPS
  b.set([0x7f,0x45,0x4c,0x46,1,1,1,0],0); put16(b,16,2); put16(b,18,8); put32(b,20,1); put32(b,24,0xbfc00000); put32(b,28,52); put32(b,32,0x700); put32(b,36,0x70001001); put16(b,40,52); put16(b,42,32); put16(b,44,4); put16(b,46,40); put16(b,48,sections.length); put16(b,50,6);
  // Text+dinit file-backed; SRAM NOBITS memory-only; config separate file-backed.
  const phs=[
    {type:1,off:0x200,v:textAddr,p:textAddr,fs:0x120,ms:0x120,flags:5,align:4},
    {type:1,off:0,v:contextAddr,p:contextAddr,fs:0,ms:32+148,flags:6,align:4},
    {type:1,off:0x340,v:configAddr,p:configAddr,fs:16,ms:16,flags:4,align:4},
    {type:1,off:0x620,v:0xbfc00000,p:0xbfc00000,fs:8,ms:8,flags:5,align:4},
  ];
  for(let i=0;i<phs.length;i++){
    const p=phs[i],x=52+i*32;put32(b,x,p.type);put32(b,x+4,p.off);put32(b,x+8,p.v);put32(b,x+12,p.p);put32(b,x+16,p.fs);put32(b,x+20,p.ms);put32(b,x+24,p.flags);put32(b,x+28,p.align);
  }
  // actual section file offsets differ from the copied bundle: .text starts at 0x200, .dinit at 0x300
  // Copy payload explicitly, matching declared sections and PT_LOAD offsets.
  Buffer.alloc(0x100,0x5a).copy(b,0x200);
  dinit.copy(b,0x300); config.copy(b,0x340); shstrings.copy(b,0x360); symstr.copy(b,0x460);
  // Correct file offsets for all allocated/metadata sections and their PH mapping.
  sections[1].off=0x200; sections[2].off=0x300; sections[5].off=0x340; sections[6].off=0x360; sections[7].off=0x4a0; sections[8].off=0x560;
  // Rewrite symtab at proper positions and strings.
  symstr.copy(b,0x560);Buffer.from([8,0,0,0,0,0,0,0]).copy(b,0x620);
  for(let i=0;i<syms.length;i++){const x=0x4a0+i*16,s=syms[i];put32(b,x,s.name?symOff(s.name):0);put32(b,x+4,s.value);put32(b,x+8,s.size);b[x+12]=s.info;put16(b,x+14,s.shndx);}
  // Ensure text+dinit are a single segment with contiguous file/vaddr offsets.
  // .dinit address is +0x100 and file offset +0x100 from .text.
  phs[0].off=0x200; phs[0].fs=0x120;
  // Section headers.
  for(let i=0;i<sections.length;i++){
    const s=sections[i],x=0x700+i*40;put32(b,x,s.nameoff);put32(b,x+4,s.type);put32(b,x+8,s.flags);put32(b,x+12,s.addr);put32(b,x+16,s.off);put32(b,x+20,s.size);put32(b,x+24,s.link);put32(b,x+28,s.info);put32(b,x+32,s.align);put32(b,x+36,s.entsize);
  }
  // symbols and .dinit offsets are locators used for mutant cases.
  const bootSpan=[{start:0x1fc00000,end:0x1fc00480}];
  const reservations=[
    {name:"debug",start:0xa000c000,end:0xa000d000,kind:"debug"},
    {name:"heap",start:0xa000e000,end:0xa000f000,kind:"heap"},
    {name:"stack",start:0xa000f000,end:0xa0010000,kind:"stack"},
  ];
  const src=Buffer.from("int rehearsal_fixture_source;\n");
  const sources=[{path:"fixture.c",sha256:h(src)}];
  const map:any={schema:MAP_SCHEMA,elf_sha256:h(b),sources,allocations:sections.filter(s=>s.flags&2).map(s=>({name:s.name,start:s.addr,end:s.addr+s.size,kind:s.type===8?"nobits":"progbits"})),reservations};
  const manifest:any={schema:MANIFEST_SCHEMA,elf_sha256:h(b),hex_sha256:"",map_sha256:"",sources};
  const policy:any={schema:POLICY_SCHEMA,source_manifest_sha256:"",provenance:"synthetic fixture only",review_identity:"synthetic-test",allowed_boot_flash_spans:bootSpan,reservations,context_symbol:"rehearsal_context",context_size:32,mailbox_symbol:"rehearsal_mailbox"};
  const hex=makeHex(b,sections,opts);
  manifest.hex_sha256=h(hex); manifest.map_sha256=h(Buffer.from(JSON.stringify(map)));
  // JSON byte order is exact and policy binds manifest including final map/hash fields.
  const manifestBytes=Buffer.from(JSON.stringify(manifest)); policy.source_manifest_sha256=h(manifestBytes);
  const policyBytes=Buffer.from(JSON.stringify(policy));
  const loc={dinitFile:0x300,ph0:52,ph1:84,ph2:116,shoff:0x700,shents:40,strtab:0x560,symtab:0x4a0,contextAddr,mailboxAddr,configAddr};
  return {input:{elf:b,hex,map:Buffer.from(JSON.stringify(map)),manifest:manifestBytes,policy:policyBytes,expectedPolicySha256:h(policyBytes),sources:{"fixture.c":src}},loc,policy,map,manifest};
}
function makeHex(elf:Buffer, sections:any[], opts:Record<string,any>):Buffer {
  const entries:{addr:number;value:number}[]=[];
  // actual Flash bytes allocated by fixture (text, dinit, config); optional omitted config/alter byte.
  for(const [addr,off,len] of [[0x1d000000,0x200,0x100],[0x1d000100,0x300,0x20],[0x1fc00bf0,0x340,16],[0x1fc00000,0x620,8]] as number[][]){
    if(opts.omitConfig&&addr===0x1fc00bf0)continue;
    for(let i=0;i<len;i++)entries.push({addr:addr+i,value:opts.mismatchHex&&addr===0x1d000000&&i===0?elf[off+i]^1:elf[off+i]});
  }
  const lines:string[]=[]; let upper=-1;
  const record=(address:number,type:number,data:Buffer)=>{const b=Buffer.alloc(data.length+5);b[0]=data.length;b.writeUInt16BE(address,1);b[3]=type;data.copy(b,4);let sum=0;for(const x of b)sum=(sum+x)&255;b[b.length-1]=(-sum)&255;lines.push(":"+b.toString("hex").toUpperCase());};
  for(let i=0;i<entries.length;){const a=entries[i].addr,hi=a>>>16;if(hi!==upper){record(0,4,Buffer.from([hi>>>8,hi&255]));upper=hi;}const n=Math.min(16,entries.length-i,0x10000-(a&0xffff));const data=Buffer.alloc(n);for(let j=0;j<n;j++){if(entries[i+j].addr!==a+j)break;data[j]=entries[i+j].value;}const actual=data.subarray(0,Math.min(n,entries.slice(i,i+n).findIndex((e,j)=>e.addr!==a+j)<0?n:entries.slice(i,i+n).findIndex((e,j)=>e.addr!==a+j)));record(a&0xffff,0,actual);i+=actual.length;}
  record(0,1,Buffer.alloc(0)); if(opts.duplicateHex)lines.splice(lines.length-1,0,lines[1]); if(opts.trailingHex)lines.push(":00000001FF"); return Buffer.from(lines.join("\n")+"\n");
}
function reject(name:string,mutate:(f:ReturnType<typeof fixture>)=>void):void{
  const f=fixture();mutate(f);
  let message='';try{auditRehearsalImage(f.input);}catch(e){message=String(e);}
  if(!message)throw new Error(`expected rejection: ${name}`);
  const targeted:Record<string,string>={
   'overlapping .dinit destinations':'pairwise .dinit','unsupported .dinit op':'unsupported .dinit operation',
   'wrong fuse word':'configuration word','nonzero dinit trailing bytes':'terminator tail',
   'duplicate required symbol':'missing/ambiguous','entry outside executable bytes':'entry not mapped',
   'unsupported ISA flags':'ISA/ABI','segment unexplained memory tail':'unexplained memory',
   'segment uses unrelated file coverage':'unexplained memory','reservation physical alias':'overlaps reservation',
  };
  if(targeted[name]&&!message.includes(targeted[name]))throw Error(name+': rejected by wrong check: '+message);
}
function refreshManifest(f:ReturnType<typeof fixture>,regenerateHex=false):void{
  if(regenerateHex)f.input.hex=makeHex(Buffer.from(f.input.elf),[],{});
  // Rebind all candidate-authored consistency hashes so each negative reaches its intended parser check.
  const b=Buffer.from(f.input.elf), shoff=b.readUInt32LE(32), ent=b.readUInt16LE(46), n=b.readUInt16LE(48);
  const allocations:any[]=[];const ns=shoff+b.readUInt16LE(50)*ent;
  const namesOff=ns+20<=b.length?b.readUInt32LE(ns+16):0;
  for(let i=0;i<n;i++){
    const x=shoff+i*ent;if(x+40>b.length)break;
    const flags=b.readUInt32LE(x+8),type=b.readUInt32LE(x+4);if(!(flags&2))continue;
    const begin=namesOff+b.readUInt32LE(x),end=b.indexOf(0,begin);const name=end>=begin?b.toString('utf8',begin,end):'invalid';
    const start=b.readUInt32LE(x+12),size=b.readUInt32LE(x+20);
    allocations.push({name,start,end:start+size,kind:type===8?"nobits":"progbits"});
  }
  f.map.elf_sha256=h(f.input.elf);f.map.allocations=allocations;f.input.map=Buffer.from(JSON.stringify(f.map));
  f.manifest.elf_sha256=h(f.input.elf); f.manifest.hex_sha256=h(f.input.hex); f.manifest.map_sha256=h(f.input.map);
  f.input.manifest=Buffer.from(JSON.stringify(f.manifest)); f.policy.source_manifest_sha256=h(f.input.manifest); f.input.policy=Buffer.from(JSON.stringify(f.policy)); f.input.expectedPolicySha256=h(f.input.policy);
}

const positive=fixture();
const result=auditRehearsalImage(positive.input);
if(result.structural_status!=="pass"||result.startup_status!=="unsupported"||result.control_flow_status!=="unsupported"||result.application_safety_status!=="unsupported"||result.delivery_approved||result.hardware_approved||result.nvm_qualified||result.manual_approval) throw new Error("positive fixture statuses/approval invariants");
let count=0;
const cases:[string,(f:ReturnType<typeof fixture>)=>void][]=[
 ["KSEG2 allocated alias",f=>{f.input.elf.writeUInt32LE(0xc0000000,0x700+40+12);refreshManifest(f);} ],
 ["PT_LOAD virtual/physical alias",f=>{f.input.elf.writeUInt32LE(0x9d000001,60);refreshManifest(f);} ],
 ["configuration file offset",f=>{f.input.elf.writeUInt32LE(0x341,0x700+5*40+16);refreshManifest(f);} ],
 ["undefined required symbol",f=>{f.input.elf.writeUInt16LE(0,0x4a0+2*16+14);refreshManifest(f);} ],
 ["unterminated ELF string",f=>{f.input.elf.fill(65,0x560,0x560+f.input.elf.readUInt32LE(0x700+8*40+20));refreshManifest(f);} ],
 ["duplicate HEX bytes",f=>{f.input.hex=makeHex(Buffer.from(f.input.elf),[],{duplicateHex:true});refreshManifest(f);} ],
 ["HEX after EOF",f=>{f.input.hex=makeHex(Buffer.from(f.input.elf),[],{trailingHex:true});refreshManifest(f);} ],
 ["overlapping .dinit destinations",f=>{const d=f.loc.dinitFile;f.input.elf.writeUInt32LE(0xa0001000,d+12);refreshManifest(f,true);} ],
 ["forbidden stack destination",f=>{const d=f.loc.dinitFile;f.input.elf.writeUInt32LE(0xa000f000,d);refreshManifest(f,true);} ],
 ["unsupported .dinit op",f=>{f.input.elf.writeUInt32LE(2,f.loc.dinitFile+8);refreshManifest(f,true);} ],
 ["zero .dinit length",f=>{f.input.elf.writeUInt32LE(0,f.loc.dinitFile+4);refreshManifest(f,true);} ],
 ["bad mailbox size",f=>{f.input.elf.writeUInt32LE(147,f.loc.symtab+7*16+8);refreshManifest(f);} ],
 ["policy reservation intersects object",f=>{f.policy.reservations[0].start=0xa0001020;f.policy.reservations[0].end=0xa0002000;f.input.policy=Buffer.from(JSON.stringify(f.policy));f.input.expectedPolicySha256=h(f.input.policy);f.input.manifest=Buffer.from(JSON.stringify(f.manifest));f.policy.source_manifest_sha256=h(f.input.manifest);f.input.policy=Buffer.from(JSON.stringify(f.policy));f.input.expectedPolicySha256=h(f.input.policy);f.map.reservations=f.policy.reservations;f.input.map=Buffer.from(JSON.stringify(f.map));f.manifest.map_sha256=h(f.input.map);f.input.manifest=Buffer.from(JSON.stringify(f.manifest));f.policy.source_manifest_sha256=h(f.input.manifest);f.input.policy=Buffer.from(JSON.stringify(f.policy));f.input.expectedPolicySha256=h(f.input.policy);} ],
 ["source hash mismatch",f=>{f.input.sources["fixture.c"]=Buffer.from("altered");} ],
 ["independent policy pin mismatch",f=>{f.input.expectedPolicySha256="0".repeat(64);} ],
 ["manifest map hash binding",f=>{f.manifest.map_sha256="0".repeat(64);f.input.manifest=Buffer.from(JSON.stringify(f.manifest));f.policy.source_manifest_sha256=h(f.input.manifest);f.input.policy=Buffer.from(JSON.stringify(f.policy));f.input.expectedPolicySha256=h(f.input.policy);} ],
 ["map ELF binding",f=>{f.map.elf_sha256="0".repeat(64);f.input.map=Buffer.from(JSON.stringify(f.map));f.manifest.map_sha256=h(f.input.map);f.input.manifest=Buffer.from(JSON.stringify(f.manifest));f.policy.source_manifest_sha256=h(f.input.manifest);f.input.policy=Buffer.from(JSON.stringify(f.policy));f.input.expectedPolicySha256=h(f.input.policy);} ],
 ["wrong fuse word",f=>{f.input.elf.writeUInt32LE(0x7fffffe3,0x34c);refreshManifest(f,true);} ],
 ["missing config HEX",f=>{f.input.hex=makeHex(Buffer.from(f.input.elf),[],{omitConfig:true});refreshManifest(f);} ],
 ["ELF/HEX mismatch",f=>{f.input.hex=makeHex(Buffer.from(f.input.elf),[],{mismatchHex:true});refreshManifest(f);} ],
 ["extra ELF allocated section map mismatch",f=>{f.input.map=Buffer.from(JSON.stringify({...f.map,allocations:[...f.map.allocations,{name:"surprise",start:0x1d000300,end:0x1d000301,kind:"progbits"}]}));f.manifest.map_sha256=h(f.input.map);f.input.manifest=Buffer.from(JSON.stringify(f.manifest));f.policy.source_manifest_sha256=h(f.input.manifest);f.input.policy=Buffer.from(JSON.stringify(f.policy));f.input.expectedPolicySha256=h(f.input.policy);} ],
 ["candidate fake approval field",f=>{f.map.delivery_approved=true;f.input.map=Buffer.from(JSON.stringify(f.map));refreshManifest(f);} ],
 ["policy span includes prohibited debug vectors",f=>{f.policy.allowed_boot_flash_spans=[{start:0x1fc00000,end:0x1fc00bf0}];f.policy.source_manifest_sha256=h(f.input.manifest);f.input.policy=Buffer.from(JSON.stringify(f.policy));f.input.expectedPolicySha256=h(f.input.policy);} ],
 ["policy outside Boot Flash",f=>{f.policy.allowed_boot_flash_spans=[{start:0x1fd00000,end:0x1fd00010}];f.input.policy=Buffer.from(JSON.stringify(f.policy));f.input.expectedPolicySha256=h(f.input.policy);} ],
 ["reserved section overlap",f=>{f.input.elf.writeUInt32LE(0xa000c000,0x700+3*40+12);f.input.elf.writeUInt32LE(32,0x700+3*40+20);refreshManifest(f);} ],
 ["ELF section count bound/header",f=>{f.input.elf.writeUInt16LE(0xffff,48);refreshManifest(f);} ],
 ["dinit missing termination",f=>{f.input.elf.fill(1,f.loc.dinitFile+24,f.loc.dinitFile+28);refreshManifest(f,true);} ],
 ["nonzero dinit trailing bytes",f=>{f.input.elf[f.loc.dinitFile+28]=1;refreshManifest(f,true);} ],
 ["duplicate required symbol",f=>{f.input.elf.writeUInt32LE(f.input.elf.readUInt32LE(f.loc.symtab+16), f.loc.symtab+7*16);refreshManifest(f);} ],
 ["entry outside executable bytes",f=>{f.input.elf.writeUInt32LE(0xa0001000,24);refreshManifest(f);} ],
 ["unsupported ISA flags",f=>{f.input.elf.writeUInt32LE(0,36);refreshManifest(f);} ],
 ["segment alignment incongruence",f=>{f.input.elf.writeUInt32LE(0x300,52+28);refreshManifest(f);} ],
 ["section alignment mismatch",f=>{f.input.elf.writeUInt32LE(8,0x700+4*40+32);f.input.elf.writeUInt32LE(0xa0001024,0x700+4*40+12);refreshManifest(f);} ],
 ["section invalid alignment",f=>{f.input.elf.writeUInt32LE(3,0x700+40+32);refreshManifest(f);} ],
 ["segment unexplained memory tail",f=>{f.input.elf.writeUInt32LE(0x124,52+20);refreshManifest(f);} ],
 ["segment uses unrelated file coverage",f=>{f.input.elf.writeUInt16LE(5,44);f.input.elf.copy(f.input.elf,52+4*32,52,84);f.input.elf.writeUInt32LE(0x9d000400,52+4*32+8);f.input.elf.writeUInt32LE(0x9d000400,52+4*32+12);refreshManifest(f);} ],
 ["required code symbol alias",f=>{f.input.elf.writeUInt32LE(0x9d000010,f.loc.symtab+3*16+4);refreshManifest(f);} ],
 ["reservation physical alias",f=>{f.policy.reservations.push({name:'boot-debug',start:0x9fc00000,end:0x9fc00008,kind:'debug'});f.map.reservations=f.policy.reservations;refreshManifest(f);} ],
];
for(const [name,mut] of cases){reject(name,mut);count++;}
// Verify a candidate-authored approval flag cannot change the always-false result: schema rejects it.
let fakeRejected=false; const fake=fixture();fake.policy.manual_approval=true;fake.input.policy=Buffer.from(JSON.stringify(fake.policy));fake.input.expectedPolicySha256=h(fake.input.policy);try{auditRehearsalImage(fake.input);}catch{fakeRejected=true;}if(!fakeRejected)throw new Error("candidate approval flag was accepted");count++;
// Legitimate initialization containment: one clear can cover both objects.
const coalesced=fixture();coalesced.input.elf.writeUInt32LE(180,0x304);coalesced.input.elf.fill(0,0x30c,0x320);refreshManifest(coalesced,true);
if(auditRehearsalImage(coalesced.input).structural_status!=='pass')throw Error('coalesced object clear rejected');
// Copy decoding is supported for ordinary mapped RAM; object nonzero authority
// remains rejected. First inline one-byte copy, then clear whole mailbox.
const copied=fixture();const table=Buffer.alloc(32);table.writeUInt32LE(0xa0001000,0);table.writeUInt32LE(1,4);table.writeUInt32LE(1,8);table[12]=7;table.writeUInt32LE(0xa0001020,16);table.writeUInt32LE(148,20);table.copy(copied.input.elf,0x300);refreshManifest(copied,true);
let copyRejected='';try{auditRehearsalImage(copied.input);}catch(e){copyRejected=String(e);}
if(!copyRejected.includes('zero-initialize'))throw Error('copy object authority rejected by wrong check '+copyRejected);count++;
const unmapped=fixture();unmapped.input.elf.writeUInt32LE(0xa0004000,0x300);refreshManifest(unmapped,true);let unmappedError='';try{auditRehearsalImage(unmapped.input);}catch(e){unmappedError=String(e);}
if(!unmappedError.includes('unmapped SRAM'))throw Error('unmapped dinit accepted/wrong check '+unmappedError);count++;
const padding=fixture();table[13]=1;table.copy(padding.input.elf,0x300);refreshManifest(padding,true);let padError='';try{auditRehearsalImage(padding.input);}catch(e){padError=String(e);}
if(!padError.includes('alignment padding'))throw Error('copy padding accepted/wrong check '+padError);count++;
// CLI reads synthetic files only. Unknown/missing pins or over-limit manifests
// must fail BEFORE touching a manifest-controlled source path.
const temp=mkdtempSync(join(tmpdir(),'ram-audit-'));
try {
 const f=fixture();for(const key of ['elf','hex','map','manifest','policy'] as const)writeFileSync(join(temp,key),f.input[key]);
 writeFileSync(join(temp,'fixture.c'),f.input.sources['fixture.c']);
 const args=['--elf',join(temp,'elf'),'--hex',join(temp,'hex'),'--map',join(temp,'map'),'--manifest',join(temp,'manifest'),'--policy',join(temp,'policy'),'--expected-policy-sha256',f.input.expectedPolicySha256];
 const cli=runAuditCli(args);if(cli.application_safety_status!=='unsupported'||cli.delivery_approved)throw Error('CLI approval regression');
 function cliReject(label:string,a:string[],match:string){let message='';try{runAuditCli(a);}catch(e){message=String(e);}if(!message.includes(match))throw Error(label+': wrong rejection '+message);count++;}
 cliReject('missing args',[],'explicit');cliReject('pin before source',args.slice(0,-1).concat('0'.repeat(64)),'policy hash');
 const many={...f.manifest,sources:Array.from({length:257},(_,i)=>({path:'absent'+i,sha256:'0'.repeat(64)}))};
 writeFileSync(join(temp,'manifest'),JSON.stringify(many));cliReject('source count preflight',args,'source count');
 writeFileSync(join(temp,'manifest'),f.input.manifest);
 const escape={...f.manifest,sources:[{path:'fixture.c',sha256:h(f.input.sources['fixture.c'])}]};
 rmSync(join(temp,'fixture.c'));symlinkSync('/etc/hosts',join(temp,'fixture.c'));
 cliReject('realpath confinement',args,'escapes manifest');
}finally{rmSync(temp,{recursive:true,force:true});}
console.log(`PASS: positive synthetic structural/CLI fixtures; ${count} negative/approval regressions rejected. No target compiler/linker or board tools invoked.`);
