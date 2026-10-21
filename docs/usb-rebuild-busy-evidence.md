# Rebuild rejection: USBBUSY sampled after power enable

The callback-mask board trial reached DRIVER_READY and rejected `USBBUSY=1` after sampling `USBPWR=1`. The previous IRQ-source check passed in this attempt. The record does not establish whether BUSY was transient, persistently active or caused by another condition. No readiness predicate or firmware code is changed by this evidence stage.

## Retained result — 2026-10-21 05:47

After the failed callback-mask PAUSE100 and instructed powered SW1/MCLR recovery, the user returned the expected HID interface, normal capabilities and fresh live STATUS ticket0/generation1/completed_cycles0/state0. PREVIOUS returned:

```text
valid=true
 ticket=1  attempted_generation=2  time_ms=1339853
 state=FAILED  phase=INIT  fault=INIT  checkpoint=REBUILD
 evidence_bits=0
 rebuild.step=DRIVER_READY
 rebuild.first_rejected=usb_busy
 rebuild.rejected_mask=1024
 rebuild.observation_bits=3
```

Packed detail is `11 | (1024 << 8) | (3 << 24)` = `0x0304000B`. The ordered driver guard passed its preparation/object/callback/detachment/IRQ-source checks, then a single `U1PWRC` sample found USBPWR and USBBUSY set. It rejected BUSY before reading `U1CON`; USBEN and later queue/BDT predicates were untested. Observation bit2 being zero therefore does not mean USBEN was clear. Wait evidence0 is an earlier wait observation, not proof of all readiness.

This demonstrates that the observed irq_enabled512 rejection was avoided in this attempt. It does not qualify callback-mask attachment, final reopen or USB-only restart. The initializer returned false during attempted generation2; fresh recovered generation1 is startup. `time_ms` is elapsed boot time, not operation duration or BUSY persistence.

The initial PREVIOUS was valid:false. This returned timestamp1339853, mask1024 and observations3 differ from the earlier `bae3050` record time415322/mask512/observations0 and are consistent with the new reported trial. Tickets/generations are boot-scoped and provide no reset-safe identity. The journal survived this reported powered recovery; broader reset/power-loss/programming retention is unqualified.

## Inspected power-up sequence

Firmware/host source: `a57fef3615db38a267ef467d9f5b793f8bd9bc78`.

- The owner progresses through drain, power-off, off-idle and retirement before entering rebuild. `USB_RetirementOffIdle()` requires USBPWR and USBBUSY clear in the powered-off phase.
- `USB_RebuildDetached()` calls `DRV_USBFS_Initialize()`. That initializer invokes `PLIB_USB_Enable()` before constructing the replacement device state.
- `plib_usbfs_header.h` maps the call to `USB_Enable_32Bit16Bit()` in `usb_ModulePower_32Bit16Bit.h`, which only sets USBPWR to1. It contains no BUSY wait.
- Rebuild completes device initialization/open and callback binding, then calls `USB_RetirementReopen()` in the same synchronous invocation. There is no dedicated post-enable BUSY wait or resampling state.
- The unchanged driver readiness predicate requires the interrupt source masked, then rejects USBBUSY, then reads USBEN and checks queues/BDT. The earlier off-idle observation does not establish readiness after power is enabled.

The device-family datasheet `DS60001168L`, register10-5, page116 (`build/device-data.txt`) defines USBBUSY1 as “USB module is active or disabled, but not ready to be enabled” and USBBUSY0 as “USB module is not active and is ready to be enabled”. USBPWR1 means the module is turned on. Note1 states that when USBPWR0 and USBBUSY1, other USB register status is invalid and module writes produce undefined results.

That inspected definition gives no settling duration and does not prove that USBBUSY must clear while powered. It also does not permit treating BUSY as safe, writing it away or relaxing the USB-off NVM guard. More device-family USB reference/driver evidence is needed to distinguish powered-active semantics from a transient enable boundary before choosing a correction.

## Verification and review

The current host decoder accepts the exact detail as DRIVER_READY/usb_busy1024/observations3. Existing exact owning-TU readiness tests exercise BUSY independently of USBEN and IRQ-source rejection; strict and ASan/UBSan retirement/binding tests were rerun and passed. Hardware and registers are mocked in those suites; they do not reproduce the board's BUSY duration or cause.

A bounded independent diagnosis review confirmed the ordered predicate interpretation, power-enable path and absence of a dedicated BUSY-clear wait. It explicitly limited the record to BUSY set at one check and left the need for a bounded powered-ready wait as a source question. No code, HEX or protection change was made.

## Next source questions and board limits

The subsequent [powered-BUSY reference investigation](usb-powered-busy-semantics.md) found official conditional rapid-re-enable wait guidance, but no general powered BUSY-clear guarantee or time bound. The project introduced the powered BUSY0 readiness check; the original generated initializer does not enforce it. This does not justify removing the guard or adding an arbitrary delay. Next design bounded evidence-only power-boundary observations, with source review before implementation/delivery. Preserve ordered register access, detached/IRQ masking, ownership, session continuity and the separately defined USB-off NVM safety gate.

Both this image's PAUSE and recovery are complete. Leave the board recovered/read-only with external supply and settings unchanged, PICkit5 disconnected. No repeat PAUSE, higher ID, reset, reprogramming, replacement package, uploads, NVM/protection changes or fault tests are authorised. Further source work and any later delivery/trial require separate verification and scope.
