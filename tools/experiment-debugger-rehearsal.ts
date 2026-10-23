#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"One explicitly authorized local non-deliverable RAM rehearsal compile/link experiment with retained objects and no retries or board tools","keywords":["rehearsal","experiment","link","local-only"],"kind":"mutating","weight":"standard","role":"entrypoint"}
 */
import{readFileSync,writeFileSync,mkdirSync,openSync,closeSync}from'node:fs';import{resolve}from'node:path';
process.chdir(resolve(import.meta.dir,'..'));
const flag='--approved-local-experiment-20261023';
if(process.argv.length!==3||process.argv[2]!==flag)throw Error('requires exact scoped local-experiment approval flag; no defaults');
const dir='build/debugger-rehearsal-experiment-20261023';mkdirSync(dir,{recursive:true});
// Durable attempt marker is written BEFORE any compiler runs. Never retry or
// overwrite that directory automatically, even if compile/link fails.
const attempt=dir+'/attempt.json';const fd=openSync(attempt,'wx');closeSync(fd);
const root='/workspace/ChrisK/tools/xc32/v6.00',pack='/workspace/ChrisK/tools/packs/Microchip/PIC32MX_DFP/1.7.380';
const sources=['rehearsal/src/main.c','rehearsal/src/configuration.c','rehearsal/src/debugger_rehearsal.c','rehearsal/src/exception_stops.S'];
const inputs=[...sources,'rehearsal/src/debugger_rehearsal.h','rehearsal/src/debugger_rehearsal_hw.h','rehearsal/src/rehearsal_entry_hw.h','rehearsal/linker/ram_rehearsal.ld'];
const sha=(b:Uint8Array)=>new Bun.CryptoHasher('sha256').update(b).digest('hex');
// The flag/record documents trusted operator scope; it is not authentication
// or a way to create new approval after the authorized single attempt.
const record:any={approval:'user web:chrisk-agent 2026-10-23T16:12:47.840Z',scope:'one local non-deliverable compile/link/HEX experiment, no host/board delivery',started:new Date().toISOString(),inputs:inputs.map(path=>({path,sha256:sha(readFileSync(path))})),commands:[],objects:[],link_attempted:false,status:'started',hardware_approved:false,delivery_approved:false};
function save(){writeFileSync(attempt,JSON.stringify(record,null,2)+'\n');}
function run(args:string[],name:string){record.commands.push(args);save();const p=Bun.spawnSync(args,{stdout:'pipe',stderr:'pipe'});writeFileSync(dir+'/'+name+'.log',p.stdout.toString()+p.stderr.toString());if(p.exitCode!==0)throw Error(name+' failed (exit '+p.exitCode+'); stop, no retry');return p.stdout.toString();}
save();
try{
 record.compiler_version=run([root+'/bin/xc32-gcc','--version'],'compiler-version');
 const common=['-mprocessor=32MX270F256D','-mdfp='+pack,'-Os','-g','-mno-mips16','-mdebugger','-fno-common','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Werror','-Irehearsal/src'];
 for(const source of sources){const name=source.split('/').pop()!.replace(/\.[^.]+$/,'');const object=dir+'/'+name+'.o';run([root+'/bin/xc32-gcc',...common,'-MMD','-MF',dir+'/'+name+'.d','-c',source,'-o',object],'compile-'+name);record.objects.push({path:object,sha256:sha(readFileSync(object))});save();}
 record.link_attempted=true;save();
 run([root+'/bin/xc32-gcc',...common,...record.objects.map((o:any)=>o.path),'-Wl,--script=rehearsal/linker/ram_rehearsal.ld,--defsym=_min_stack_size=1024,--defsym=_min_heap_size=512,--gc-sections,-Map='+dir+'/rehearsal.map','-o',dir+'/rehearsal.elf'],'link');
 record.status='linked-not-reviewed';save();
 run([root+'/bin/xc32-bin2hex',dir+'/rehearsal.elf'],'hex');
 for(const name of ['rehearsal.elf','rehearsal.hex','rehearsal.map'])record[name+'_sha256']=sha(readFileSync(dir+'/'+name));
 record.status='artifact-only-unreviewed';record.completed=new Date().toISOString();save();
 console.log('Local unreviewed artifact generated; no safety, executive or delivery approval. Inspect '+attempt);
}catch(e){record.status='stopped';record.error=String(e);record.completed=new Date().toISOString();save();console.error(record.error);process.exitCode=1;}
