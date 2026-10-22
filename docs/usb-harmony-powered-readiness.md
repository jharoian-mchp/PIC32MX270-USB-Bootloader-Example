# Harmony-compatible powered readiness — experimental source change

Powered detached rebuilding now requires USBPWR1 and retains USBBUSY as evidence rather than a reopening blocker. The user explicitly directed this source change on 2026-10-21 at21:16: “Follow the Harmony USB template, but make note of the concern.” No new package or hardware trial is authorised. The board remains recovered/read-only.

This changes the readiness decision. It is not merely instrumentation, and it does not establish PIC32MX270 A1 hardware safety or successful USB-only restart. The [contract review](usb-powered-readiness-contract-review.md) remains the record of the unresolved device-specific concern and vendor-source rationale.

## Changed powered predicate

The driver still checks preparation, in-use/open/READY state, callback, detached/not-in-ISR state, endpoint/BDT pointers and USB interrupt-source masking in the same order. After those checks, it reads U1PWRC once and records power/BUSY observations and the existing one-shot power sample.

- USBPWR0 rejects with **mask16384 (`usb_power_off`, bit14)** before U1CON or queue/BDT accesses, including the disabled-but-BUSY condition.
- USBPWR1 permits the existing U1CON USBEN check regardless of BUSY. USBEN1 still rejects with mask2048.
- Empty endpoint queues and no BDT UOWN entries remain required; masks4096 and8192 are unchanged.
- BUSY is never written, cleared by software or treated as permission to retire old resources. Its bit remains in observations and power-previous samples.

No delay, polling, power-cycle, mode write or new enable call was added. The original generated Harmony power-enable/configuration sequence is retained. The callback/source-mask fix, full HID/device readiness, owner interrupt exclusion, private-handle publication and gated attach remain unchanged.

The host adds bit14's name but continues decoding historical bit10/mask1024 as `usb_busy`; earlier retained records must not be relabelled. Schema4, both30-byte replies and the read-only power query remain unchanged. A successful driver check can now carry observations3 (USBPWR1/BUSY1); that describes the sample, not hardware-idle state.

## Unchanged power-off and flash boundaries

`USB_RetirementOffIdle()`, power-off authorisation, old-layer retirement and preparation still require USBPWR0/USBBUSY0 together with the existing callback/IRP/queue/BDT ownership conditions. The NVM hardware gate still treats either USBPWR or USBBUSY set as active, and USB/DMA exclusion must persist throughout flash ownership.

The central retirement and NVM port sources were not changed. A powered-ready result cannot authorise USB-off flash operations, prove old DMA activity is absent outside the retirement contract, or bypass poisoned/fenced state. This separation is essential to the approved experimental policy.

## Concern retained

The latest board samples were PRE0/0, POST1/1 and READY1/1, ending in the earlier BUSY rejection. They did not measure USBEN or later queues/BDT. The public Harmony templates power USB and return software READY without a BUSY wait, supporting the chosen template-compatible sequence, but do not certify hot reconstruction for PIC32MX270 A1.

DS61126F/DS60001168L describe BUSY1 as active or disabled but not ready to be enabled and advise waiting around rapid re-enable. The exact powered configure/enable semantics remain unresolved. Reading U1CON with USBPWR1/BUSY1 and permitting subsequent reconstruction/attach is the explicitly accepted experimental source decision; it is not a manufacturer clarification. Do not assert BUSY is always1 while powered, continuously set between samples, or harmless for every USB operation.

The concern remains open for qualification/documentation follow-up. No vendor inquiry was sent and no new hardware evidence was collected. Any future package review must disclose this decision and keep programming/initial checks separate from a PAUSE trial.

## Tests and review

Passing verification:

- `make test` and `make test-sanitize`: 30 Python methods and strict/sanitized C/mock suites.
- `make usb-retirement-check`: five checked generated overlays, enabled XC32 compilation and exact owning-TU tests.
- `make power-debug-image-audit` and `bun tools/test-restart-image-audit.ts --power-debug`: temporary48KB link,128-byte retained/startup/configuration/HEX checks and negative image regressions.
- Disposable five-file base-strip/reapply/check round-trip; original MCC base hashes unchanged.
- `git diff --check`; direct diff verification that central retirement and NVM sources are unchanged.

Exact owning-TU tests cover both powered BUSY values, power-off idle/busy rejection with no U1CON access, all earlier driver object/IRQ failures, USBEN, queues and BDT rejection, HID/device readiness failures, full reopening and source-masked publication/attach. Successful powered BUSY retains observations3. The production `UPDATE_NvmPortUsbActive()` body is extracted into the harness and checked for all four power/BUSY combinations: only0/0 is idle. Power samples remain one-shot and raw tick behavior is unchanged.

A broad source review timed out. A completed narrower predicate review found no logic or rejection-mask defect and explicitly noted the unresolved behavioral qualification. The mock tests do not establish safe operation on the target.

The temporary diagnostic link is text33237/data88/bss2052, with32829 bytes used in its49152-byte program region. Normal default remains26670/80/1568, origin1D008000 and known recovery HEX hash `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`. The debug origin stays1D00C000. Returning enabled diagnostics to32KB remains a later task. Configuration/protection settings are unchanged.

## Later single-cycle hardware result — 2026-10-22 03:51

The delivered experimental image completed [one authorised controlled USB-only cycle](usb-harmony-powered-first-cycle.md), with verified removal/reconnect and independently confirmed live ticket1/generation2/cycles1/state7 plus HID/debug probe. This is narrower than general hardware qualification: the A1 concern, repeated cycles and later NVM/upload work remain open. No additional cycle or reset is authorised.

## Board and delivery scope

At this source-stage cutoff no replacement archive had been created and the board still ran `2de2f84`. The later delivered `f1d9e3c` image and its spent single-cycle trial are recorded above. Keep PICkit5 disconnected and external power/settings unchanged. No new PAUSE, higher ID, reset, reprogramming, upload, NVM/protection change or fault test is authorised. The subsequent [experimental candidate review](usb-harmony-powered-candidate-review.md) passes its bounded offline checks and defines future initial-only delivery requirements. It grants no package or board permission; a separate programming and later trial decision is still required.
