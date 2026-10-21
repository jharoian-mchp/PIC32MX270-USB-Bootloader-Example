# Powered USBBUSY: reference findings and unresolved readiness

The official USB reference supports waiting for USBBUSY after rapid disable/re-enable, but it does not establish a general BUSY-clear deadline or guarantee BUSY0 while powered and detached. The current powered BUSY0 predicate is project-defined readiness policy. This investigation does not justify removing it or adding an arbitrary delay; no firmware or board change was made.

The callback-mask trial sampled USBPWR1/USBBUSY1 at DRIVER_READY and failed before reading USBEN. [Retained evidence](usb-rebuild-busy-evidence.md) establishes one sample, not BUSY duration, cause or permanent state. The board remains recovered/read-only.

## Official reference

Retrieved Microchip's [PIC32 Family Reference Manual, Section 27: USB On-The-Go (OTG), DS61126F](https://ww1.microchip.com/downloads/en/DeviceDoc/61126F.pdf). Local reference: `build/usb-fr-mx-61126f.pdf`, extracted with `pdftotext -layout` to `build/usb-fr-mx-61126f.txt`. PDF SHA-256: `a6ecf6521c5e3a2f2400c62b8b2acee062b724ef09a7402191d53e3c396fcdd6`.

The device-specific PIC32MX1XX/2XX datasheet DS60001168L lists Section27/DS60001126 among its references and implements USBBUSY in U1PWRC register10-5, page116. The retrieved F revision is older and explicitly makes BUSY availability device-dependent; the device datasheet confirms that the bit exists here. No claim is made that F is the latest manual revision.

### Register definition — page27-15

USBBUSY1 means:

> USB OTG module is active or disabled, but not ready to be enabled

USBBUSY0 means:

> USB OTG module is not active and is ready to be enabled

The same definitions appear in the inspected device datasheet. Both state that when USBPWR0 and USBBUSY1, status from other USB registers is invalid and writes produce undefined results. USBPWR1 turns the module on. These definitions do not make BUSY a general transaction-completion timer or promise that it will clear in a powered state.

### Initialization guidance — section27.3.7.1, page27-49

USBPWR1 starts the USB clock and transceiver, permits interrupts and selects USB ownership of the pins. Clearing USBPWR resets the module and internal registers; initialization must be repeated after enabling. The manual adds:

> If the USB OTG module was previously active and was quickly disabled and re-enabled, there is a chance that the module may still be finishing the previous bus activity. In this situation, the firmware should wait for the USBBUSY bit (U1PWRC<3>) to become cleared before attempting to configure and enable the module.

The note is conditional, gives no interval/timeout and refers readers to device-specific bit availability. Its wording does not unambiguously place the wait relative to USBPWR reassertion versus USBEN/mode activation. The adjacent mode section27.3.7.3 distinguishes USBPWR from USBEN/SOFEN; USBEN enables device connection through the D+ pull-up, and other control registers must be initialized before enabling through the mode bits.

The existing off-idle gate is consistent with the guidance and with the explicit disabled-busy register warning. That does not prove the manual mandates our exact sequencing or that the later powered-ready check should be deleted. Neither source establishes a delay such as1ms/10ms as a correction.

## Generated driver and project policy

| Path | Inspected behavior |
|---|---|
| `usb_ModulePower_32Bit16Bit.h` | `USB_Enable_32Bit16Bit()` sets USBPWR1; disable sets0. No wait or readiness result |
| `usb_ModuleBusy_Default.h` | `USB_ModuleIsBusy_Default()` directly reads USBBUSY; no settling interpretation |
| `drv_usbfs.c`, `DRV_USBFS_Initialize()` | Enables power, configures driver/device state and BDT, returns SYS_STATUS_READY without a BUSY wait |
| `drv_usbfs_device.c`, `F_DRV_USBFS_DEVICE_Initialize()` | Clears software attached/suspended/VBUS flags and disables peripheral interrupts; no BUSY check. Its descriptive comment mentions session-valid enable, but the body disables all interrupt groups |
| `usb_OpModeSelect_Default.h` | Clears USBEN/SOFEN, HOSTEN and OTGEN before choosing a mode; DEVICE sets USBEN, NONE leaves them clear |
| `drv_usbfs_device.c`, Attach/Detach | Calls mode selection for DEVICE/NONE respectively. Rebuild does not call Attach before readiness |
| `usb_retirement.c` | Off-idle requires USBPWR0/BUSY0 before retirement/preparation |
| `drv_usbfs.c`, replacement readiness | Rechecks software/IRQ predicates, samples U1PWRC and requires BUSY0 before reading USBEN/queues/BDT |
| `update_nvm_port.h` | Treats USBPWR or USBBUSY set as active; flash ownership therefore requires both clear |

The powered replacement-ready BUSY0 check was introduced by the project in `a4632ad` with detached rebuilding. It is not a condition enforced by the original generated initializer. The generated SYS_STATUS_READY is software-object status; it does not independently prove hardware readiness or authorize USB-off flash writes. Absence of a driver wait does not negate the reference's conditional re-enable guidance.

In this implementation, off-idle and preparation are checked before power is reasserted. Rebuild then powers USB, constructs/binds the replacement and checks readiness synchronously. The earlier BUSY0 observation is evidence before that new enable, not a promise about the later sample. The source shows no separate post-enable observation window. The record does not show USBEN, whether the host caused activity while detached, clock/transceiver settling, or why BUSY was asserted.

## Verification and limits

- Verified official PDF identity/text, register definitions and initialization guidance against the local device datasheet.
- Inspected exact PLIB power/busy/mode templates, generated initializer/device initializer, retirement/preparation/readiness and the NVM guard. Git history identifies the project's introduction of powered BUSY0 readiness.
- Reran strict exact owning-TU retirement and default-off/opt-in callback/attach tests; they pass. Mock registers cannot establish actual powered BUSY behavior.
- A broad independent source investigation timed out. A bounded independent interpretation review completed and confirmed the main limits: the wait placement is ambiguous, and neither the manual nor the board sample establishes a permanent powered BUSY value or a clearing deadline. That review was supplied the inspected excerpts; it was not a separate hardware/manual experiment.
- The workspace web-search helper could not run because `turndown` was unavailable. The official PDF was retrieved directly with `curl`; no dependencies or environment settings were changed. Guessed newer document URLs did not yield usable references.

No firmware source, HEX, readiness ordering, USB-off NVM predicate or protection setting changed. This research supplies no new hardware qualification.

## Next bounded work

The subsequent [power-observation design](usb-power-observation-design.md) specifies three one-shot evidence sites, raw CP0 timing, partial-record semantics, a proposed retained-schema extension and a separate read-only query. It adds no wait or readiness correction and is not implemented. Any instrumentation still needs source tests, fixed-reservation/startup verification and separate delivery/trial approval.

A proposed wait must have justified semantics and a bound, remain under the required ownership/interrupt fence and not depend on an interrupt-driven timebase while interrupts are masked. It must not silently turn a failed readiness check into success. The independent USB-off NVM gate remains USBPWR0/BUSY0 with DMA exclusion throughout flash ownership.

Keep the board recovered/read-only with external power/settings unchanged and PICkit5 disconnected. The PAUSE and recovery permissions are spent. No repeat PAUSE, higher ID, reset, reprogramming, new package, upload/NVM/protection changes or fault tests are authorised.
