# RAM rehearsal linker, native-map and runtime review — offline

The compile-only rehearsal sources at `7130af6` are ready for a separately scoped linker/format implementation and local artifact experiment. They are not ready for delivery. No dedicated rehearsal linker, native-map adapter or linked rehearsal ELF/HEX exists. This review inspects existing diagnostic artifacts and installed XC32 support only; it changes no target source, parser, linker, firmware settings or board state.

The user acknowledged the next offline review on2026-10-23. Final linking was explicitly reserved for later approval and was not performed. The board stays on canonical source `09da506`, running/read-only, last recorded STATUS1/2/1/7, PICkit5 disconnected with external power/settings unchanged. All earlier trial/recovery permissions are spent. The A1 powered-BUSY concern and NVM/protection/connected-write qualifications are unchanged.

## Existing artifact evidence

The inspected canonical artifact is a production USB diagnostic, not the proposed DEBUG-ON RAM rehearsal. It provides real XC32v6.00/DFP1.7.380 format examples, not a template whose runtime can be accepted unchanged.

| Existing file | SHA-256 |
|---|---|
| `build/restart-check/bootloader.map` | `c250b95933f78a809329052a3ad95a0d6923c54c0bc35fe4e6a216c1b9f72a1d` |
| `build/restart-check/bootloader.elf` | `8a155684a047862f6622a725109027ffd256b960d9a2da7648a041e89ef8109a` |
| `build/restart-check/bootloader.hex` | `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7` |

Its ELF is little-endian MIPS32r2/o32/noreorder ET_EXEC, entry `0xBFC00000`. There are32 program headers, of which31 are PT_LOAD. The current parser's exact machine/ABI flags match this particular artifact; unsupported variants must remain explicit rather than accepted by assumption.

The compiled rehearsal entry object has `rehearsal_context` size `0xC8` (200 bytes) and `rehearsal_mailbox` size `0x94` (148 bytes). Those are XC32 object observations for current flags. Their section-relative offsets are not final SRAM addresses. Future compile/link changes require new symbol/layout verification; a policy must not learn trusted expected object sizes solely from the candidate it audits.

## Dedicated linker requirements

Use a separate script under `rehearsal/`, retaining required device/runtime mechanics from the inspected linker with provenance and a small reviewed diff. Do not change the diagnostic linker, application layout or saved MPLAB projects. No script is created by this review.

Required layout contract:

- Program region `0x9D000000`, length32768; physical prefix `[0x1D000000,0x1D008000)`. EBase/general/vector placement must remain in that prefix and respect actual MIPS/device alignment and vector spacing.
- Uncached data `[0xA0000000,0xA0010000)`. Ordinary context/mailbox BSS, runtime data, stack, any heap and debug reservations must be independently allocated and checked for overlap. No `.persist`, reset-retained authority or RAM function/BMX setup is selected.
- Preserve required reset/BEV/debug-vector/configuration locations. Existing script regions are Boot Flash `0xBFC00000` length`0x490`, debug executable reservation `0x9FC00490` length`0x760`, and four4-byte configuration regions `0xBFC00BF0/BF4/BF8/BFC`.
- Exact source configuration remains proposed DEBUG-ON `7FFFFFE0/FD74CE5B/FFF97AFA/FFFFFFFF`, physically stored in DEVCFG3→0 order. No extra records, altered protection/JTAG/ICS/clock fields or automatic alternative profile acceptance.
- The known conditional `_DEBUGGER` reservations are `.dbg_excpt=8`, `.dbg_code=0x760` and `.dbg_data=0x200`. Their actual addresses/sizes must appear in final ownership checks even if NOLOAD/absent from HEX. A script reservation does not certify installed executive content or all RAM usage.
- Linker best-fit BSS/data allocation and compiler-generated `.dinit` must be preserved or replaced only through reviewed runtime-compatible rules. Clear/copy destinations may contain their intended objects, but initialization records must be pairwise disjoint and avoid reserved stack/heap/debug regions.

The diagnostic script's default stack minimum1024 and heap minimum512 are baseline settings, not proven rehearsal requirements. Do not silently choose heap0, resize stack or infer capacity from remaining RAM. The local artifact experiment should record the selected values explicitly and leave capacity/actual executive usage unqualified until reviewed. Default linker behavior may place stack/heap without named output sections; use actual symbol/range evidence, not a guessed subtraction from top-of-RAM SP.

Keeping the vendor vector/runtime section definitions is initially preferable to deleting them merely to obtain a small image. Unused interrupt-vector dispatch, extra runtime members or questionable boot/debug records may then make the experiment fail its audit. An inspectable failure is useful evidence; it does not justify moving the application origin, enlarging the32KB prefix, discarding required exceptions or widening the audit policy.

## Native XC32 map adapter contract

The actual map differs from the initial normalized JSON inventory. Relevant observed text forms include:

- `Archive member included to satisfy reference by file (symbol)`, followed by archive/member, referencing object and symbol entries, sometimes wrapped across lines.
- Discarded input sections before `Memory Configuration`; these are not allocated output sections.
- `Memory Configuration` at line996 and `Linker script and memory map` at line1011 in this artifact. Region origin/length/attributes follow the first heading.
- Output sections may put name/address/size on one line or split a long section name onto a separate line. Indented input sections, wildcard expressions, symbol assignments, fills and archive contributions follow. Duplicate occurrences of a name are not automatically duplicate output sections.
- `LOAD` entries include temporary `/tmp/cc...o`, LTO `.ltrans.o`, runtime objects and archives. `LOAD` entries occur both near the main map heading and near its end. Their order matters for inventory; names alone do not prove source/include provenance.
- Four configuration output sections, `.dinit`, zero-length init/debug sections and best-fit RAM output sections, followed by `OUTPUT(... elf32-tradlittlemips)`.

The existing map contains `_min_stack_size=0x400` and `_min_heap_size=0x200`, but does not expose usable `_stack`/`_heap` address assignments in the inspected textual map. The ELF does provide `_stack=0xA000FFF8` and `_heap=0xA00008A8`. These symbol values alone still do not define a trustworthy stack interval or capacity; a missing map reservation must be reported, not fabricated.

An adapter needs a versioned, bounded grammar with a strict state separating discarded/input/output/region/LOAD/symbol records. Preserve raw map bytes/hash and attach extraction diagnostics. Reject ambiguous output rows, unknown allocation forms, truncated/wrapped records or incompatible regions. Cross-check extracted output address/size/type against the independently parsed ELF; map labels do not determine NOBITS/PROGBITS without ELF evidence.

Capture explicit source/object/header/compiler command provenance before linking. Stable retained objects are preferable to a one-shot compile/link whose temporary filenames later disappear. For every retained object, record source/include/flags/hash; separately record exact runtime archive/member and compiler-selected object hashes. The map cannot reconstruct trustworthy source/header provenance from anonymous `/tmp` or LTO names.

Do not rewrite raw candidate map text or rename ELF sections just to fit the normalized fixture format. A future native-map schema should carry raw-map hash, extraction version/status, independently bound source/object/runtime inventory and actual reserved-range evidence. The initial parser currently has no such adapter or schema. Until implemented and independently tested, `native_map_status` is unsupported and application safety remains unsupported.

## Actual ELF/HEX format gaps

| Observed production output | Current initial parser restriction | Required later support/rejection evidence |
|---|---|---|
| Four `.config_BFC00BF*` sections/loads, each4 bytes | One normalized16-byte `.config` | Accept only the exact four names/addresses/sizes and DEVCFG3→0 bytes after regression tests; reject missing/duplicate/extra/alias config records |
| Flash PT_LOAD spans contain vector alignment holes omitted by HEX | Every segment byte must be tiled by sections | Classify only independently verified section gaps/padding. Require section file-offset/vaddr/paddr coherence, every delivered byte explained and all allocated file-backed bytes present; reject extra rogue segments reusing file bytes or loads into a gap |
| SRAM PT_LOAD includes file bytes for `.data` plus larger memory size | SRAM NOBITS/memory-only PT_LOAD only | Separate initialized SRAM sections from burned Flash bytes. Prove actual `.dinit` supplies initialization, and reject physical SRAM HEX loads or cached aliases. Do not skip SRAM checks wholesale |
| Zero-length allocated output sections in the map; some empty ELF sections | Allocated section size must be positive | Explicitly model empty non-owning sections, with bounded names/types/flags/address validation; do not let zero-length spans hide rogue loads |
| `_dinit_addr` is a linker-defined symbol; functions/aliases may have runtime-specific type/size | Narrow required symbol type/size checks | Support only proven symbol conventions; verify section/range/data-init semantics independently, not by relaxing all required symbols |
| Conditional debug Boot Flash NOLOAD regions and SRAM `.dbg_data` | Strict initial parser rejects memory-only Flash sections | Model declared reservations separately from installed code. Prove no application bytes/objects overlap them; leave executive unknown |

The existing vector segment beginning `0x9D0002E0` has file size752 bytes and200 bytes in allocated vector sections; the remainder is552 bytes of segment gaps. Other vector spans also have gaps. The existing restart auditor checks exact section-byte delivery while tolerating those ELF segment holes. Its behavior is reference evidence only; copying its broad exemptions would reintroduce the initial parser's global file-coverage and extra-segment defects.

The canonical ELF's SRAM loads include PROGBITS and NOBITS plus a retained journal; the rehearsal must omit the journal. Native RAM section handling must still prove context/mailbox zero initialization, precise object/reservation bounds and no overlap with executive or stack/heap. Best-fit allocator placement may prevent assuming one contiguous BSS range.

No format rule is relaxed by this review. The current synthetic parser remains intentionally narrow, with structural success unable to produce safety/delivery approval. Future real-format changes require positive native-shape fixtures and mutants targeting each exemption, plus checks that failure is not hidden by stale ELF/HEX/hash bindings.

## Runtime and debug reset blocker

The existing production map lists `libdinit.a(pic32_data_init.o)` and clear/copy dispatch members, weak reset/bootstrap/general/NMI/default-interrupt members from `libpic32.a`, plus compiler-selected `debug-exception-return.o` and `pic32_software_reset.o`. `_on_reset`/`_on_bootstrap` resolve to inert return/nop bodies in this artifact, and constructor arrays shown in its map are empty. The future rehearsal uses strong hook/exception sources, but that does not prove which members/debug glue will be linked or executed.

Installed `debug-exception-return.o` defines `__DbgExecReturn` at `0xBFC00480`, size44 bytes, and references `__pic32_software_reset`. The existing ELF resolves the latter at `0x9D007AA0`. Its object disassembly contains DI and stores to `SYSKEY` and `RSWRSTSET`, followed by a read of `RSWRST` and a stop loop. This is executable reset capability, not a harmless name or NOLOAD reservation.

The rehearsal's application exception paths must remain non-resuming and non-resetting. Debugger exceptions are a separate mechanism: an installed executive intentionally resumes from an approved breakpoint, but its final vector, return PC/state and fallback/reset behavior require explicit evidence. Neither ignoring reset symbols because main cannot call them nor assuming `-mdebugger` eliminates the fallback is acceptable. Do not remove runtime objects, alias reset routines to a success path or insert a custom debug-return stub without a new source/vector/ABI review.

XC32's target-help describes `-mdebugger` as allocating space for the debugger executive and enabling debugging features; `-mappio-debug` enables APPIO support. Compiler debug-info flags, source `__DEBUG`, linker `_DEBUGGER`, executive installation and APPIO/monitor support are distinct. The intended experiment must record the exact compiler-supported debugger mode and resulting runtime inputs. APPIO/semihosting/stdio/monitor transports are excluded from the RAM rehearsal.

Selected startup still needs final inspection: initial SP/GP/SRS, pre-main Count/Compare/Status/Cause/EBase/IntCtl writes, reset/bootstrap hooks, external data-init dispatch and any libc init/constructor path. The strong non-resuming exception-stop object is only source/object evidence; resolve all vector branches and duplicate/weak symbols in the final link. The library's fresh post-consumption Count measurement must stay separate from CRT initialization and debugger halt-time behavior.

## Proposed next experiment and approval boundary

The next implementation/artifact step should be limited to:

1. Create/review a dedicated rehearsal linker and explicit retained-object/source/runtime inventory workflow, with no default build or IDE/programmer binding.
2. Add independently tested native-map/ELF format support where justified, leaving startup/control-flow/application safety unsupported until exact final review.
3. Under separate explicit approval, perform one local non-deliverable rehearsal build/link/HEX experiment in an isolated build directory with XC32v6.00/DFP1.7.380, canonical32KB reservation and the implemented RAM-only sources/configuration. Record exact options, native map, retained objects, ELF/HEX hashes, runtime inventory and audit failures. No application/metadata/scratch loads or NVM source may enter its input list.
4. Stop and report if the link fails, layout changes unexpectedly, reset/debug paths remain unresolved or parser support is insufficient. No speculative retries changing protection, clock, debug-return semantics, program reservation or application origin. A successful compiler link alone is not a successful safety audit.

This review does **not** perform or authorize that experiment. A proposed request would use XC32v6.00/DFP1.7.380, explicit retained compilation of the four rehearsal sources, `-Os -g -mno-mips16` with no LTO, and the documented `-mdebugger` option for the experimental local link. Preserve the baseline minimum stack1024/heap512 for that first inspection only; these values are not capacity qualification. Use a dedicated reviewed linker preserving the32KB prefix, KSEG1 data and explicit conditional debug reservations. No APPIO/semihosting, diagnostic inputs, generated initializers, production linker edits or automatic flag/profile fallback.

These are proposed experiment choices, not settings applied here. If this compiler mode is rejected or adds unexplained runtime/debug records, stop and report without switching debugger modes, replacing reset glue or relinking a differently scoped image. The native-map adapter may be tested/implemented from existing/synthetic shapes before the link; unsupported actual output is recorded rather than silently exempted. Approval must name the linker/format work and local artifact generation separately from every delivery or host action. The artifact is for local inspection only: no ZIP/image attachment, IDE project delivery, programmer connection, reset, settings change, live RAM inspection/edit, PAUSE, NVM/protection command or host trial.

A later application safety record still needs pinned policy identity and independent exact-hash startup/store/call/vector/memory review. Native-map parsing and audit JSON cannot authenticate a candidate-authored approval. Actual installed executive/configuration/Boot Flash/erase behavior and IDE reset/power/freeze/session settings, recovery and RAM trial permissions remain separate.

## Verification and review status

Evidence inspected: existing native map and ELF headers/sections/program headers/symbols, linker region/section definitions, compiled rehearsal object sizes, installed debug-return/software-reset objects, runtime archive member names and compiler target help. Read-only layout extraction is saved as an ignored research artifact under `build/debugger-link-review/`. No rehearsal link, HEX conversion, firmware rebuild or board access was performed.

Existing source/compile-only and parser checks repass: `make debugger-rehearsal-source-check debugger-rehearsal-image-test` verifies the unchanged sources/objects and46 synthetic rejection cases. Existing canonical artifact and negative audits repass without relinking, via `bun tools/audit-restart-image.ts` and `bun tools/test-restart-image-audit.ts`. Mechanical checks confirm31 PT_LOADs, four config sections, the observed752/200/552-byte vector segment split, native-map/runtime markers and relative links. Existing map/ELF/HEX hashes above and recovery HEX `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` are unchanged. Firmware inputs, settings and source diffs remain unchanged; `git diff --check` passes.

These checks do not verify the proposed native-map adapter or linker. A delegated broad map/runtime inspection and broader readiness review timed out; parent inspection supplied the concrete findings above. A completed narrow independent scope review finds no permission bypass or known-safe-runtime claim. This is bounded review only, not exhaustive format/runtime assurance. No actual host, rehearsal link or runtime correctness claim follows from this milestone.
