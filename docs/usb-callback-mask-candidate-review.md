# Callback-mask candidate review — 2026-10-21

The `a57fef3` callback-mask candidate passes the bounded offline source and image review. No replacement archive was created or delivered, and this review authorises no programming, PAUSE, reset or other board action. The board stays recovered/read-only with PICkit5 disconnected and external power/settings unchanged.

The delivered `bae3050` trial failed at DRIVER_READY/irq_enabled512. The new candidate addresses that software mismatch without changing the readiness predicate. Later hardware checks and USB-only restart remain unqualified.

## Identified artifacts

| Item | Value |
|---|---|
| Candidate source | `a57fef3615db38a267ef467d9f5b793f8bd9bc78` |
| Local candidate HEX SHA-256 | `f9dd06c6f176bf33db1c8b85639b2e3e49498b3e379d35d8e5ae00f2f35a06fd` |
| Known default recovery HEX SHA-256 | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |
| Toolchain | XC32 v6.00, DFP1.7.380, `-Os -flto` |
| Candidate text/data/bss | 32117 / 88 / 1952 bytes |
| Default text/data/bss | 26670 / 80 / 1568 bytes |
| Reservation / application origin | 32 KB / `0x1D008000`, unchanged |
| Retained journal | 96-byte NOBITS at `0xA0000000`, excluded from linked `.dinit` |
| Journal / PREVIOUS | Schema 3 / 30-byte payload, unchanged |
| Configuration words | `FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3`, matching the default local image |
| Audit flags | `hardware_qualified=false`, `board_delivery_approved=false` |

These hashes identify local review artifacts; they are not a delivered firmware package. The reservation has little margin. Protection, application layout, LED/SW1/ADC/mux baseline and inactive upload/NVM paths are unchanged.

## Interrupt boundary and review findings

[The source fix](usb-callback-mask-fix.md) stores callback/context while blocked but skips both OTG session-valid and CPU-source enables. Normal unblocked registration still enables interrupts; hooks0 preserves generated behavior. Admission prevents attachment while fenced or poisoned. After admitted attachment configures the original device/peripheral interrupts, it enables the CPU source.

The original detached-readiness predicates remain ordered and unchanged. Reopen requires all of them to pass before lifting the fence; it does not enable the source or attach. The owner keeps global interrupt exclusion through rebuild, successful private-handle publication and probe initialization. Attachment remains in the existing eligible-state, cable/mux-gated owner path. Partial failures do not publish the replacement handle, and FAILED does not enter that attachment path.

The changed-source review completed during implementation and found no concrete must-fix. The broader final candidate review timed out. A narrower final review of the owner/publication/attach boundary completed and found no concrete must-fix within that scope. It could not assess whether the initializer might fail after successful Reopen because that file was outside its supplied excerpt.

Direct inspection of `USB_RebuildDetached()` closes that coverage question: Reopen is its final fallible operation. On success it assigns the private handle, records COMPLETE, restores saved global interrupt state and returns true; no later check changes `ok` to false. The evidence observer writes only the IRQ-serialized SRAM journal. This inspection does not establish target interrupt timing or electrical behavior.

## Offline verification

- Implementation verification passed `make test`, `make test-sanitize`, `make usb-retirement-check`, `make restart-image-audit` and negative image regressions. It includes exact callback/attach tests with hooks0/1 and actual owning-TU readiness hooks, plus the disposable five-file regeneration round-trip. No firmware changes occurred during this candidate review.
- This review rebuilt `make restart-image-audit`, reran `bun tools/test-restart-image-audit.ts` and checked all five overlays. Both local images passed the linked ELF/HEX/configuration/symbol/startup checks; the default recovery hash is unchanged.
- Copies of the candidate ELF and HEX were independently passed through the same artifact audit in a disposable directory. Their hashes, 96-byte journal placement/startup exclusion and false hardware/board approval flags matched. Copies were removed after verification.
- `pic32_hid.py`, `usb_restart_diag.py` and `requirements-hid.txt` are byte-identical to the delivered schema-3 host source `bae3050`. The host still decodes 30-byte PREVIOUS; `0x0002000B` decodes as DRIVER_READY/irq_enabled512/observations0. No offline query or mock test accessed the board.

No passing source or artifact check establishes USBBUSY, USBEN, later queues/BDT readiness, pending-interrupt timing or USB re-enumeration after this candidate's rebuild. The older failed trial's zero observations were unsampled hardware registers, not passes. Powered MCLR retention was observed for the delivered image; broader reset/power/programming preservation is unqualified.

## Subsequent initial-only approval — 05:03

The user subsequently approved preparing the matching callback-mask package for programming and initial list/probe/STATUS/PREVIOUS only. The [initial-check procedure](usb-callback-mask-initial-check.md) defines the later limited scope. No PAUSE or fault test is authorised. The earlier no-package/no-board-approval statements describe this review's cutoff.

## Requirements for a separately approved package

A future package needs explicit preparation/delivery approval, matching candidate firmware and schema-3 host scripts in a new directory, the known recovery HEX, an audit of the actual packaged ELF/HEX, source/procedure identification, manifest and checksums. Verify ZIP entries, extracted bytes and host tests before attachment. Do not mix candidate HEX with older package manifests or instructions.

The schema is unchanged, so a future startup may capture a valid older schema-3 record if SRAM survives programming/startup. PREVIOUS must be reported without treating it as current-candidate success; valid:false is also possible. Preserve the already reported failure evidence separately. Programming or memory-layout changes can invalidate or alter retained evidence.

Any approved programming/initial evaluation should stop after IPE verification and list/probe/STATUS/PREVIOUS outputs, with the external supply active and PICkit5 disconnected before the bounded startup. Stop and report failures without unapproved resets or recovery. A PAUSE trial needs a later separate decision after those initial results. Both the delivered image's PAUSE and recovery permissions are spent; they cannot be transferred to this candidate.
