/** SCRIPT_JDOC
 * {"summary":"Parse explicit native RAM rehearsal ELF/HEX/map with pinned gap and tail rules; safety and delivery always unsupported","keywords":["rehearsal","native","elf","map","structural"],"kind":"read-only","weight":"standard","role":"entrypoint"}
 */
/** Explicit native structural subset. No source/runtime provenance, general
 * instruction proof, policy authentication, stack-capacity or delivery pass. */
import {createHash} from 'node:crypto';
import {parseElf,parseHex,readBounded,type NativeGap} from './rehearsal-image';
import {readNativeMap} from './rehearsal-native-map';
const sha=(b:Uint8Array)=>createHash('sha256').update(b).digest('hex');
function check(x:unknown,m:string):asserts x{if(!x)throw Error(m);}
function obj(x:unknown):x is Record<string,any>{return !!x&&typeof x==='object'&&!Array.isArray(x);}
function keys(x:Record<string,any>,wanted:string[],label:string){check(Object.keys(x).sort().join('|')===[...wanted].sort().join('|'),label+' missing/extra keys');}
export interface NativeInput {elf:Uint8Array;hex:Uint8Array;map:Uint8Array;policy:Uint8Array;expectedPolicySha256:string;}
export function nativePolicy(bytes:Uint8Array,pin:string){
 check(bytes.length>0&&bytes.length<=65536,'native policy size');check(/^[a-f0-9]{64}$/.test(pin)&&sha(bytes)===pin,'native policy pin mismatch');
 const p=JSON.parse(new TextDecoder('utf-8',{fatal:true}).decode(bytes));check(obj(p),'native policy object');
 keys(p,['schema','provenance','gaps','dinit_tail_hex'],'native policy');
 check(p.schema==='pic32mx270-native-format-policy-v1'&&typeof p.provenance==='string'&&p.provenance.length>0&&p.provenance.length<=512,'native policy schema/provenance');
 check(['','00000000','66666666'].includes(p.dinit_tail_hex),'unsupported native dinit tail rule');
 check(Array.isArray(p.gaps)&&p.gaps.length<=256,'native gap count');
 const gaps:NativeGap[]=p.gaps;for(const g of gaps){check(obj(g),'gap object');keys(g,['start','end','kind'],'native gap');
  check(Number.isInteger(g.start)&&Number.isInteger(g.end)&&g.end>g.start&&g.end<=0x100000000,'gap arithmetic');
  check((g.start>=0x9d000000&&g.end<=0x9d008000)||(g.start>=0xbfc00000&&g.end<=0xbfc00480),'gap domain');
  check(g.kind==='zero-file'||g.kind==='no-file','native gap kind');
 }
 const sorted=[...gaps].sort((a,b)=>a.start-b.start);for(let i=1;i<sorted.length;i++)check(sorted[i].start>=sorted[i-1].end,'overlapping native gap rules');
 return {gaps,dinitTail:p.dinit_tail_hex as string,provenance:p.provenance as string};
}
export function auditNativeRehearsal(input:NativeInput){
 const p=nativePolicy(input.policy,input.expectedPolicySha256);const elf=parseElf(input.elf,{gaps:p.gaps});
 const map=readNativeMap(input.map);const memory=parseHex(input.hex,{allowed_boot_flash_spans:[{start:0x1fc00000,end:0x1fc00480}]});
 const phys=(a:number)=>a>=0x80000000?(a&0x1fffffff)>>>0:a;
 const alloc=elf.sections.filter(s=>(s.flags&2)&&s.size>0),files=alloc.filter(s=>s.type!==8);
 const expected=new Map<number,number>();
 for(const segment of elf.loads){
  const contained=alloc.filter(s=>s.addr>=segment.vaddr&&s.size<=segment.memsz-(s.addr-segment.vaddr));
  const permissions=contained.reduce((flags,s)=>flags|((s.flags&4)?1:0)|((s.flags&1)?2:0),4);
  check(segment.flags===permissions,'native segment permission over/under-grant');
 }
 for(const s of alloc){
  const owners=elf.loads.filter(l=>s.addr>=l.vaddr&&s.size<=l.memsz-(s.addr-l.vaddr)&&
   (s.type===8?s.addr>=l.vaddr+l.filesz:s.size<=l.filesz-(s.addr-l.vaddr)&&s.off===l.off+s.addr-l.vaddr));
  check(owners.length===1,'native section needs unique mapped owner '+s.name);
  check(!(s.flags&4)||!!(owners[0].flags&1),'native executable segment permission');
  check(!(s.flags&1)||!!(owners[0].flags&2),'native writable segment permission');
  check(map.sections.filter(m=>m.name===s.name&&m.address===s.addr&&m.bytes===s.size).length===1,'native ELF/map allocation mismatch '+s.name);
  if(s.type===8)continue;
  for(let i=0;i<s.size;i++){const at=phys(s.addr)+i;check(!expected.has(at),'native delivered overlap');expected.set(at,elf.buf[s.off+i]);}
 }
 for(const row of map.sections.filter(m=>m.bytes>0))check(elf.sections.some(s=>s.name===row.name&&s.addr===row.address&&s.size===row.bytes),'native map row absent from ELF '+row.name);
 check(memory.size===expected.size,'native HEX count/extra bytes');for(const [at,value]of expected)check(memory.get(at)===value,'native ELF/HEX mismatch/missing bytes');
 const configs=files.filter(s=>s.name.startsWith('.config'));check(configs.length===4,'native exact four config sections');
 const words=[0xffffffff,0xfff97afa,0xfd74ce5b,0x7fffffe0];
 for(let i=0;i<4;i++){const address=0xbfc00bf0+i*4,name='.config_'+address.toString(16).toUpperCase();const s=configs.find(s=>s.name===name);check(s&&s.addr===address&&s.size===4&&s.type===1&&s.flags===2,'native config identity');check(elf.buf.readUInt32LE(s.off)===words[i],'native config word');}
 const symbols=(name:string)=>{const hits=elf.symbols.filter(s=>s.name===name);check(hits.length===1&&hits[0].shndx!==0,'native symbol missing/ambiguous '+name);return hits[0];};
 const code=(name:string)=>{const s=symbols(name),section=elf.sections[s.shndx];check(section&&(section.flags&6)===6&&!(section.flags&1)&&s.type===2&&s.size>0&&s.value%4===0&&s.value>=section.addr&&s.size<=section.size-(s.value-section.addr),'native code symbol bounds '+name);return s;};
 const reset=code('_reset'),main=code('main'),pre=code('RAM_RehearsalPreEdit'),readout=code('RAM_RehearsalReadout');
 check(reset.value===0xbfc00000&&elf.buf.readUInt32LE(24)===reset.value,'native reset entry');
 const codes=[reset,main,pre,readout];for(const s of [main,pre,readout])check(s.value>=0x9d000000&&s.size<=0x9d008000-s.value,'native code prefix');
 for(let i=0;i<codes.length;i++)for(let j=i+1;j<codes.length;j++)check(phys(codes[i].value)+codes[i].size<=phys(codes[j].value)||phys(codes[j].value)+codes[j].size<=phys(codes[i].value),'native required symbol alias');
 const objects=[['rehearsal_context',200],['rehearsal_mailbox',148]] as const;
 const data=objects.map(([name,size])=>{const s=symbols(name),section=elf.sections[s.shndx];check(section?.type===8&&(section.flags&3)===3&&s.type===1&&s.size===size&&s.value%4===0&&s.value>=section.addr&&s.size<=section.size-(s.value-section.addr)&&s.value>=0xa0000200&&s.size<=0xa0010000-s.value,'native data object size/bounds '+name);return s;});
 check(data[0].value+data[0].size<=data[1].value||data[1].value+data[1].size<=data[0].value,'native object overlap');
 for(const [name,addr,size]of [['.dbg_data',0xa0000000,512],['.dbg_excpt',0xbfc00480,8],['.dbg_code',0x9fc00490,0x760]] as const){const s=alloc.find(s=>s.name===name);check(s&&s.type===8&&s.addr===addr&&s.size===size,'native fixed debugger reservation '+name);}
 check(symbols('_DEBUGGER').shndx===0xfff1&&symbols('_DEBUGGER').type===0&&symbols('_DEBUGGER').value===1,'native debugger mode symbol');
 const absRam=(name:string)=>{const s=symbols(name);check(s.shndx===0xfff1&&s.type===0&&s.value>=0xa0000000&&s.value<=0xa0010000,'native RAM boundary '+name);return s.value;};
 const heap=absRam('_heap'),eheap=absRam('_eheap'),splim=absRam('_splim'),stack=absRam('_stack');
 check(heap>=0xa0000200&&eheap>heap&&eheap-heap===512&&splim>=eheap&&stack>splim&&stack%8===0&&stack+8<=0xa0010000,'native heap/stack symbol ordering');
 for(const [name,value]of [['_min_heap_size',512],['_min_stack_size',1024]] as const){const s=symbols(name);check(s.shndx===0xfff1&&s.type===0&&s.value===value,'native baseline absolute memory minima');}
 // Treat the symbol interval conservatively as reserved, without claiming
 // capacity/executive usage. Never derive a stack range merely from min size.
 for(const s of alloc.filter(s=>s.addr>=0xa0000000&&s.addr<0xa0010000&&s.name!=='.dbg_data'))check(s.addr+s.size<=heap||s.addr>=eheap,'native allocation overlaps heap');
 for(const s of alloc.filter(s=>s.addr>=0xa0000000&&s.addr<0xa0010000))check(s.addr+s.size<=splim||s.addr>=stack+8,'native allocation overlaps stack-symbol interval');
 const ds=elf.sections.filter(s=>s.name==='.dinit');check(ds.length===1,'native dinit unique');const d=ds[0];check(d.type===1&&d.flags===2&&d.addr>=0x9d000000&&d.size>=16&&d.size%4===0,'native dinit shape');
 const daddr=symbols('_dinit_addr');
 check(daddr.value===d.addr&&(daddr.shndx===0xfff1||daddr.shndx===d.i)&&[0,1].includes(daddr.type),'native runtime dinit definition/address');
 const records:{destination:number;bytes:number;operation:number}[]=[];let cursor=d.off,end=d.off+d.size,terminated=false;
 while(cursor+4<=end){const dst=elf.buf.readUInt32LE(cursor);if(!dst){check(elf.buf.subarray(cursor+4,end).toString('hex')===p.dinitTail,'native dinit tail differs from pinned rule');terminated=true;break;}
  check(cursor+12<=end,'native dinit truncated header');const n=elf.buf.readUInt32LE(cursor+4),op=elf.buf.readUInt32LE(cursor+8);check(n>0&&dst>=0xa0000200&&n<=0xa0010000-dst,'native dinit destination');check(op===0,'native clear-only decoder support');
  const spans=alloc.filter(s=>s.type===8&&s.addr>=0xa0000200&&s.addr<0xa0010000).sort((a,b)=>a.addr-b.addr);let at=dst;for(const s of spans){if(s.addr>at)break;if(s.addr+s.size>at)at=Math.min(dst+n,s.addr+s.size);}check(at===dst+n,'native dinit unmapped storage');
  check(dst+n<=heap||dst>=eheap,'native dinit heap overlap');check(dst+n<=splim||dst>=stack+8,'native dinit stack overlap');
  for(const old of records)check(dst+n<=old.destination||dst>=old.destination+old.bytes,'native dinit pairwise overlap');
  records.push({destination:dst,bytes:n,operation:op});check(records.length<=1024,'native dinit count');cursor+=12;
 }
 check(terminated&&records.length>0,'native missing dinit termination');for(const s of data)check(records.some(r=>s.value>=r.destination&&s.value+s.size<=r.destination+r.bytes),'native object not zero initialized');
 return {schema:'pic32mx270-native-structural-audit-v1',native_structural_status:'pass',policy_status:'pin_matched_not_approval',elf_sha256:sha(input.elf),hex_sha256:sha(input.hex),map_sha256:sha(input.map),policy_sha256:sha(input.policy),hex_bytes:memory.size,allocated_sections:alloc.length,initialization:records,gaps:p.gaps,dinit_tail_hex:p.dinitTail,heap:{start:heap,end:eheap},stack_symbols:{splim,stack},startup_status:'unsupported',control_flow_status:'unsupported',application_safety_status:'unsupported',installed_executive_status:'unknown',delivery_approved:false,hardware_approved:false,nvm_qualified:false,limits:['Native structural subset only; no source provenance or general instruction/runtime proof.','ABI bytes match one reviewed metadata shape, not hardware ISA qualification.','Gap/tail policy pin is trusted caller reproducibility, not approval/authentication.','Stack-symbol reservation checks do not prove stack capacity or executive usage.']};
}
export function runNativeCli(argv:string[]){
 const args:Record<string,string>={};for(let i=0;i<argv.length;i++){const key=argv[i];check(['--elf','--hex','--map','--policy','--expected-policy-sha256'].includes(key)&&!Object.hasOwn(args,key)&&argv[i+1]&&!argv[i+1].startsWith('--'),'native explicit arguments');args[key]=argv[++i];}check(Object.keys(args).length===5,'native requires all explicit arguments');
 const policy=readBounded(args['--policy'],65536);nativePolicy(policy,args['--expected-policy-sha256']);
 return auditNativeRehearsal({elf:readBounded(args['--elf']),hex:readBounded(args['--hex']),map:readBounded(args['--map'],2*1024*1024),policy,expectedPolicySha256:args['--expected-policy-sha256']});
}
if(import.meta.main){try{console.log(JSON.stringify(runNativeCli(process.argv.slice(2)),null,2));}catch(e){console.error('unsupported: '+String(e));process.exitCode=1;}}
