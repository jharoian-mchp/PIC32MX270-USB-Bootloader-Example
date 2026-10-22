# Canonical 32 KB candidate review — 2026-10-22

The `09da506` candidate passes this bounded offline build, startup, artifact and host-compatibility review. No replacement archive was created or delivered. This review authorises no programming, reset or PAUSE. The board stays running/read-only on the successful experimental 48 KB image; that single-trial permission is spent.

## Candidate identity

| Item | Reviewed value |
|---|---|
| Firmware/host/build source | `09da506e0ce228015c358f99415e08b68f6a19a0` |
| Local candidate HEX SHA-256 | `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7` |
| Known normal recovery HEX SHA-256 | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |
| Toolchain | XC32 v6.00, DFP1.7.380, `-Os -flto -fno-data-sections` |
| Reservation / application origin | 32768 bytes / `0x1D008000` |
| ELF text/data/bss | 32284 / 88 / 2052 bytes |
| Program-region usage | 31876 / 32768 bytes; 892 nominal bytes unused |
| Retained journal | Schema4,128-byte NOBITS at `0xA0000000`, excluded from startup `.dinit` |
| Queries | PREVIOUS and power-previous30-byte payloads, unchanged |
| Configuration words | `FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3`, matching default recovery |
| Audit flags | `hardware_qualified=false`, `board_delivery_approved=false` |

These are local review artifacts, not delivered firmware. The [restoration](usb-restart-32kb-restoration.md) changes only the opt-in build flag; runtime sources, diagnostics, wire formats and predicates are unchanged from the previously successful experimental source. Normal recovery and historical48KB build flags are unchanged.

## Completed checks and review limits

- Rebuilt `make restart-image-audit` and reran `bun tools/test-restart-image-audit.ts`. Canonical Flash spans, ELF/HEX equality, configuration, required symbols, excluded update/NVM-write/handoff entrypoints and retained startup placement passed, as did negative artifact regressions.
- Audited actual copied ELF/HEX in a disposable directory. Hashes,32KB/origin1D008000,128-byte journal and false hardware/board flags matched. Copies were removed afterwards.
- Checked the four actual `.dinit` destination spans for mutual overlap: copies atA0000080/5 bytes andA0000168/80 bytes, clears atA0000088/221 bytes andA0000200/1700 bytes. They are disjoint and outside the retained journal. The generic audit does not enforce every possible pairwise initialization overlap; this candidate's inspected table passed that separate check.
- Direct source comparison from `f1d9e3c` to this candidate found no runtime changes under `bootloader/src`, `host`, `tools` or `tests`. Full strict/sanitizer tests and enabled XC32 checks passed during restoration; no code changed during this review.
- Offline host fixtures accept the canonical capabilities with plain `probe` and reject the mismatched debug profile. Old BUSY and new power-off rejection labels remain valid. Both30-byte prior replies distinguish absent records, valid identity without samples and historical power evidence. No HID command was sent to the board.
- A bounded independent build/startup review found no must-fix in supplied scope. It confirmed target isolation and the listed disjoint startup spans while explicitly withholding hardware/runtime-equivalence qualification.

`-fno-data-sections` reduces data-section splitting and startup metadata but changes garbage-collection granularity, SRAM placement and initialization timing. Source equality is not binary or hardware equivalence. The earlier48KB [single successful cycle](usb-harmony-powered-first-cycle.md) does not qualify this image. No repeat cycle, electrical safety, old-reference provenance, retention or NVM/protection/upload qualification is inferred.

## Concern and unchanged safety boundaries

The Harmony-powered decision still requires USBPWR1 and records BUSY rather than rejecting it during powered reconstruction. All software, IRQ, USBEN, queue/BDT and full-stack readiness checks remain. Retirement/preparation and NVM ownership still require USBPWR0/BUSY0 and their independent exclusion/ownership conditions.

The [PIC32MX270 A1 powered-BUSY concern](usb-harmony-powered-readiness.md) remains open. A smaller link and a previous successful cycle do not establish a manufacturer clarification or universal safe interpretation of BUSY. The proposed package must retain this concern explicitly.

## Requirements for a separately approved initial-only package

Obtain explicit approval before preparing/delivering or programming a replacement. Use a new directory and exact candidate/recovery hashes, source/procedure identification, manifest, all-file checksums, copied-artifact audit and extracted host verification. The manifest must identify **32KB/origin1D008000**, not the older48KB layout. Do not use older HEX or procedure files merely because the schema is compatible.

Programming must be confined to the reviewed diagnostic bootloader/boot/configuration content. The canonical image no longer extends beyond1D008000, but programmer erase scope may exceed HEX spans. The prior no-preservation confirmation for the old overlap interval does not approve unrelated erase or newly changed user data. Preserve protection, external power and board settings; review IPE's reported scope and stop on unexpected ranges or errors without retries.

After approved programming and one bounded startup with PICkit5 disconnected and independent supply preserved, collect IPE verification and five initial outputs: list, **plain `probe --timeout 2` without `--power-debug`**, STATUS, PREVIOUS and power-previous. Require exactly one expected interface before queries and explicit native-command exit checks. A debug probe intentionally rejects the canonical layout; do not use that failure as retry permission.

Schema4 is unchanged, but data coalescing can move ordinary SRAM state and programming/startup may invalidate retained evidence. A valid historical prior record or valid:false is possible. Report either unchanged; a matching ticket/generation/time does not authenticate a trial, and an older record is not evidence that this canonical candidate restarted. No reset solely to capture a current successful journal is requested.

A PAUSE decision follows only after initial results and separate explicit approval. This review grants no new trial, fault test, upload, NVM/protection change, reset or programmer action. Keep the current board running/read-only, PICkit5 disconnected and external power/settings unchanged.
