import {readFileSync,mkdirSync,writeFileSync} from 'node:fs';
import {resolve,dirname} from 'node:path';
import {spawnSync} from 'node:child_process';
const root=resolve(dirname(new URL(import.meta.url).pathname),'..');
const cc=resolve(process.env.XC32_ROOT??'/workspace/ChrisK/tools/xc32/v6.00','bin/xc32-gcc');
const pack=process.env.PIC32MX_DFP_ROOT??'/workspace/ChrisK/tools/packs/Microchip/PIC32MX_DFP/1.7.380';
const log:string[]=[];
function run(args:string[]){log.push(JSON.stringify([cc,...args]));const r=spawnSync(cc,args,{cwd:root,encoding:'utf8'});log.push(r.stdout??'',r.stderr??'');writeFileSync(resolve(root,'build/customer-demo/build.log'),log.join('\n'));if(r.status!==0)throw new Error('XC32 command failed; see build/customer-demo/build.log');}
mkdirSync(resolve(root,'build/customer-demo'),{recursive:true});
for(const [name,project,opt,ld] of [['bootloader','bootloader.X','-O2','btl.ld'],['application','app.X','-O1','app_32k.ld']]){
 const base=resolve(root,'customer-demo',name),xml=readFileSync(resolve(base,project,'nbproject/configurations.xml'),'utf8');
 const sources=[...new Set([...xml.matchAll(/<itemPath>([^<]+\.c)<\/itemPath>/g)].map(m=>resolve(base,project,m[1])))];
 const out=resolve(root,'build/customer-demo',name);mkdirSync(out,{recursive:true});
 const objects:string[]=[];
 for(const [i,source] of sources.entries()) {const o=resolve(out,`${i}.o`);run(['-mprocessor=32MX270F256D',`-mdfp=${pack}`,opt,'-Wall','-Wextra','-ffunction-sections','-fdata-sections','-I'+resolve(base,'src'),'-I'+resolve(base,'src/config/default'),'-c',source,'-o',o]);objects.push(o);}
 run(['-mprocessor=32MX270F256D',`-mdfp=${pack}`,...objects,'-Wl,--script='+resolve(base,'src/config/default',ld),'-Wl,--defsym=_min_heap_size=512,--gc-sections,-Map='+resolve(out,name+'.map'),'-o',resolve(out,name+'.elf')]);
 const converter=resolve(dirname(cc),'xc32-bin2hex');const r=spawnSync(converter,[resolve(out,name+'.elf')],{cwd:root,encoding:'utf8'});log.push(converter,r.stdout??'',r.stderr??'');writeFileSync(resolve(root,'build/customer-demo/build.log'),log.join('\n'));if(r.status!==0)throw Error('bin2hex failed');
 writeFileSync(resolve(out,'sources.json'),JSON.stringify(sources,null,2));console.log(`PASS: ${name} ${sources.length} source compile/link/HEX`);
}
