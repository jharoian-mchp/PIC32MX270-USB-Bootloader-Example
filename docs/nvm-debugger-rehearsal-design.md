# No-launch debugger rehearsal — source-only design, 2026-10-22

A separate RAM-only rehearsal can test MPLAB X/PICkit 5 halted editing, snapshot visibility and resumed Count/IRQ/bus state without installing an NVM backend. This document proposes its profile, mailbox and rendezvous contract. It supplies no C implementation, configuration changes, main, linked image, scratch page, debugger commands or board permission.

The [offline investigation](nvm-debugger-mailbox-investigation.md) established documentation and source evidence only. MPLAB X's effective startup/reset behaviour is unknown; IPE's observed Hold In Reset state is separate. The current board remains on canonical source `09da506`, running/read-only, with PICkit5 disconnected and external power/settings unchanged. Earlier trial/recovery permissions are spent. The A1 powered-BUSY concern and all NVM/protection qualifications remain open.

## Rehearsal boundary

The proposed application has no page parameter, NVM owner, adapter initializer, Flash read/write/erase interface or destructive token. The RAM challenge cannot advance a real NVM transaction. It must not link `isolated_nvm.c`, `isolated_nvm_target.c`, `update_flash.c`, `update_nvm.c`, generated `NVM_Initialize()` or an alternative NVM launcher. The current target port contains adapter references and rejects DEBUG ON; linking it and relying on garbage collection to remove writes would undermine this boundary.

A future separate source list contains only:

- A RAM-only rehearsal main/state machine and snapshot helper.
- An explicitly reviewed rehearsal configuration translation unit.
- A read-only PIC32MX270F256D register/profile interface.
- Only the required audited reset/C-runtime and non-resuming exception support.

The existing status masks and publication discipline can guide a later small read-only implementation, with drift tests against the target profile. No shared profile refactoring or production-port edit is selected here. Do not add the rehearsal sources to the diagnostic Makefile discovery, MPLAB projects or active source lists.

No SYS/CLK/NVM/USB/DMA/I2C/LED/GPIO startup, generated tasks, transport, application handoff or peripheral enable is permitted. SW1 and its debounce/gesture are deliberately omitted: this rehearsal checks RAM transport and debug return, not the owner's arming logic. Existing gesture mock evidence remains separate. The later NVM entrypoint must still enforce stable release, token reservation and a fresh gesture.

Main disables CPU interrupts once immediately after audited C initialization. Thereafter it samples and rejects unexpected IRQ enable; it cannot re-mask IRQ after debugger return to manufacture a pass. The application writes ordinary SRAM and the initial interrupt-disable control only. It does not write peripheral SFRs, NVM keys/control, Count/Compare, Cause.DC or Debug/COUNTDM. The debugger's installation/executive operations require separate review; the application's RAM-only contract does not certify every action of the hardware tool.

## Explicit profile proposal

The future build must identify an explicit `RAM_REHEARSAL_DEBUG_V1` mode in source/build metadata. There is no automatic fallback, runtime mailbox-selected profile, acceptance of both debug/non-debug words or permissive masking of unrelated fuses.

| Word | Existing non-debug target | Proposed rehearsal | Difference |
|---|---|---|---|
| DEVCFG0 | `7FFFFFE3` | `7FFFFFE0` | Only DEBUG low two bits3→0 |
| DEVCFG1 | `FD74CE5B` | `FD74CE5B` | None |
| DEVCFG2 | `FFF97AFA` | `FFF97AFA` | None |
| DEVCFG3 | `FFFFFFFF` | `FFFFFFFF` | None |

DFP1.7.380 encodes DEBUG ON as0 under mask3. Both DEVCFG0 values keep JTAGEN OFF, ICS_PGx4 and PWP/BWP/CP OFF. The words above are a design proposal only; changing or programming them needs later explicit permission. Preserve the existing target's strict DEBUG-OFF checks and its tests.

The running profile requires nominal12MHz/DIV3/MUL24/DIV2 PRIPLL (48MHz SYSCLK), PB DIV1, OSCCON COSC/NOSC3, OSWEN0, CF0 and eventual SLOCK1; Cause.DC0, CPU Status.IE0 and Debug.DM0. Only DM from a normal-mode Debug read is interpreted; other Debug fields are raw/uninterpretable evidence. No normal-mode COUNTDM validation or write is allowed. USBPWR0/USBBUSY0, central DMA.ON0 and NVM WR0/no WRERR/LVDERR are required throughout this rehearsal. An observed violation is terminal, never cleared or waited away. Only initial pending SLOCK and Count progress may use the finite observation loop.

Initial preflight uses one initial sample, at most256 loop samples and a fresh final sample. Post-challenge observation uses the same finite256-loop cap. These caps limit software observations, not physical elapsed time. Count must change from the first post-consumption sample; the post-resume delta must not be calculated from the pre-breakpoint Count. A halted counter or stalled CPU can prevent useful evidence or reporting. Raw Count progress does not establish24MHz or a safe NVM wait.

## RAM challenge and exact proposed layout

Use ordinary zero-initialized BSS, with separate private state and a word-aligned uncached KSEG1 mailbox. Final addresses are linker outputs, not constants chosen here. Every object must reside in `[0xA0000000,0xA0010000)` and be disjoint from stack/heap/runtime/executive reservations. No persistent/retained token or cross-reset continuation.

Proposed mailbox layout is37 aligned32-bit words (148 bytes):

| Word offset | Field | Writer |
|---|---|---|
| 0 | `guard_low = 13579BDF` | Target initializes once |
| 1 | `edit_word`, initially0 | Operator may edit at the pre-edit rendezvous; target clears once when consumed |
| 2 | `guard_high = 2468ACE0` | Target initializes once |
| 3 | `sequence` | Target only |
| 4–12 | Nine report header words below | Target only |
| 13–24 | Twelve-word `before` raw sample | Target only |
| 25–36 | Twelve-word `after` raw sample | Target only |

Header order: `version=1`, `mode=1` (RAM rehearsal only), `state`, `fault`, `edit_attempts`, `observations`, `challenge_observed`, `resume_count_first`, `resume_count_delta`.

Each raw sample has the exact order `osccon, count, status, cause, debug, pwrc, dma, nvm, cfg0, cfg1, cfg2, cfg3`. `before` is the last valid pre-edit sample; `after` is the latest resumed observation or failure sample. Uncollected samples are zero and their absence follows from state. In a completed report, `resume_count_delta = after.count - resume_count_first` in unsigned32-bit arithmetic; it excludes all pre-consumption debugger delay. Multiple unseen wraps remain possible.

The fixed challenge is `0x52414D31` (RAM1), distinct from the NVM owner token `0x4E564D31`. Values0 and all other values have no launch capability. First observed nonzero edit spends the boot's RAM attempt before validation: copy it privately, set the private spent flag/`edit_attempts=1` and clear `edit_word`. Publish OBSERVING only after sequence, guards, profile and challenge validation succeeds. Wrong value, guard damage, unexpected sequence or profile violation is terminal; no retry, re-edit acceptance or reinitialization. Zero stays inert indefinitely and never becomes failure merely because the user takes time to edit.

At the pre-edit halt, the later approved operator may write only word1 using an explicitly established32-bit SRAM edit. Capture before/after raw mailbox bytes, unchanged adjacent guards/sequence/report and completed edit readback before continuing. Endianness and address alias must match the ELF and actual GUI/tool transfer. Guards detect nearby damage; they do not prove an atomic32-bit transfer or detect all non-adjacent writes. The rehearsal cannot certify atomicity beyond the tool evidence actually collected. During a confirmed producer halt, even a bytewise tool transfer can complete before consumption; the workflow must establish completion/readback rather than infer it from alignment alone.

The polling order is explicit: take one local `edit_word` sample; if nonzero and not already spent, copy it privately, set the private spent flag/attempt count and clear that word **before** validating guards, sequence, profile or challenge. If guard damage and a nonzero edit are present together, the RAM attempt is still spent; guard failure then wins over challenge/profile acceptance. Validate sequence, guards, profile and challenge in that order; zero does not bypass integrity/profile checks while WAIT_EDIT. OBSERVING may be published only after these validations succeed. A sequence failure invalidates the report marker and fails privately; no normal report publication follows. A guard failure with a consistent sequence may publish FAILED/fault4 while preserving both damaged guards, but it never enters either rendezvous and the guarded snapshot API rejects that mailbox. A later raw-memory dump is evidence only and needs separate permission. No guard repair or re-edit can revive it.

The private expected sequence brackets every payload publication: odd first, XC32 `sync`, aligned payload words, `sync`, even last. Zero, odd, changed-during-copy and reserved `0xFFFFFFFE` snapshots are unavailable. Detected disagreement/exhaustion invalidates payload with the reserved marker, fails privately and cannot revalidate stale READY. Guard fields are outside that report sequence. The guarded snapshot API checks them before and after copying, in addition to sequence validation, and rejects any damage without writing its output. Failure publication into a sequence-consistent mailbox is a separate target operation, not a snapshot side effect. `edit_word` is not report payload. Snapshots never authorize a profile/address/retry or authenticate debugger-modifiable storage.

## State transitions and rendezvous

Proposed state words:1 CHECKING,2 WAIT_EDIT,3 OBSERVING,4 COMPLETE,5 FAILED. Fault words:0 none,1 profile,2 Count did not progress,3 invalid challenge,4 guards,5 sequence/exhaustion. For profile faults, the raw sample preserves the condition; a FAILED report may itself be unavailable after sequence damage.

1. **CHECKING.** Initialize new storage/token0/guards and report checking; establish the exact running profile. Any unexpected active bus or WR/error yields terminal failure. No NVM owner, initialization callback or page exists at any state.
2. **WAIT_EDIT publication.** Publish WAIT_EDIT and the validated `before` sample. Enter one pre-edit rendezvous call, once per boot, only after valid even publication, intact guards, token0 and a fresh USB/DMA/WR-inactive running sample. A hardware instruction breakpoint may stop there under a later approved procedure. The call contains no `sdbbp`, reset, token read/clear or state transition; the exact address/instruction/delay-slot/resume semantics must be verified in the eventual compiled code.
3. **WAIT_EDIT polling after that call.** If the breakpoint was absent/missed, edit_word0 remains inert. The target does not loop back through the pre-edit call or recreate the breakpoint. Poll only approved status/guards/private sequence and edit_word. No report mutation occurs while simply waiting safely. If a nonzero edit arrives, copy/spend/clear once, then validate sequence/guards/profile/challenge in the order defined above and publish OBSERVING only on success; begin fresh post-consumption Count sampling. No timing budget was active across the halt. Unexpected profile changes sampled during safe zero-token waiting fail terminally. If a nonzero token is present in that iteration, spending precedes profile rejection.
4. **OBSERVING.** Within at most256 read-only samples, require continued exact configuration/bus/IRQ/Count-control validity and at least one change from the fresh resumed Count anchor. Publish raw evidence with odd/even ordering. Take a final fresh full-profile sample before COMPLETE. If the final counter returns to the anchor value, reject progress evidence rather than accepting a stale intermediate change. No application callback can launch NVM or restore IRQs.
5. **COMPLETE or FAILED.** Publish once and keep payload immutable thereafter. A separate one-shot readout rendezvous is eligible only after a fresh check confirms USBPWR0/BUSY0, DMA0 and raw WR0, with intact guards and a valid even report/private sequence. For successful COMPLETE, the entire profile must still match. A clean wrong-challenge or Count-progress failure may reach readout when inactivity and report consistency are freshly established; an active bus, WR, unknown exception or damaged snapshot cannot. Failed preflight also needs these same independently established conditions; its label alone supplies no halt authority.

Unexpected activity found while establishing readout eligibility never forces USB/DMA/WR off. Before final immutable publication, it may replace a planned COMPLETE with FAILED. After publication, set a one-way private `readout_denied` flag/reason and retain the new raw observation in private storage only. This applies to later profile/activity/guard violations. The terminal **payload** is immutable. There is one explicit validity-word exception: if the target detects a changed private-expected sequence, it writes only the reserved sequence marker `0xFFFFFFFE` with `sync` ordering and records private denial, even after terminal publication. No payload/guard/edit word is changed. This makes a fabricated changed-even sequence unusable by ordinary snapshot readers rather than leaving a stale apparently-valid COMPLETE. Sequence invalidation is not result publication or recovery; after it, no more mailbox writes or rendezvous are allowed. All other readout denials leave the mailbox untouched. No denial changes the reported rehearsal result, republishes a payload, invokes a different failure breakpoint or permits retry. A still-valid public report remains evidence of earlier sampled instants, not the later readout-gate condition; an invalidated sequence means its payload is unavailable. No other automatic breakpoint follows a denial. Captured raw samples represent observation instants, not continuous inactivity. No independently running source may enable those peripherals; final source/runtime/executive review is required.

At both rendezvous, a later debugger action may enter DM1. The running preflight is not called while the user application is halted; DM1 at the debugger is a halt-state observation, not acceptance by the running profile. After return, fresh running DM0/IE0/DC0/configuration/bus checks are mandatory. Do not use a breakpoint that skips instructions, writes the PC or reexecutes initialization. No manually inserted trap, software breakpoint, debugger expression invoking target functions, Run-to-Cursor or peripheral auto-watch is part of the proposal.

The future terminal loop neither reinitializes objects nor reenters a rendezvous. A missing completion breakpoint can leave the target running harmlessly; it does not permit an unapproved manual halt. Exception/reset paths must not relaunch or pretend to complete the handshake. Reset can erase RAM evidence and allows fresh BSS in a new boot; this design cannot authenticate an uninterrupted boot from SRAM alone. Session logs/image provenance and separately scoped reset permission remain necessary.

## What the rehearsal can establish

An approved future host trial could establish, for one exact build/settings/tool/boot:

- Successful debug installation/release distinct from IPE and the actual pre-edit program counter.
- Correct target-symbol/SRAM alias mapping, completed edit readback and unchanged neighbours.
- Fresh consistent reports after resume, one challenge consumed, matching observed echo and terminal report.
- Sampled DM0/IE0/DC0/USB-off/DMA-off/WR0 and Count change in resumed application execution.

It cannot establish an atomic debugger bus transfer solely from SRAM values, absence of every transient interrupt/peripheral event, accurate frequency, COUNTDM behaviour during arbitrary halts, NVM timings/settlement, debugger noninterference during a real write, stable SW1 arming, protection/recovery or any upload lifecycle. The rehearsal permits debugger stops because it has no NVM transaction. A future destructive image must forbid halt/step/reset from token consumption through proven WR-clear/pending-settlement completion and keep its real gesture/owner/adapter checks.

## Source, mock and final-image acceptance gates

Before implementation, complete independent design review and obtain source-only implementation permission. A later implementation milestone needs tests for:

- Exact debug profile; reject production cfg0, DEBUG value1/2, unrelated cfg-bit changes, DM1/IE1/DC1, wrong oscillator, USB/DMA activity, WR/errors. Only initial pending SLOCK/Count progress may wait.
- Every token value class, RAM1 distinct from the destructive token, first nonzero spending before rejection, no re-edit/retry, token0 indefinitely inert, held/absent SW1 irrelevant.
- Guard damage, input/output alias and misaligned storage rejection, all report field snapshot cuts, odd/zero/exhausted/private sequence mismatch, no output mutation on reject; independent guards versus sequence checks.
- Arbitrary pre-edit Count advance including wrapping or halt/stopped Count: no resumed anchor derives from it. Post-consumption changed/stopped/wrapped Count, final-sample profile/progress failure and the256 observation cap. None proves hardware elapsed time.
- Missing/pre-edit/readout breakpoints, one-shot call ordering, no token consumption before pre-edit point, terminal payload immutability, no state recovery, unsafe readout omitted and no halt on active WR/USB/DMA. After terminal publication, profile/guard denials change no mailbox words; detected sequence damage writes only the unavailable marker once and makes changed-even stale payload rejectable.
- Instrumented SFR allowlist: only initial interrupt disable, otherwise SRAM publication plus approved register reads. No backend/Flash/NVM initializer or write path for any state/token/fault, including runtime/exception handling.

Later compile-only/disassembly checks must validate the uninstrumented translation units, exact profile, publication barriers and rendezvous function boundaries. A final standalone debug-image auditor must be separate from the USB restart auditor. It must verify canonical32KB load placement, Boot Flash/configuration records, no application/metadata/scratch loads, pairwise `.dinit` destinations, KSEG1 mailbox/private state/stack/heap/executive non-overlap, all reachable startup/exception code and absence of application NVM-write capability. Symbol absence alone is insufficient under LTO.

Hardware breakpoint addresses cannot be assigned before a final link; source-line breakpoints alone do not prove dominance. Require a preserved non-inlined rendezvous with inspected instruction/delay-slot location and explicit source/build flags; no instruction at a breakpoint may authorize a launch or consume the challenge. The compiler profile must be explicit and its effects audited, rather than assuming `-g` disables optimization.

Actual debugger installation can add an executive outside compiler HEX contents. The final method review must account for installed bytes/runtime storage/tool firmware and no software breakpoint/NVM-writing debug command during the rehearsal run. No “absence of NVM symbols” claim applies to the manufacturer's uninspected executive. If executive/reset/runtime behaviour cannot be established sufficiently, the method cannot be accepted from the application audit alone.

## Separate approvals and future delivery

No exact page is needed for a RAM-only rehearsal. Do not choose one for convenience or infer approval from the earlier48KB diagnostic overlap permission.

Before a future rehearsal delivery, require separately approved profile/configuration/debug-executive changes, reviewed final image and actual programming/erase/recovery scope. PICkit5 documentation says Debug mode programs Configuration Memory and Boot Flash, and selected program-memory ranges do not bound Erase. Existing projects' broad ranges/erase settings/preserve options are not rehearsal permission. A no-launch target application can still be installed by a destructive programmer operation.

Capture actual MPLAB Xv6.35/PICkit5 debug-startup/reset/Hold In Reset/power/freeze/tool-pack choices before host execution under a reviewed procedure. Preserve external supply, ICS_PGx4 and unrelated fuses/protection; any necessary settings difference requires explicit approval. Do not reconnect PICkit5, change project settings, reset, collect RAM, program a rehearsal or send GUI/debugger commands now.

After a successful approved rehearsal, a real NVM debug-profile/entrypoint requires separate source review and its own linked-image/owner/row/unlock/settlement audit. Exact disposable-page loss, physical clock/VDD/configuration evidence, one transaction and recovery are distinct permissions. Nothing in the RAM challenge grants them.

## Current verification status

This is an unimplemented design. Its profile arithmetic, mailbox layout, transition boundaries and relative source links can be checked offline; host mocks and XC32 object checks apply only to the already implemented inactive owner/target, not to this proposed rehearsal. Mechanical checks pass for the DEBUG-only fuse difference, unchanged JTAG/ICS/protection fields, challenge separation and disjoint37-word/148-byte layout. A bounded independent design review found no critical safety defect; it requested explicit simultaneous guard/edit ordering and private readout denial after immutable publication. Both are specified above. Follow-up requested clarification of post-terminal sequence invalidation; terminal payload immutability and the one-time sequence validity-marker exception are now explicit, preserving stale-readout rejection without result publication or recovery. A final bounded follow-up confirms both ordering issues and the explicit validity-word exception are closed, with no additional concrete contradiction in its scope. This is design evidence only, not implementation or hardware approval.

Relative source links and `git diff --check` pass. `make isolated-nvm-check` repassed the three existing isolated mock suites and real owner/target XC32 compile-only checks; these execute none of this proposed rehearsal. Firmware sources, Makefile and existing MPLAB project settings match `d146392`. Canonical/recovery HEX hashes remain `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7` and `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`. No new target source, configuration or linked image accompanies this milestone.
