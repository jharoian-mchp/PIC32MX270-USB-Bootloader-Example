#!/usr/bin/env bun
/** SCRIPT_JDOC
 * {"summary":"Package the exact existing RAM rehearsal ELF and gated proposal with offline verification; no rebuild, link or hardware commands","keywords":["ram","rehearsal","package","delivery","checksums"],"kind":"mutating","weight":"standard","role":"entrypoint"}
 */
import{readFileSync,writeFileSync,mkdirSync,rmSync,cpSync,readdirSync}from'node:fs';import{resolve,dirname}from'node:path';
import{auditNativeRehearsal}from'./rehearsal-native-image';
process.chdir(resolve(import.meta.dir,'..'));
const dir='build/package-ram-rehearsal-cb89e7db',out='/workspace/exports/ram-rehearsal-cb89e7db.zip',artifact='build/debugger-rehearsal-experiment-20261023';
const h=(b:Uint8Array)=>new Bun.CryptoHasher('sha256').update(b).digest('hex');
const expected={elf:'cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a',map:'f6dc317709d4299e431f32f03575f1271f64f4523a8754bdacd0032b7eed4eec',canonical:'f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7',recovery:'8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c'};
function check(x:unknown,m:string):asserts x{if(!x)throw Error(m);}
// The input artifact is never built/linked/copied into any active project.
check(h(readFileSync(artifact+'/rehearsal.elf'))===expected.elf,'candidate ELF identity');check(h(readFileSync(artifact+'/rehearsal.map'))===expected.map,'candidate map identity');check(h(readFileSync('build/restart-check/bootloader.hex'))===expected.canonical,'canonical restoration identity');check(h(readFileSync('build/bootloader/bootloader.hex'))===expected.recovery,'probe recovery identity');
rmSync(dir,{recursive:true,force:true});mkdirSync(dir,{recursive:true});
const copy=(from:string,to:string)=>{mkdirSync(dirname(dir+'/'+to),{recursive:true});cpSync(from,dir+'/'+to);};
copy(artifact+'/rehearsal.elf','ram-rehearsal.elf');copy(artifact+'/rehearsal.map','ram-rehearsal.map');copy('build/restart-check/bootloader.hex','usb-canonical32-restore.hex');copy('build/bootloader/bootloader.hex','usb-probe-recovery.hex');copy('docs/nvm-debugger-package-proposal.md','START-HERE.md');copy(artifact+'/attempt.json','evidence/original-local-attempt.json');copy(artifact+'/linked-input-ram_rehearsal.ld','evidence/exact-linked-ram_rehearsal.ld');
for(const name of ['executable','store','control'])copy('docs/nvm-debugger-'+name+'-ledger.csv','evidence/'+name+'-ledger.csv');
const sourceNames=['main.c','configuration.c','debugger_rehearsal.c','exception_stops.S','debugger_rehearsal.h','debugger_rehearsal_hw.h','rehearsal_entry_hw.h'];for(const name of sourceNames)copy('rehearsal/src/'+name,'source/'+name);
const attempt=JSON.parse(readFileSync(artifact+'/attempt.json','utf8'));
for(const input of attempt.inputs){let path=input.path.endsWith('ram_rehearsal.ld')?artifact+'/linked-input-ram_rehearsal.ld':input.path;check(h(readFileSync(path))===input.sha256,'original input changed '+path);}
const originalCommit='7b5d38b9cc72805fb78606e0239ad38668ad8dbf';
for(const name of sourceNames){const path='rehearsal/src/'+name,record=attempt.inputs.filter((i:any)=>i.path===path);
 check(record.length===1&&h(readFileSync(dir+'/source/'+name))===record[0].sha256,'bundled source lacks recorded hash '+name);
 const historical=Bun.spawnSync(['git','show',originalCommit+':'+path],{stdout:'pipe',stderr:'pipe'});
 check(historical.exitCode===0&&h(historical.stdout)===record[0].sha256,'original source commit mismatch '+name);
}
const gaps:any[]=[];for(let start=0x9d0002a8;start<0x9d000760;start+=32)gaps.push({start,end:start+24,kind:'zero-file'});gaps.push({start:0xbfc0014c,end:0xbfc00380,kind:'zero-file'},{start:0xbfc00390,end:0xbfc00480,kind:'no-file'});
const policy=Buffer.from(JSON.stringify({schema:'pic32mx270-native-format-policy-v1',provenance:'exact local artifact structural format evidence only; not independent safety or board approval',gaps,dinit_tail_hex:'66666666'},null,2)+'\n');const policyHash=h(policy);
const result=auditNativeRehearsal({elf:readFileSync(artifact+'/rehearsal.elf'),hex:readFileSync(artifact+'/rehearsal.hex'),map:readFileSync(artifact+'/rehearsal.map'),policy,expectedPolicySha256:policyHash});
check(result.application_safety_status==='unsupported'&&!result.delivery_approved&&!result.hardware_approved&&!result.nvm_qualified,'audit approval invariants');writeFileSync(dir+'/evidence/native-format-policy.json',policy);writeFileSync(dir+'/evidence/native-structural-result.json',JSON.stringify(result,null,2)+'\n');
const report={schema:'ram-rehearsal-package-evidence-v1',elf_sha256:expected.elf,linked_hex_sha256:h(readFileSync(artifact+'/rehearsal.hex')),native_map_sha256:expected.map,format_policy_sha256:policyHash,application_model:'bounded serialized main/new fixed-lifetime storage; no NVM owner/adapter/page',planning_disposition_commit:'1bac5df348fc7402ad9d5ccc9bad8ca0a23bc59e',known_limitations:['Historical system-header/compiler/runtime input closure incomplete; three temporary compiler objects absent.','Retained CRT sdbbp has conditional normal-path unreachability, not all-fault proof.','Installed executive contents, actual debugger halt/resume and stack use unknown.','Structural pass is not application safety or hardware approval.'],source_input_commit:'7b5d38b9cc72805fb78606e0239ad38668ad8dbf',script_inputs_only_no_rebuild:true,board_delivery_permission:false,ram_challenge_permission:false,nvm_permission:false};writeFileSync(dir+'/evidence/BOUNDARIES.json',JSON.stringify(report,null,2)+'\n');
const ps=String.raw`# Offline file verification only; no network, compiler or device/tool commands.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$Root = [IO.Path]::GetFullPath($PSScriptRoot)
$Manifest = Get-Content -LiteralPath (Join-Path $Root 'PACKAGE-MANIFEST.json') -Raw | ConvertFrom-Json
if ($Manifest.schema -ne 'ram-rehearsal-package-v1' -or $Manifest.candidate_elf_sha256 -ne 'cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a') { throw 'Wrong package/candidate' }
if ($Manifest.scope.programming_authorized -ne $false -or $Manifest.scope.debugger_connection_authorized -ne $false -or $Manifest.scope.target_ram_access_authorized -ne $false -or $Manifest.scope.ram_challenge_authorized -ne $false -or $Manifest.scope.nvm_authorized -ne $false -or $Manifest.scope.recovery_authorized -ne $false) { throw 'Wrong scope: this package is offline delivery only' }
$Expected = @($Manifest.files)
if ($Expected.Count -eq 0 -or @($Expected | Sort-Object -Unique).Count -ne $Expected.Count) { throw 'Manifest inventory invalid' }
foreach ($Name in $Expected) {
  if ($Name -notmatch '^[A-Za-z0-9._/-]+$' -or $Name.StartsWith('/') -or $Name -match '(^|/)\.\.(/|$)' -or $Name -eq 'SHA256SUMS.txt') { throw 'Unsafe manifest filename' }
}
$Listed = @()
foreach ($Line in @(Get-Content -LiteralPath (Join-Path $Root 'SHA256SUMS.txt'))) {
  if ($Line -notmatch '^([a-fA-F0-9]{64})  ([A-Za-z0-9._/-]+)$') { throw 'Malformed checksum row' }
  $Digest = $Matches[1].ToLowerInvariant(); $Name = $Matches[2]
  if ($Expected -notcontains $Name -or $Listed -contains $Name) { throw 'Unexpected/duplicate checksum file' }
  $Listed += $Name
  $Path = Join-Path $Root ($Name.Replace('/', [IO.Path]::DirectorySeparatorChar))
  $Item = Get-Item -LiteralPath $Path -Force
  if ($Item.PSIsContainer -or (($Item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0)) { throw 'Unsafe package entry' }
  if ((Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $Digest) { throw "Checksum mismatch: $Name" }
}
if ($Listed.Count -ne $Expected.Count -or @(Compare-Object ($Expected | Sort-Object) ($Listed | Sort-Object)).Count -ne 0) { throw 'Checksum inventory incomplete' }
$Actual = @()
foreach ($Entry in @(Get-ChildItem -LiteralPath $Root -Recurse -Force)) {
  if (($Entry.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) { throw 'Reparse point in package' }
  if (-not $Entry.PSIsContainer) { $Actual += $Entry.FullName.Substring($Root.Length + 1).Replace('\', '/') }
}
$Inventory = @($Expected) + @('SHA256SUMS.txt')
if ($Actual.Count -ne $Inventory.Count -or @(Compare-Object ($Inventory | Sort-Object) ($Actual | Sort-Object)).Count -ne 0) { throw 'Extra/missing package files' }
if ((Get-FileHash -LiteralPath (Join-Path $Root 'ram-rehearsal.elf') -Algorithm SHA256).Hash.ToLowerInvariant() -ne $Manifest.candidate_elf_sha256) { throw 'Candidate ELF identity mismatch' }
Write-Output "PASS: $($Listed.Count) file hashes and exact package inventory. Offline delivery only; no board action authorized."
`;
writeFileSync(dir+'/VERIFY-PACKAGE.ps1',ps);
function files(root:string,prefix=''):string[]{return readdirSync(root,{withFileTypes:true}).flatMap(e=>e.isDirectory()?files(root+'/'+e.name,prefix+e.name+'/'):[prefix+e.name]).sort();}
const listed=[...files(dir),'PACKAGE-MANIFEST.json'].sort();const manifest={schema:'ram-rehearsal-package-v1',package_name:'ram-rehearsal-cb89e7db',delivery_approval:'user web:chrisk-agent 2026-10-23T22:29:07.016Z',candidate_elf_sha256:expected.elf,candidate_native_map_sha256:expected.map,original_source_input_commit:report.source_input_commit,planning_disposition_commit:report.planning_disposition_commit,proposal_sha256:h(readFileSync(dir+'/START-HERE.md')),target:'PIC32MX270F256D',toolchain:'XC32v6.00/DFP1.7.380',program_reservation_bytes:32768,application_origin:'0x1D008000',config_physical_order:['FFFFFFFF','FFF97AFA','FD74CE5B','7FFFFFE0'],scope:{file_delivery_authorized:true,programming_authorized:false,debugger_connection_authorized:false,target_ram_access_authorized:false,ram_challenge_authorized:false,nvm_authorized:false,recovery_authorized:false},files:listed};writeFileSync(dir+'/PACKAGE-MANIFEST.json',JSON.stringify(manifest,null,2)+'\n');writeFileSync(dir+'/SHA256SUMS.txt',listed.map(name=>h(readFileSync(dir+'/'+name))+'  '+name).join('\n')+'\n');
check(JSON.stringify(files(dir))===JSON.stringify([...listed,'SHA256SUMS.txt'].sort()),'staging exact file inventory');
// Minimal standard ZIP writer, store method: no dependencies, filesystem
// discovery side effects or shell helpers; all file paths are already fixed.
function crc32(b:Uint8Array){let c=0xffffffff;for(const x of b){c^=x;for(let j=0;j<8;j++)c=(c>>>1)^((c&1)?0xedb88320:0);}return(c^0xffffffff)>>>0;}
let offset=0;const local:Buffer[]=[],central:Buffer[]=[];for(const name of files(dir)){const data=readFileSync(dir+'/'+name),n=Buffer.from(name),crc=crc32(data);const l=Buffer.alloc(30);l.writeUInt32LE(0x04034b50,0);l.writeUInt16LE(20,4);l.writeUInt16LE(0x0800,6);l.writeUInt16LE(0,8);l.writeUInt16LE(0x5d57,12);l.writeUInt32LE(crc,14);l.writeUInt32LE(data.length,18);l.writeUInt32LE(data.length,22);l.writeUInt16LE(n.length,26);local.push(l,n,data);const c=Buffer.alloc(46);c.writeUInt32LE(0x02014b50,0);c.writeUInt16LE(20,4);c.writeUInt16LE(20,6);c.writeUInt16LE(0x0800,8);c.writeUInt16LE(0x5d57,14);c.writeUInt32LE(crc,16);c.writeUInt32LE(data.length,20);c.writeUInt32LE(data.length,24);c.writeUInt16LE(n.length,28);c.writeUInt32LE(offset,42);central.push(c,n);offset+=l.length+n.length+data.length;}
const cd=Buffer.concat(central),e=Buffer.alloc(22);e.writeUInt32LE(0x06054b50,0);e.writeUInt16LE(files(dir).length,8);e.writeUInt16LE(files(dir).length,10);e.writeUInt32LE(cd.length,12);e.writeUInt32LE(offset,16);const zip=Buffer.concat([...local,cd,e]);mkdirSync(dirname(out),{recursive:true});writeFileSync(out,zip);console.log(JSON.stringify({archive:out,bytes:zip.length,sha256:h(zip),entries:files(dir).length,checksum_files:listed.length,staging:dir,powershell_status:'not executed locally; static and equivalent extracted-file checks required'},null,2));
