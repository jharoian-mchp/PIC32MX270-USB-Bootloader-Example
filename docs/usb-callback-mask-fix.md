# Callback binding keeps detached rebuild interrupts masked

The source-only fix defers callback registration's interrupt enables until admitted attachment. The driver-ready predicate still rejects an enabled USB interrupt source. Strict/sanitizer tests, generated-source checks and the fixed 32 KB image audit pass. The change has not been installed or hardware-qualified; no replacement package or new board trial is authorised.

The delivered `bae3050` image failed at DRIVER_READY/irq_enabled/mask512. [Retained evidence and source diagnosis](usb-rebuild-irq-diagnosis.md) explain that callback registration enabled the USB interrupt source before readiness validation. The board is recovered and stays read-only.

## Deferred enabling

`DRV_USBFS_ClientEventCallBackSet()` still validates the handle and stores the callback/context. In device mode, the opt-in overlay checks `USB_RetirementBlocked()` before the existing OTG session-valid and CPU interrupt-source enable operations. A fenced or poisoned stack therefore receives no new interrupt enable from this setter. During preparation, binding stores the replacement callbacks while both interrupt layers remain masked.

The check uses the full blocked state, including preparation and partial-failure quarantine. It does not silently clear an already enabled source: readiness still rejects that condition with mask512. The ordered HID/device/driver predicates and USB register read order are unchanged.

`USB_RetirementReopen()` clears the fence only after the existing readiness predicates pass. It does not attach or enable the interrupt source. The owner publishes the new handle, initialises the probe and reaches its existing cable/mux-gated attachment path under global interrupt exclusion.

The opt-in `DRV_USBFS_DEVICE_Attach()` now acquires admission before changing attach state or touching USB hardware. A fenced/poisoned stack returns without either operation. An admitted call uses the original pull-up/device-mode and peripheral-interrupt configuration, then enables the CPU USB interrupt source. Existing attachment already configures the OTG session-valid interrupt; no new deferred state or replay API is needed. The source enable was necessary here because the generated attach function previously relied on callback registration having enabled it.

Default `USB_RETIREMENT_HOOKS=0` behavior is unchanged: ordinary callback registration enables its interrupts as before, and default attachment retains its original implementation. The default recovery HEX hash remains `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`.

## Tests and regeneration

`tools/test-usb-retirement.ts` now extracts the exact callback setter and attach bodies in addition to the owning-TU readiness hooks. The expanded integration test verifies:

- Ordinary callback registration stores callback/context and enables both interrupt layers; invalid/non-device paths do not enable them.
- Fenced/preparing/poisoned registration defers both enables while retaining callback/context updates.
- Fenced, failed-readiness and poisoned attach calls make no attach/register changes.
- An externally enabled source still rejects readiness with mask512; USBEN/USBBUSY failures retain their independent rejection checks.
- Successful reopen leaves the source and OTG session interrupt masked; admitted attach enables peripheral interrupts before the CPU source and releases admission.

`tests/usb_callback_binding_test.c` separately compiles the exact generated functions with hooks0 and hooks1. It checks default-off parity, invalid handles, non-device callbacks, blocked binding/attach and ordinary enabled attachment. These two executables run from the existing retirement test target, including ASan/UBSan invocations.

`tools/usb-retirement-overlay.ts` strips the new inline edits and admission guard before checking the original MCC base hashes. Its function matching now excludes comment prototypes while preserving source offsets. A disposable-workspace round-trip stripped all five overlays, verified the unchanged bases, reapplied the new overlay and obtained byte-identical generated files. This catches wrong placement and regeneration drift without changing the base hashes.

Verification passed:

- `make test` and `make test-sanitize`: existing 21 C executables and generated-stack harnesses, two added default-off/opt-in binding executables, and 22 Python methods.
- `make usb-retirement-check`: overlay validation, enabled XC32 owning-TU compilation and exact-hook/binding tests.
- `make restart-image-audit` and `bun tools/test-restart-image-audit.ts`: linked spans/HEX/configuration/symbol/startup checks and negative artifact regressions.
- `git diff --check` and the disposable five-file overlay round-trip.

The local opt-in link is text32117/data88/bss1952; default text26670/data80/bss1568 is unchanged. The 32 KB reservation, application origin `0x1D008000`, schema-3/30-byte PREVIOUS format and retained 96-byte journal remain unchanged. The opt-in image has little margin. Audit flags remain `hardware_qualified=false` and `board_delivery_approved=false`.

A bounded source/test review found no concrete must-fix: binding defers both enables, admitted attach restores the source after peripheral configuration, and the owner preserves global exclusion and cable/mux scheduling. The earlier design-only review timed out; the actual changed-source review completed. Hardware/PLIB/EVIC accesses remain mocked in host tests.

## Board and delivery limits

This addresses the source mismatch responsible for the first observed readiness rejection. It does not establish USBBUSY, USBEN, later queue/BDT readiness, enumeration after rebuild or successful USB-only restart on hardware. The delivered board still runs the older image and its PAUSE/recovery permissions are spent.

Keep PICkit5 disconnected, external power/settings unchanged and the recovered board read-only. No repeat PAUSE, higher ID, reset, reprogramming, replacement package, NVM writes, protection changes, uploads or fault tests are authorised. The subsequent [candidate review](usb-callback-mask-candidate-review.md) passes its bounded offline source/artifact checks and defines future package requirements. No archive or board permission follows from that review; explicit preparation/delivery and evaluation approval are still required.
