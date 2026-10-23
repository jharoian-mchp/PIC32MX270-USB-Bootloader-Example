# RAM artifact provenance and application-review contract — offline

The existing RAM artifact has reproducible project-input/object hash matches and a native structural subset pass. It does not yet have complete build provenance or a full startup/control-flow review. This contract defines the evidence and independent review needed before any application-safety finding; it grants no image delivery, debugger connection or board permission.

The user requested continuation on2026-10-23 after native parser commit `381bc52`. This milestone is documentation and read-only evidence inspection. No target source/parser change, compiler, link, HEX conversion, debugger command, programmer connection, reset, RAM access or NVM action occurred. The one local link permission is spent; the artifact remains local. The board stays canonical `09da506`, running/read-only, PICkit5 disconnected and power/settings unchanged. IPE's historical Hold In Reset setting is not evidence of MPLAB X's effective setting.

## Exact artifact and evidence already available

This contract applies to the following existing files only:

| Artifact | SHA-256 |
|---|---|
| ELF | `cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a` |
| HEX | `918d3cab3801a75860a17b2adfd3cd03590299886db5add8e600d1796670ce33` |
| Native map | `f6dc317709d4299e431f32f03575f1271f64f4523a8754bdacd0032b7eed4eec` |

The retained experiment record in `build/debugger-rehearsal-experiment-20261023/attempt.json` contains the approval scope/time, eight input hashes, four object hashes, compile/link command arrays, compiler version, output hashes and outcome. Its SHA-256 at this review is `b44916fab9e67da46dfb818efdcbcc890bfcc9c92920b80d486f107d43770440`. Matching that record is reproducibility evidence for a trusted operator workflow, not authenticated proof that a compiler ran those exact bytes.

Parent read-only hash verification confirms all eight recorded project inputs and four retained objects match their recorded hashes. The exact linker bytes used in the experiment are checked using `linked-input-ram_rehearsal.ld`, SHA `632cb125dca3cbd0b6a29d4e8902dfbbb75b485c980f63a0a83710fba1771354`. The repository linker later had trailing whitespace removed and has a different documented hash; it must not be substituted as the linked input. No relink occurred.

Current source headers and retained `.d` files agree on the project dependency names. Those files were generated with `-MMD`, which omits system headers. Configuration includes compiler/DFP headers not listed in its dependency file. Therefore the recorded source/header set is not a closed build input graph.

The native map identifies output contributions from `libdinit.a(pic32_data_init_core.o)` and `pic32_data_init_clear.o`, vector0–43 members, C-runtime `memcpy.o`/`memset.o`, and software-debug-break support. Archive member names appearing in discarded sections or `LOAD` lists are not selected-code evidence. In particular, the map's `pic32_software_reset.o` mention is discarded in this artifact; no corresponding linked reset symbol was found. Review output sections and executable bytes, not a text-wide archive search.

Three compiler-generated map-loaded objects are missing: `/tmp/ccj5P7su.o`, `/tmp/cc1ND63T.o` and `/tmp/cc3HDAWM.o`. The reset/startup and exception-entry contribution is associated with `/tmp/ccj5P7su.o`; vector-dispatch references include `/tmp/cc3HDAWM.o`. Their exact pre-link bytes cannot be recovered from the retained directory, and their historical provenance must not be asserted from a similar current toolchain file.

Installed archives can be read and hashed now, or their current members compared with the linked bytes after accounting for relocations. This supplies current-content/association evidence only. The experiment did not record build-time compiler/linker/assembler binary hashes, system-header closure, archive/member hashes or hermetic search-path/environment inputs. A version string and current hash cannot retrospectively prove build-time identity. No environment values or secrets were collected to fill these gaps.

## Review record and external pinning

A future reviewed evidence bundle should have a strict versioned schema such as `pic32mx270-ram-application-review-v1`. It must carry independently recorded reviewer identity/date/scope, explicit policy identity/hash and exact artifact/input identities. This is a proposed record contract; no validator or approval record is implemented in this milestone.

Required fields/groups:

1. **Target/artifact:** device/build profile, ELF/HEX/native-map hashes, exact linked linker hash, configuration words, compiler/DFP version, command-record hash and native parser version/hash/policy pin. Bind every later result to the same artifact bytes.
2. **Project inputs:** source/header paths and hashes, source commit plus any uncommitted content, retained object hashes, compilation flags/dependencies and the documented whitespace-only post-link linker difference.
3. **Toolchain/runtime:** selected compiler-generated objects, tools, system headers, libraries/archive members, linker script/include closure and evidence of selection. Each item needs an evidence class and status, not merely a path or an archive mentioned by `LOAD`.
4. **Coverage:** exact executable address intervals, function/block identities, direct and indirect edge ledger, stores and target/range proofs, startup/callback/vector paths, allowed phase-specific writes, stack/heap/reservation assessment and assumptions. Explicitly list unsupported or unreviewed intervals/edges.
5. **Independent findings:** each reviewer’s bounded result, relevant negative fixtures or contradictory findings, disposition, evidence references/hashes and whether that coverage is enough under the independently chosen acceptance policy.
6. **Statuses:** separate provenance, structural, startup, control-flow, memory/capacity, installed-executive and application-safety statuses. Delivery/hardware/NVM approval remains false for this application review.

The expected policy and review-record identity/hash must be pinned outside the candidate's self-authored manifest, through the trusted operator/reviewer workflow. Hash comparison prevents unnoticed substitution only relative to that supplied external pin. An unsigned local record can still be fabricated by the caller; the parser cannot manufacture reviewer independence or authenticate an operator. No `approved:true`, edited transcript, matching filename or self-consistent hashes can replace independent acceptance.

For this proposed unsigned workflow, automated identity status is always `asserted_unverified`; it cannot support an automated application-safety PASS. An independently confirmed human application review is a separate disposition outside the parser, identifying the exact evidence hash and scope through the trusted communication channel. A validator may detect disagreement with an externally pinned record/label, but cannot detect an attacker who fabricates the label and its supplied pin together. No signature scheme, trusted reviewer key, automated identity verification or acceptance-promoting API is selected or implemented here. If those are required later, design them separately; do not claim the unsigned record supplies them.

Use evidence labels precisely:

- `recorded_at_build`: captured by the experiment record; state its trust limitations.
- `retained_hash_match`: current bytes match recorded input/object identity.
- `current_content_association`: current toolchain/member content and relocation-aware association with linked bytes, without historical build closure.
- `linked_binary_review`: exact final bytes/addresses reviewed directly; valid even where source association is incomplete, but not proof of historical source/tool identity.
- `missing` or `unsupported`: evidence unavailable or not covered by the implemented check.

Do not reclassify missing historical inputs as `recorded_at_build`. A complete provenance result cannot be obtained from the missing temporary objects. Any policy that accepts a bounded linked-binary review despite incomplete historical reproduction must explicitly scope that choice and its residual uncertainty; this milestone makes no such acceptance. No relink is requested automatically to make the records look complete.

## Full startup/control-flow coverage required

The prior [runtime review](nvm-debugger-artifact-runtime-review.md) establishes normal main non-return and conditional trap unreachability. The [native parser](nvm-debugger-native-parser.md) checks supported file/layout facts. Neither substitutes for the following exact-hash review.

### Executable inventory and edges

Account for every delivered executable section and its complete instruction range. Map aliases/shared stops and zero-length sections explicitly; distinguish non-executable data, `.MIPS.abiflags`, file padding and NOLOAD reservations. Verify entry/branch destinations and MIPS delay-slot semantics from linked words, including branch-likely annulment and PC+8 return continuations. A disassembly line count or symbol list is not coverage.

The existing disassembly has seven `jalr` sites: startup hooks/data-init at `BFC00040`, `BFC00050`, `BFC00118`; data-init dispatch `9D000ECC`; skipped zero initializer `9D001034`; main call `9D00104C`; and the post-main CRT trap call `9D001064`. Earlier review resolves these within normal-flow assumptions, but a full ledger must cover all direct branches/calls/returns, tail calls and computed targets too. Literal-pointer scans do not exclude computed pointers or corrupted execution state.

For each indirect edge record how the target is derived: fixed relocated address, validated table index, known null branch, preserved return address or an unsupported source. Unknown target is unsupported; do not infer safety from absence of an NVM symbol. No arbitrary-PC debugger action, skipped instruction or corruption-induced return is part of the accepted application model.

### Memory writes and call contracts

A corrected line-local disassembly scan counts100 `sw` and15 `sb` instructions, no `sh`, and one `sdbbp`. These are coverage aids, not proof that they are all memory-writing opcodes, that every store executes, or that every destination is safe. Scanners must not let `\s` consume newlines and accidentally merge nop/next-line instructions; the scan was corrected to include all seven `jalr` sites.

For every memory-writing instruction, classify destination/value/width/range and its valid phase. Include stack-frame spills/restores, delay-slot stores, compiler helpers, clear loops and any conditional SFR-writing code. Other store forms, cache/atomic instructions or instructions unrecognized by the review grammar must be explicitly reviewed or unsupported. Do not grant a whole-segment/address-space permission to unknown writes.

Normal application writes may touch only fixed context/mailbox, valid stack/local output buffers and reviewed runtime data. Establish source/compiled correspondence and valid buffer/pointer ranges for `memcpy`/`memset`; import names alone provide no destination guarantee. No target expression callable through the debugger or runtime hook may reinitialize, relaunch, restore IRQ or supply arbitrary addresses.

For the two data-init records, verify positive bounded lengths, allocated SRAM destinations, exact zero-clear dispatch and termination before the `0x66` tail. The clear decoder and table index must be reviewed from this linked routine; a source/template description is insufficient. No table-selected callback or write outside the known object spans is acceptable.

### Startup, exceptions and one-shot behaviour

Enumerate all reset-to-main work: stack/global-pointer setup, the zero RAM-function branch bypassing retained BMX stores, pre-main Count/Compare/Status/Cause/EBase/IntCtl writes and ordering, inert hooks, data-init dispatch, constructor/init entry and main. Accept the enumerated pre-main CP0 writes only before the measurement boundary; the library remains read-only thereafter, except for one main-entry IRQ-disable.

Constructor/preinit/init/other callback arrays and libc initialization paths need actual linked proof, including the zero `_init` pointer branch. Distinguish retained code from normal reachability: the CRT `sdbbp` return loop is present even though main's accepted ordinary path cannot return. A reviewer must decide explicitly whether the conditional unreachability finding meets the independently selected application policy; no automatic trap deletion or wildcard exclusion is permitted.

For NMI/bootstrap/general/default interrupt routes, prove every linked vector and shared stop destination does not return, reset, publish a false snapshot or enter a readout rendezvous. The pre-stack NMI path must not use BSS/stack before establishment. General exception register/state semantics and faulting-delay-slot behavior must be considered in the bounded CPU model. Debug exceptions remain outside the application-only model until installed-executive evidence is reviewed separately.

Confirm one Init in main, no main return/rearm/automatic challenge, nonzero attempt spent before rejection, and guards/private sequence/terminal failure consistency in the linked library. Prove the pre-edit/readout calls are gated, one-shot and inert, and verify pointer/buffer ownership through terminal sequence invalidation and post-debug return checks. Unsolicited debug callbacks must not be silently included in the application assumption set.

### Stack/capacity and asynchronous limits

Native layout checks bind the200-byte context,148-byte mailbox, fixed debug reservations and heap/stack boundary symbols. They do not certify maximum call depth/frame use, executive stack use or hardware overflow enforcement. Review compiled frame/call paths and buffer lifetimes, including the largest nested path and runtime/exception behavior. No allocation routine or hidden stack-using executive can be assumed absent solely because the user source uses no malloc.

Interrupt exclusion and sampled USB/DMA/WR inactivity are application conditions, not measured guarantees over arbitrary external interference. A CPU stall, halted Count, wrong clock, memory corruption or debugger-injected PC change may defeat progress/reporting. The review must state assumptions and fail/unknown behavior without turning them into a guaranteed timeout or recovery claim.

## Status decision and rejection matrix

| Condition | Required result |
|---|---|
| Wrong artifact/linker/policy/review hash, unsupported schema or missing identity | Reject the record; application safety unsupported |
| Recorded source/object hash mismatch or source/header selected differently | Provenance failed or unsupported; no automatic promotion |
| Missing system-header/compiler-generated/runtime build inputs | Historical build provenance incomplete; use only accurately labelled binary/current association evidence |
| Executable interval, memory-writing instruction, callback, indirect target or vector uncovered | Corresponding review status unsupported; application safety cannot pass |
| Nonempty/unexplained constructor callback, later IRQ restore or runtime reset/NVM/peripheral write reachable under accepted model | Application review fails; no patch/relink authority inferred |
| Conditional CRT trap unreachability, valid under stated assumptions only | Record a bounded finding; requires explicit independent disposition, never a parser-wide exemption |
| Capacity/executive storage or breakpoint-return model unknown | Leave that gate unsupported/unknown; no debugger trial approval |
| Native structural pass with source/runtime/control-flow review incomplete | Structural pass only; application safety unsupported |
| Independently confirmed human exact-hash application review meets all agreed application gates | Record an external application-only disposition and its scope; the unsigned validator still cannot authenticate identity or promote its own safety status. Installed-executive/delivery/hardware/NVM approvals stay distinct and unapproved |

A future consistency validator must reject stale records, missing coverage, candidate-authored approval fields and policy bytes differing from the externally supplied expected pin. It cannot determine whether a caller supplied a permissive policy and forged pin together. Tests should include changed objects/system-header inventories/runtime selection, discarded-member false attribution, missing temporary-object evidence, uncovered store/indirect edge, modified delay-slot return/guard dominance, reviewer-label mismatch with the independently pinned record, and partial or identity-unverified review incorrectly promoted to pass. Synthetic tests can validate schema/status mechanics; they cannot authenticate a real reviewer, detect self-consistent forged identities or create runtime proof.

## Current disposition and next evidence

Current retained project hashes and native structural facts are verified. Historical build provenance is incomplete; full linked startup/store/call/stack review is not complete. No reviewed application-safety record or acceptance validator is created now, and the native parser continues to return unsupported safety. The existing artifact's normal trap unreachability remains a conditional bounded finding.

The subsequent [bounded linked instruction/store/call review](nvm-debugger-instruction-coverage-review.md) provides executable, store and control ledgers for the existing ELF, including specific pointer/range and frame assumptions. It does not close full application safety, missing provenance or installed-executive gates. Additional offline review must retain those gaps explicitly. No additional artifact generation is allowed under the spent permission. If missing historical inputs ultimately require a new recorded build, seek a new narrowly scoped approval first; do not silently recreate temporary objects or relink to claim the original provenance was recovered.

Even a future application-only review would not establish the executive installed by MPLAB X/PICkit5, actual reset/Hold In Reset/clock/freeze/settings, programmer erase/recovery scope or RAM edit/snapshot workflow. Those require separately approved evidence and host permissions. No source change, tool connection, programming, reset, live RAM operation, NVM-page selection or hardware qualification follows from this contract.

## Verification for this documentation milestone

All eight recorded project inputs and four objects match current bytes under the documented linker exception; three map-loaded compiler temporary objects are absent. Map/runtime contributions and dependency omissions were inspected read-only. Native/strict parser regressions repass against independent fixtures and the existing artifact, but do not validate this unimplemented acceptance contract.

A bounded independent provenance inventory distinguished recorded hashes from missing evidence; parent hashing independently verifies the retained current-byte matches. A bounded contract review requested clarification that an unsigned validator cannot authenticate reviewer identity. The revised contract leaves that identity asserted/unverified, forbids automated safety promotion and separates external human disposition. A completed bounded follow-up confirms that identity assertion and external human disposition are now consistently separated; authentication itself remains unavailable. Native44-case and strict46-case parser regressions pass, retained evidence counts/hashes and document links/diff checks pass, and existing artifact/canonical/recovery hashes are unchanged. These verify the documentation's recorded facts, not the proposed validator or a full application review. No compiler/linker/board invocation is used.
