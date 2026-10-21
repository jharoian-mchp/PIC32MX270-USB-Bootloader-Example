# Schema-3 diagnostic candidate review — 2026-10-21

The `bae3050` diagnostic candidate passes the bounded source and local image review. No replacement archive was created, delivered or authorised. Keep the current board recovered and read-only. No repeat PAUSE, higher request ID, reset, reprogramming, uploads, NVM writes, protection changes or fault tests are authorised by this review.

The board's instrumented 18384b2 trial failed at INIT/REBUILD. Its exact rejected sub-step is unknown. The candidate adds evidence recording; USB-only restart is still hardware-unqualified.

## Reviewed candidate

| Item | Value |
|---|---|
| Firmware source | `bae305099bc46797926477bd98d77b5c0db12464` |
| Local candidate HEX SHA-256 | `7dbcc93f3a2414707913a752d7f99761833c2a65aa175999c776c8b9054f44a2` |
| Local recovery HEX SHA-256 | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |
| Toolchain | XC32 v6.00, DFP1.7.380, `-Os -flto` |
| Candidate text/data/bss | 32041 / 88 / 1952 bytes |
| Default text/data/bss | 26670 / 80 / 1568 bytes |
| Linker reservation / application origin | 32 KB / `0x1D008000`, unchanged |
| Retained journal | 96 bytes at `0xA0000000`, NOBITS, excluded from linked `.dinit` |
| Configuration words | `FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3`, matching the default local image |
| Audit flags | `hardware_qualified=false`, `board_delivery_approved=false` |

These hashes identify local review artifacts, not a delivered package. The recovery HEX matches the previously delivered recovery image. Protection settings remain unchanged. The candidate fits the reservation with little margin.

## Source and artifact checks

- Strict tests and ASan/UBSan passed in the source stage: 21 C executables, 22 Python test methods and generated-stack harnesses. No firmware code changed during this review.
- A bounded independent review of the owner, retained journal, detail observer and host parser found no concrete must-fix in its scope. The shown journal paths are IRQ-serialized; the observer writes evidence only. Previous evidence is captured at startup and kept separate from the current-operation cache. PREVIOUS never authorises a restart.
- `make restart-image-audit` rebuilt both local images and passed ELF/HEX/configuration/startup checks. `bun tools/test-restart-image-audit.ts` passed its negative artifact regressions.
- An offline C review fixture built with `-std=c11 -Wall -Wextra -Werror` accepted schema 3 and rejected otherwise valid, recomputed-CRC schema 1 and 2 records. Schema migration therefore discards the old retained record; installing a future candidate cannot add missing sub-step detail to the old failure.
- An offline Python fixture sent the old 26-byte PREVIOUS response to the current parser and received `invalid previous-record response length`. The matching 30-byte response passed. Both cases issued exactly one PREVIOUS query and no PAUSE. Fixtures were removed after verification; no board was accessed.

[Rebuild stages, first-rejection masks and ordered observations](usb-restart-phase-evidence.md#rebuild-sub-step-and-first-rejection) define the evidence format. Unchecked later predicates and unsampled registers are not passes. Retention is observed only for the reported powered MCLR recovery of the older image; power loss, programming and broader reset behaviour are unqualified.

## Subsequent limited approval — 04:01

After this review, the user approved preparing the matching package for programming and initial list/probe/STATUS/PREVIOUS only. The [initial-check procedure](usb-rebuild-detail-initial-check.md) defines that later scope. No PAUSE, fault tests, uploads or protection changes are authorised. The no-delivery statements above describe the review's earlier state.

## Requirements before a replacement delivery

A future package requires separate approval to prepare and deliver it. It must contain matching schema-3 firmware and 30-byte PREVIOUS scripts in a new directory, the known recovery HEX, exact artifact audit JSON, source/build identification, manifest and checksums. Re-audit the actual files and verify archive contents and extracted host scripts before attaching anything.

Keep using the current board's 18384b2 package-local scripts for its schema-2/26-byte reply. Do not mix them with the candidate host or HEX.

Any eventual programming and initial read-only checks need an explicit bounded procedure and authorisation. A PAUSE trial needs a separate decision after those results; neither this review nor package preparation grants one. Preserve board power/settings, the LED and SW1/ADC/mux baseline, and inactive upload/NVM paths. No change to USB-off safety gates, session continuity or protection policy is part of this candidate.
