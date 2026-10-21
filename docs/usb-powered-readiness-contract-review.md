# Powered BUSY versus detached readiness — contract review

Power-off retirement and powered reconstruction need distinct contracts. The new samples and Microchip's generated-driver templates support reviewing that distinction, but do not yet justify accepting BUSY1 at powered readiness. No firmware predicate, wait or board setting is changed in this review.

The board remains recovered/read-only. Its latest trial and recovery permissions are spent. No PAUSE, reset, reprogramming, new package, NVM/protection change, upload or fault test is authorised.

## Evidence carried forward

[The paired power-debug record](usb-power-debug-board-evidence.md) has ticket1/attempted generation2/time319308 and FAILED/INIT/REBUILD/DRIVER_READY/usb_busy1024. Metadata487 records PRE power/BUSY0/0, POST1/1 and READY1/1. The earlier IRQ check passed. USBEN, queue and BDT checks were not reached.

This establishes a clean sampled pre-enable state followed by two powered BUSY samples. It does not prove continuous BUSY, a clearing deadline, or BUSY1 as the normal invariant of every powered module. Nominal deltas include logging and normal driver work. No successful reopen or attachment is observed.

## Two separate phases

| Phase | Existing requirements | What this review establishes |
|---|---|---|
| Retire old USB resources / prepare rebuild | Fence; drained callbacks, IRPs, queues and BDT ownership; successful power-off authorisation; USBPWR0/BUSY0 before private-state retirement/preparation | Preserve these hard gates. PRE0/0 is consistent with them; no later powered observation overrides them |
| USB-off flash ownership | USBPWR0/BUSY0, DMA exclusion and the full NVM launch/ownership contract throughout execution | Preserve independently. A powered device is never authorised for NVM by a rebuilt software object |
| Reconstruct powered, detached USB | Trusted builder; replacement callbacks/state; IRQ source masked; USBEN0; empty queues and unowned BDT; current project additionally requires BUSY0 | The present board fails BUSY0 before later checks. The original generated driver does not enforce a BUSY-clear condition, but its software READY is not proof of hardware safety |

The current powered driver-ready predicate also does not explicitly require USBPWR1; it records power as evidence and rejects BUSY before reading USBEN. This is a project-owned readiness contract, introduced with detached rebuilding in `a4632ad`, rather than an unchanged vendor API condition.

A proposed phase-specific powered contract could explicitly require USBPWR1, retain every existing software/IRQ/USBEN/queue/BDT check and record BUSY separately. That is a policy candidate only. Changing BUSY from a rejection to an observation changes the safety decision and must not be presented as mere instrumentation or already approved.

## Official documentation and vendor corroboration

[The official reference review](usb-powered-busy-semantics.md) quotes DS61126F and device datasheet DS60001168L: BUSY1 means active or disabled but not ready to be enabled; disabled USBPWR0/BUSY1 makes other USB-register status invalid and writes undefined. The rapid disable/re-enable note advises waiting for BUSY clear before configure/enable, with ambiguous placement relative to USBPWR versus USBEN and no time bound.

The note's specifically invalid register condition is USBPWR0/BUSY1. Seeing USBPWR1 avoids that particular condition; it is not a blanket proof that all powered BUSY1 configuration, DMA or attachment operations are safe.

Retrieved Microchip's public Harmony USB templates at pinned commit [`67ce30c2b74e9f23f355988cfa7470c15e6ec392`](https://github.com/Microchip-MPLAB-Harmony/usb/commit/67ce30c2b74e9f23f355988cfa7470c15e6ec392), dated2025-03-26. Relevant files:

- [`driver/usbfs/src/drv_usbfs.c.ftl`](https://github.com/Microchip-MPLAB-Harmony/usb/blob/67ce30c2b74e9f23f355988cfa7470c15e6ec392/driver/usbfs/src/drv_usbfs.c.ftl): power enable at line157, followed by register setup/device initialization/BDT assignment and software READY at line212; no ModuleIsBusy call or dedicated BUSY wait in this initializer.
- [`usb_ModulePower_32Bit16Bit.h.ftl`](https://github.com/Microchip-MPLAB-Harmony/usb/blob/67ce30c2b74e9f23f355988cfa7470c15e6ec392/driver/usbfs/src/templates/usb_ModulePower_32Bit16Bit.h.ftl): enable sets USBPWR1 and disable clears it.
- [`usb_ModuleBusy_Default.h.ftl`](https://github.com/Microchip-MPLAB-Harmony/usb/blob/67ce30c2b74e9f23f355988cfa7470c15e6ec392/driver/usbfs/src/templates/usb_ModuleBusy_Default.h.ftl): returns the raw USBBUSY bit; no powered-state interpretation.

Local cached files are `build/vendor-*.ftl`; SHA-256 values respectively are `cf795f79c745594898811e7f037adcf38af6afdee0c07ab1f62dca575689dd2c`, `cbae676beca6211c43e1819134070d29f075dfdb5037003de9961969bd47cba0`, and `aa8e99bf096e5489ff292c57413c26035cd60c19f89d406caa660c43b94f4df9`.

These sources corroborate the locally generated sequence, not a hot-restart guarantee for PIC32MX270 A1. A generated initializer returning READY does not certify old-reference retirement or establish that later hardware predicates pass. Existing initialization already performs some register writes between POST and READY; their presence is source behavior, not validation of their safety under the observed BUSY state.

The device datasheet's peripheral-disable note also requires the USB module not busy after clearing its ON bit and before setting USBMD (page239). No PMD change is proposed. The inspected USB erratum concerns idle-interrupt reassertion; it supplies no permission to bypass the observed BUSY readiness check.

## Decision gate and clarification request

The smallest unresolved contract question is device-specific:

> For PIC32MX270F256D revision A1, after prior callbacks/IRPs/queues are drained, USBPWR is cleared and USBBUSY is observed0 before re-enabling USBPWR, is USBBUSY1 with USBPWR1 and a still-detached USBEN0 device expected or safe during reconstruction? Must firmware wait for BUSY0 before register configuration or before USBEN/mode activation, and does that requirement apply before or after asserting USBPWR? What conditions assert/clear USBBUSY in this powered state, and is a documented bound available?

The later USBEN/queue/BDT state in our failed trial was not sampled. This question must not assert that those conditions were measured; it asks about the intended detached configuration. Do not send a claim of universal BUSY1 while powered or a proposed delay as an established workaround.

A vendor/device-specific answer or equivalent authoritative source would permit a justified contract decision. Until then, retain the existing BUSY rejection. Three existing snapshots cannot supply the missing permission. No vendor inquiry was sent as part of this review; this text is a prepared clarification question, not a new external action.

If clarification supports a phase-specific powered rule, implement it as an explicit source-only safety-contract change with tests for powered-off rejection, BUSY combinations, IRQ/software/USBEN/queue/BDT failures and intact NVM/off-idle gates. Preserve first-rejection evidence, quarantine, generation continuity and trusted ownership. Candidate review, initial programming scope and any later trial remain separate gates. An arbitrary delay or diagnostic guard bypass is not selected here.

## Subsequent user-directed source decision — 21:16

The user directed following the Harmony USB template while retaining the concern. The [experimental powered-readiness implementation](usb-harmony-powered-readiness.md) requires USBPWR1 and makes BUSY evidence-only during powered reconstruction, retaining all other guards and the hard power-off/NVM gates. This is an explicit decision change under the user's direction, not a device-specific manufacturer clarification. The concerns and prepared question above remain open; no hardware trial or new package is authorised.

## Verification and review limits

- Compared local initializer, mode/power/busy templates, preparation/readiness/NVM checks and historical introduction of the powered BUSY0 rule.
- Retrieved and pinned the public vendor templates and verified their hashes/commit identity. They do not contain a BUSY wait in the inspected initializer.
- Reran strict exact retirement/callback-binding tests; they pass. They use mocked hardware and cannot decide powered BUSY semantics.
- A bounded independent contract review supported separating the phases but found the supplied evidence insufficient to make powered BUSY non-gating. It identified the device-specific configure/enable clarification as the remaining requirement; no hardware approval follows.

Web search attempts were limited by a search-engine challenge and unauthenticated GitHub code-search restrictions. Vendor raw files and repository metadata were retrieved directly; no dependencies or environment settings were changed. The review does not establish that a manufacturer answer is unavailable.

No firmware, layout, protection or HEX changed. The temporary48KB debug build remains available as previously reviewed; the normal production target remains32KB. Further board actions are not requested.
