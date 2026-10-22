# Standalone RAM rehearsal startup and image audit — design only

The [inactive RAM rehearsal](nvm-debugger-rehearsal-implementation.md) needs a dedicated entrypoint/configuration/linker and separate final-image auditor before it can be offered for debugger use. This design defines those boundaries from installed XC32v6.00/DFP1.7.380 startup sources and the existing image parser. No new target source, fuse translation unit, linker, auditor implementation, ELF/HEX, host procedure or board operation is supplied.

The user requested the next design/review step after `e344919`. The current board remains on canonical diagnostic source `09da506`, running/read-only with PICkit5 disconnected and external power/settings unchanged. All earlier PAUSE/recovery permissions are spent. IPE's observed Hold In Reset setting does not establish MPLAB X's effective setting. No disposable page is needed or selected for a RAM-only rehearsal, and the A1/NVM/protection qualifications stay open.

## Dedicated build and entrypoint contract

A future separate default-unbuilt target must have an explicit input list outside diagnostic/application projects. No recursive source discovery, default inclusion in `all`, programmer target or debugger launch command is permitted. The eventual compiler link and HEX conversion require distinct permission beyond today's design and existing library compile-only checks.

Proposed user inputs are limited to:

- `rehearsal/src/main.c`: fixed static context/mailbox, one initial IRQ-disable, one library Init and serialized Tasks forever.
- `rehearsal/src/configuration.c`: exact `RAM_REHEARSAL_DEBUG_V1` configuration, with no startup/peripheral definitions or callable initializer.
- `rehearsal/src/debugger_rehearsal.c`: existing RAM library with the real read-only hardware interface.
- Separate reviewed exception/NMI/no-op reset/bootstrap support if required by the selected runtime.
- An explicit rehearsal linker and the resolved XC32 C-runtime/library members actually required by that link.

Those names describe future files. None is created here. Do not include generated SYS/CLK/GPIO/NVM/USB/DMA initialization, application/diagnostic tasks, monitor/stdio transport, isolated NVM owner/target, Flash/NVM adapter or firmware-update source. C-library `memcpy`/`memset` used by the current object are expected, but their resolved definitions and any helper dependencies must still be inspected.

Main begins only after valid C storage/stack. It disables CPU interrupts once immediately, with the toolchain hazard/order sequence and an effective Status.IE check. It never restores IRQ, changes clock/cache/peripheral ownership or reruns this operation after debug return. Statically allocated context/mailbox symbols must be kept inspectable with exact sizes/alignment. The initial disable supplements audited CRT interrupt setup; it cannot excuse a startup hook or constructor enabling a peripheral before main.

Main calls `RAM_RehearsalInit()` once on new BSS objects, then calls `RAM_RehearsalTasks()` serially regardless of Init's Boolean result. A preflight failure is already terminal; Tasks must preserve its denial and never recreate state. Invalid C storage is prevented by the fixed declarations/final map, not handled by trying a second Init. Main never returns, calls exit, sleeps/WAITs, yields to generated tasks or invokes either rendezvous directly. There is no reset loop, automatic token, watchdog kick, external reporting callback or alternate NVM entrypoint.

The existing library privately decides pre-edit and readout eligibility. It may deny both forever. Missing breakpoints, Count stalls, corrupt mailbox or unexpected register state never authorize a manual halt/reset/reprogram command. Main-loop boundedness is not a wall-time guarantee.

## Exact configuration and independent IDE settings

The proposed configuration words are the library's exact `7FFFFFE0/FD74CE5B/FFF97AFA/FFFFFFFF`. Relative to the canonical DEBUG-OFF word `7FFFFFE3`, only DEBUG bits1:0 change3→0. Preserve JTAGEN OFF, ICS_PGx4, protection OFF, PRIPLL/HS/DIV3/MUL24/DIV2/PB1, watchdog OFF and all unrelated fuse bits. A configuration-TU implementation must use DFP-defined encodings and prove emitted bytes match all four words. No runtime-selected profile, accepted alternate words or software clock switching.

This is a source/build proposal, not approval to program DEBUG or settings. Define the compiler/linker/debugger mode explicitly and record full command lines, `_DEBUGGER` versus `__DEBUG`/`-g`, actual runtime members and tool versions. A source macro or an ELF configuration word alone does not establish the tool's installed executive or final fuse contents.

The saved MPLAB projects use `Use system settings` for debug-startup/reset behaviour. A future rehearsal project must not import their effective behaviour by assumption. Host review needs the actual v6.35/PICkit5 startup/reset/Hold In Reset/power/freeze/tool-pack/session choices, and no unsolicited stepping, runtime-watch halt, Run-to-Cursor, automatic reset or software breakpoint. Offline inspection of saved settings is separate from attaching to the current board. Preserve external supply and all existing settings until a later procedure explicitly approves differences.

## Reset-to-main audit boundary

The application read-only phase starts after reviewed C initialization and the one main IRQ-disable. Installed startup code has additional effects before that point. They must be enumerated and justified; do not apply the library object's no-`mtc0` rule blindly to CRT code or accept arbitrary startup writes because it is vendor-supplied.

### Installed source findings, not selected-runtime proof

DFP `xc32/startup/crt0.S`:

- `_reset` transfers to `_startup`; `_startup` checks NMI before ordinary initialization, establishes SP/GP and shadow-set GP state, then calls `_on_reset` before C data initialization (§source lines92–205).
- For PIC32MX it enables INIT_DATA. Contemporary source calls the external XC32 `__pic32_data_init`; its implementation comes from installed runtime archives, not the DFP source. The legacy inline table loop is under `__LIBBUILD__` and labelled pre-v2.10. Do not model it as the selected v6 runtime without resolving the actual object.
- L1 cache/TLB and BMX/RAM-function setup are conditional. With no RAM functions the BMX branch should be bypassed; the final link must prove it. Conditional OFF168/PRISS/INTCON/device-family branches likewise require actual preprocessor/selected-runtime evidence rather than generic assumptions.
- Startup sets Count0, Compare−1, BEV/EBase, IntCtl, Cause `0x00800000` and Status with IE0. It calls `_on_bootstrap`, then clears BEV before `_main_entry`. These are pre-measurement initialization effects, not evidence of resumed Count behaviour. Main and the library must not repeat Count/Compare/Cause/Debug writes after the one-shot measurement boundary.
- `_main_entry` can call `__libc_init_array` and `_init`/constructors before main. If main returns, `__crt0_exit` may invoke weak `__exception_handler_break`; `software-debug-break.c` defines an `sdbbp` trap. The rehearsal must not reach that return path or depend on its breakpoint.

`crt0_pic.S` has a different initialization path, including a disabled `#if 0` cache/MMU/BSS block. It is not an interchangeable selected runtime. Capture the actual linked CRT/member and flags; absence of source-level explicit startup in the project does not remove compiler-selected runtime code.

Installed `general-exception.S` creates an88-byte stack frame, calls `_general_exception_handler`, restores state and executes `eret` if that handler returns. Installed `debug-exception-return.S` may redirect DEPC to `__pic32_software_reset` before `deret`. These are separate general/debug mechanisms. A debug executive intentionally resumes from an approved breakpoint; an unexpected application exception must not recover, reset, authorize a challenge or enter a generic readout point.

The existing canonical ELF currently resolves `_on_reset`/`_on_bootstrap` to `jr ra; nop`, and exposes a separate linked `__pic32_data_init`. That is evidence for the existing artifact only. New inputs/mode/flags can select different members or constructor content; the future audit must resolve them again.

### Proposed startup-effect allowlist

| Phase/effect | Proposed acceptance condition |
|---|---|
| SP/GP/shadow-set initialization | Exact selected CRT instructions, valid non-overlapping SRAM stack, correct ISA/SRS settings and hazard sequences |
| C data initialization | Exact delivered `.dinit` and resolved routine; supported operations/termination only; each destination wholly approved SRAM, pairwise disjoint and outside stack/heap/executive; no hidden non-SRAM writes |
| Pre-main Count/Compare/Cause/Status/EBase/IntCtl/BEV | Enumerate actual operands/order, verify IE0 and DC0 at the application boundary; permit only reviewed runtime initialization before measurement |
| `_on_reset`, `_on_bootstrap` | Explicitly resolved inert no-op return bodies; no unresolved user hook, peripheral initializer or hidden callback |
| Constructors/init arrays | Prefer none for this C-only image. Empty/no-op paths may remain only after exact linked verification; reject nonempty callback arrays or unknown `_init`/libc-init side effects |
| Cache/TLB/BMX/PRISS/INTCON/RAM-function setup | Prefer absent/unreachable for this device and input list. Any actual reachable write requires separate source/device review and explicit policy; no permissive all-SFR exception |
| Main IRQ-disable | Exactly one source call site before library initialization; no later IE enable/restore or re-mask after debug return |
| Application normal operation | SRAM publication/private state, approved read-only SFR/CP0 access and inert rendezvous only; no peripheral control, NVM keys/mode/WR, Count/Compare/Cause.DC/Debug write or transport |

The final pre-main inventory must enumerate **every** linked callback or dispatch source: `_on_reset`, `_on_bootstrap`, `_init`, preinit/init/constructor arrays (`.preinit_array`, `.init_array`, `.ctors` and any runtime-specific tables), the complete reachable `__libc_init_array` path, data-init format dispatch targets and any selected runtime user hooks. Unknown indirect target, unexpected nonempty array or unresolved hook is unsupported. Early `_on_reset` and pre-stack NMI/bootstrap paths cannot read or publish context/mailbox/BSS merely because main later initializes them. Verify actual initialization dominance for every access and all callback/store effects; C-only source and empty project-level constructors are insufficient evidence.

A custom CRT is not selected here to avoid the stock startup effects. Choosing one later creates a new reset/ISA/stack/initialization/exception correctness task. If the actual runtime violates the reviewed allowlist, stop rather than silently widen it or force a register clear.

## Exceptions, NMI and unsupported startup failure reporting

The first rehearsal needs non-resuming application general/bootstrap/NMI/default-interrupt handling with no `sdbbp`, automatic reset, peripheral repair, owner initialization or readout rendezvous. Hardware debug exceptions are handled by the separately verified executive; do not route unexpected application faults into that resumable path.

Before stack/BSS establishment, an NMI/bootstrap error may have no safe C-report storage. A minimal stack-independent terminal branch can stop progress without publishing RAM evidence, but it must be designed/reviewed separately for the actual vector and CPU state. After valid storage, any optional exception evidence must use a distinct mechanism; normal mailbox publication cannot be assumed safe from a faulting instruction. First-stage simplicity favours no exception mailbox writes and no claims of a guaranteed failure snapshot.

Final vector acceptance requires a linked inventory of reset, NMI, bootstrap/general/default-interrupt/debug exception addresses and their resolved code/handler destinations, including every possible return or fallback branch. Application NMI/bootstrap/general/default-interrupt paths must be proven non-resuming and not reset/retry/readout paths. Pre-stack routes need valid stack-independent instructions or a separately established safe stack before any C access. Unknown/unresolved/returning destinations are unsupported and block application safety acceptance. Debug-vector/executive resume is allowed only for the approved hardware-breakpoint mechanism after separate installed-executive/session review; compiler NOLOAD reservations and vendor source names supply no such approval.

The current generated exceptions are not selected for reuse: debug variants contain conditional software-breakpoint support, and generic exception glue can execute `eret` if a handler returns. A future link must prove its non-returning destinations and that unused reset-return/trap paths cannot be reached. Unexpected exception/reset invalidates host method evidence; it never permits a challenge retry under the old transaction permission.

## Linker and storage requirements

Use a separate linker under `rehearsal/`; do not change `bootloader/src/config/default/btl.ld` or standalone application layout. Preserve program `[0x1D000000,0x1D008000)`, physical application origin `0x1D008000`, required Boot Flash/reset/exception/configuration records and uncached SRAM `[0xA0000000,0xA0010000)`. EBase/vector placement must remain within the reserved program prefix and follow actual device alignment/spacing. No application/metadata/scratch-page load or RAM executable function is selected.

Both the current bootloader linker and installed device linker define debug vector `0xBFC00480`, executive region `_DBG_CODE_ADDR=0x9FC00490`, `_DBG_CODE_SIZE=0x760`, and conditional `_DEBUGGER` NOLOAD reservations. The device/current linker also reserves `.dbg_data=0x200` in KSEG1 RAM under `_DEBUGGER`. These are known script reservations, not complete installed executive size/content evidence. Final debugger/tool support can require additional regions; exact verified intervals must be explicit before accepting placement.

The proposed mailbox is148 bytes and its context has toolchain-dependent pointer/alignment padding; use actual ELF symbol sizes, not a host `sizeof` or guessed address. Require independent BSS allocation/initialization for context/mailbox and an audited SP/stack range, runtime data and any heap/debug reservation. No `.persist`, reset-retained token, `.ramfunc`, COMMON hole or alias into executive memory. Context/mailbox/runtime allocations and stack/heap/debug reservations must be mutually disjoint in the actual map/ELF. `.dinit` destination records must be pairwise disjoint from other initialization records and must avoid stack/heap/debug/no-init reservations; they are expected to cover their intended initialized objects. Do not reject legitimate containment of a context/mailbox within its own clear/copy destination span. Require coverage and initial values separately from overlap checks.

The existing baseline stack minimum is1024 bytes and heap minimum512 bytes; neither is a rehearsal requirement or established worst-case executive/exception usage. The minimal application uses no allocation, but choosing heap0 or changing stack size is a later explicit build decision, subject to runtime/executive and call-frame review. Default linker top-of-RAM SP alone does not certify a disjoint or adequate stack.

NOLOAD reservations belong in memory-ownership checks even when absent from HEX. Code placement must account for debug exception/executive and configuration boundaries separately. Do not allow the whole Boot Flash simply because the executive occupies some bytes there. ELF physical/virtual aliases must canonicalize to correct program/Boot Flash/SRAM domains with subtraction-based bounds and reject KSEG2/unknown mappings.

## Separate auditor and evidence outputs

A future `tools/audit-debugger-rehearsal-image.ts` must require explicit ELF, compiler HEX, map/link/source manifest and trusted profile/audit policy inputs. It must not default to the canonical restart artifact, invoke a build/link/programmer, connect to hardware or infer permission from success. Missing/unsupported data fails closed. Reuse independently tested parser concepts from `audit-restart-image.ts`; do not weaken its USB owner/journal/configuration rules to accommodate another mode.

Expected config/allowed spans/source list must come from separately reviewed policy tied to the source/build commit, not be learned from the candidate being checked.

The initial trust anchor is a reviewed policy version/commit and exact SHA-256, recorded independently in the operator-approved review record before audit execution. Require a separately supplied expected policy hash, compare the policy bytes against it before interpretation, and include that hash plus provenance/review identity in every result. The candidate's manifest must not supply its own trusted hash or review identity. A caller can forge an entire unsigned local record; hash matching provides reproducibility under the trusted-operator workflow, not authentication against that caller. Missing external pin, wrong hash, unexplained policy revision or absent independent approval yields `policy_status=unsupported` and blocks any policy-dependent safety/delivery acceptance. A manifest can record hashes and results; self-consistent candidate hashes or self-asserted approval Booleans do not create an external trust anchor. Manual source/control-flow review evidence must identify artifact hashes/options and scope; a flag supplied by the artifact author alone is insufficient.

The auditor should report distinct outcomes:

1. **Application artifact checks**: pass/fail/unsupported for parsed ELF/HEX/configuration/storage/startup/control-flow policy. A mechanically passing application audit may still have manual-review requirements and unknown installed executive evidence.
2. **Debugger installation evidence**: unknown/not-reviewed/reviewed for actual executive bytes/reservations/configuration changes, settings/session/tool versions and program/erase scope. No speculative copying of runtime reservations counts as actual evidence.
3. **Delivery/hardware approvals**: false unless separately recorded for the exact artifact/profile/install/recovery/host procedure. A parser pass never authorizes a programmer connection, reset or token write. NVM qualification remains false for every RAM rehearsal outcome.

Initial audit implementation may cover structural/profile/storage checks without claiming automatic proof of arbitrary machine code. Report independent `structural_status`, `startup_status`, `control_flow_status` and `policy_status`. A structural PASS with startup/control-flow review pending is **not** an application safety PASS. Set `application_safety_status=pass` only when every structural/profile/storage check passes under the externally pinned policy and an independent review record covers exact final ELF/HEX hashes, link/map/source/options/toolchain identities, selected runtime members/callbacks/vectors and all reachable store/call/rendezvous paths. Match the record against the current artifact inputs; missing/stale/unsupported coverage leaves safety status unsupported and delivery false. Do not use a candidate-authored Boolean in place of that record.

The review record is part of the trusted human workflow, not a cryptographic certificate created by the parser. Its independently recorded identity/hash and bounded scope must appear in output. Installed-executive/settings/erase-scope and operator delivery approvals remain distinct even after an application safety pass.

### Structural and profile checks

- Parse Intel HEX checksums/types/address arithmetic/end-of-file and reject duplicate/overlapping conflicting or unsupported records, hidden trailing data, rogue addresses and aliases. Validate every delivered byte against ELF load/allocated sections and the explicit profile. Require exact file-byte coverage; reject missing or unexplained HEX bytes.
- Parse little-endian ELF32/MIPS machine/class/version, bounded headers/string tables/program and section tables, safe offset/size arithmetic, executable/data flags, mem/file sizes and ranges. Reject unsupported aliases, out-of-file data, code-shaped undefined symbols and ambiguous sections/symbols.
- Separate file-backed program/reset/configuration bytes, SRAM runtime allocations and NOLOAD debug/stack/heap spans. Do not compare all memory-only bytes with HEX or treat NOLOAD executive reservation as known code. Validate coherent section/program-header coverage and actual map evidence rather than trusting section names alone.
- Enforce canonical32KB program placement and exact Boot Flash reset/vector/config spans authorized by policy. Config words match all four exact values; permit no extra config/debug record, production/debug word alternatives or other fuse differences. Compiler HEX and debugger-installed differences are separate inventories; the latter need reviewed evidence, not a parser exemption.
- Parse `.dinit` from actual loadable delivered bytes and verify the selected v6 data-init routine's format/termination. Support only independently tested clear/copy operations; reject unknown/reserved encodings, zero-length/overflow records, nonzero unsupported tails and malformed padding. Prove every destination lies wholly in permitted SRAM and is pairwise disjoint; exclude stack/heap/executive/no-init regions and verify context/mailbox initialization. No reset-retained/nonzero edit authority.
- Require correctly defined code/data symbols and sizes for main/context/mailbox/rendezvous/vectors, aligned addresses in permitted regions and exact148-byte mailbox layout. Reject aliases into stack/executive and duplicate/ambiguous ownership. Source enum/header assertions supplement actual compiled layout evidence.

### Startup and control-flow checks

Record all link inputs/runtime members/includes and flags; real read-only header selection must be demonstrated, never inferred from a missing mock symbol. Require disassembly of final startup/hooks/data-init/constructor/runtime/exception/rehearsal/rendezvous code under the selected ISA. LTO symbol absence alone does not prove an NVM/peripheral path is absent.

Enumerate reachable stores/calls and verify the phase-specific allowlist. Simple opcode/import/source checks remain regression aids. Indirect runtime/data-init dispatch tables, MIPS delay slots, small-data addressing, stack stores and alias construction require exact operand/control-flow reasoning. A permitted SFR symbol import does not make writes to that SFR permissible. Unknown target/operand or reachable unreviewed callback is unsupported and blocks acceptance; no whole-address-space wildcard.

Prove reset reaches audited startup once, context/mailbox C initialization precedes main/Init, one initial interrupt-disable dominates Init, no return/reinitialization/constructor/peripheral enable path can recreate authority, and main calls only Init once then serialized Tasks. Prove fail/denial has no forced bus clear/reset/report transport/alternate breakpoint. Library test coverage must apply to exactly linked source/options, with regressions rerun after relevant changes.

Resolve the pre-edit/readout symbols to actual kept non-inlined instruction blocks. Verify inert return/nop contents, caller branches/guard domination, delay-slot semantics and no token consumption before pre-edit or payload mutation by the rendezvous. Actual hardware breakpoint semantics and resume PC must be established by a separately approved host rehearsal, not inferred from `noinline` or a source line. No `sdbbp`/software breakpoint/exception handler trap may replace them.

The auditor need not claim a general MIPS verifier. For an initial bounded exact image, a documented final disassembly/store/call review tied to its hash can supplement structural checks. Such review must be explicit in the output and repeated if source/toolchain/flags/debug executive change.

### Required negative fixtures

Structural regressions must reject bad CRC/EOF/trailing records, duplicate/alias/rogue loads, missing/mismatched ELF bytes, corrupted headers/sections/string tables/offsets, unsupported machine/ISA, mem/file span errors, undefined required symbols, bad config/protection/ICS/DEBUG values, broad or overlapping load spans, cached/unaligned/oversized/wrong mailbox objects, `.dinit` pairwise overlap/forbidden destination/unknown format/missing termination/rogue table source and executive/stack/heap collision.

Source/control-flow review regressions must reject a mock header, generated SYS/NVM/USB inclusion, startup hook/constructor enabling a peripheral, RAM-function/BMX branch, later IRQ restore, post-measurement Count reset, automatic challenge/retry/main return, reachable `sdbbp`/software-reset/debug-return fallback, direct ungated rendezvous, moved/token-consuming delay-slot instruction and arbitrary SFR-address store. Do not claim every mutant is automatically recognized by a structural parser: each fixture must name the implemented check or documented manual-review boundary that rejects it. Uncovered cases remain unsupported, never silently pass.

Debug-evidence regressions must distinguish absent installed executive bytes, reservation-only assumptions, unexplained debugger-written config/Boot Flash, wrong effective IDE settings/programming scope, changed tool/executive hash and missing host/recovery approval. These cannot be synthesized into hardware approval by editing an audit JSON field.

## Verification and sequencing

This is a design-only milestone. Existing library compile/mocks and structural audits support it; there is no standalone application artifact to audit. Installed source inspection identifies required review gates but does not establish selected runtime correctness.

Passing offline verification:

- Read-only inspection of installed `crt0.S`, `crt0_pic.S`, exception/debug-return support, current/device linkers and existing canonical startup/data-init disassembly. No selected rehearsal CRT, executive or main exists.
- Mechanical source/linker-reference checks, DEBUG-only word arithmetic, unchanged32KB span and relative links; `git diff --check` passes.
- `make debugger-rehearsal-check`: existing strict RAM mocks, real XC32v6.00/DFP1.7.380 compile-only object/import/opcode/rendezvous checks repass.
- `bun tools/audit-restart-image.ts` and `bun tools/test-restart-image-audit.ts`: existing canonical artifact and negative audits repass, without relinking firmware. These tools remain unchanged and are not a rehearsal auditor.
- Current firmware sources/Makefile/project settings and canonical/recovery HEX hashes remain unchanged. Hashes are `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7` and `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`.

A bounded independent startup trace reported verified source effects and distinguished them from an unselected runtime. The independent design review required complete pre-main callback inventory, external policy identity, structural/safety status separation and resolved exception destinations. The added requirements close those gaps; a completed bounded follow-up finds no new contradiction. These are design/source findings only, with no exhaustive proof, standalone-image pass or hardware approval.

Installed DFP startup source SHA-256 values: `crt0.S` `18e10584882d8e28479e9afd06153ab50999d7c0d076f7b3bd76a6978ef22f0e`; `crt0_pic.S` `a164dccb1bc5b412b8a77b1af3070ebc6860d2324ac52d4d4f5830ecf82d4177`; `general-exception.S` `f55b45d897e86c7f910f4615ed4a3b03ca4b04cc38288c3e8ce767edb45b68ab`; `debug-exception-return.S` `9b2a188b0972c89fa74de5348da99070f3274c6f6924dccda357f567c82764f3`. These identify inspected sources, not the installed executive or final CRT bytes.

Later steps are separately scoped:

1. Review/implement standalone source, configuration, exception support and structural auditor tests under source-only/compile-only permission; no automatic final link, IDE project/program or board command.
2. Explicitly authorize a local non-deliverable link/HEX for artifact inspection, resolve real runtime/debug mode and implement/check supported final audit coverage. Artifact generation alone provides no deployment permission.
3. Review actual debugger installation/erase/profile/settings/executive evidence and an exact recovery/host procedure; obtain distinct image/install/tool-connection permissions before delivering a no-launch rehearsal.
4. Record the approved RAM trial's actual ordered edit/snapshot/resume evidence. It still qualifies no NVM page, timing, arming/settlement, protection or uploader.

No programmer GUI/CLI commands, settings changes or page choice are provided. The next possible work is source-only implementation after design review, with a final link and every host action separate.
