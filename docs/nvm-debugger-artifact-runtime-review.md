# Exact RAM artifact runtime and native coverage review — read-only

For the existing RAM rehearsal artifact, normal reset/main execution has no path back to the CRT's `sdbbp` return loop under the documented control-flow assumptions. The delivered data-init records dispatch to the clear routine, not the trap. This resolves the ordinary caller question within a bounded review; it does not establish a complete application safety pass, debugger compatibility or delivery approval.

This review follows [the one local link result](nvm-debugger-local-link-result.md) and user `next` at2026-10-23T17:46:26.641Z. It reads the existing ELF/HEX/map/disassembly and current source only. No compiler, link, HEX conversion, firmware/source/parser patch, programmer connection, debugger command, reset, RAM access or board operation occurred. The single earlier artifact-generation permission remains spent. The image stays local and non-deliverable.

## Artifact identity and review method

| Existing local artifact | SHA-256 |
|---|---|
| ELF | `cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a` |
| HEX | `918d3cab3801a75860a17b2adfd3cd03590299886db5add8e600d1796670ce33` |
| Native map | `f6dc317709d4299e431f32f03575f1271f64f4523a8754bdacd0032b7eed4eec` |

Files remain in ignored `build/debugger-rehearsal-experiment-20261023/`. An artifact-specific read-only cross-check of their contents produces `read-only-review/evidence.json`. It is not the production structural auditor, has no general acceptance API and does not create an approved policy or normalize data into a safety PASS. Output records unsupported application safety and false delivery approval.

The check asserts exact hashes before parsing, verifies section-to-load file/address relationships, delivered bytes, native-map allocation rows, physical non-overlap, data-init/table contents and selected symbols. Independent `xc32-objdump`, `xc32-nm` and `readelf` observations confirm the relevant code, data and ABI fields. A completed independent bounded trace covers reset/main/CRT callers; it explicitly excludes general machine-code proof and installed executive behaviour.

## Ordinary CRT trap reachability

| Linked control point | Evidence |
|---|---|
| `_reset` `BFC00000` | Calls `_startup` `BFC00008`; delay slot is nop |
| Startup NMI branch | Nonzero NMI condition routes to the shared stop `9D000290` before stack/BSS work |
| `_on_reset` / `_on_bootstrap` | Calls target inert return/nop bodies `9D000268` / `9D000270` |
| Data-init call | Startup calls `__pic32_data_init` `9D000E88`, resolved below |
| RAM-function/BMX setup | Linked branch tests literal `_ramfunc_begin=0` and skips the three retained BMX stores at `BFC00078/8C/A0` on this normal path |
| `_main_entry` `9D001020` | Initializer pointer materializes as0 and bypasses its indirect call; relevant constructor/init arrays are empty in the map |
| Main call `9D00104C` | `jalr t0` targets `main` `9D000F70`; return continuation would be `9D001054` after delay slot `9D001050` |
| `main` | One DI and IE check; IE failure spins at `9D000F9C`; accepted path calls Init once then Tasks repeatedly via `9D000FB8` and unconditional branch `9D000FC0` |
| `__crt0_exit` `9D001054` | Materializes handler `9D0001F4`; indirect call at `9D001064`, delay slot `9D001068`, return continuation `9D00106C`, then loops |
| `__exception_handler_break` `9D0001F4` | Contains `sdbbp`, then `jr ra; nop`; the trap remains in the delivered image |

MIPS jump-and-link return addresses are PC+8, after the delay slot. The correct return address for the trap call at `9D001064` is `9D00106C`, not the delay-slot address `9D001068` mentioned in an intermediate delegated trace. The exact disassembly supplies that correction.

There are seven `jalr` sites in the delivered disassembly: three startup calls, the data-init dispatch, the zero initializer call, main call and CRT trap call. Their targets are resolved above or below under this artifact's normal path. No direct call/jump to the trap from the rehearsal path was found. A word scan of delivered non-executable allocated data finds no literal `9D0001F4` pointer; that scan alone cannot exclude computed pointers or corrupted execution state.

The strongest bounded conclusion is conditional: with normal reset entry, intact code/data/stack/return state, the current library's trusted serialized storage contract, and no debugger redirect/skip, main never returns to `__crt0_exit`. Standard NMI/bootstrap/general/default vector paths stop rather than branch to the trap. The trap is retained fallback code; its presence alone is not evidence that this program executes it normally. It also cannot be dismissed for every possible fault or executive-induced redirect.

Unknown installed executive actions, altered PC/return addresses, stack or instruction corruption, unexpected exception state and manually skipping instructions are outside that conclusion. No trap removal, source change, runtime override or relink was performed. The original no-resuming-failure design still requires an explicitly accepted exact-hash control-flow/exception policy before any application safety approval.

## Linked data-init dispatch and corrected tail evidence

The delivered `.dinit` section is32 bytes at `9D0010A0`, matching `_dinit_addr`. It contains:

| Record offset | Destination | Length | Operation |
|---|---|---|---|
| 0 | `A00002C8`, mailbox | 148 | clear0 |
| 12 | `A0000200`, context | 200 | clear0 |
| 24 | Zero destination | — | Terminates decoding |
| 28–31 | `66 66 66 66` | 4 | Bytes after the terminator |

The post-terminator bytes are an important native-format finding. Earlier reports recorded the zero destination and two records without accounting for the tail. The whole section is **not** zero-padded after termination. These four `0x66` bytes match both ELF and delivered HEX. Their native producer/padding semantics are not generalized by this review. The initial structural parser requires zero tails and therefore remains incompatible with this section. No parser rule was changed to accept it.

`_dinit_func_table` is delivered `.rodata` at `9D000248`,20 bytes. Its five little-endian targets are `9D000228`, then four entries `9D000E80`. Opcode0 selects `__dinit_clear` at `9D000228`; the other four targets alias inert dummy return/nop decoders. Both actual records encode zero in the field read by the linked decoder's signed-halfword load, so they select entry0. Unknown/nonzero operations would require separate analysis; none occurs in these bytes.

The decoder saves startup return state in a register, loads destination/length/op, calls the table target at `9D000ECC`, aligns the next record pointer and stops on a zero destination. The clear routine uses a positive byte count and stores zero across each specified destination. Thus these two records clear the disjoint context/mailbox spans and do not dispatch to the CRT trap. The decoder does not traverse the four bytes beyond the zero destination on this path.

## Native allocation and delivered-byte coverage

The artifact-specific checks establish:

- All3,728 HEX data bytes match file-backed allocated ELF sections and a uniquely congruent PT_LOAD mapping. No unexplained HEX byte, missing allocated byte, overlapping delivered byte or physical application/metadata/scratch-page load was found. NOLOAD reservations are not compared to delivered HEX.
- All79 nonempty allocated ELF sections have matching name/address/size rows in the native-map inventory. Matching allocation facts do not reconstruct complete source/runtime provenance or certify that every map-only row is a live allocation.
- Nonempty allocated section physical intervals are disjoint, including the debug reservations and application SRAM. Zero-sized sections own no bytes.
- `.dbg_data` is `A0000000..A0000200`; context is `A0000200..A00002C8`; mailbox is `A00002C8..A000035C`. The clear records cover the two ordinary-BSS objects and do not touch the declared debug-RAM reservation.
- `_heap=A0000360` and `_eheap=A0000560` give an explicit512-byte heap interval, rather than inferring it from the requested minimum. The additional `_splim=A0000578` and `_stack=A000FFF8` symbols are recorded; they do not establish worst-case stack use, dynamic debug-executive needs or a hardware stack-overflow guarantee.
- Configuration bytes remain exact DEVCFG3/2/1/0 `FFFFFFFF/FFF97AFA/FD74CE5B/7FFFFFE0`. They supply no live readback or debugger-installed configuration evidence.

The file-backed segment at `9D0002A0` covers1,224 file bytes with912 bytes outside its allocated file-backed sections. The reset/Boot Flash segment covers912 file bytes with564 such gap bytes; its larger memory extent also includes declared debug-vector reservation/gaps. These holes are not missing section data or delivered HEX. They remain incompatible with the strict initial parser's per-segment full-tiling rule. A future parser implementation must prove gap classification without allowing unrelated segment offset reuse or arbitrary rogue loads.

Another discovered section is `.MIPS.abiflags` at `9D000208`,24 bytes, ELF processor-specific type `0x7000002A`, allocated and non-executable. Its bytes are delivered and match ELF. `readelf -A` decodes ABI version0, MIPS32r2,32-bit GPR/soft-float and advertised DSP/DSP-R2/MCU ASE flags. This metadata is not itself proof of executable instruction compatibility or hardware features used. The initial parser's allocated-PROGBITS/NOBITS-only subset rejects this type. Broader type/ABI support requires explicit positive/mutant tests and device/ISA review; no parser relaxation occurred here.

## What remains unsupported

This evidence improves the exact artifact review but does not complete the native structural parser or the final manual safety contract. Remaining gates include:

- The subsequent [source-only native structural parser](nvm-debugger-native-parser.md) adds tested handling for four config sections, pinned file/memory gaps, ABI metadata, empty sections, fixed NOLOAD debug reservations and the explicit data-init tail. Its native result still cannot supply startup/control-flow/application safety or delivery approval; source/runtime provenance and installed executive evidence remain outside its scope.
- Exhaustive reachable-store/call and callback/runtime review, stack-frame and source/object/runtime provenance validation, and independent exact-hash acceptance of the conditional CRT trap reachability conclusion. Intended pointer writes in library code still depend on valid trusted objects/stack; symbol absence is insufficient for arbitrary SFR-store safety.
- Installed MPLAB X/PICkit5 executive contents/reservations, actual debug exception/resume semantics, Count/IRQ effects, effective settings, programming/erase scope and recovery evidence. The file contains debugger reservations; it does not contain the executive that would establish those behaviours.
- Separate profile/image/install/host permissions. No local ELF/HEX is delivered, no host test is approved, and no NVM/protection qualification follows from this RAM-only review.

Therefore application safety stays unsupported and delivery/hardware/NVM approval stays false. Normal-path trap unreachability is a bounded conditional finding, not a full-image approval. The board remains canonical and read-only, PICkit5 disconnected and external power/settings unchanged. The one link allowance remains spent.

## Verification scope

The read-only artifact checker passes after explicitly recording the processor-specific ABI section and the nonzero `.dinit` tail instead of pretending the initial parser supports them. Hashes bind every finding above to the same ELF/HEX/map; the generated evidence records unsupported safety and false delivery. Intermediate review-helper path/type/zero-tail assertions failed and were corrected as observations, without changing target artefacts or production parser rules.

A completed independent bounded reset/main/trap trace agrees with the normal-loop conclusion, and parent inspection resolves its previously uninspected data-init table plus corrects the MIPS return-address detail. A second bounded report/evidence review finds no concrete must-fix overclaim in its scope. No exhaustive formal MIPS or debugger-runtime proof is claimed. Documentation links/diff checks and unchanged artifact/firmware hashes pass. No compiler, linker or board tool is invoked for this review.
