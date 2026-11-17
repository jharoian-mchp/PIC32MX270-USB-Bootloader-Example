# Source-managed RAM rehearsal project

`RAM_Rehearsal_Managed.X` is a separate managed source project (configuration type 2), based on the existing application's XML schema. It contains only four RAM-rehearsal translation units, three headers and the dedicated linker script. Original application and bootloader projects are unchanged.

## Offline result — 2026-10-29

One authorised compile/link passed with XC32 v6.00 and DFP 1.7.380. The external `Makefile.offline` records the Debug-profile build and refuses a repeated `approved-once` operation. No MPLAB IDE or board tool was used.

- ELF SHA-256: `24c75baa4c46e275ca29db3be4baa0891b19090b3d4c448a0f4351eb117b0a4c`
- HEX SHA-256: `918d3cab3801a75860a17b2adfd3cd03590299886db5add8e600d1796670ce33`
- All 3,728 delivered HEX bytes match the retained rehearsal.
- Allocated ELF section bytes and layout match the retained rehearsal, including clear records and debugger reservations.
- Reset: `BFC00000`; pre-edit: `9D000278`; readout: `9D000288`.
- Context: `A0000200`, 200 bytes; mailbox: `A00002C8`, 148 bytes.
- Source/entry mock and compile checks, strict image negatives and native image tests passed.

The new ELF differs in non-loadable/debug metadata. HEX equality does not establish Windows IDE output equality, debug-executive safety or memory-edit eligibility. Previous target ABI-metadata omission remains a recorded readback difference.

## MPLAB X v6.35 verification still needed

Opening the project offline should cause MPLAB to generate its normal `nbproject/Makefile-*.mk` files. Those generated files are intentionally not fabricated or copied from an unrelated project. The conventional root Makefile depends on them and cannot run until generation.

Review the generated Debug commands before any new build or board approval:

- Only `../src/main.c`, `configuration.c`, `debugger_rehearsal.c`, `exception_stops.S`.
- Compiler XC32 v6.00; device PIC32MX270F256D; DFP 1.7.380.
- C optimisation `-Os`, symbols enabled, normal MIPS32, function/data sections, no LTO, no-common; rehearsal header include path only.
- Dedicated `../linker/ram_rehearsal.ld` selected as the linker script, stack 1024 and heap 512.
- Vendor device startup **enabled** (`no-device-startup-code=false`). The original application used its own startup arrangement; the standalone rehearsal requires the vendor CRT. This is intentional.
- Normal IDE Debug definitions such as `__DEBUG`, `_DEBUGGER` and `__MPLAB_DEBUG`, plus PICkit-specific flags: verify what v6.35 actually generates. The offline build used `-D__DEBUG -mdebugger` and linker definitions `_DEBUGGER=1`, `__MPLAB_DEBUG=1`. IDE equivalence is untested; no extra invented flags have been forced into the XML.

Saved tool selection is PICkit 5, startup Halt at Reset vector, reset Reset Vector, target power unchecked, software breakpoints false and FRC-in-debug false. Per-peripheral freeze selections are carried from the template and require explicit review against the operator's intended settings. Erase-before-program and other programming settings are not hardware permission. Device configuration comes from the rehearsal source and must be reviewed with the generated image.

Do not alter the active halted debugger session to open or test this project. Use a separate offline/tool-disconnected instance, and do not set this project as the active main project of the live session. If separation is unavailable, wait for a separately authorised session-end decision.

**No Windows build, Debug start, programming, RAM write, reset, IPE handoff, recovery or session termination is authorised by this package.** The approved single offline build allowance is spent. Send generated Makefile rules and project properties for review; do not build again to test them without approval. A future Windows-generated ELF is a new artifact until checked.

## Read-only package verification

The delivery ZIP includes `SHA256SUMS.txt` for its source/project files and offline artifacts. The source copies preserve the original Microchip notices. `ORIGIN.json` identifies the template and scoped approval. The existing verified rehearsal package/ELF is not replaced.
