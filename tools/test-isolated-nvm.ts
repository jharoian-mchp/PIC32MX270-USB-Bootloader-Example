#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Test inactive isolated NVM owner with exact-source snapshot interleavings and mock flash","keywords":["nvm","isolated","arming","snapshot","timeout"],"kind":"mixed","weight":"standard","role":"entrypoint"}
 */
import {readFileSync,writeFileSync,mkdirSync} from 'node:fs';
import {resolve} from 'node:path';
process.chdir(resolve(import.meta.dir,'..'));
const dir='build/isolated-nvm';mkdirSync(dir,{recursive:true});
try {
 const source=readFileSync('bootloader/src/isolated_nvm.c','utf8');
 const read='#define COPY(name) copy.name=m->report.name';
 const publish='o->mailbox->sequence=sequence+1U;Order();';
 if(source.split(read).length!==2||source.split(publish).length!==2)throw Error('snapshot extraction drift');
 // Insert deterministic callbacks ONLY in the test copy: cut a snapshot after
 // each field and assert readers reject every odd publication. No production hook.
 writeFileSync(dir+'/isolated-body.inc',source.replace(read,
  '#define COPY(name) do { copy.name=m->report.name;SnapshotCut(m); } while(0)').replace(publish,publish+'PublishCut(o);'));
 const cc=(process.env.HOST_CC??'cc').trim().split(/\s+/);
 const args=[...cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-Ibootloader/src','-I'+dir,
  'bootloader/src/update_flash.c','tests/isolated_nvm_test.c','-o',dir+'/test'];
 const c=Bun.spawnSync(args,{stdout:'inherit',stderr:'inherit'});if(c.exitCode)throw Error('compile failed');
 const r=Bun.spawnSync([dir+'/test'],{stdout:'inherit',stderr:'inherit'});if(r.exitCode)throw Error('test failed');
 // Ensure the production TU itself compiles without injected test instrumentation.
 const p=Bun.spawnSync([...cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-Ibootloader/src','-c',
  'bootloader/src/isolated_nvm.c','-o',dir+'/owner.o'],{stdout:'inherit',stderr:'inherit'});
 if(p.exitCode)throw Error('production TU compile failed');
 const integration=Bun.spawnSync([...cc,'-std=c11','-O2','-Wall','-Wextra','-Werror',
  '-Itests/nvm_mocks','-Ibootloader/src','bootloader/src/isolated_nvm.c','bootloader/src/update_flash.c',
  'bootloader/src/update_nvm.c','tests/isolated_nvm_adapter_test.c','-o',dir+'/adapter-test'],{stdout:'inherit',stderr:'inherit'});
 if(integration.exitCode)throw Error('adapter integration compile failed');
 const actual=Bun.spawnSync([dir+'/adapter-test'],{stdout:'inherit',stderr:'inherit'});
 if(actual.exitCode)throw Error('adapter integration test failed');
} catch(e){console.error(String(e));process.exit(1);}
