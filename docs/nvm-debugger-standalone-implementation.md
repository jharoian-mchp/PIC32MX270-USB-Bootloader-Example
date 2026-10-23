# Standalone RAM rehearsal sources and structural auditor — compile-only

The standalone RAM entry, exact configuration and non-resuming exception sources are implemented under `rehearsal/src`. The initial image auditor passes synthetic structural/CLI fixtures and46 rejection cases. Strict host tests, ASan/UBSan and XC32 compile-only checks pass. No rehearsal linker, linked target ELF/HEX, selected CRT/vector layout, installed executive evidence or board procedure exists.

This milestone follows the [standalone design](nvm-debugger-standalone-design.md) and [inactive RAM library](nvm-debugger-rehearsal-implementation.md). The user approved the next source/compile-only step. Existing diagnostic/application firmware, saved MPLAB settings, board power/tool state and HEX files are unchanged. No programming, PICkit connection, debugger command, reset, RAM read/write, PAUSE, page choice, NVM/protection action or upload was performed.

## Implemented source boundary

| File | Implemented behaviour |
|---|---|
| `rehearsal/src/main.c` | Fixed ordinary-BSS context/mailbox; mask IRQ once, check effective IE, Init once, Tasks forever regardless of Init result. Never return, reset, reinitialize or invoke a rendezvous directly |
| `rehearsal/src/rehearsal_entry_hw.h` | One `__builtin_disable_interrupts()` followed by `ehb`/`sync`; Status.IE read. No restore/re-mask API |
| `rehearsal/src/configuration.c` | Configuration pragmas only, no callable initializer or peripheral definitions |
| `rehearsal/src/exception_stops.S` | Strong inert `_on_reset`/`_on_bootstrap` return bodies; shared stack-independent branch/nop stops for NMI/bootstrap/general/context/default interrupt destinations |
| `tools/rehearsal-image.ts` | Bounded exported buffer-based structural parser and explicit-file CLI |
| `tools/audit-debugger-rehearsal-image.ts` | CLI wrapper; no build, link or hardware invocation |
| `tools/test-debugger-rehearsal-entry.ts` and C/mock header | Exact-entry host test and optional real object checks |
| `tools/test-debugger-rehearsal-image.ts` | In-memory synthetic ELF/HEX/map/manifest/policy fixtures and temporary CLI-file tests; no target compiler/linker |

`debugger-rehearsal-source-check` compiles the library, main, configuration and assembly separately to XC32 objects and checks their contents. `debugger-rehearsal-image-test` runs synthetic parser tests. `make test` includes the host entry/parser suites; `make test-sanitize` instruments the C suites. Neither new target links or converts an image. No IDE project, linker script or default firmware source discovery was changed.

The emitted configuration object contains these little-endian words at its physical configuration sections: BF0 DEVCFG3=`FFFFFFFF`, BF4 DEVCFG2=`FFF97AFA`, BF8 DEVCFG1=`FD74CE5B`, BFC DEVCFG0=`7FFFFFE0`. Only DEBUG differs from the diagnostic's `7FFFFFE3`; JTAG/ICS/protection/clock/unrelated choices are preserved in source. Object emission supplies no permission to program those fuses or evidence of debugger-installed bytes.

The main object imports only `RAM_RehearsalInit` and `RAM_RehearsalTasks`. Its disassembly contains one DI plus hazard/order instructions and an IE check. The entry test substitutes hardware functions and verifies one mask/Init followed by three Tasks calls for both accepted/rejected initialization; `longjmp` exits the test's otherwise non-returning loop. It does not execute target interrupt instructions.

The exception object has no stack/SRAM/SFR stores, call, trap, `eret`, `deret` or reset. Its named exception destinations share a branch/nop loop; reset/bootstrap hooks return without state access. These strong sources are intended to replace corresponding default destinations, including the stack-using general exception context. Actual runtime-member selection, duplicate/weak resolution, vector addresses, early NMI/bootstrap control flow and debugger resume are still unverified without a final link. No exception reporting or automatic breakpoint is added.

## Structural parser inputs and trust limit

The CLI requires all six explicit arguments: `--elf`, `--hex`, `--map`, `--manifest`, `--policy`, `--expected-policy-sha256`. It has no default artifact, baseline or output-file side effect; it prints JSON on structural success and exits nonzero on malformed/unsupported inputs. All file inputs are bounded; source paths are confined to the manifest directory after realpath resolution, with duplicate/count/aggregate checks and descriptor-based bounded reads. It never invokes a compiler, linker, programmer or debugger.

Three versioned JSON schemas are supported initially:

- `pic32mx270-debugger-rehearsal-policy-v1`: independently pinned policy bytes, manifest hash, provenance/review labels, explicit Boot Flash spans/reservations, context/mailbox names and target context size.
- `pic32mx270-debugger-rehearsal-source-manifest-v1`: exact ELF/HEX/normalized-map hashes and source path/hash list.
- `pic32mx270-debugger-rehearsal-normalized-map-v1`: candidate-normalized allocated-section/reservation/source inventory bound to ELF/source hashes.

Exact schema keys are enforced. Source bytes must match the manifest; policy bytes must match the separately supplied pin before CLI source reads. No real approved policy or source/link manifest is created by this milestone. Synthetic test labels/pins have no operator approval or deployment meaning.

A matching pin proves only that supplied policy bytes equal the trusted caller's expected bytes. Provenance/review labels within those bytes are not independent authentication. The caller can fabricate an unsigned policy and its own expected hash. This parser therefore always leaves safety and delivery incomplete. The normalized map checks consistency with the ELF, not extraction from or correctness of an actual XC32 linker map. Native map parsing, external review-record validation and installed-executive evidence are not implemented.

## Supported structural subset

The parser accepts bounded little-endian ET_EXEC ELF32/MIPS with exact MIPS32r2/o32/noreorder flags, bounded tables/names/symbols, defined required symbols and valid power-of-two/congruent alignments. Entry must match the defined Boot Flash reset symbol and executable mapped bytes. Context/mailbox require correctly sized aligned uncached NOBITS SRAM, disjoint from one another and reserved stack/heap/debug spans. Canonical program loads stay in `[1D000000,1D008000)`; Boot Flash loads require explicit device-bounded policy spans. Unknown KSEG2/physical SRAM/cached SRAM aliases are rejected.

Each PT_LOAD must be independently and completely explained by congruently file/memory-mapped allocated sections. The initial strict format does **not** accept unexplained alignment holes, memory-only Flash debug-executive reservations or arbitrary ELF extensions. This may reject a legitimate XC32 output until its actual format is independently reviewed and support is added. Rejection supplies no image-safety conclusion.

HEX accepts bounded physical-address data/extended-linear/EOF records with exact checksum/count and allocated ELF-byte coverage. Duplicate bytes, unsupported record types, aliases, trailing records, rogue spans and missing/mismatched bytes fail. Configuration is checked in physical DEVCFG3/2/1/0 order. The initial normalized `.config` section format is a supported fixture format; actual linked XC32 section naming/layout needs review before broader acceptance.

The `.dinit` section must be delivered allocated program data, match a uniquely defined runtime `_dinit_addr` (including an absolute linker symbol), and decode supported clear0/inline-copy1 records with bounded positive lengths, zero alignment padding/termination tails and no pairwise destination overlap. Every destination must be mapped allocated uncached SRAM and avoid reserved spans. Context/mailbox must be covered by zero-clear records; a containing shared clear span is valid, while copy-only object authority is rejected. Runtime decoder execution and actual selected opcode format are not proven by table parsing alone.

All policy reservations and allocated-section checks canonicalize Boot Flash aliases. Structural object/storage constraints supplement source assertions; they do not prove stack capacity, constructor/hook side effects, legitimate manufacturer executive contents or general MIPS store/call safety.

## Status output and incomplete gates

A supported synthetic artifact reports `structural_status=pass` and `policy_status=pass` for byte/schema consistency. Every successful result also reports:

- `startup_status=unsupported`;
- `control_flow_status=unsupported`;
- `application_safety_status=unsupported`;
- `installed_executive_status=unknown`;
- `delivery_approved=false`, `hardware_approved=false`, `nvm_qualified=false`, `manual_approval=false`.

Candidate-authored approval fields are rejected by schema; the initial API has no way to set those statuses to safety/delivery PASS. The standalone design's eventual external policy approval, exact-hash manual-review record, native map, selected startup/vector/executive and actual installation/settings evidence need separate implementation/review. Passing a synthetic structural fixture cannot satisfy those gates.

## Tests and bounded review

Final passing verification:

- `make test` and `make test-sanitize`: full strict/ASan/UBSan C suites and30 Python methods, including exact-entry tests and46 structural/CLI rejection cases.
- `make debugger-rehearsal-source-check`: real XC32v6.00/DFP1.7.380 objects only; exact config-byte, one-DI/hazard, inert hook/stack-independent exception and existing RAM library checks.
- `make isolated-nvm-check`: existing three isolated owner/adapter/target suites and compile-only checks repass unchanged.
- Existing canonical image and negative audits repass via `bun tools/audit-restart-image.ts` and `bun tools/test-restart-image-audit.ts`, without linking firmware.
- All existing HEX hashes before/after tests match; canonical `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7`, recovery `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`. Diagnostic/application source lists and saved project settings remain unchanged.

Synthetic cases cover wrong config/order, missing/mismatched/duplicate HEX and EOF, ELF strings/headers/ISA/alignment, PT_LOAD virtual/physical aliases and unrelated file coverage or memory tails, undefined/aliased required symbols, bad mailbox size/reservation alias, `.dinit` opcode/zero-length/termination/padding/overlap/reserved/unmapped destinations, coalesced clear containment, copy-only object initialization rejection, source/map/manifest/hash mismatch, candidate fake approvals, policy bounds/pins, CLI argument/source-count checks and source symlink escape. Targeted cases assert the intended rejection message so they cannot pass solely because a stale ELF/HEX binding fails first. Temporary CLI fixture files are cleaned up.

The delegated initial parser implementation timed out after writing files; parent inspection and verification resumed locally. Inspection corrected physical config ordering, reset location and absolute linker-symbol assumptions. A bounded parser review found unchecked entry/alignment/ISA, global file coverage allowing unexplained segments, reservation aliases and premature/unbounded CLI reads. Those were corrected and regression-tested. A broad revised-parser follow-up timed out; a completed narrow follow-up confirms closure of those reported coverage/alias/read-limit holes. Another bounded review finds no concrete entry/configuration/exception source defect under the intended runtime boundary. These are limited source reviews, not exhaustive parser assurance or final-image/hardware approval.

## Next step and permissions

The [offline linker/native-map/runtime review](nvm-debugger-link-readiness-review.md) records actual XC32 format gaps and the production debug-return/software-reset path as a future-link blocker. A rehearsal linker/native-map adapter, explicit implemented local link workflow and broader real-XC32 structural support are not yet supplied. Any local target link/HEX must be separately authorized as non-deliverable artifact inspection. Resolve exact runtime hooks/constructors/data-init/exception/debug-return paths, KSEG1 layout and executive reservations before a final application safety review; do not silently widen parser rules to accept an unexpected artifact.

Installing the RAM-only application may still erase/program Flash, configuration and Boot Flash. Actual IDE startup/reset/Hold In Reset/power/freeze/tool-pack settings, debugger-installed executive bytes and erase/recovery scope require separate evidence and approval. IPE's observed setting does not establish IDE behaviour. No programmer connection or host token edit follows from this implementation.

NVM/protection/connected uploads and an exact disposable-page transaction remain later separate stages. Keep the canonical diagnostic running/read-only, PICkit5 disconnected, and external power/settings unchanged.
