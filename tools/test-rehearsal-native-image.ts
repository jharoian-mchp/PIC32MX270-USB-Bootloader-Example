#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Test explicit native RAM rehearsal format with synthetic ELF/map/HEX mutants and optional existing-artifact read-only regression","keywords":["native","rehearsal","ELF","audit","negative"],"kind":"read-only","weight":"standard","role":"entrypoint"}
 */
import{strict as a}from'node:assert';import{readFileSync,mkdtempSync,writeFileSync,rmSync}from'node:fs';import{resolve,join}from'node:path';import{tmpdir}from'node:os';import{createHash}from'node:crypto';
import{auditNativeRehearsal,runNativeCli,type NativeInput}from'./rehearsal-native-image';import{parseElf}from'./rehearsal-image';
process.chdir(resolve(import.meta.dir,'..'));const hash=(b:Uint8Array)=>createHash('sha256').update(b).digest('hex');
function hex(sections:any[],b:Buffer){const lines:string[]=[];let upper=-1;const record=(off:number,kind:number,data:Buffer)=>{const row=Buffer.alloc(data.length+5);row[0]=data.length;row.writeUInt16BE(off,1);row[3]=kind;data.copy(row,4);row[row.length-1]=(-row.reduce((n,v)=>n+v,0))&255;lines.push(':'+row.toString('hex').toUpperCase());};for(const s of sections.filter(s=>(s.flags&2)&&s.size&&s.type!==8)){const addr=s.addr&0x1fffffff;for(let i=0;i<s.size;i+=16){const at=addr+i,hi=at>>>16;if(hi!==upper){record(0,4,Buffer.from([hi>>>8,hi&255]));upper=hi;}record(at&65535,0,b.subarray(s.off+i,s.off+Math.min(i+16,s.size)));}}record(0,1,Buffer.alloc(0));return Buffer.from(lines.join('\n')+'\n');}
function fixture(){
 const b=Buffer.alloc(0x3000),sections:any[]=[{name:'',type:0,flags:0,addr:0,off:0,size:0,align:0}];
 const add=(name:string,type:number,flags:number,addr:number,off:number,size:number,align=4)=>{sections.push({name,type,flags,addr,off,size,align});return sections.length-1;};
 const text=add('.text',1,6,0x9d000000,0x400,64),abi=add('.MIPS.abiflags',0x7000002a,2,0x9d000048,0x448,24,8),di=add('.dinit',1,2,0x9d000060,0x460,32),reset=add('.reset',1,6,0xbfc00000,0x500,8);
 add('.dbg_excpt',8,3,0xbfc00480,0x508,8,1);add('.dbg_code',8,3,0x9fc00490,0x600,0x760,1);add('.dbg_data',8,3,0xa0000000,0,512,1);
 const context=add('.bss.rehearsal_context',8,0x200003,0xa0000200,0,200),mailbox=add('.bss.rehearsal_mailbox',8,0x200003,0xa00002c8,0,148);
 for(let i=0;i<4;i++){const addr=0xbfc00bf0+i*4;add('.config_'+addr.toString(16).toUpperCase(),1,2,addr,0x600+i*4,4);b.writeUInt32LE([0xffffffff,0xfff97afa,0xfd74ce5b,0x7fffffe0][i],0x600+i*4);}
 add('.sbss',8,0x10200003,0xa0000200,0,0);add('.init_array',1,3,0x9d000080,0x480,0);
 b.fill(0x5a,0x400,0x440);Buffer.from('0000200201000003000000000b0000000100000000000000','hex').copy(b,0x448);b.set([8,0,0,0,0,0,0,0],0x500);
 b.writeUInt32LE(0xa0000200,0x460);b.writeUInt32LE(200,0x464);b.writeUInt32LE(0xa00002c8,0x46c);b.writeUInt32LE(148,0x470);b.fill(0x66,0x47c,0x480);
 const shstr=add('.shstrtab',3,0,0,0x800,0,1),str=add('.strtab',3,0,0,0xa00,0,1),sym=add('.symtab',2,0,0,0xc00,0);sections[sym].link=str;sections[sym].entsize=16;
 let names='\0';for(const s of sections.slice(1)){s.no=names.length;names+=s.name+'\0';}Buffer.from(names).copy(b,0x800);sections[shstr].size=names.length;
 const symbols:any[]=[{name:'',value:0,size:0,type:0,index:0}];
 const put=(name:string,value:number,size:number,type:number,index:number)=>symbols.push({name,value,size,type,index});
 put('main',0x9d000000,8,2,text);put('RAM_RehearsalPreEdit',0x9d000010,8,2,text);put('RAM_RehearsalReadout',0x9d000018,8,2,text);put('_reset',0xbfc00000,8,2,reset);put('rehearsal_context',0xa0000200,200,1,context);put('rehearsal_mailbox',0xa00002c8,148,1,mailbox);
 for(const [n,v]of [['_DEBUGGER',1],['_dinit_addr',0x9d000060],['_heap',0xa0000360],['_eheap',0xa0000560],['_splim',0xa0000578],['_stack',0xa000fff8],['_min_heap_size',512],['_min_stack_size',1024]] as const)put(n,v,0,0,0xfff1);
 let ss='\0';symbols.forEach((s,i)=>{const no=i?ss.length:0;if(i)ss+=s.name+'\0';const at=0xc00+i*16;b.writeUInt32LE(no,at);b.writeUInt32LE(s.value,at+4);b.writeUInt32LE(s.size,at+8);b[at+12]=i?0x10+s.type:0;b.writeUInt16LE(s.index,at+14);});Buffer.from(ss).copy(b,0xa00);sections[str].size=ss.length;sections[sym].size=symbols.length*16;
 const loads=[{v:0x9d000000,off:0x400,files:128,mem:128,flags:5},{v:0xbfc00000,off:0x500,files:8,mem:0x488,flags:7},{v:0x9fc00490,off:0x600,files:0,mem:0x760,flags:6},{v:0xa0000000,off:0,files:0,mem:512,flags:6},{v:0xa0000200,off:0,files:0,mem:348,flags:6},...Array.from({length:4},(_,i)=>({v:0xbfc00bf0+i*4,off:0x600+i*4,files:4,mem:4,flags:4}))];
 b.set([127,69,76,70,1,1,1]);b.writeUInt16LE(2,16);b.writeUInt16LE(8,18);b.writeUInt32LE(1,20);b.writeUInt32LE(0xbfc00000,24);b.writeUInt32LE(52,28);b.writeUInt32LE(0x2000,32);b.writeUInt32LE(0x70001001,36);b.writeUInt16LE(52,40);b.writeUInt16LE(32,42);b.writeUInt16LE(loads.length,44);b.writeUInt16LE(40,46);b.writeUInt16LE(sections.length,48);b.writeUInt16LE(shstr,50);
 loads.forEach((l,i)=>{const p=52+i*32;b.writeUInt32LE(1,p);b.writeUInt32LE(l.off,p+4);b.writeUInt32LE(l.v,p+8);b.writeUInt32LE(l.v,p+12);b.writeUInt32LE(l.files,p+16);b.writeUInt32LE(l.mem,p+20);b.writeUInt32LE(l.flags,p+24);b.writeUInt32LE(4,p+28);});
 sections.forEach((s,i)=>{const p=0x2000+i*40;[s.no??0,s.type,s.flags,s.addr,s.off,s.size,s.link??0,0,s.align,s.entsize??0].forEach((v,j)=>b.writeUInt32LE(v,p+j*4));});
 const policy={schema:'pic32mx270-native-format-policy-v1',provenance:'synthetic native fixture only; not approval',gaps:[{start:0x9d000040,end:0x9d000048,kind:'zero-file'},{start:0xbfc00008,end:0xbfc00480,kind:'no-file'}],dinit_tail_hex:'66666666'};
 const input:NativeInput={elf:b,hex:Buffer.alloc(0),map:Buffer.alloc(0),policy:Buffer.alloc(0),expectedPolicySha256:''};
 function refresh(){input.hex=hex(sections,b);input.map=Buffer.from('Memory Configuration\nName Origin Length Attributes\nkseg0_program_mem 0x9d000000 0x8000 xr\nkseg1_data_mem 0xa0000000 0x10000 w!x\nLinker script and memory map\nLOAD fixture.o\n'+sections.filter(s=>s.flags&2).map(s=>s.name+' 0x'+s.addr.toString(16)+' 0x'+s.size.toString(16)).join('\n')+'\nOUTPUT(fixture.elf elf32-tradlittlemips)\n');input.policy=Buffer.from(JSON.stringify(policy));input.expectedPolicySha256=hash(input.policy);}
 refresh();return {input,b,sections,symbols,loads,policy,refresh,loc:{abi:0x448,dinit:0x460,shoff:0x2000,phoff:52,sym:0xc00}};
}
const positive=fixture(),result=auditNativeRehearsal(positive.input);a.equal(result.native_structural_status,'pass');a.equal(result.application_safety_status,'unsupported');a.equal(result.delivery_approved,false);a.throws(()=>parseElf(positive.input.elf),'strict parser is not relaxed');
const cases:[string,(f:ReturnType<typeof fixture>)=>void,string][]=[
 ['ABI bytes',f=>f.b[f.loc.abi+1]^=1,'ABI'],['ABI writable',f=>f.b.writeUInt32LE(3,f.loc.shoff+2*40+8),'writable Flash'],
 ['gap nonzero',f=>f.b[0x440]=1,'gap bytes'],['gap extra rule',f=>f.policy.gaps.push({start:0x9d000080,end:0x9d000084,kind:'zero-file'}),'unused'],
 ['gap kind',f=>f.policy.gaps[0].kind='no-file','file boundary'],['gap bounds',f=>f.policy.gaps[0].start=0x9d00003c,'unapproved'],
 ['tail bytes',f=>f.b[0x47c]=0,'tail differs'],['wrong tail rule',f=>f.policy.dinit_tail_hex='00000000','tail differs'],
 ['config word',f=>f.b.writeUInt32LE(0x7fffffe3,0x60c),'config word'],['config size',f=>f.b.writeUInt32LE(3,0x2000+13*40+20),'gap'],
 ['zero-init overlapping',f=>f.b.writeUInt32LE(0xa0000200,0x46c),'pairwise'],['init debug RAM',f=>f.b.writeUInt32LE(0xa0000000,0x460),'dinit destination'],
 ['init copy unsupported',f=>f.b.writeUInt32LE(1,0x468),'clear-only'],['init missing terminator',f=>f.b.writeUInt32LE(0xa0001000,0x478),'truncated'],
 ['data wrong size',f=>f.b.writeUInt32LE(199,0xc00+5*16+8),'data object'],['data cached alias',f=>f.b.writeUInt32LE(0x80000200,0xc00+5*16+4),'data object'],
 ['heap overlap',f=>f.b.writeUInt32LE(0xa0000240,0xc00+9*16+4),'heap'],['stack order',f=>f.b.writeUInt32LE(0xa0000500,0xc00+11*16+4),'ordering'],
 ['debug size',f=>f.b.writeUInt32LE(0x75c,0x2000+6*40+20),'gap'],['entry wrong',f=>f.b.writeUInt32LE(0xa0000200,24),'entry'],
 ['metadata section type',f=>f.b.writeUInt32LE(0x7000002b,0x2000+2*40+4),'gap'],['unknown flags',f=>f.b.writeUInt32LE(0x400003,0x2000+8*40+8),'flags'],
 ['rogue reused segment',f=>{f.b.writeUInt16LE(10,44);f.b.copy(f.b,52+9*32,52,84);f.b.writeUInt32LE(0x9d000100,52+9*32+8);f.b.writeUInt32LE(0x9d000100,52+9*32+12);},'file ranges overlap'],
 ['empty array wrong type',f=>{const i=f.sections.findIndex(s=>s.name==='.init_array');f.b.writeUInt32LE(15,0x2000+i*40+4);},'empty'],
 ['empty array hidden callback bytes',f=>{const i=f.sections.findIndex(s=>s.name==='.init_array');f.b.writeUInt32LE(4,0x2000+i*40+20);},'writable Flash'],
 ['cached SRAM section',f=>{f.b.writeUInt32LE(0x80000200,0x2000+8*40+12);},'gap'],
 ['duplicate native section identity',f=>{f.b.writeUInt32LE(f.b.readUInt32LE(0x2000+8*40),0x2000+9*40);f.b.writeUInt32LE(0xa0000200,0x2000+9*40+12);},'identity'],
 ['native symbol count bounded',f=>{const i=f.sections.findIndex(s=>s.name==='.symtab');f.b.writeUInt32LE(4097*16,0x2000+i*40+20);},'file range'],
 ['native gap overlaps another rule',f=>{f.policy.gaps.push({start:0x9d000044,end:0x9d000048,kind:'zero-file'});},'overlapping'],
 ['native gap outside prefix',f=>{f.policy.gaps[0].end=0x9d008004;},'domain'],
 ['native fake approval',f=>{(f.policy as any).delivery_approved=true;},'extra'],
 ['debug executable hidden',f=>{f.b.writeUInt32LE(7,0x2000+6*40+8);},'flags'],
 ['file-to-section offset mismatch',f=>{f.b.writeUInt32LE(0x404,0x2000+1*40+16);},'gap'],
 ['flash segment permission overgrant',f=>f.b.writeUInt32LE(7,52+24),'permission over'],
 ['metadata minimum forged section definition',f=>f.b.writeUInt16LE(1,0xc00+13*16+14),'absolute memory'],
 ['runtime dinit forged section definition',f=>f.b.writeUInt16LE(1,0xc00+8*16+14),'dinit definition'],
 ['RAM boundary wrong symbol type',f=>{f.b[0xc00+9*16+12]=0x11;},'RAM boundary'],
 ['debug mode wrong symbol type',f=>{f.b[0xc00+7*16+12]=0x11;},'debugger mode'],
 ['stack top outside SRAM',f=>f.b.writeUInt32LE(0xa0010000,0xc00+12*16+4),'ordering'],
];
let count=0;for(const [label,mutate,reason]of cases){const f=fixture();mutate(f);f.refresh();let message='';try{auditNativeRehearsal(f.input);}catch(e){message=String(e);}a.ok(message.includes(reason),label+': '+message);count++;}
const mismatch=fixture();mismatch.input.expectedPolicySha256='0'.repeat(64);a.throws(()=>auditNativeRehearsal(mismatch.input),/pin mismatch/);count++;
const mapbad=fixture();mapbad.input.map=Buffer.from(mapbad.input.map.toString().replace('.dinit 0x9d000060 0x20','.dinit 0x9d000060 0x1c'));a.throws(()=>auditNativeRehearsal(mapbad.input),/map allocation mismatch/);count++;
const extra=fixture();extra.input.hex=Buffer.from(extra.input.hex.toString()+':00000001FF\n');a.throws(()=>auditNativeRehearsal(extra.input),/after EOF/);count++;
const temp=mkdtempSync(join(tmpdir(),'native-rehearsal-'));try{for(const key of ['elf','hex','map','policy'] as const)writeFileSync(join(temp,key),positive.input[key]);const args=['--elf',join(temp,'elf'),'--hex',join(temp,'hex'),'--map',join(temp,'map'),'--policy',join(temp,'policy'),'--expected-policy-sha256',positive.input.expectedPolicySha256];a.equal(runNativeCli(args).delivery_approved,false);a.throws(()=>runNativeCli([]),/requires/);a.throws(()=>runNativeCli(args.slice(0,-1).concat('0'.repeat(64))),/pin/);count+=2;}finally{rmSync(temp,{recursive:true,force:true});}
if(process.argv[2]){
 const dir=process.argv[2],elf=readFileSync(dir+'/rehearsal.elf'),hex=readFileSync(dir+'/rehearsal.hex'),map=readFileSync(dir+'/rehearsal.map');
 a.equal(hash(elf),'cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a');a.equal(hash(hex),'918d3cab3801a75860a17b2adfd3cd03590299886db5add8e600d1796670ce33');a.equal(hash(map),'f6dc317709d4299e431f32f03575f1271f64f4523a8754bdacd0032b7eed4eec');
 const gaps:any[]=[];for(let start=0x9d0002a8;start<0x9d000760;start+=32)gaps.push({start,end:start+24,kind:'zero-file'});gaps.push({start:0xbfc0014c,end:0xbfc00380,kind:'zero-file'},{start:0xbfc00390,end:0xbfc00480,kind:'no-file'});
 const policy=Buffer.from(JSON.stringify({schema:'pic32mx270-native-format-policy-v1',provenance:'read-only exact artifact regression, layout policy not safety approval',gaps,dinit_tail_hex:'66666666'}));
 const checked=auditNativeRehearsal({elf,hex,map,policy,expectedPolicySha256:hash(policy)});a.equal(checked.hex_bytes,3728);a.equal(checked.allocated_sections,79);a.equal(checked.application_safety_status,'unsupported');console.log('PASS: unchanged existing native artifact structural subset; safety unsupported/executive unknown/delivery false.');
}
console.log(`PASS: independent native synthetic fixture, strict-parser preservation and ${count} targeted/CLI rejection cases; no compile/link/board tools.`);
