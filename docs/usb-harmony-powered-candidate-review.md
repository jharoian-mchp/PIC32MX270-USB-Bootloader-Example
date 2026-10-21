# Harmony-powered candidate review — 2026-10-21

The `f1d9e3c` candidate passes the bounded offline source, compatibility and artifact review. No replacement archive was created or delivered. This review grants no programming, PAUSE, reset or other board action. The PIC32MX270 A1 powered-BUSY concern remains explicitly open.

The user directed the source policy on 2026-10-21 at21:16: follow the Harmony USB template and record the concern. This accepts an experimental source decision; it is not a device-specific manufacturer clarification or hardware qualification.

## Candidate identity

| Item | Reviewed value |
|---|---|
| Firmware/host source | `f1d9e3cfdb08d30f6768df8cf2a1930c8f4bd97b` |
| Local candidate HEX SHA-256 | `6e0dc7304bc6c1468be328fc97277c3cfb997b55291aff360855c84bc7f88873` |
| Known default recovery HEX SHA-256 | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |
| Toolchain | XC32 v6.00, DFP1.7.380, `-Os -flto` |
| Diagnostic reservation / application origin | 49152 bytes / `0x1D00C000` |
| ELF text/data/bss | 33237 / 88 / 2052 bytes |
| Program-region map usage | 32829 / 49152 bytes |
| Retained journal | Schema4,128-byte NOBITS at `0xA0000000`, excluded from `.dinit` |
| Wire compatibility | PREVIOUS and power-previous30-byte payloads, unchanged |
| Configuration words | `FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3`, matching normal local image |
| Audit flags | `hardware_qualified=false`, `board_delivery_approved=false` |

The hashes identify local review artifacts, not a delivered package. Normal default firmware remains26670/80/1568, with32KB reservation and application origin1D008000. The enabled diagnostic has not been returned to32KB.

## Changed acceptance and unchanged gates

[The source policy](usb-harmony-powered-readiness.md) requires USBPWR1 after the existing software and IRQ-source checks. Power-off rejects with bit14/mask16384 (`usb_power_off`) before U1CON/queue/BDT access. BUSY remains recorded evidence in the powered phase; USBEN0, empty queues and unowned BDT remain required. Historical bit10/mask1024 still decodes as `usb_busy` and must not be reassigned.

This changes a readiness outcome: a powered, BUSY replacement can now pass when all other prerequisites pass. It is not evidence-only instrumentation. No arbitrary wait, polling, register write, mode change or extra enable was introduced by this policy.

Central retirement/preparation/off-idle and the NVM port/adapter sources are byte-unchanged from `2de2f84`. Their hard USBPWR0/BUSY0 and ownership/DMA gates remain independent. Powered readiness cannot authorise flash access or establish old-reference retirement outside those gates.

The owner holds global interrupt exclusion through rebuild and successful handle publication/probe initialization. Reopen clears the fence only after preparation and all HID/device/driver checks pass. Normal owner attachment remains gated by eligible state and cable/mux validity. Partial failures retain quarantine; nothing in this change automatically retries or resumes an operation.

The prior board's BUSY failure did not reach USBEN or queue/BDT checks. This candidate may reject a later condition; no passing result for those conditions can be inferred from the source change.

## Completed review and verification

- Rebuilt `make power-debug-image-audit` and reran `bun tools/test-restart-image-audit.ts --power-debug`. The expanded image passes scoped ELF/HEX/configuration/retained-startup checks and negative artifact regressions, including normal32KB audit rejection of an expanded image.
- Audited actual copied candidate ELF/HEX in a disposable directory. Their hashes,48KB/origin1D00C000 metadata,128-byte journal startup exclusion and false approval flags matched. Copies were removed after verification.
- Rechecked all five generated overlays and reran strict exact owning-TU retirement/callback-binding tests. Implementation-stage full strict/sanitizer/XC32 tests are documented in the source-policy note; no firmware changes occurred during this review.
- Inspected the changed driver predicate and unchanged central retirement/preparation gates. Direct diff comparison confirms the NVM and central retirement sources were not modified by `f1d9e3c`.
- Offline host checks confirm historical BUSY and new power-off rejection names, successful driver evidence with BUSY observations3, schema4 power snapshots and explicit debug capabilities. Normal probe rejects debug origin; normal offline planning remains unchanged.
- A bounded independent owner/reopen and concern-documentation review found no must-fix within its supplied scope. It did not qualify the actual driver/register behavior or target timing; implementation's narrower predicate review and local tests cover logic, with hardware behavior still unqualified.

No HID command was sent to the board. Passing artifact/source tests do not prove hot reconstruction, pending-interrupt timing, electrical behavior or USB-only restart. The [device-specific concern and prepared clarification](usb-powered-readiness-contract-review.md) remain unresolved; no vendor answer was obtained.

## Future initial-only package requirements

A new package needs explicit preparation/delivery and programming approval. It must identify this exact source and HEX, retain the temporary48KB layout warning and unresolved A1 concern, and contain matching scripts, known recovery HEX, actual copied-artifact audit JSON, source/procedure identification, manifest and checksums in a new directory. Verify ZIP entries, extracted bytes and host tests before attachment.

The expanded diagnostic may overwrite the former application interval `0x1D008000..0x1D00C000`. The user previously confirmed no code/data in that interval needed preservation for the earlier debug installation. Do not silently extend that confirmation to new data or unrelated erase ranges; a new procedure must confirm that the overlap/preservation scope is still applicable and prevent broad erase. Artifact bounds are not a certificate of programmer erase scope. Protection and supply settings remain unchanged.

Any approved initial evaluation should collect IPE verification plus list, `probe --power-debug`, STATUS, PREVIOUS and power-previous only. Require exactly one expected interface and stop on errors with explicit PowerShell native-exit checks. Keep the existing independent supply active, disconnect PICkit5 after programming, and bound the initial SW1/MCLR startup. No automatic retry or recovery is allowed.

Schema4 is unchanged, so a new startup may return an older valid retained record if SRAM survives, including the prior BUSY rejection. Such evidence is historical; it does not show the new powered rule was exercised. Valid:false is also possible after programming. Compare paired identity/time fields without asserting reset-safe identity or forcing either outcome.

The host still contains a PAUSE command. Package possession, initial-check approval, old trial permissions and a valid retained record never authorise it. A single trial needs a later separate decision after reviewing initial outputs, with stop-before-recovery boundaries.

Keep the board recovered/read-only with PICkit5 disconnected and external power/settings unchanged. Its previous PAUSE/recovery permissions are spent. No new package, programming, PAUSE, NVM/protection change, upload or fault test is authorised by this review.
