# Native RAM artifact parsing — structural subset, source-only

The native-format parser now checks the existing local RAM artifact without changing or relinking it. Independent native-shaped synthetic fixtures and44 targeted/CLI rejection cases pass; the original strict-parser suite remains intact. A native structural pass still reports startup/control-flow/application safety unsupported, installed executive unknown and delivery/hardware/NVM approval false.

The user requested the next source-only parser step after the [read-only runtime review](nvm-debugger-artifact-runtime-review.md). No target source, linker, configuration, executable, debugger settings or board state changed. No compiler, target link, HEX conversion, programmer connection, reset, RAM access, image delivery or NVM operation was performed. The earlier single local-link allowance remains spent.

## Separate native path and retained strict behaviour

`tools/rehearsal-native-image.ts` provides `auditNativeRehearsal()` and an explicit-file CLI. It shares bounded ELF/HEX/file primitives with `tools/rehearsal-image.ts`, but enables native-format handling only through an explicit native options path. The original `auditRehearsalImage()` and CLI retain their strict normalized-map/section/zero-tail policy. Their existing46 rejection cases repass; native-shaped fixtures are rejected by the old strict mode.

`tools/test-rehearsal-native-image.ts` builds an independent synthetic native ELF/HEX/map fixture and mutants without invoking any compiler or linker. Its optional directory argument runs a read-only, hash-asserted regression on the existing local artifact. `make debugger-rehearsal-native-image-test` and `make test` run synthetic cases only; the normal test flow does not depend on an artifact directory.

The native CLI requires `--elf`, `--hex`, `--map`, `--policy` and `--expected-policy-sha256`, with no defaults. Policy bytes are bounded and checked against the separately supplied pin before other input files are read. The CLI prints a result and has no build, hardware or approval-setting operation. Native ELF and map inputs stay bounded, section/name/header arithmetic is checked, and native symbol tables are capped at4096 entries. Strict mode keeps its earlier1024-symbol cap.

## Explicit format rules

### Segment gaps and permissions

A versioned `pic32mx270-native-format-policy-v1` supplies a provenance label, non-overlapping gap intervals and an exact data-init tail rule. Gaps may be `zero-file` or `no-file`, lie only in the canonical32KB code prefix or pre-debug Boot Flash span, and must match observed segment gaps exactly once. File gaps must contain only zero bytes; memory-only gaps must lie beyond the segment's file extent. Unknown, unused, overlapping or boundary-crossing declarations are rejected.

A PT_LOAD cannot consist solely of a declared gap: it must own mapped allocated sections. File-backed section file-offset/address relationships must agree with their unique segment, and all section/file/memory intervals are checked for overlaps/aliases. Every delivered HEX byte must match exactly one allocated file-backed ELF byte. No HEX data in a gap, extra segment reusing another segment's file bytes, rogue SRAM load or KSEG2/cached-SRAM alias is exempted.

Segment permissions must equal the union of their contained allocated sections. This rejects extra permissions, including a writable-executable code-only segment. The existing Boot Flash segment legitimately has RWE flags because it contains RX reset code and the fixed WA-flagged NOLOAD debug-vector reservation; that representation is accepted only under the explicit section identities. ELF permissions do not establish physical Flash writability, runtime store safety or debugger behaviour.

### Metadata, empty sections and identities

The native path recognizes the reviewed24-byte `.MIPS.abiflags` processor-specific section type `0x7000002A` with exact metadata bytes, flags and alignment. Those bytes describe the observed MIPS32r2/soft-float/ASE metadata shape. A match provides no independent device-frequency, executable instruction compatibility or ISA hardware qualification. Unknown section types or metadata values fail.

XC32 best-fit allocation can repeat `.text` and other section names. Native handling retains section indices and checks nonempty identity by name/address plus physical interval ownership; duplicate allocated identities and aliases fail. Config/data-init/debug critical identities still require exact unique layouts. Zero-length allocated sections own no bytes and cannot create a mapping exemption. Empty native init/preinit/fini-array types are recognized only by the correct names; introducing callback bytes or unknown flags/type combinations is rejected.

Four configuration sections must match exact `.config_BFC00BF0/BF4/BF8/BFC` identities,4-byte sizes/locations and physical DEVCFG3/2/1/0 values `FFFFFFFF/FFF97AFA/FD74CE5B/7FFFFFE0`. The parser programs nothing and supplies no live readback or executive-installed fuse evidence.

### Debug and SRAM allocation

Three fixed NOBITS debug reservations are recognized: `.dbg_data` at `A0000000`,512 bytes; `.dbg_excpt` at `BFC00480`,8 bytes; `.dbg_code` at `9FC00490`,`0x760` bytes. Extra or malformed debug sections, executable SRAM and allocated physical overlap fail. Their known linker flags are structural representation only; contents and actual installed executive requirements remain unknown.

The current native profile requires defined correctly typed200-byte `rehearsal_context` and148-byte `rehearsal_mailbox` objects, aligned and bounded in uncached NOBITS SRAM outside declared debug RAM. Main/reset/rendezvous code symbols must be defined, contained in executable sections, disjoint and in their permitted program/reset domains. A symbol name/value alone cannot supply a valid object/code definition.

`_DEBUGGER`, `_heap`, `_eheap`, `_splim`, `_stack` and minimum-size symbols require absolute NOTYPE definitions and valid values/order. The heap is exactly512 bytes; minimum stack1024 remains an inspection profile. Allocations and initialization must avoid the explicit heap and conservatively treated stack-symbol interval through `_stack+8`. Symbol-based bounds do not prove stack capacity, executive usage, hardware stack enforcement or provenance. No stack/heap interval is fabricated solely by subtracting a minimum size from RAM top.

### Initialization and post-terminator tail

The native `.dinit` path supports positive clear0 records only; copy/decompression opcodes are deliberately unsupported. `_dinit_addr` must be absolute or defined in the actual `.dinit` section, with the correct type and value. All destinations must be covered by allocated uncached SRAM, pairwise disjoint, avoid debug/heap/stack intervals and cover both objects for zero initialization. A containing shared clear record remains legitimate.

A zero destination terminates records. The exact remaining tail must match the pinned policy, with only no tail, four zeros or four `0x66` bytes recognized. The existing artifact regression selects `66666666` based on the previous exact-file inspection; this is a format rule, not proof of why the linker emitted it or of every runtime decoder. Arbitrary additional tail bytes or changed patterns fail. The strict original parser continues to require zero tails.

## Native map consistency and missing provenance

The bounded native-map inventory reader is used directly. Every nonempty allocated ELF section must match exactly one native-map name/address/size row; every nonzero native-map output row must correspond to an ELF section. Repeated best-fit names and empty declarations are handled without inventing addresses. Raw map hash is reported in the result.

These checks improve allocation consistency, but do not authenticate the raw map or reconstruct source/header/object provenance, archive member selection, runtime callback bodies or full linker semantics. `LOAD`/archive inventory and policy provenance labels cannot replace the retained command/input records or an independent exact-hash manual review. The native path has no source-manifest/manual-review acceptance API and cannot claim a complete standalone safety audit.

A matching policy pin supports reproducibility for a trusted caller. It is not authentication or an operator approval: a caller can fabricate policy bytes and their expected hash. The existing-artifact test's policy is a test-only layout description fixed by known addresses/hash assertions; it is not a new delivery policy or an approved safety record. Policy matching is reported as `pin_matched_not_approval`.

## Verification and review scope

The independent synthetic fixture exercises four configs, fixed debug reservations, native BSS flags, ABI metadata, explicit zero-filled/memory-only segment gaps, clear records/tail and linker metadata.44 rejection/CLI cases cover changed gap data/rules/bounds/kinds, unknown ABI/type/flags, empty callback sections, config mismatch, bad/overlapping initialization, wrong object/heap/stack definitions, required symbol aliases, gap-only rogue segments, permission over-grants, forged linker symbol definitions, map mismatch, bad pins, extra approval fields and CLI missing arguments. Cases assert targeted rejection messages rather than relying on stale artifact hashes or checksums.

The optional unchanged-artifact regression asserts these exact hashes before native parsing:

- ELF `cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a`;
- HEX `918d3cab3801a75860a17b2adfd3cd03590299886db5add8e600d1796670ce33`;
- Map `f6dc317709d4299e431f32f03575f1271f64f4523a8754bdacd0032b7eed4eec`.

It then checks3,728 delivered bytes,79 nonempty allocated sections and native structural status while asserting unsupported safety and false delivery. The local artifact remains ignored and unattached. Native policy/layout fixtures are newly tested; no image is regenerated.

A bounded review found segment permission over-grants and linker metadata validated only by value. Exact permission unions and absolute/section-linked symbol-definition checks fix those gaps; targeted regressions pass. A larger follow-up timed out, then a completed narrow follow-up confirms those two structural fixes while retaining provenance/safety limits. These reviews provide no exhaustive parser or general MIPS proof.

Full host `make test` and `make test-sanitize`, the unchanged original46-case parser suite,44 new native cases and the read-only artifact regression pass after the reviewed changes. Final source/diff/document checks and unchanged artifact/firmware hashes are recorded before commit. No target compiler/linker or board utility is used in this milestone. Existing canonical/recovery HEX hashes and the linked rehearsal artifact remain unchanged.

## Remaining approvals

This native structural subset removes specific format incompatibilities; it does not resolve the full safety gates recorded in the [runtime review](nvm-debugger-artifact-runtime-review.md). Startup/data-init executable semantics, all reachable stores/calls, CRT trap/exception assumptions, source/object/runtime provenance, installed executive, effective IDE settings and erase/recovery behaviour need independent exact-hash evidence.

The parser always leaves `startup_status`, `control_flow_status` and `application_safety_status` unsupported, installed executive unknown and delivery/hardware/NVM approval false. No API or manifest field can set them to a safety/delivery PASS. A native structural result is not an image-install or debugger-token permission.

No relink, new target image, source/fuse/runtime change, programmer connection, reset or host RAM trial is authorised by this parser work. The single prior local link permission remains spent. Keep the canonical board running/read-only, PICkit5 disconnected and external power/settings unchanged. The A1 concern, exact NVM page approval, timing/arming/settlement/protection and connected uploads are separate later qualifications.
