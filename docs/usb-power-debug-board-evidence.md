# Power-debug board evidence — 2026-10-21 20:30

The retained samples show USBPWR/USBBUSY0/0 before module enable,1/1 after enable and1/1 at the driver's readiness check. The initializer returned false at DRIVER_READY/usb_busy1024. The earlier IRQ-source check passed, but USBEN and queue/BDT checks were not reached. USB-only restart is still failed and unqualified.

## Recovery and paired snapshots

After the single authorised power-debug PAUSE100 failed reconnect and list was empty, the instructed powered SW1/MCLR recovery restored the expected HID interface. The user returned normal debug capabilities with origin1D00C000 and fresh live STATUS ticket0/generation1/completed_cycles0/state0. Both retained queries returned valid:true and matching ticket1/generation2/time_ms319308.

PREVIOUS:

```text
state=FAILED  phase=INIT  fault=INIT  checkpoint=REBUILD
 ticket=1  attempted_generation=2  time_ms=319308  evidence_bits=0
 rebuild.step=DRIVER_READY
 first_rejected=usb_busy  rejected_mask=1024  observation_bits=3
```

Power-previous metadata487 (`0x01E7`) decodes as:

| Site | Valid | USBPWR | USBBUSY | Raw Count delta from PRE |
|---|---|---:|---:|---:|
| PRE_ENABLE | yes | 0 | 0 | Anchor not transmitted |
| POST_ENABLE | yes | 1 | 1 | 6097 ticks |
| READY_POWER | yes | 1 | 1 | 54314 ticks |

PRE/POST read Count then U1PWRC around the existing `PLIB_USB_Enable()` call; READY reuses the original readiness power sample then reads Count. PRE is committed before the call, POST after it returns, and READY through the original detail journal commit. All three sites were recorded; their validity is distinct from whether bits were set.

The valid PRE observation establishes that the selected power/BUSY bits were both clear at the pre-enable sample. Powered BUSY was sampled set after the enable call and again at readiness. This places the observed assertion after a clean sampled power-off state; it does not identify the hardware mechanism that asserts BUSY.

The record is consistent with the reported trial: initial retained queries were invalid, and its time319308 differs from the older callback-mask record time1339853. Matching boot-scoped ticket/generation/time supports the paired snapshots, not reset-safe identity or authentication. The journal survived this reported powered recovery and decoded successfully; power-loss/programming/arbitrary-reset retention is not qualified. Live generation1 is recovery startup, not completed generation2 reopening.

## Timing limits

At the declared nominal24MHz Count profile,6097 ticks corresponds to about0.254042ms and54314 ticks to about2.263083ms from PRE Count. Their numerical difference is48217 ticks, nominally2.009042ms. These are illustrative nominal conversions only, not measured wall-time intervals or continuous BUSY durations.

Sample pairs are non-atomic, deltas are modulo2^32, and the intervals include journal commits and normal initialization work. Three snapshots do not prove BUSY stayed set between them, whether it cleared and reasserted, whether it was transient or persistently active, or how long clearing should take. No timeout or delay value is derived from this record. Instrumentation changes timing; a later instrumented success would not qualify an uninstrumented image.

Observation3 at READY means USBPWR and USBBUSY were read as set. The driver guard then short-circuited before U1CON/USBEN and the queue/BDT predicates. It provides no values or passing result for those later checks. Wait evidence0 is a separate earlier wait observation. Failed INIT means the rebuild callback returned false; it is not a coordinator wait timeout.

## Verification and next source question

The current decoder was run offline against the exact identity,6097/54314 deltas and metadata487; it decoded all three samples and the first BUSY rejection as reported. Nominal conversion arithmetic was independently checked. A bounded interpretation review agreed with the principal limits; no board test or firmware edit was performed as part of this analysis.

The subsequent [powered-readiness contract review](usb-powered-readiness-contract-review.md) separates powered reconstruction from the hard power-off/NVM gates and corroborates the generated sequence with pinned vendor templates. It does not justify ignoring BUSY; a device-specific configure/enable clarification is still needed. Preserve the existing rejection and the separate USBPWR0/USBBUSY0 NVM/retirement gates. No arbitrary delay or unsafe register access is selected.

Both this image's PAUSE and recovery permissions are spent. Keep the board recovered/read-only with external power/settings unchanged and PICkit5 disconnected. No new PAUSE, higher ID, reset, reprogramming, replacement image, upload/NVM/protection change or fault test is authorised.
