# RAM rehearsal local link — artifact generated, safety unsupported

One authorised local non-deliverable RAM rehearsal compile/link/HEX experiment completed on2026-10-23. The output preserves the canonical32KB prefix and emits the expected DEBUG-ON configuration, but it has no complete structural/startup/control-flow safety audit or installed debugger-executive evidence. A retained CRT `sdbbp` path and incomplete native-format support block delivery. The ELF/HEX stay local; only this report is offered for download.

The user approved the exact local scope at `2026-10-23T16:12:47.840Z`: dedicated linker/native-format work and one link using XC32v6.00/DFP1.7.380, `-Os -g -mno-mips16`, no LTO, `-mdebugger`,32KB placement and baseline stack/heap. No programming, PICkit connection, reset, settings change, live RAM access, image delivery or NVM operation was authorised or performed. The one experiment allowance is spent; no relink/fallback occurred.

## Implemented changes and attempt record

`rehearsal/linker/ram_rehearsal.ld` derives from the existing `bootloader/src/config/default/btl.ld` with provenance SHA-256 `28ca92bbe829fee84f36211a43d978bf9700f521c9549dae027e36b886928287`. It retains vendor region/vector/runtime/data machinery and adds assertions for the32KB program prefix,64KB uncached SRAM and zero `.persist` size. Existing production linker and firmware/project inputs are unchanged.

`tools/rehearsal-native-map.ts` adds bounded native XC32 inventory extraction; `tools/test-rehearsal-native-map.ts` tests wrapped output rows, empty wildcard declarations, repeated best-fit section names, region profiles and rejection cases. `make debugger-rehearsal-native-test` runs it without compiling/linking. This is partial native-format work: it does not convert native maps into the structural auditor's normalized schema, infer stack/heap bounds, cross-check all allocations with ELF or prove source/runtime selection. Its status is `inventory_only`, safety unsupported and delivery false. Empty declarations invent no address/range; archive references may include discarded contributors.

`tools/experiment-debugger-rehearsal.ts` performs the explicitly flagged local experiment, outside default Make targets and IDE projects. It records a durable exclusive-create attempt marker before compiler execution, retained source/header/linker hashes, compile/link command arrays, object hashes and logs. A failed compile/link stops without retry; an existing attempt is refused before compiler invocation. The dated flag and local record document the trusted operator workflow, not cryptographic approval or renewed authority.

The actual experiment started at16:21:26.027Z and completed its artifact generation at16:21:26.220Z. It retained four objects from `main.c`, `configuration.c`, `debugger_rehearsal.c` and `exception_stops.S`, with dependency files. Options additionally include `-fno-common -ffunction-sections -fdata-sections -Wall -Wextra -Werror`; link used the dedicated script, `--gc-sections`, `_min_stack_size=1024`, `_min_heap_size=512`, and a native map. No diagnostic/update/NVM source, LTO, APPIO option or programmer utility entered the explicit input list.

The attempt and local artifacts are under ignored `build/debugger-rehearsal-experiment-20261023/`. The exact linked linker bytes are preserved there as `linked-input-ram_rehearsal.ld`. Repository cleanup later removed inherited trailing whitespace only; its new hash is distinct and the artifact was not relinked. Provenance comparisons account for that documented source-only formatting change. Source-only and synthetic tests may still run; the experiment script cannot reuse that attempt directory. A repeat refusal was verified with EEXIST and an unchanged attempt record, without another compiler/link invocation.

## Actual artifact observations

| Local file | SHA-256 |
|---|---|
| `rehearsal.elf` | `cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a` |
| `rehearsal.hex` | `918d3cab3801a75860a17b2adfd3cd03590299886db5add8e600d1796670ce33` |
| `rehearsal.map` | `f6dc317709d4299e431f32f03575f1271f64f4523a8754bdacd0032b7eed4eec` |
| Exact linker used for experiment | `632cb125dca3cbd0b6a29d4e8902dfbbb75b485c980f63a0a83710fba1771354` |
| Repository linker after whitespace-only cleanup | `a0ad518fa3a48271d5a578cc555a87b559d38cbf92ec786478ea19000778c778` |

XC32 size reports text3728, data0, bss2756; these categories include runtime/reservation effects and are not a stack-capacity certificate. The ELF has24 program headers and entry `0xBFC00000`. The linker region remains `0x9D000000/32768`, with64KB KSEG1 data. No application origin/reservation enlargement or selected disposable page follows from the experiment.

Configuration bytes at physical addresses are:

- `1FC00BF0` DEVCFG3 `FFFFFFFF`;
- `1FC00BF4` DEVCFG2 `FFF97AFA`;
- `1FC00BF8` DEVCFG1 `FD74CE5B`;
- `1FC00BFC` DEVCFG0 `7FFFFFE0`.

These match the implemented rehearsal profile. They are compiler artifact observations, not live-board readback or evidence of the final debugger's installed bytes.

| Linked item | Observed placement |
|---|---|
| `_DEBUGGER` | Absolute value1 |
| `.dbg_excpt` | `BFC00480`,8 bytes, NOBITS reservation |
| `.dbg_code` | `9FC00490`,`0x760` bytes, NOBITS reservation |
| `.dbg_data` | `A0000000`,512 bytes, NOBITS reservation |
| `rehearsal_context` | `A0000200`,200 bytes |
| `rehearsal_mailbox` | `A00002C8`,148 bytes |
| `_heap` / minima | `A0000360`; heap512/stack1024 explicitly requested |
| `_stack` | `A000FFF8`; actual capacity/executive stack use not qualified |
| `RAM_RehearsalPreEdit` | `9D000278`,8-byte inert return/nop block |
| `RAM_RehearsalReadout` | `9D000288`,8-byte inert return/nop block |
| Shared application exception stop | `9D000290`, branch/nop loop |
| `.dinit` | `9D0010A0`,32 bytes |

The `.dinit` observations contain two clear0 records:148 bytes to mailbox `A00002C8` and200 bytes to context `A0000200`, followed by zero termination. Those destinations are disjoint and outside the declared debug-RAM reservation. This is bounded table inspection, not proof of all selected data-init routine effects, stack/heap reservation intervals or installed executive usage.

Disassembly shows one DI in main, effective-IE failure looping, one library Init call and repeated Tasks calls with no ordinary main return. The strong reset/bootstrap hooks are inert returns; general/bootstrap/NMI/default-interrupt destinations alias the non-resuming stop. Linked default vectors0–43 forward to that stop. Final processor/debugger breakpoint semantics are not tested merely because the intended instruction blocks exist.

## Retained runtime trap and audit stop

Unlike the existing production diagnostic, this `-mdebugger` artifact has no `__DbgExecReturn` or `__pic32_software_reset` symbol in its observed symbol inventory. That difference does not establish debugger safety or installed-executive equivalence.

It **does** retain weak `__exception_handler_break` at `0x9D0001F4`, containing `sdbbp` then return, and `__crt0_exit` at `0x9D001054`, whose post-main path calls the handler before looping. Main's visible normal path never returns, but that does not substitute for an exact-hash startup/control-flow reachability and fault-path review. The linked trap capability is not accepted, removed or silently exempted in this milestone. No altered-runtime relink, fallback debug mode or replacement trap/reset stub was attempted.

The compiler produced debugger NOLOAD reservations rather than actual executive bytes. MPLAB X/PICkit5 installed executive, debug vector/resume PC/Count/IRQ behaviour, boot/configuration changes and real memory usage remain unknown. IPE's historical Hold In Reset setting does not establish MPLAB X's setting. None of these gaps can be filled by changing approval fields in an audit JSON file.

Native-map inventory extraction succeeds for the actual rehearsal map, but the initial structural parser still does not support all real XC32 section/segment/configuration/debug-reservation formats. No candidate-authored normalized policy was fabricated to obtain a pass. Structural/native allocation verification, selected runtime/callback/constructor/store/call review and installed executive evidence remain unsupported. The native reader's inventory status is not a final image audit.

Therefore `application_safety_status=unsupported`, installed executive unknown and delivery/hardware/NVM approval false. Artifact generation succeeded; safety/delivery did not.

## Verification and bounded review

- `make test` and `make test-sanitize`: full strict/ASan/UBSan C suites with30 Python methods,46 synthetic structural/CLI rejection cases and native inventory/linker-provenance tests pass.
- Source/compile-only rehearsal and structural/native tests repass. Optional read-only native-map tests succeed on the existing canonical map and the newly generated local rehearsal map.
- Linker provenance/region/reset/vector/data definitions were compared against the unchanged baseline. A bounded independent source review found no substantive linker deviation or inventory approval mismatch; it did not certify runtime safety.
- A separate bounded read-only artifact review confirms the observed main/exception/table/reservation facts and flags CRT trap reachability/installed executive as future gates. No exhaustive MIPS or hardware proof was performed.
- Canonical/recovery HEX hashes remain `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7` and `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`. Existing firmware/project settings and source inputs are unchanged. `git diff --check` and document links pass.

Temporary native format findings were addressed with fixtures before the one link: empty declarations can lack allocation rows, wildcard inputs differ from output names, and best-fit rows can repeat names at distinct addresses. The reader preserves these inventory facts without inventing ownership. It remains distinct from the structural safety parser and raw map provenance is retained.

## Next gate

Stop here for this experiment. The allowance is spent and the image remains non-deliverable. Further work should first review the retained CRT trap's reachability and complete native ELF/map/reservation/runtime evidence under the existing artifact, with no relink or host actions. Any source/runtime change or additional artifact generation needs a separately scoped decision; do not patch away the trap or widen a parser rule solely to claim success.

A rehearsal delivery would still need exact-hash independent startup/control-flow/memory approval, real debugger-executive/settings/erase-scope evidence and separate install/recovery/host permissions. NVM page loss, timing/arming/settlement/protection and connected uploads are later distinct qualifications. The board remains canonical, running/read-only, PICkit5 disconnected and externally powered with settings unchanged.
