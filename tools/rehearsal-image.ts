/**
 * Bounded structural-only parser for the initial PIC32MX270 RAM rehearsal image.
 * It is not an ELF loader, MIPS verifier, startup/control-flow proof, or approval.
 */
import { createHash } from "node:crypto";
import { openSync, readSync, closeSync, fstatSync, realpathSync } from "node:fs";
import { dirname, resolve, sep } from "node:path";

const MAX_FILE = 8 * 1024 * 1024;
const MAX_SECTION = 1024 * 1024;
const MAX_TABLE = 1024;
const MAX_HEX_BYTES = 1024 * 1024;
const RAM_START = 0xA0000000;
const RAM_END = 0xA0010000;
const PROGRAM_START = 0x1D000000;
const PROGRAM_END = 0x1D008000;
const BOOT_START = 0x1FC00000;
const BOOT_END = 0x1FC00BF0;
const CONFIG_START = 0x1FC00BF0;
const CONFIG_END = 0x1FC00C00;
// Physical configuration order is DEVCFG3,2,1,0 at BF0,BF4,BF8,BFC.
const CFG_WORDS = [0xFFFFFFFF, 0xFFF97AFA, 0xFD74CE5B, 0x7FFFFFE0];
const POLICY_SCHEMA = "pic32mx270-debugger-rehearsal-policy-v1";
const MAP_SCHEMA = "pic32mx270-debugger-rehearsal-normalized-map-v1";
const MANIFEST_SCHEMA = "pic32mx270-debugger-rehearsal-source-manifest-v1";

export interface AuditInput {
  elf: Uint8Array;
  hex: Uint8Array;
  map: Uint8Array;
  manifest: Uint8Array;
  policy: Uint8Array;
  expectedPolicySha256: string;
  sources: Record<string, Uint8Array>;
}

export interface AuditResult {
  schema: "pic32mx270-debugger-rehearsal-audit-v1";
  structural_status: "pass";
  startup_status: "unsupported";
  control_flow_status: "unsupported";
  policy_status: "pass";
  application_safety_status: "unsupported";
  installed_executive_status: "unknown";
  delivery_approved: false;
  hardware_approved: false;
  nvm_qualified: false;
  manual_approval: false;
  hashes: { elf: string; hex: string; map: string; manifest: string; policy: string; expected_policy: string };
  policy_provenance: string;
  policy_review_identity: string;
  checks: { sections: number; load_segments: number; hex_bytes: number; dinit_records: number; symbols: string[]; source_files: number };
  limits: string[];
}

function fail(message: string): never { throw new Error(message); }
function check(ok: unknown, message: string): asserts ok { if (!ok) fail(message); }
function hash(bytes: Uint8Array): string { return createHash("sha256").update(bytes).digest("hex"); }
function isObj(x: unknown): x is Record<string, any> { return !!x && typeof x === "object" && !Array.isArray(x); }
function exactKeys(x: Record<string, any>, keys: string[], label: string): void {
  check(Object.keys(x).sort().join("\0") === [...keys].sort().join("\0"), `${label}: missing or extra fields`);
}
function json(bytes: Uint8Array, label: string): any {
  check(bytes.length > 0 && bytes.length <= MAX_FILE, `${label}: size limit`);
  let value: any;
  try { value = JSON.parse(Buffer.from(bytes).toString("utf8")); } catch { fail(`${label}: invalid JSON`); }
  check(isObj(value), `${label}: expected object`);
  return value;
}
function uint(value: unknown, label: string): number {
  check(Number.isSafeInteger(value) && Number(value) >= 0 && Number(value) <= 0xFFFFFFFF, `${label}: expected uint32`);
  return Number(value);
}
function addFits(start: number, size: number, limit = 0x1_0000_0000): boolean {
  return start <= limit && size <= limit - start;
}
function cstr(buf: Buffer, base: number, size: number, offset: number, label: string): string {
  check(offset < size && base <= buf.length && size <= buf.length - base, `${label}: string offset/range`);
  // Bound each scan independently; never search an 8MB file for every name.
  const bytes=buf.subarray(base+offset,base+offset+Math.min(1025,size-offset));
  const end=bytes.indexOf(0);check(end>=0,`${label}: unterminated/oversized string`);
  try { return new TextDecoder("utf-8", { fatal: true }).decode(bytes.subarray(0,end)); }
  catch { return fail(`${label}: invalid UTF-8`); }
}
function physical(addr: number): number {
  check(addr >= 0 && addr <= 0xFFFFFFFF, "address outside 32-bit range");
  if (addr < 0x80000000) return addr;
  check(addr < 0xC0000000, "unsupported KSEG2/unknown address alias");
  return addr & 0x1FFFFFFF;
}
function domain(vaddr: number, size: number, label: string): "program" | "boot" | "config" | "ram" {
  check(size > 0 && addFits(vaddr, size), `${label}: empty/overflowing range`);
  if (vaddr >= RAM_START && vaddr < RAM_END) {
    check(size <= RAM_END - vaddr, `${label}: SRAM range`);
    return "ram";
  }
  check(vaddr >= 0x80000000 && vaddr < 0xC0000000, `${label}: require direct-mapped KSEG0/KSEG1`);
  const p = physical(vaddr);
  if (p >= PROGRAM_START && p < PROGRAM_END) {
    check(size <= PROGRAM_END - p, `${label}: outside fixed 32KB program prefix`);
    return "program";
  }
  if (p >= BOOT_START && p < BOOT_END) {
    check(size <= BOOT_END - p, `${label}: outside device Boot Flash`);
    return "boot";
  }
  if (p >= CONFIG_START && p < CONFIG_END) {
    check(size <= CONFIG_END - p, `${label}: outside configuration region`);
    return "config";
  }
  fail(`${label}: outside allowed device memory domains`);
}
function parsePolicy(bytes: Uint8Array, manifestHash: string, expectedHash: string): any {
  const actual = hash(bytes);
  check(/^[0-9a-f]{64}$/.test(expectedHash) && actual === expectedHash.toLowerCase(), "policy hash does not match independently supplied pin");
  const p = json(bytes, "policy");
  exactKeys(p, ["schema", "source_manifest_sha256", "provenance", "review_identity", "allowed_boot_flash_spans", "reservations", "context_symbol", "context_size", "mailbox_symbol"], "policy");
  check(p.schema === POLICY_SCHEMA, "unsupported policy schema");
  check(typeof p.provenance === "string" && p.provenance.length > 0 && p.provenance.length <= 512, "policy provenance missing/boundedness");
  check(typeof p.review_identity === "string" && p.review_identity.length > 0 && p.review_identity.length <= 256, "policy review identity missing/boundedness");
  check(p.source_manifest_sha256 === manifestHash, "policy source-manifest binding mismatch");
  check(p.context_symbol === "rehearsal_context" && p.mailbox_symbol === "rehearsal_mailbox", "unsupported policy object names");
  check(Number.isInteger(p.context_size) && p.context_size > 0 && p.context_size <= 4096, "invalid policy context size");
  check(Array.isArray(p.allowed_boot_flash_spans) && p.allowed_boot_flash_spans.length > 0 && p.allowed_boot_flash_spans.length <= 64, "policy must explicitly list bounded Boot Flash spans");
  let prev = BOOT_START;
  for (const s of p.allowed_boot_flash_spans) {
    check(isObj(s), "malformed Boot Flash span"); exactKeys(s, ["start", "end"], "Boot Flash span");
    const start = uint(s.start, "Boot Flash start"), end = uint(s.end, "Boot Flash end");
    check(start >= prev && end > start && end <= BOOT_END, "Boot Flash span outside device domain or overlapping");
    check(end <= 0x1FC00480 || start >= 0x1FC00BF0, "policy span includes reserved debugger executive/vector tail"); prev = end;
  }
  check(Array.isArray(p.reservations) && p.reservations.length >= 3 && p.reservations.length <= 128, "policy reservations missing/boundedness");
  const names = new Set<string>(), kinds = new Set<string>(); let reservations: [number, number, string][] = [];
  for (const r of p.reservations) {
    check(isObj(r), "malformed policy reservation"); exactKeys(r, ["name", "start", "end", "kind"], "reservation");
    check(typeof r.name === "string" && r.name.length > 0 && r.name.length<=256 && !names.has(r.name), "duplicate/invalid reservation name"); names.add(r.name);
    check(["stack", "heap", "debug", "noinit"].includes(r.kind), "unsupported reservation kind"); kinds.add(r.kind);
    const start = uint(r.start, "reservation start"), end = uint(r.end, "reservation end");
    check(end > start, "empty reservation");
    const d = domain(start, end - start, `reservation ${r.name}`);
    check(d === "ram" || d === "boot", "reservation outside SRAM/Boot Flash");
    reservations.push([start, end, r.name]);
  }
  check(["stack", "heap", "debug"].every(k => kinds.has(k)), "explicit stack, heap and debugger reservations required");
  const canonicalReservations=reservations.map(([start,end,name])=>{const d=domain(start,end-start,`reservation ${name}`);return [d==="ram"?start:physical(start),d==="ram"?end:physical(start)+(end-start),d,name] as [number,number,string,string]}).sort((a,b)=>a[2].localeCompare(b[2])||a[0]-b[0]);
  for(let i=0;i<canonicalReservations.length;i++)for(let j=i+1;j<canonicalReservations.length&&canonicalReservations[j][2]===canonicalReservations[i][2];j++)check(canonicalReservations[j][0]>=canonicalReservations[i][1],"policy reservations overlap/alias");
  return p;
}

export interface Section { i:number; name:string; type:number; flags:number; addr:number; off:number; size:number; link:number; info:number; align:number; entsize:number; }
export interface Sym { name:string; value:number; size:number; bind:number; type:number; shndx:number; }
export interface ElfInfo { buf:Buffer; sections:Section[]; symbols:Sym[]; loads:{off:number; vaddr:number; paddr:number; filesz:number; memsz:number; flags:number}[]; }
export interface NativeGap { start:number; end:number; kind:'zero-file'|'no-file'; }
export interface NativeFormatOptions { gaps:NativeGap[]; }
/** Strict existing format unless native options are explicitly supplied by the
 * separately pinned native reader. It never changes safety/approval status. */
export function parseElf(bytes: Uint8Array,native?:NativeFormatOptions): ElfInfo {
  check(bytes.length >= 52 && bytes.length <= MAX_FILE, "ELF size/header limit");
  const b=Buffer.from(bytes);
  check(b.subarray(0,7).equals(Buffer.from([0x7f,0x45,0x4c,0x46,1,1,1])), "expected ELF32 little-endian version 1");
  check(b.readUInt16LE(16)===2 && b.readUInt16LE(18)===8 && b.readUInt32LE(20)===1, "expected ET_EXEC MIPS ELF version 1");
  check(b.readUInt16LE(40)===52, "ELF header size");
  const entry=b.readUInt32LE(24), eflags=b.readUInt32LE(36);
  check(eflags===0x70001001, "unsupported MIPS ISA/ABI flags; require MIPS32r2 o32 noreorder");
  const phoff=b.readUInt32LE(28), shoff=b.readUInt32LE(32), phents=b.readUInt16LE(42), phnum=b.readUInt16LE(44), shents=b.readUInt16LE(46), shnum=b.readUInt16LE(48), shstr=b.readUInt16LE(50);
  check(phnum>0 && phnum<=MAX_TABLE && phents>=32 && phoff<=b.length && phnum<=Math.floor((b.length-phoff)/phents), "program-header table bounds/count");
  check(shnum>0 && shnum<=MAX_TABLE && shents>=40 && shoff<=b.length && shnum<=Math.floor((b.length-shoff)/shents), "section-header table bounds/count");
  check(shstr>0 && shstr<shnum, "extended/missing section-name table unsupported");
  const raw: Section[]=[];
  for(let i=0;i<shnum;i++){
    const x=shoff+i*shents, type=b.readUInt32LE(x+4), flags=b.readUInt32LE(x+8), addr=b.readUInt32LE(x+12), off=b.readUInt32LE(x+16), size=b.readUInt32LE(x+20), link=b.readUInt32LE(x+24), info=b.readUInt32LE(x+28), align=b.readUInt32LE(x+32), entsize=b.readUInt32LE(x+36);
    check(size<=MAX_SECTION, "section exceeds 1MB bound");
    check(align===0||(align>0&&(align&(align-1))===0),"section alignment invalid");
    if(size&&align>1)check(addr%align===0&&(type===8||off%align===0),"section alignment mismatch");
    if(type!==8) check(off<=b.length && size<=b.length-off, "section file range out of bounds");
    else check(off<=b.length, "NOBITS offset out of bounds");
    raw.push({i,name:"",type,flags,addr,off,size,link,info,align,entsize});
  }
  check(b.subarray(shoff,shoff+40).every(x=>x===0),"ELF null section header must be zero");
  const names=raw[shstr]; check(names.type===3 && names.size>0, "section-name table must be STRTAB");
  const used=new Set<string>();
  for(const s of raw){
    s.name=cstr(b,names.off,names.size,b.readUInt32LE(shoff+s.i*shents),"section name");
    if(s.i!==0)check(s.name.length>0,"non-null section needs a name");
    if(s.name){check(native||!used.has(s.name), `duplicate section name ${s.name}`); used.add(s.name);}
    if(native&&s.size>0)check(!raw.slice(0,s.i).some(p=>p.name===s.name&&p.addr===s.addr&&p.size>0),"duplicate native section identity");
  }
  const loads:any[]=[];
  for(let i=0;i<phnum;i++){
    const x=phoff+i*phents, type=b.readUInt32LE(x); if(type!==1) continue;
    const off=b.readUInt32LE(x+4), vaddr=b.readUInt32LE(x+8), paddr=b.readUInt32LE(x+12), filesz=b.readUInt32LE(x+16), memsz=b.readUInt32LE(x+20), flags=b.readUInt32LE(x+24), align=b.readUInt32LE(x+28);
    check(filesz<=memsz && off<=b.length && filesz<=b.length-off && memsz>0 && addFits(vaddr,memsz), "PT_LOAD file/memory bounds");
    check(filesz<=MAX_SECTION && memsz<=MAX_SECTION, "PT_LOAD exceeds 1MB bound");
    const vd=domain(vaddr,memsz,"PT_LOAD virtual span"), vp=physical(vaddr), pp=physical(paddr);
    check(vp===pp, "PT_LOAD virtual/physical alias mismatch");
    if(vd==="ram") check(vaddr>=RAM_START && paddr>=RAM_START && paddr<RAM_END, "SRAM PT_LOAD must use uncached SRAM alias");
    else check(["program","boot","config"].includes(vd), "invalid PT_LOAD domain");
    check(align===0 || (align & (align-1))===0, "PT_LOAD alignment");
    if(align>1)check(vaddr%align===off%align,"PT_LOAD alignment congruence");
    check((flags&~7)===0&&(flags&4)!==0&&!(vd==="ram"&&(flags&1)),"PT_LOAD flags invalid");
    loads.push({off,vaddr,paddr,filesz,memsz,flags});
  }
  check(loads.length>0, "no PT_LOAD segments");
  if(native){const fileRanges=loads.filter(p=>p.filesz>0).sort((a,b)=>a.off-b.off);
    for(let i=1;i<fileRanges.length;i++)check(fileRanges[i].off>=fileRanges[i-1].off+fileRanges[i-1].filesz,"native PT_LOAD file ranges overlap/reuse");}
  check(entry%4===0&&loads.some(p=>(p.flags&1)!==0&&entry>=p.vaddr&&entry<p.vaddr+p.filesz),"ELF entry not mapped executable bytes");
  const loadRanges=loads.map((p:any)=>{const d=domain(p.vaddr,p.memsz,"PT_LOAD");return [d==="ram"?p.vaddr:physical(p.vaddr),d==="ram"?p.vaddr+p.memsz:physical(p.vaddr)+p.memsz,d] as [number,number,string]}).sort((a,b)=>a[0]-b[0]);
  for(let i=1;i<loadRanges.length;i++) check(loadRanges[i][0]>=loadRanges[i-1][1]||loadRanges[i][2]!==loadRanges[i-1][2], "overlapping/aliased PT_LOAD memory ranges");
  const empty=raw.filter(s=>(s.flags&2)!==0&&!s.size);
  if(native)for(const s of empty){
    const array=(s.type===16&&s.name==='.preinit_array')||(s.type===14&&s.name==='.init_array')||(s.type===15&&s.name==='.fini_array');
    check((s.type===1||s.type===8||array)&&!(s.flags&~0x10200007),"unsupported empty native section");
    check((s.addr>=0x9D000000&&s.addr<=0x9D008000)||(s.addr>=RAM_START&&s.addr<=RAM_END),"empty native section address");
  }
  const alloc=raw.filter(s => (s.flags&2)!==0&&(!native||s.size>0));
  check(alloc.reduce((n,s)=>n+s.size,0)<=MAX_SECTION,"total allocated size limit");
  // Strict initial subset: every segment byte must be explained by exactly
  // mapped sections. Alignment holes/loader-only padding are unsupported,
  // never accepted by a global file-offset set from another segment.
  const usedGaps=new Set<NativeGap>();
  for(const ph of loads){
    const mapped=alloc.filter(s=>s.addr>=ph.vaddr&&s.size<=ph.memsz-(s.addr-ph.vaddr)&&
      ((s.type===8&&s.addr>=ph.vaddr+ph.filesz)||
       ((s.type===1||(native&&s.type===0x7000002A))&&s.size<=ph.filesz-(s.addr-ph.vaddr)&&s.off===ph.off+s.addr-ph.vaddr)));
    check(mapped.length>0,native?"PT_LOAD must own mapped sections, not gaps alone":"PT_LOAD has unexplained memory/file range or mapping");
    if(ph.filesz)check(mapped.some(s=>s.type!==8),"file segment lacks mapped file section");
    const sorted=[...mapped].sort((a,b)=>a.addr-b.addr);let cursor=ph.vaddr;
    const gap=(start:number,end:number)=>{
      if(start===end)return;
      check(native,"PT_LOAD has unexplained memory/file range or mapping");
      const g=native.gaps.filter(g=>g.start===start&&g.end===end);
      check(g.length===1&&!usedGaps.has(g[0]),"unapproved/ambiguous native segment gap");
      const cut=ph.vaddr+ph.filesz;
      check((g[0].kind==='zero-file'&&end<=cut)||(g[0].kind==='no-file'&&start>=cut),"native gap crosses file boundary");
      if(g[0].kind==='zero-file')check(b.subarray(ph.off+start-ph.vaddr,ph.off+end-ph.vaddr).every(x=>x===0),"nonzero native segment gap bytes");
      usedGaps.add(g[0]);
    };
    for(const s of sorted){check(s.addr>=cursor,"PT_LOAD overlapping mapped sections");gap(cursor,s.addr);cursor=s.addr+s.size;}
    gap(cursor,ph.vaddr+ph.memsz);
  }
  if(native)check(usedGaps.size===native.gaps.length,"unused native gap declaration");
  for(const s of alloc){
    const abi=native&&s.type===0x7000002A&&s.name==='.MIPS.abiflags';
    check(s.size>0 && (s.type===1 || s.type===8 || abi), `unsupported allocated section ${s.name}`);
    const d=domain(s.addr,s.size,`allocated section ${s.name}`);
    const debug=native&&s.type===8&&((s.name==='.dbg_excpt'&&s.addr===0xBFC00480&&s.size===8)||
      (s.name==='.dbg_code'&&s.addr===0x9FC00490&&s.size===0x760)||(s.name==='.dbg_data'&&s.addr===RAM_START&&s.size===512));
    if(s.name.startsWith('.dbg'))check(debug,"invalid native debug reservation");
    if(d==="ram") check(s.addr>=RAM_START && s.addr<RAM_END && s.type===8, `SRAM section ${s.name} must be uncached NOBITS`);
    else check(s.type===1||abi||debug, `Flash section ${s.name} must be file-backed PROGBITS`);
    check((s.flags&4)===0 || d!=="ram", "executable SRAM section unsupported");
    const nativeBss=native&&d==='ram'&&s.type===8&&s.name.startsWith('.bss.');
    check((s.flags&~(nativeBss?0x200007:7))===0&&!(s.flags&1&&s.flags&4),"unsupported allocated section flags");
    check(!(d==="ram"&&s.type===8&&(s.flags&1)===0),"RAM allocation must be writable");
    check((d==="ram")||!(s.flags&1)||debug,"writable Flash section unsupported");
    if(abi)check(s.flags===2&&s.align===8&&s.size===24&&d==='program'&&
      b.subarray(s.off,s.off+s.size).toString('hex')==='0000200201000003000000000b0000000100000000000000',"unsupported native MIPS ABI profile");
  }
  const ranges=alloc.map(s=>[s.addr,s.addr+s.size,s.name] as [number,number,string]).sort((a,b)=>a[0]-b[0]);
  for(let i=1;i<ranges.length;i++) check(ranges[i][0]>=ranges[i-1][1], `allocated section overlap ${ranges[i-1][2]}/${ranges[i][2]}`);
  const canonical=alloc.map(s=>{const d=domain(s.addr,s.size,s.name);return [d==="ram"?s.addr:physical(s.addr),d==="ram"?s.addr+s.size:physical(s.addr)+s.size,d,s.name] as [number,number,string,string]}).sort((a,b)=>a[2].localeCompare(b[2])||a[0]-b[0]);
  for(let i=0;i<canonical.length;i++)for(let j=i+1;j<canonical.length&&canonical[j][2]===canonical[i][2];j++)check(canonical[j][0]>=canonical[i][1],`allocated section physical alias overlap ${canonical[i][3]}/${canonical[j][3]}`);
  const symtabs=raw.filter(s=>s.type===2);
  check(symtabs.length===1, "require exactly one bounded SYMTAB");
  const st=symtabs[0]; check(st.entsize===16 && st.size>=16 && st.size%16===0 && st.size/16<=(native?4096:MAX_TABLE) && st.link<raw.length, "invalid ELF32 symbol table layout/count");
  const str=raw[st.link]; check(str.type===3 && str.size>0, "symbol string table must be STRTAB");
  const syms:Sym[]=[];
  for(let o=0;o<st.size;o+=16){
    const x=st.off+o, nameOff=b.readUInt32LE(x), value=b.readUInt32LE(x+4), size=b.readUInt32LE(x+8), info=b[x+12], shndx=b.readUInt16LE(x+14);
    const name=cstr(b,str.off,str.size,nameOff,"symbol name");
    check(shndx===0 || shndx===0xFFF1 || shndx<raw.length, "symbol section index out of range");
    syms.push({name,value,size,bind:info>>4,type:info&15,shndx});
  }
  return {buf:b,sections:raw,symbols:syms,loads};
}
export function parseHex(bytes: Uint8Array, policy:any): Map<number,number> {
  check(bytes.length>0 && bytes.length<=MAX_FILE, "HEX size limit");
  const text=Buffer.from(bytes).toString("ascii"); check(Buffer.from(text,"ascii").equals(Buffer.from(bytes)), "HEX must be ASCII");
  const normalized=text.replace(/\r\n/g,"\n"); const lines=(normalized.endsWith("\n")?normalized.slice(0,-1):normalized).split("\n");
  const mem=new Map<number,number>(); let upper=0,eof=false;
  for(let lineNo=0;lineNo<lines.length;lineNo++){
    const line=lines[lineNo]; check(line.length>0, "blank/hidden HEX line"); check(!eof, "HEX data/trailing record after EOF");
    check(/^:[0-9a-fA-F]+$/.test(line) && line.length%2===1, `HEX syntax line ${lineNo+1}`);
    const r=Buffer.from(line.slice(1),"hex"), n=r[0]; check(r.length===n+5, "HEX byte count");
    let sum=0; for(const x of r) sum=(sum+x)&255; check(sum===0, "HEX checksum");
    const off=r.readUInt16BE(1), kind=r[3];
    if(kind===4){check(n===2&&off===0, "unsupported/malformed HEX extension"); upper=r.readUInt16BE(4)*65536; continue;}
    if(kind===1){check(n===0&&off===0, "malformed HEX EOF"); eof=true; continue;}
    check(kind===0 && n>0 && off+n<=65536, "unsupported HEX record or span");
    const a=upper+off; check(addFits(a,n), "HEX address overflow");
    for(let i=0;i<n;i++){
      const at=a+i, value=r[4+i];
      const ok=(at>=PROGRAM_START&&at<PROGRAM_END)||(at>=CONFIG_START&&at<CONFIG_END)||policy.allowed_boot_flash_spans.some((s:any)=>at>=s.start&&at<s.end);
      check(ok, `HEX byte outside approved program/Boot Flash domains: ${at.toString(16)}`);
      check(!mem.has(at), "duplicate/overlapping HEX byte"); mem.set(at,value);
    }
    check(mem.size<=MAX_HEX_BYTES, "HEX delivered-byte/read exceeds 1MB");
  }
  check(eof&&mem.size>0, "HEX missing EOF/data"); return mem;
}
function symbol(info:ElfInfo,name:string,type:number,policy?:any):{sym:Sym;section:Section}{
  const found=info.symbols.filter(s=>s.name===name);
  check(found.length===1, `required symbol missing/ambiguous: ${name}`);
  const s=found[0]; check(s.shndx!==0 && s.shndx<info.sections.length && s.type===type && s.size>0, `undefined/wrong-type/zero-size symbol ${name}`);
  const sec=info.sections[s.shndx]; check((sec.flags&2)!==0 && s.value>=sec.addr && s.size<=sec.size-(s.value-sec.addr), `symbol outside allocated section: ${name}`);
  if(type===2){
    check((sec.flags&4)!==0 && (sec.flags&1)===0 && s.value%4===0 && domain(s.value,s.size,name)!=="ram", `code symbol placement invalid: ${name}`);
  }
  return {sym:s,section:sec};
}
function audit(input:AuditInput):AuditResult {
  for(const [n,b] of Object.entries({elf:input.elf,hex:input.hex,map:input.map,manifest:input.manifest,policy:input.policy})) check(b.length>0&&b.length<=MAX_FILE, `${n} file size limit`);
  const manifestHash=hash(input.manifest), policy=parsePolicy(input.policy,manifestHash,input.expectedPolicySha256);
  const m=json(input.manifest,"source manifest");
  exactKeys(m,["schema","elf_sha256","hex_sha256","map_sha256","sources"],"source manifest");
  check(m.schema===MANIFEST_SCHEMA, "unsupported source-manifest schema");
  check(m.elf_sha256===hash(input.elf)&&m.hex_sha256===hash(input.hex)&&m.map_sha256===hash(input.map), "manifest ELF/HEX/map hash binding mismatch");
  check(Array.isArray(m.sources)&&m.sources.length>0&&m.sources.length<=256, "manifest source list missing/boundedness");
  let srcTotal=0; const sourceNames=new Set<string>(); const sourceList:any[]=[];
  for(const s of m.sources){
    check(isObj(s), "malformed manifest source"); exactKeys(s,["path","sha256"],"manifest source");
    check(typeof s.path==="string" && s.path.length>0 && s.path.length<=1024 && !s.path.startsWith("/") && !s.path.includes("\\") && !s.path.split("/").includes("..") && !sourceNames.has(s.path), "unsafe/duplicate manifest source path");
    sourceNames.add(s.path); const data=input.sources[s.path]; check(Object.hasOwn(input.sources,s.path)&&data instanceof Uint8Array, `missing/invalid manifest source bytes: ${s.path}`);
    srcTotal+=data.length; check(srcTotal<=MAX_FILE && data.length<=MAX_FILE, "source read limit");
    check(typeof s.sha256==="string"&&/^[0-9a-f]{64}$/.test(s.sha256)&&hash(data)===s.sha256, `source hash mismatch: ${s.path}`);
    sourceList.push({path:s.path,sha256:s.sha256});
  }
  check(Object.keys(input.sources).length===sourceNames.size&&Object.keys(input.sources).every(x=>sourceNames.has(x)), "extra unmanifested source bytes");
  const map=json(input.map,"normalized map");
  exactKeys(map,["schema","elf_sha256","sources","allocations","reservations"],"normalized map");
  check(map.schema===MAP_SCHEMA&&map.elf_sha256===hash(input.elf), "normalized map schema/ELF binding mismatch");
  check(JSON.stringify(map.sources)===JSON.stringify(sourceList), "map/source-manifest source binding mismatch");
  check(Array.isArray(map.allocations)&&map.allocations.length<=MAX_TABLE, "map allocation inventory malformed");
  check(Array.isArray(map.reservations)&&map.reservations.length<=128, "map reservation inventory malformed");
  check(JSON.stringify(map.reservations)===JSON.stringify(policy.reservations), "map reservations differ from pinned policy");
  const elf=parseElf(input.elf), hex=parseHex(input.hex,policy);
  const alloc=elf.sections.filter(s=>(s.flags&2)!==0);
  const inventory=alloc.map(s=>({name:s.name,start:s.addr,end:s.addr+s.size,kind:s.type===8?"nobits":"progbits"}));
  const sortedInv=[...inventory].sort((a,b)=>a.name.localeCompare(b.name));
  const sortedMap=[...map.allocations].sort((a:any,b:any)=>String(a.name).localeCompare(String(b.name)));
  check(JSON.stringify(sortedMap)===JSON.stringify(sortedInv), "normalized map allocated-section inventory differs from ELF");
  // Every file-backed allocated Flash section is mapped exactly through one PT_LOAD.
  const expected=new Map<number,number>();
  for(const s of alloc){
    const d=domain(s.addr,s.size,`section ${s.name}`), p=physical(s.addr);
    if(d==="boot") check(policy.allowed_boot_flash_spans.some((r:any)=>p>=r.start&&p+s.size<=r.end), `Boot Flash section outside pinned span: ${s.name}`);
    if(d==="config") check(s.name===".config"&&p===CONFIG_START&&s.size===16, "configuration must be exact 16-byte .config at fixed address");
    if(d==="program"||d==="boot"||d==="config"){
      let matched=false;
      for(const ph of elf.loads){
        if(physical(ph.vaddr)!==physical(ph.paddr)) continue;
        if(s.addr<ph.vaddr || s.addr+s.size>ph.vaddr+ph.filesz) continue;
        const expectedOff=ph.off+(s.addr-ph.vaddr);
        check(s.off===expectedOff, `section file/segment mapping mismatch: ${s.name}`);
        check(((s.flags&4)===0||(ph.flags&1)!==0)&&((s.flags&1)===0||(ph.flags&2)!==0), `PT_LOAD permissions do not cover section ${s.name}`); matched=true; break;
      }
      check(matched, `allocated Flash section not file-mapped by PT_LOAD: ${s.name}`);
      for(let i=0;i<s.size;i++){const at=p+i;check(!expected.has(at), "allocated ELF Flash section overlap");expected.set(at,elf.buf[s.off+i]);}
    } else {
      check(s.addr>=RAM_START && s.addr+s.size<=RAM_END, "SRAM section alias/range");
      const seg=elf.loads.find(ph=>ph.vaddr<=s.addr&&s.addr+s.size<=ph.vaddr+ph.memsz);
      check(!!seg && seg.filesz===0 && physical(seg.paddr)===physical(seg.vaddr), `NOBITS SRAM section missing memory-only PT_LOAD: ${s.name}`);
    }
  }
  check(expected.size>0&&expected.size===hex.size, "HEX must exactly cover allocated ELF Flash bytes");
  for(const [at,value] of expected) check(hex.get(at)===value, `ELF/HEX byte mismatch at ${at.toString(16)}`);
  const cfg=elf.sections.filter(s=>s.name===".config"); check(cfg.length===1, "exactly one configuration section required");
  const cfgBytes=Buffer.alloc(16);
  for(let i=0;i<16;i++){const at=CONFIG_START+i;const v=hex.get(at);check(v!==undefined, "missing configuration byte");cfgBytes[i]=v;}
  for(let i=0;i<4;i++) check(cfgBytes.readUInt32LE(i*4)===CFG_WORDS[i], `configuration word ${i} mismatch`);
  const named=elf.sections.filter(s=>s.name===".dinit"); check(named.length===1, "exactly one .dinit section required");
  const dsec=named[0]; check(dsec.type===1&&(dsec.flags&2)!==0&&dsec.size>=16&&dsec.size%4===0&&domain(dsec.addr,dsec.size,".dinit")==="program", ".dinit must be allocated file-backed program Flash PROGBITS");
  const dSymbols=elf.symbols.filter(s=>s.name==="_dinit_addr");
  check(dSymbols.length===1&&dSymbols[0].value===dsec.addr&&
    (dSymbols[0].shndx===0xFFF1||dSymbols[0].shndx===dsec.i),"_dinit_addr must be uniquely defined at .dinit (absolute linker symbol allowed)");
  const main=symbol(elf,"main",2), reset=symbol(elf,"_reset",2), pre=symbol(elf,"RAM_RehearsalPreEdit",2), readout=symbol(elf,"RAM_RehearsalReadout",2);
  const codeRanges=[main.sym,reset.sym,pre.sym,readout.sym];
  for(let i=0;i<codeRanges.length;i++)for(let j=i+1;j<codeRanges.length;j++){
    const a=codeRanges[i],b=codeRanges[j];
    check(physical(a.value)+a.size<=physical(b.value)||physical(b.value)+b.size<=physical(a.value),"required code symbols overlap/alias");
  }
  check(domain(main.sym.value,main.sym.size,"main")==="program", "main must lie in fixed program prefix");
  check(domain(reset.sym.value,reset.sym.size,"_reset")==="boot"&&physical(reset.sym.value)===BOOT_START,"reset must lie at Boot Flash reset vector");
  check(elf.buf.readUInt32LE(24)===reset.sym.value,"ELF entry differs from reset symbol");
  check(domain(pre.sym.value,pre.sym.size,"pre-edit")==="program"&&domain(readout.sym.value,readout.sym.size,"readout")==="program", "rendezvous functions must lie in program prefix");
  const context=symbol(elf,policy.context_symbol,1), mailbox=symbol(elf,policy.mailbox_symbol,1);
  check(context.sym.size===policy.context_size&&mailbox.sym.size===148, "context/mailbox exact symbol sizes");
  for(const [n,x] of [["context",context], ["mailbox",mailbox]] as const){
    check(x.section.type===8&&x.sym.value>=RAM_START&&x.sym.value<RAM_END&&x.sym.value%4===0, `${n} must be aligned uncached NOBITS SRAM`);
    check(domain(x.sym.value,x.sym.size,n)==="ram", `${n} outside SRAM`);
  }
  check(context.sym.value+context.sym.size<=mailbox.sym.value||mailbox.sym.value+mailbox.sym.size<=context.sym.value, "context/mailbox overlap or alias");
  const objects=[context.sym,mailbox.sym];
  for(const o of objects) for(const r of policy.reservations) check(o.value+o.size<=r.start||o.value>=r.end, `object ${o.name} overlaps reservation ${r.name}`);
  // PIC32/XC32 data-init subset: destination,length,op; op 0 clears, op 1 copies inline.
  const init:{destination:number;length:number;operation:number}[]=[]; let cursor=dsec.off, end=dsec.off+dsec.size, terminated=false;
  while(cursor+4<=end){
    const dst=elf.buf.readUInt32LE(cursor);
    if(dst===0){terminated=true; for(let j=cursor;j<end;j++)check(elf.buf[j]===0,"nonzero .dinit terminator tail"); break;}
    check(cursor+12<=end,"truncated .dinit record");
    const len=elf.buf.readUInt32LE(cursor+4),op=elf.buf.readUInt32LE(cursor+8);
    check(len>0&&len<=MAX_SECTION&&addFits(dst,len), "zero/overflowing .dinit destination length");
    check(domain(dst,len,".dinit destination")==="ram"&&dst>=RAM_START&&dst+len<=RAM_END, ".dinit destination must be uncached SRAM");
    check(op===0||op===1, "unsupported .dinit operation");
    const storage=alloc.filter(s=>s.type===8&&s.addr>=RAM_START&&s.addr<RAM_END).sort((a,b)=>a.addr-b.addr);
    let initialized=dst;
    for(const s of storage){if(s.addr>initialized)break;if(s.addr<=initialized&&s.addr+s.size>initialized)initialized=Math.min(dst+len,s.addr+s.size);}
    check(initialized===dst+len,".dinit destination includes unmapped SRAM storage");
    cursor+=12;
    if(op===1){check(len<=end-cursor,"truncated .dinit copy payload");cursor+=len;}
    const aligned=(cursor+3)&~3; check(aligned<=end,".dinit padding overflow");
    for(let j=cursor;j<aligned;j++)check(elf.buf[j]===0,"nonzero .dinit alignment padding"); cursor=aligned;
    for(const r of policy.reservations) check(dst+len<=r.start||dst>=r.end, `.dinit destination overlaps reserved ${r.name}`);
    for(const x of init) check(dst+len<=x.destination||dst>=x.destination+x.length, "pairwise .dinit destination overlap");
    init.push({destination:dst,length:len,operation:op});
  }
  check(terminated&&init.length>0,".dinit missing supported records/zero termination");
  for(const o of objects) check(init.some(x=>x.operation===0&&o.value>=x.destination&&o.value+o.size<=x.destination+x.length), `.dinit does not zero-initialize ${o.name}`);
  // Canonical addresses detect KSEG0/KSEG1 Boot Flash aliases. A matching
  // named whole NOBITS reservation section is legitimate ownership, not data.
  for(const s of alloc) for(const r of policy.reservations){
    const sd=domain(s.addr,s.size,s.name),rd=domain(r.start,r.end-r.start,r.name);
    const sa=sd==="ram"?s.addr:physical(s.addr),ra=rd==="ram"?r.start:physical(r.start);
    const same=s.type===8&&s.name===r.name&&sa===ra&&s.size===r.end-r.start;
    check(sd!==rd||sa+s.size<=ra||sa>=ra+r.end-r.start||same,`allocated section ${s.name} overlaps reservation ${r.name}`);
  }
  return {
    schema:"pic32mx270-debugger-rehearsal-audit-v1", structural_status:"pass", startup_status:"unsupported", control_flow_status:"unsupported", policy_status:"pass", application_safety_status:"unsupported", installed_executive_status:"unknown", delivery_approved:false, hardware_approved:false, nvm_qualified:false, manual_approval:false,
    hashes:{elf:hash(input.elf),hex:hash(input.hex),map:hash(input.map),manifest:manifestHash,policy:hash(input.policy),expected_policy:input.expectedPolicySha256.toLowerCase()},
    policy_provenance:policy.provenance, policy_review_identity:policy.review_identity,
    checks:{sections:elf.sections.length,load_segments:elf.loads.length,hex_bytes:hex.size,dinit_records:init.length,symbols:["main","_reset","RAM_RehearsalPreEdit","RAM_RehearsalReadout","_dinit_addr",policy.context_symbol,policy.mailbox_symbol],source_files:sourceList.length},
    limits:["Structural ELF32 little-endian MIPS/HEX/storage checks only; no general MIPS or control-flow proof.","Startup and control-flow are always unsupported; application safety is unsupported regardless of candidate-supplied flags.","Map is a normalized allocation inventory tied to ELF/source hashes, not proof of a real XC32 linker map.","Policy SHA-256 pin supports a trusted-operator workflow; policy provenance/review identity are pinned labels, not authentication or independent approval.","Installed debugger executive, delivery, hardware, and NVM qualification remain unknown/false."]
  };
}

/** Audits caller-supplied buffers; throws on every malformed or unsupported input. */
export function auditRehearsalImage(input: AuditInput): AuditResult { return audit(input); }

function cliArgs(argv: string[]): Record<string,string> {
  const out:Record<string,string>={};
  for(let i=0;i<argv.length;i++){
    const k=argv[i]; check(["--elf","--hex","--map","--manifest","--policy","--expected-policy-sha256"].includes(k),`unknown argument ${k}`);
    check(i+1<argv.length&&!argv[i+1].startsWith("--")&&!out[k],`missing/duplicate value for ${k}`); out[k]=argv[++i];
  }
  check(Object.keys(out).length===6,"explicit --elf --hex --map --manifest --policy --expected-policy-sha256 are required; no defaults"); return out;
}
export function readBounded(path:string,limit=MAX_FILE):Buffer {
  const fd=openSync(path,"r");try {
    const st=fstatSync(fd);check(st.isFile()&&st.size>0&&st.size<=limit,`file size limit: ${path}`);
    const b=Buffer.alloc(st.size+1);let n=0;
    while(n<b.length){const got=readSync(fd,b,n,b.length-n,null);if(!got)break;n+=got;}
    check(n===st.size&&fstatSync(fd).size===st.size,`file changed while reading: ${path}`);
    return b.subarray(0,n);
  }finally{closeSync(fd);}
}
export function runAuditCli(argv=process.argv.slice(2)):AuditResult {
  const a=cliArgs(argv), manifestBytes=readBounded(a["--manifest"]), manifest=json(manifestBytes,"source manifest");
  check(Array.isArray(manifest.sources)&&manifest.sources.length>0&&manifest.sources.length<=256,"manifest source count limit");
  // Validate/pin all metadata before opening any manifest-controlled sources.
  exactKeys(manifest,["schema","elf_sha256","hex_sha256","map_sha256","sources"],"source manifest");
  check(manifest.schema===MANIFEST_SCHEMA,"unsupported source-manifest schema");
  const policyBytes=readBounded(a["--policy"]);
  parsePolicy(policyBytes,hash(manifestBytes),a["--expected-policy-sha256"]);
  const sources:Record<string,Uint8Array>=Object.create(null);const base=realpathSync(dirname(resolve(a["--manifest"])));
  let total=0;
  for(const s of manifest.sources){
    check(isObj(s)&&typeof s.path==="string"&&s.path.length>0&&s.path.length<=1024&&!s.path.startsWith("/")&&!s.path.includes("\\")&&!s.path.split("/").includes("..")&&!(s.path in sources),"unsafe/duplicate source path");
    const path=realpathSync(resolve(base,s.path));check(path.startsWith(base+sep),"source path escapes manifest directory");
    check(total<MAX_FILE,"aggregate source limit");const bytes=readBounded(path,MAX_FILE-total);total+=bytes.length;sources[s.path]=bytes;
  }
  return auditRehearsalImage({elf:readBounded(a["--elf"]),hex:readBounded(a["--hex"]),map:readBounded(a["--map"]),manifest:manifestBytes,policy:policyBytes,expectedPolicySha256:a["--expected-policy-sha256"],sources});
}
if (import.meta.main) {
  try { console.log(JSON.stringify(runAuditCli(),null,2)); }
  catch(error){ console.error(`unsupported: ${error instanceof Error?error.message:String(error)}`); process.exitCode=1; }
}
