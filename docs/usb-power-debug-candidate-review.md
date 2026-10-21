# Enlarged power-observation candidate review — 2026-10-21

The `2de2f84` candidate passes this bounded offline source, layout and artifact review. No replacement archive was created or delivered. The user's temporary size-growth approval permits the debug build; this review grants no programmer operation, reset or PAUSE trial. Keep the board recovered/read-only, with PICkit5 disconnected and external power/settings unchanged.

The current board runs the older callback-mask image. Its retained DRIVER_READY/usb_busy1024/observations3 failure remains unresolved. The candidate adds observations, not a BUSY wait or readiness correction.

## Candidate identity and layout

| Item | Reviewed value |
|---|---|
| Firmware/host source | `2de2f84` |
| Local debug HEX SHA-256 | `310fc0ded96480df67e95b3b3c8dd4946792c31e9a468098bac116f125a0d35f` |
| Known default recovery HEX SHA-256 | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |
| Toolchain/profile | XC32 v6.00, DFP1.7.380, `-Os -flto`, explicit power-debug macros |
| Diagnostic program reservation | 49152 bytes / 48 KB |
| Diagnostic application origin | `0x1D00C000` |
| ELF text/data/bss | 33241 / 88 / 2052 bytes |
| Program-region map usage | 32833 / 49152 bytes |
| Retained journal | Schema4, 128-byte NOBITS at `0xA0000000`, excluded from linked `.dinit` |
| Queries | PREVIOUS0x68/30-byte payload; power-previous0x69/30-byte payload |
| Configuration words | `FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3`, matching default local image |
| Audit flags | `hardware_qualified=false`, `board_delivery_approved=false` |

These hashes identify local review artifacts, not a delivered package. The normal linker still reserves32768 bytes, with origin1D008000 and default footprint26670/80/1568. The enabled schema4 image does not fit that reservation. Returning diagnostics to32KB remains required after the USB issue is understood.

## Completed checks and review scope

- Rebuilt `make power-debug-image-audit` and reran `bun tools/test-restart-image-audit.ts --power-debug`. The debug image and default recovery pass their scoped ELF/HEX/configuration checks. Negative tests reject corruption, unsafe loads, undefined required symbols, startup writes overlapping the entire journal (including its added final32 bytes), and acceptance of the expanded candidate by the normal32KB audit.
- Independently copied the candidate ELF/HEX into a disposable directory and passed those actual copies through the debug artifact audit. Their hashes, layout metadata,128-byte journal and false approval flags matched. Copies were removed afterwards.
- Inspected PRE/POST driver call sites and READY's reused original power sample. The rebuild initializer disables global interrupts; the owner holds exclusion through rebuild and journal recording. No readiness outcome, ordering, USB-off NVM predicate, register write or wait was added by the instrumentation.
- A bounded independent review of retained recording, latching, metadata and host query validation found no concrete must-fix within supplied files. That review did not independently inspect call sites or linker placement; local source/preprocessor inspection and artifact checks cover those areas. It supplies no hardware approval.
- Target-preprocessed normal and debug profiles: application-start, jump and reported physical origin agree within each profile. Normal remains1D008000; debug is1D00C000. Offline host checks show normal/debug capabilities reject each other, and the normal image planner is unchanged.
- Offline power-query checks distinguish no record, valid identity with no samples/time0, and complete PRE0/0, POST1/1, READY1/1 snapshots. The two replies share boot-captured identity/time only, not reset-safe authentication or a sequence join.

Implementation tests and sanitizer/XC32 verification are recorded in the [implementation notes](usb-power-observation-implementation.md). No firmware changes occurred during this candidate review. The default recovery image hash is unchanged, and application handoff/update/NVM-write entrypoints are excluded from the diagnostic image.

The host utility includes a generation-bound PAUSE command as well as read-only commands. A future initial-check procedure must explicitly prohibit PAUSE; possession of that command or a valid retained record provides no authority to run it. No HID command was sent to the board during this review.

## Overlap and preservation gate

**The debug reservation extends into the former application range `0x1D008000..0x1D00C000` (end exclusive).** A programmer installing this image can overwrite application code/data there. Programmer erase settings may affect additional flash; the artifact audit does not certify the programmer's selected erase scope.

Before offering a programming procedure, establish whether application code or other data in the affected flash must be preserved. If it does, confirm an existing verified recovery artifact or obtain a separately approved backup. Do not read/erase/reprogram the board just to discover or back up contents without authorisation. If nothing there needs preserving, record the user's confirmation. Neither known capabilities-only recovery firmware nor the source LED baseline alone proves a backup of current board data.

Any future approval must explicitly accept the temporary48KB diagnostic layout and its overlap, identify the exact diagnostic/recovery files, and bound programming plus initial checks. Keep protection and supply settings unchanged. Review the actual programmer operation/erase scope before use, and stop on any unexpected target or memory-range report.

## Subsequent overlap-aware initial approval — 18:42

The user confirmed no application code/data in the overlapped interval needs preserving and approved temporary48KB programming plus list/probe/STATUS/PREVIOUS/power-previous only. The [initial-check procedure](usb-power-debug-initial-check.md) defines that later limited scope. No PAUSE is authorised. Earlier no-delivery statements describe this review's cutoff; erase/programming outside the stated scope still requires separate approval.

## Requirements for a future package

A separately approved package must use a new directory and matching `2de2f84` firmware/host sources, exact copied-artifact audit JSON, manifest identifying the48KB reservation/origin1D00C000/schema4/128-byte journal, checksums, known recovery HEX and an overlap-aware procedure. Verify all archive entries, extracted bytes and host tests before attaching it. Do not reuse an older32KB manifest or source identification.

Proposed initial outputs are IPE programming/verification and five diagnostic results: list, explicit `probe --power-debug`, STATUS, PREVIOUS and power-previous. Require exactly one expected HID interface before querying. Stop and report any error without reset/programming retries or automatic recovery. A PAUSE trial requires a later separate decision after those outputs; older trial permissions are spent.

Schema4 rejects older retained layouts, so initial PREVIOUS/power-previous may have no validated record. Programming and changed SRAM layout can invalidate evidence. Do not erase or manipulate SRAM to force a desired result; preserve the earlier reported failure separately. Valid records remain historical evidence, not new-candidate success.

Three samples and raw Count deltas cannot establish BUSY duration, a settling deadline, transient-versus-stuck behavior or successful USB-only restart. Reads/journal work perturb timing. No new package, board programming, PAUSE, NVM/protection change, upload or fault test is authorised by this report.
