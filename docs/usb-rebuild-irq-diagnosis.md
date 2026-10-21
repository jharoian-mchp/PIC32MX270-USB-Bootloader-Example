# Rebuild rejection: USB interrupt source enabled

The schema-3 board trial failed at `DRIVER_READY` because the USB interrupt source was enabled. The generated callback-registration path enables that source before the detached-readiness guard checks it. The source call chain and an offline reproduction support this software mismatch; no fix or new board trial is part of this diagnosis.

## Retained board evidence — 2026-10-21 04:33

After the failed PAUSE100 and the instructed powered SW1/MCLR recovery, the user returned the expected HID interface, normal capabilities and fresh live STATUS ticket0/generation1/completed_cycles0/state0. PREVIOUS returned:

```text
valid=true
 ticket=1  attempted_generation=2  time_ms=415322
 state=FAILED  phase=INIT  fault=INIT  checkpoint=REBUILD
 evidence_bits=0
 rebuild.step=DRIVER_READY
 rebuild.first_rejected=irq_enabled
 rebuild.rejected_mask=512
 rebuild.observation_bits=0
```

The USB-only initializer returned false during attempted generation2. The retained word is `11 | (512 << 8)` = `0x0002000B`. The first rejection is the per-source EVIC enable predicate, distinct from global CPU interrupt enable. The ordered guard did not reach its `U1PWRC`/`U1CON` reads, so observation0 does not establish USBPWR, USBBUSY or USBEN values. Later queue/BDT checks were also untested. Wait evidence0 is an earlier observation, not complete readiness.

The record survived this reported powered recovery and validated/decoded afterwards. This is schema-3 retention evidence for that sequence only; power loss, programming and arbitrary resets are unqualified. Live generation1 is fresh startup, not completed USB-only reopen. The boot-millisecond timestamp is not the operation duration.

## Source call chain

The reviewed firmware/host source is `bae305099bc46797926477bd98d77b5c0db12464`.

| Source | Relevant operation |
|---|---|
| `initialization.c`, `drvUSBFSInit` | Device mode, `interruptSource=INT_SOURCE_USB` |
| `configuration.h` | `DRV_USBFS_INTERRUPT_MODE=true` |
| `initialization.c`, `USB_RebuildDetached()` | Saves/disables global interrupts, prepares/builds/opens USB, binds device and HID, then calls `USB_RetirementReopen()` |
| `usb_device.c`, `USB_RetirementBindDevice()` | Calls `d->driverInterface->eventHandlerSet(...)` |
| `drv_usbfs_device.c`, `gDrvUSBFSDeviceInterface` | Maps `.eventHandlerSet` to `DRV_USBFS_ClientEventCallBackSet` |
| `drv_usbfs.c`, `DRV_USBFS_ClientEventCallBackSet()` | Stores callback/context; in device mode enables OTG session-valid interrupt and calls `M_DRV_USBFS_InterruptSourceEnable(...)` |
| `drv_usbfs_variant_mapping.h` | Interrupt-mode macro maps to `SYS_INT_SourceEnable(source)` |
| `usb_retirement.c`, `USB_RetirementReopen()` | Checks HID, device and driver readiness in order |
| `drv_usbfs.c`, `USB_RetirementDriverReplacementReady()` | Rejects `EVIC_SourceIsEnabled(...)` with mask512 before USB register observations |

No source-disable occurs between device callback registration and that guard in this rebuild sequence. Global CPU interrupt exclusion does not clear the EVIC source-enable bit. The callback setter therefore conflicts with the requirement that the rebuilt device remain source-masked before reopening.

The board directly establishes the failed predicate. The source identifies an enable write that explains it; the retained record does not log each register writer. The older schema-2 trial lacks this detail, so its exact rejection cannot be assigned retrospectively from the new result. The PICkit5 Hold In Reset setting is separate tool evidence, not the software cause demonstrated here.

## Offline reproduction and bounded review

A temporary fixture extracted the exact generated `DRV_USBFS_ClientEventCallBackSet()` body and used the existing owning-TU retirement hooks. It called `USB_RetirementBindDevice()` in device mode under a mocked global interrupt mask. Callback registration enabled the mocked source while global interrupts remained masked; `USB_RetirementReopen()` then rejected DRIVER_READY/mask512/observation0 and kept the stack fenced. The independent later-predicate tests continued only after a test-only mock reset; no such reset was added to firmware.

The fixture passed strict `cc -std=c11 -O2 -Wall -Wextra -Werror` and ASan/UBSan builds. The existing `tools/test-usb-retirement.ts` also passed. Hardware/EVIC/PLIB accesses were mocked; temporary fixture files were removed. No firmware source, HEX, protection setting or board state was changed.

A bounded independent source review confirmed the callback mapping, device-mode enable and lack of an intervening source-disable. It also confirmed that mask512 precedes power observations. The review does not qualify a repaired restart.

## Next source work and board limits

The subsequent [source-only callback-mask fix](usb-callback-mask-fix.md) defers both setter interrupt enables while blocked, preserves the IRQ-disabled readiness predicate and restores interrupt delivery through admitted attach. Tests and local image checks pass; no installation or board trial has occurred. Later hardware predicates may still fail once this first mismatch is addressed.

Leave the board recovered/read-only with external power/settings unchanged and PICkit5 disconnected. The PAUSE and recovery permissions are spent. No repeat PAUSE, higher ID, reset, reprogramming, replacement package, uploads, NVM/protection changes or fault tests are authorised. Source changes, package review and any hardware evaluation require their own verification and scope.
