# Existing RAM artifact instruction/store/call coverage — bounded review

The existing RAM artifact now has an exact-word executable inventory and per-address store/control ledgers. Bounded source/binary reviews account for fixed-object, local-buffer and runtime-clear destinations under the documented caller contracts. This completes enumeration and several review slices; historical provenance, installed-executive behavior and a complete independent application-safety disposition are still missing. No parser or safety status is promoted.

The user acknowledged this offline coverage step at2026-10-23T19:25:41.531Z after the [provenance contract](nvm-debugger-provenance-acceptance.md). Only existing ELF/HEX/map/disassembly and source were read. No target source, parser, linker, firmware setting, local image or board state changed; no compiler, relink, HEX conversion, programmer connection, reset or RAM operation ran. The one local-link allowance remains spent and the artifact stays local/non-deliverable.

## Artifact identity and ledger files

| Artifact | SHA-256 |
|---|---|
| Existing ELF | `cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a` |
| Existing HEX | `918d3cab3801a75860a17b2adfd3cd03590299886db5add8e600d1796670ce33` |
| Existing native map | `f6dc317709d4299e431f32f03575f1271f64f4523a8754bdacd0032b7eed4eec` |

The following CSVs contain the concrete enumeration rather than only aggregate claims:

- [Executable section ledger](nvm-debugger-executable-ledger.csv):67 nonempty executable sections, address ranges, byte/word counts, store counts and observed frame allocations.
- [Store ledger](nvm-debugger-store-ledger.csv):115 word/byte store instruction addresses/words, base/source registers, offsets, destination contracts and assumptions.
- [Control ledger](nvm-debugger-control-ledger.csv):233 decoded control instructions with target/register, delay-slot address/mnemonic, link-return continuation and branch-likely status.

CSV SHA-256 values are executable ledger `e1b205d52b7c23a55a9f66ea8d2c106e925c0c78be044538b85d72830aaa2700`, store ledger `d77564b6cca1cb9062499e41f22d10923d81d55c0f0d90d10bf7a92577daa890`, and control ledger `3bc678fabf62b8626d99a669a565ce72466ba148839cb06af65f9d8011af9514`.

A read-only review helper under ignored `build/instruction-coverage-review/` checks the exact ELF hash, traverses all nonempty SHF_EXECINSTR sections, compares every four-byte instruction word against the existing disassembly, and requires each direct target and delay slot to be decoded executable code. It does not build an image or implement a general MIPS verifier. The resulting inventory is tied to these bytes, not to a hypothetical compiler/source reconstruction.

## Coverage totals and instruction forms

The67 executable sections contain3,636 bytes /909 four-byte instruction words. All909 words match unique decoded addresses in the disassembly, with no unmatched executable word. This excludes non-executable ABI/rodata/data-init/configuration bytes and NOLOAD debugger reservations. The native structural checks independently account for all3,728 delivered HEX bytes, as previously recorded in the [native parser report](nvm-debugger-native-parser.md).

The inventory finds100 `sw`,15 `sb`, no `sh`, eight pre-main `mtc0`, one main DI and one retained CRT `sdbbp`. In the observed word set, the store-opcode scan found no additional atomic/coprocessor/unaligned store or cache instruction form. This is a check of this instruction set, not a guarantee that a broad regex recognises every possible instruction or that corrupt execution cannot interpret other bytes as code.

Control forms are53 `jal`,7 `jalr`,28 `jr`,45 `j`, and100 conditional/unconditional branches. Each direct target lies in decoded executable words and each delayed transfer has a decoded PC+4 delay slot. MIPS link continuations are PC+8. Branch-likely delay slots execute only on the taken path; they cannot be evaluated as unconditional stores/register changes. The ledger flags those sites. Destination membership does not prove a path is safe, reachable or correctly bounded.

All44 fixed default-vector `j` instructions route to the shared stop at `9D000290`. The remaining non-call `j` at `9D0008D8` is the Tasks tail call to Finish; its delay slot releases the80-byte Tasks frame. Thus it must not be counted as a nested retained Tasks frame. The [prior runtime review](nvm-debugger-artifact-runtime-review.md) resolves the seven `jalr` sites and ordinary CRT trap reachability; return-state and table assumptions still apply.

## Store contracts and source/binary slices

Context is `A0000200..A00002C8` (200 bytes); mailbox is `A00002C8..A000035C` (148 bytes). Context layout is mailbox pointer at+0, report+4..135, denied raw+136..183, expected sequence+184, denial reason+188, and control flags+192..198. Mailbox layout is guards/edit/sequence at+0/+4/+8/+12 and report+16..147. Every caller must preserve those fixed pointers, valid objects, serialized API and normal ABI/stack state; public C structs are not authenticated debugger inputs.

| Store group | Count | Bounded destination evidence |
|---|---:|---|
| Retained conditional BMX SFR stores | 3 | Startup addresses `BF882010`, `BF882020`, `BF882030`; literal `_ramfunc_begin=0` bypasses them in the normal reset path |
| Stack spills | 19 | Constant offsets within their observed active frames, subject to valid SP/ABI and adequate stack |
| Tasks state/sample/edit stores | 12 | Fixed context fields and one mailbox edit clear before validation; terminal branch bypasses ordinary edit consumption |
| Init context/guard/edit/sequence stores | 15 | Fixed200/148-byte object fields following pointer alignment/overlap validation; called once by main |
| Publish payload/sequence/private expected | 36 |33 aligned payload words at mailbox+16..144; sequence at+12 before/after; private expected at context+184 |
| Hardware-read sample output stores | 12 | Caller-supplied48-byte local raw sample, offsets0..44 |
| Invalidate/Integrity/Finish private or validity stores | 15 | Fixed report/control fields within context; Invalidate changes only mailbox+12, preserving terminal payload |
| Generic memcpy store loop | 1 | Actual linked callers copy48 raw-sample bytes to before/after/denied contexts, not arbitrary destinations |
| Generic memset store loop | 1 | Init zeros context200 and its report132 at context+4 |
| Data-init clear store loop | 1 | Exact two positive clear records: mailbox148/context200, opcode0 dispatch |
| **Total** | **115** | Destination contracts are scoped evidence, not fault-independent proof |

### Publication and terminal control

A completed bounded review of Publish/Sequence/Invalidate/Finish/Integrity and the inlined readout branch finds no concrete out-of-range store under these object/ABI assumptions. Publish `9D000C7C` validates private sequence and terminal status, writes odd sequence, performs33 report-word stores, then sync/even sequence and private expected update. The compiler puts the private update in a branch delay slot at `9D000DF8`; that delay slot remains part of the normal success path.

Sequence/Invalidate preserves the reserved marker protocol. Invalidate writes mailbox sequence only, then private fault/terminal/denial fields; when already terminal it does not republish payload. Integrity validates sequence before guards. A guard-failure path may publish a FAILED report under a consistent sequence, but Finish establishes readout denial for profile/guard failures and the damaged guarded snapshot remains unusable. The explicit one-time validity-word exception remains the only post-terminal mailbox write.

Readout is inlined into Tasks. The branch validates integrity, monotonic denial and full profile before setting the one-attempt flag and calling the inert `RAM_RehearsalReadout` at `9D000288`. After return, fresh integrity/profile checks can deny future readout without remasking IRQ, clearing a bus or republishing the result. Terminal polling may perform fresh status reads; it cannot repeat the rendezvous or consume a replacement challenge.

No direct debugger function-call, arbitrary PC skip or executive register/stack mutation belongs to those caller contracts. The installed executive is not covered by the ELF's inert nop/return rendezvous blocks.

### Init/Tasks and helper buffers

Parent source/disassembly inspection and a completed smaller independent buffer/frame review confirm the48-byte raw buffer at caller `sp+16..63` in Init's96-byte and Tasks'80-byte frames. It fits below their saved registers. The hardware-read routine has an8-byte frame and stores its12 captured words to that caller output, never to a peripheral register. Its SFR/CP0 operands match the read-only profile interface.

Init's two memset calls have exact destination/length pairs context+0/200 and context+4/132. The raw copies target context+40..87 for `before`. Tasks copies to+88..135 for `after` and+136..183 for denied raw. `memcpy` and `memset` are leaf byte loops in this artifact. Their generic routines remain unsafe for arbitrary pointers; only the concrete caller ranges above have been reviewed.

Init validates null/alignment/object overlap, initializes once and uses finite256-loop readiness observations. The fresh pre-edit path spends/rejects a premature edit and calls the inert pre-edit block only after integrity/profile/final publication. Tasks copies/spends/clears the first nonzero edit before sequence/guard/profile/challenge rejection, anchors Count from a fresh post-consumption sample, caps its observation loop and requires a final matching profile/nonzero delta. Both routines return to main on terminal/rejected states without recreating the objects. The observed normal main loop has no return, retry or automatic challenge.

A larger delegated Init/Tasks review timed out; parent inspection and the later completed smaller review cover the buffer/frame/failure-chain details above. They are bounded slices, not independent approval of every execution outcome or a complete machine-checked state invariant proof.

### Runtime and startup effects

The data-init loop dispatches opcode0 to the delivered clear routine at `9D000228`; both records have positive exact object lengths and terminate on zero destination before four `0x66` bytes. The table and zero-clear path were reviewed in the prior exact-artifact report. No table target to the CRT trap is used by these records.

The eight pre-main `mtc0` sites initialize Count, Compare, Status/BEV, EBase, IntCtl, Cause/DC and final Status state. These are retained reviewed startup effects before the application's fresh timing/IRQ boundary. Main performs one DI plus hazards/sync and effective-IE check. No subsequent application CP0 write or interrupt-enable instruction was found in this word inventory.

Conditional BMX stores are present but bypassed by the literal-zero branch; they were not removed. The retained `sdbbp` at `9D0001F4` also remains. Ordinary main cannot return into its CRT caller under the normal-state assumptions, but corrupted PC/stack or unknown executive actions are outside that finding. This milestone does not accept a new trap policy, modify runtime code or relink.

## Application frame estimate and its limits

Observed allocating frames: `_main_entry`24, main24, Init96, Tasks80, hardware-read8, Publish24, Integrity24, Finish24 and conditional Sequence24. Leaves include Profile, Invalidate, memory helpers, inert hooks/rendezvous and the shared exception stop. The executable ledger lists each allocation site.

The conservative deepest scoped failure chain is `_main_entry → main → Init → Integrity → Finish → Publish → Sequence`:24+24+96+24+24+24+24 = **240 bytes from incoming `_main_entry` SP**. Startup initially sets SP to `A000FFF8`,8 bytes below RAM top, so that path's frame lower bound would be `A000FF08` under intact ABI state. This is an observed application-path estimate, not an executive/corruption/exception maximum or proof that arbitrary stack writes are contained.

The earlier scoped192-byte figure did not include all entry/main and Integrity path contributions. The new240-byte total includes them explicitly. Tasks' similar Integrity/Finish/Publish/Sequence path is smaller; Tasks tail-calling Finish releases its own frame first. No application recursion or variable-size stack allocation was found in these reviewed paths.

Native `_splim`, `_stack`, heap and debug reservations remain structural symbols; they do not prove physical overflow enforcement, runtime/executive use or every asynchronous CPU state. Installed debug-executive frames, actual debugger halt/return effects, missing temporary startup-object provenance and exception-state behaviour are not included in240 bytes. Stack capacity and host method are still unsupported independently of the large visible SRAM gap.

## Verification and remaining gates

Exact executable words were compared with the ELF and existing disassembly, and direct control targets/delay slots were checked mechanically. Store/control/section ledger row totals and destination categories were checked against the inventory. Native44-case and strict46-case parser regressions repass, including read-only inspection of the unchanged local artifact; no target compiler ran.

Bounded independent slices cover publication/terminal handling, buffer/failure frames and prior reset/trap/data-init behaviour. A larger Init/Tasks review and larger final report/ledger review timed out. A completed narrow final check confirms count/frame arithmetic and consistent unsupported safety claims. Parent inspection supplies the broader enumeration and caller contracts; no independent full-program closure is claimed. None of these reviews is a formal whole-program verification or authenticated reviewer certificate.

Historical system-header/tool/generated-CRT/runtime input provenance remains incomplete, as listed in the [provenance contract](nvm-debugger-provenance-acceptance.md). No replacement inputs were fabricated. The unsigned identity limitation, installed executive, native-policy external acceptance, actual IDE settings/erase/recovery evidence and host trial permissions remain unresolved.

A full application-safety disposition still requires an independent decision on complete linked call/return/pointer-state invariants, the retained but normally bypassed stores/trap, frame/capacity assumptions and missing provenance under an explicitly chosen policy. The ledgers enumerate the addresses and attach bounded contracts; they cannot establish those assumptions for every fault or debugger action. Native parser status remains unchanged: application safety unsupported, installed executive unknown, delivery/hardware/NVM approvals false.

No image is attached or delivered. Only the report and ledgers are downloadable. Any further source/runtime change or new artifact generation needs a separate scoped approval; no patch, relink or board action follows from this review. The board remains canonical/read-only with PICkit5 disconnected and external power/settings unchanged, and the A1/NVM/protection qualifications remain open.
