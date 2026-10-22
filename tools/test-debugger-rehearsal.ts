#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Test inactive RAM-only debugger rehearsal with mock register reads and deterministic guarded snapshot/rendezvous interleavings","keywords":["debugger","rehearsal","ram","snapshot","count"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import {readFileSync,writeFileSync,mkdirSync} from 'node:fs';
import {resolve} from 'node:path';
process.chdir(resolve(import.meta.dir,'..'));
const dir='build/debugger-rehearsal';mkdirSync(dir,{recursive:true});
function run(args:string[]){const p=Bun.spawnSync(args,{stdout:'inherit',stderr:'inherit'});if(p.exitCode!==0)throw Error('failed: '+args.join(' '));}
try {
 const source=readFileSync('rehearsal/src/debugger_rehearsal.c','utf8');
 const cuts=[
  ['#define COPY(n) copy.n=m->report.n','#define COPY(n) do { copy.n=m->report.n;SnapshotCut(m); } while(0)'],
  ['#define COPY(n) copy.before.n=m->report.before.n','#define COPY(n) do { copy.before.n=m->report.before.n;SnapshotCut(m); } while(0)'],
  ['#define COPY(n) copy.after.n=m->report.after.n','#define COPY(n) do { copy.after.n=m->report.after.n;SnapshotCut(m); } while(0)'],
  ['r->mailbox->sequence=seq+1U;Order();','r->mailbox->sequence=seq+1U;Order();PublishCut(r);'],
  ['#define COPY(n) r->mailbox->report.n=r->report.n','#define COPY(n) do { r->mailbox->report.n=r->report.n;PublishCut(r); } while(0)'],
  ['#define COPY(n) r->mailbox->report.before.n=r->report.before.n','#define COPY(n) do { r->mailbox->report.before.n=r->report.before.n;PublishCut(r); } while(0)'],
  ['#define COPY(n) r->mailbox->report.after.n=r->report.after.n','#define COPY(n) do { r->mailbox->report.after.n=r->report.after.n;PublishCut(r); } while(0)'],
 ];
 let body=source;
 for(const [from,to] of cuts){if(body.split(from).length!==2)throw Error('rehearsal extraction drift');body=body.replace(from,to);}
 writeFileSync(dir+'/rehearsal-body.inc',body);
 const cc=(process.env.HOST_CC??'cc').trim().split(/\s+/);
 const flags=['-std=c11','-O2','-Wall','-Wextra','-Werror','-Itests/rehearsal_mocks','-Irehearsal/src'];
 // Deliberately no owner, Flash, adapter or NVM source in either command.
 run([...cc,...flags,'-I'+dir,'tests/debugger_rehearsal_test.c','-o',dir+'/test']);
 run([dir+'/test']);
 run([...cc,...flags,'-c','rehearsal/src/debugger_rehearsal.c','-o',dir+'/production.o']);
 const nm=Bun.spawnSync(['nm','-u',dir+'/production.o'],{stdout:'pipe',stderr:'pipe'});
 if(nm.exitCode!==0)throw Error('nm failed');
 const imports=nm.stdout.toString().trim().split('\n').filter(Boolean).map(l=>l.trim().replace(/^U\s+/,''));
 const sanitized=cc.some(x=>x.includes('-fsanitize='));
 const unexpected=imports.filter(x=>x!=='RAM_RehearsalHwRead'&&
  !(sanitized&&/^__(asan|ubsan)_/.test(x)));
 if(unexpected.length)throw Error('unexpected production import: '+unexpected.join(', '));
 console.log('PASS: uninstrumented host source imports only mocked read and requested sanitizer instrumentation; no NVM/backend/update/USB dependency.');
 if(process.env.XC32_ROOT){
  const root=process.env.XC32_ROOT;
  const targetNm=Bun.spawnSync([root+'/bin/xc32-nm','-u',dir+'/rehearsal-xc32.o'],{stdout:'pipe',stderr:'pipe'});
  if(targetNm.exitCode!==0)throw Error('target nm failed');
  const allowed=new Set(['DEVCFG0','DEVCFG1','DEVCFG2','DEVCFG3','DMACON','NVMCON','OSCCON','U1PWRC','memcpy','memset']);
  const bad=targetNm.stdout.toString().trim().split('\n').filter(Boolean).map(l=>l.trim().replace(/^U\s+/,'')).filter(x=>!allowed.has(x));
  if(bad.length)throw Error('unexpected target import: '+bad.join(', '));
  const targetDis=Bun.spawnSync([root+'/bin/xc32-objdump','-dr',dir+'/rehearsal-xc32.o'],{stdout:'pipe',stderr:'pipe'});
  if(targetDis.exitCode!==0)throw Error('target objdump failed');
  const dis=targetDis.stdout.toString();writeFileSync(dir+'/disassembly.txt',dis);
  if(!/\bsync\b/.test(dis)||!/\bmfc0\b/.test(dis)||/\b(sdbbp|mtc0|ei|di)\b/.test(dis))throw Error('unexpected target CP0/trap/barrier code');
  for(const symbol of ['RAM_RehearsalPreEdit','RAM_RehearsalReadout']){
   const block=dis.match(new RegExp('<'+symbol+'>:\\n([\\s\\S]*?)(?:\\n\\n|$)'))?.[1];
   if(!block||!block.includes('jr\tra')||!block.includes('nop')||/\b(jal|sw|mtc0|sdbbp)\b/.test(block))throw Error('inert rendezvous code drift '+symbol);
  }
  console.log('PASS: real XC32 object allowlisted imports, sync/read-only CP0 and inert return/nop rendezvous; object evidence only, no final image.');
 }
} catch(e){console.error(String(e));process.exit(1);}
