#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Check standalone RAM rehearsal entry with mocks and optional XC32 config/exception object evidence without linking","keywords":["ram","rehearsal","entry","config","exception"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import{resolve}from'node:path';import{mkdirSync}from'node:fs';
process.chdir(resolve(import.meta.dir,'..'));const dir='build/debugger-rehearsal';mkdirSync(dir,{recursive:true});
function run(args:string[]):string{const p=Bun.spawnSync(args,{stdout:'pipe',stderr:'pipe'});if(p.exitCode!==0)throw Error(args.join(' ')+'\n'+p.stderr.toString());return p.stdout.toString();}
try{
 const cc=(process.env.HOST_CC??'cc').trim().split(/\s+/);
 console.log(run([...cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-Itests/rehearsal_mocks','-Irehearsal/src','tests/debugger_rehearsal_entry_test.c','-o',dir+'/entry-test']));
 console.log(run([dir+'/entry-test']).trim());
 if(process.env.XC32_ROOT){
  const root=process.env.XC32_ROOT;const dis=(file:string)=>run([root+'/bin/xc32-objdump','-dr',dir+'/'+file+'-xc32.o']);
  const main=dis('main');
  if((main.match(/\bdi\b/g)??[]).length!==1||!/\behb\b/.test(main)||!/\bsync\b/.test(main)||!main.includes('RAM_RehearsalInit')||!main.includes('RAM_RehearsalTasks')||/\b(ei|sdbbp|mtc0)\b/.test(main))throw Error('main IRQ/entry object drift');
  const imports=run([root+'/bin/xc32-nm','-u',dir+'/main-xc32.o']).trim().split('\n').map(l=>l.trim().replace(/^U\s+/,''));
  if(imports.some(x=>!['RAM_RehearsalInit','RAM_RehearsalTasks'].includes(x)))throw Error('unexpected main import');
  const cfg=run([root+'/bin/xc32-objdump','-s',dir+'/configuration-xc32.o']);
  for(const [suffix,word]of [['BF0','ffffffff'],['BF4','fa7af9ff'],['BF8','5bce74fd'],['BFC','e0ffff7f']]){
   const body=cfg.match(new RegExp('Contents of section \\.config_BFC00'+suffix+':\\n([^\\n]+)'))?.[1];
   if(!body?.includes(word))throw Error('configuration bytes drift '+suffix);
  }
  const exceptions=dis('exception_stops');
  if(/\b(sw|sb|sh|sdbbp|eret|deret|mtc0|jal|jalr)\b/.test(exceptions))throw Error('exception hook has write/call/resume');
  for(const name of ['_on_reset','_on_bootstrap','_nmi_handler','_bootstrap_exception_handler','_general_exception_handler','_general_exception_context','_DefaultInterrupt'])if(!run([root+'/bin/xc32-nm',dir+'/exception_stops-xc32.o']).includes(name))throw Error('missing exception symbol '+name);
  if(!/\bjr\s+ra\b/.test(exceptions)||!/\bb\s+/.test(exceptions))throw Error('exception stop/inert hook drift');
  console.log('PASS: standalone XC32 objects: exact config words, one DI/hazard, inert hooks and stack-independent non-resuming exception stops; no target link or vector proof.');
 }
}catch(e){console.error(String(e));process.exit(1);}
