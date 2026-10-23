/** Initial bounded XC32 native-map inventory extraction. No safety verdict,
 * normalized-map conversion, linker invocation or invented stack reservations. */
import {createHash} from 'node:crypto';
export interface NativeMapInventory {
 schema:'pic32mx270-native-map-inventory-v1'; native_map_status:'inventory_only';
 map_sha256:string; regions:{name:string;origin:number;length:number;attributes:string}[];
 sections:{name:string;address:number;bytes:number;line:number}[];
 loads:string[]; archives:string[]; output:string; declarations_without_allocation:string[];
 startup_status:'unsupported';application_safety_status:'unsupported';delivery_approved:false;
 limitations:string[];
}
function check(v:unknown,m:string):asserts v{if(!v)throw Error(m);}
export function readNativeMap(bytes:Uint8Array):NativeMapInventory{
 check(bytes.length>0&&bytes.length<=2*1024*1024,'native map size limit');
 const text=new TextDecoder('utf-8',{fatal:true}).decode(bytes),lines=text.split(/\r?\n/);
 check(lines.length<=30000&&lines.every(l=>l.length<=4096),'native map line limits');
 const memory=lines.indexOf('Memory Configuration'),map=lines.indexOf('Linker script and memory map');
 check(memory>=0&&map>memory&&lines.lastIndexOf('Memory Configuration')===memory&&lines.lastIndexOf('Linker script and memory map')===map,'native map unique headings/order');
 const regions:NativeMapInventory['regions']=[];const regionNames=new Set<string>();
 for(let i=memory+1;i<map;i++){
  const line=lines[i];if(!line.trim()||line.startsWith('Name '))continue;
  const m=line.match(/^(\S+)\s+0x([0-9a-fA-F]{1,8})\s+0x([0-9a-fA-F]{1,8})(?:\s+([^\s]+))?\s*$/);
  check(m,'unsupported native memory-region row at '+(i+1));
  check(!regionNames.has(m[1]),'duplicate memory region');regionNames.add(m[1]);
  const origin=parseInt(m[2],16),length=parseInt(m[3],16);
  check(length>0&&length<=0x100000000-origin,'memory region overflow');
  regions.push({name:m[1],origin,length,attributes:m[4]??''});
 }
 const expected=[['kseg0_program_mem',0x9d000000,32768],['kseg1_data_mem',0xa0000000,65536]] as const;
 for(const [name,origin,length]of expected)check(regions.some(r=>r.name===name&&r.origin===origin&&r.length===length),'native map profile mismatch '+name);
 const sections:NativeMapInventory['sections']=[],loads:string[]=[];
 let output='';const declarations:string[]=[];
 for(let i=map+1;i<lines.length;i++){
  const line=lines[i];
  if(line.startsWith('LOAD ')){check(line.length>5,'empty LOAD');loads.push(line.slice(5));continue;}
  if(line.startsWith('OUTPUT(')){check(!output&&/^OUTPUT\(.+ elf32-tradlittlemips\)\s*$/.test(line),'unsupported/duplicate OUTPUT');output=line;continue;}
  if(!line.startsWith('.'))continue; // indented input sections/fills/symbols never output ownership
  let m=line.match(/^(\.[^\s]+)\s+0x([0-9a-fA-F]{1,8})\s+0x([0-9a-fA-F]{1,8})(?:\s+load address 0x[0-9a-fA-F]{1,8})?\s*$/);
  if(!m){
   const name=line.match(/^(\.[^\s]+)\s*$/);check(name,'unsupported native output row at '+(i+1));
   check(i+1<lines.length,'truncated native output row');
   const next=lines[i+1];
   const wildcard=next.trim();
   if(/^\*(?:\.elf)?\(\.[A-Za-z0-9_.*-]+(?:\s+\.[A-Za-z0-9_.*-]+)*\)$/.test(wildcard)){
    if(!declarations.includes(name[1]))declarations.push(name[1]);continue;
   }
   const tail=lines[++i].match(/^\s+0x([0-9a-fA-F]{1,8})\s+0x([0-9a-fA-F]{1,8})(?:\s+load address 0x[0-9a-fA-F]{1,8})?\s*$/);
   check(tail,'unsupported native wrapped output row at '+(i+1)+': '+lines[i]);m=[tail[0],name[1],tail[1],tail[2]] as RegExpMatchArray;
  }
  const address=parseInt(m[2],16),size=parseInt(m[3],16);
  check(size<=0x100000000-address,'output-section overflow');check(!sections.some(s=>s.name===m![1]&&s.bytes>0&&size>0&&s.address===address),'duplicate allocated native output row '+m[1]);
  sections.push({name:m[1],address,bytes:size,line:i+1});check(sections.length<=4096,'native output count limit');
 }
 check(output&&loads.length>0&&loads.length<=4096&&sections.length>0,'native map lacks bounded OUTPUT/LOAD/sections');
 const archives=[...new Set([...text.matchAll(/([^\s()]+\.a)\(([^()\r\n]+)\)/g)].map(m=>m[1]+'('+m[2]+')'))];
 check(archives.length<=4096,'archive inventory limit');
 return{schema:'pic32mx270-native-map-inventory-v1',native_map_status:'inventory_only',map_sha256:createHash('sha256').update(bytes).digest('hex'),regions,sections,loads,archives,output,declarations_without_allocation:declarations,startup_status:'unsupported',application_safety_status:'unsupported',delivery_approved:false,
  limitations:['Output-region/section/LOAD/archive inventory only, not allocation or source-provenance proof.','No native ELF cross-check, normalized audit-map conversion, stack/heap reconstruction or selected runtime verification.','Discarded archive references may appear in archive inventory; final member selection requires linked evidence.','Empty declarations/repeated best-fit names are inventory labels, not independently verified allocations; no-address declarations invent no span.']};
}
