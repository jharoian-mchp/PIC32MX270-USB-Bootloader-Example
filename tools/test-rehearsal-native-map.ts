#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Test bounded native XC32 map inventory and dedicated rehearsal linker provenance without linking","keywords":["rehearsal","native-map","linker","inventory"],"kind":"read-only","weight":"standard","role":"entrypoint"}
 */
import{readFileSync}from'node:fs';import{resolve}from'node:path';import{strict as a}from'node:assert';import{readNativeMap}from'./rehearsal-native-map';
process.chdir(resolve(import.meta.dir,'..'));
const fixture=`Discarded input sections\n .fake 0x0 0x999\nMemory Configuration\n\nName Origin Length Attributes\nkseg0_program_mem 0x9d000000 0x8000 xr\nkseg1_data_mem 0xa0000000 0x10000 w!x\nLinker script and memory map\nLOAD build/main.o\n.reset 0xbfc00000 0x10\n .reset 0xbfc00000 0x10 build/main.o\n.long_name\n                0x9d000000 0x20\nLOAD libdummy.a\nOUTPUT(build/test.elf elf32-tradlittlemips)\n`;
const enc=(s:string)=>Buffer.from(s);const p=readNativeMap(enc(fixture));a.equal(p.sections.length,2);
const empty=readNativeMap(enc(fixture+'\n.vector_44\n *(.vector_44)\n.gcc_except_table\n *(.gcc_except_table .gcc_except_table.*)\n.debug\n *.elf(.debug)\n'));a.equal(empty.declarations_without_allocation[0],'.vector_44');a.equal(empty.sections.length,2);
const repeat=readNativeMap(enc(fixture+'\n.reset 0xbfc00000 0x0\n.reset 0xbfc00100 0x8\n'));a.equal(repeat.sections.length,4);a.equal(p.loads.length,2);a.equal(p.delivery_approved,false);
const bad=[fixture.replace('0x8000','0xc000'),fixture.replace('0x20','0x100000001'),fixture.replace('.reset 0xbfc00000 0x10','.reset bad 0x10'),fixture+'\n.reset 0xbfc00000 0x10\n',fixture.replace('0x9d000000 0x20','unexpected'),fixture+'\nMemory Configuration\n',fixture.replace('OUTPUT(build/test.elf elf32-tradlittlemips)','OUTPUT(x otherformat)'),fixture.replace('LOAD build/main.o','LOAD ')];
for(const s of bad)a.throws(()=>readNativeMap(enc(s)));
const source=readFileSync('bootloader/src/config/default/btl.ld','utf8'),link=readFileSync('rehearsal/linker/ram_rehearsal.ld','utf8');
a.equal(new Bun.CryptoHasher('sha256').update(source).digest('hex'),'28ca92bbe829fee84f36211a43d978bf9700f521c9549dae027e36b886928287');
for(const line of source.split('\n').filter(l=>l.includes(': ORIGIN')||l.includes('>kseg')||l.includes('_ADDR')||l.includes('_DBG_CODE_SIZE')))a.ok(link.includes(line.trimEnd()),line);
a.ok(link.includes('ASSERT(SIZEOF(.persist) == 0'));a.ok(link.includes('LENGTH(kseg0_program_mem) == 32768'));
// Explicit optional artifact argument is read-only; no default dependency on a build.
if(process.argv[2]){const real=readNativeMap(readFileSync(process.argv[2]));a.ok(real.sections.length>0);console.log(`PASS: existing native map inventory ${real.sections.length} output rows/${real.loads.length} LOAD entries; safety unsupported.`);}
console.log(`PASS: native-map inventory fixtures/${bad.length} rejection cases and minimal dedicated-linker provenance; no compiler/linker or hardware invoked.`);
