# Opt-in detached USB-only rebuild

`USB_RebuildDetached` reconstructs USB controller/device state and callback routes under the retirement fence, then reopens admission after private readiness checks. **It defaults off and has no active caller or board restart command.** This completes the staged initializer library work, not target restart or old-reference hardware qualification. NVM/session/clock/GPIO initialization is untouched.

## Retirement and preparation

The retirement gate now records three separate completed layers: HID, device and driver. Each owning retirement hook marks its layer only after authorized off/idle/drained reset. Combined/invalid mask reports are rejected; the caller cannot claim all layers with one combined value.

`USB_RetirementPrepare(generation)` requires the fence, successful power-off authorization, all three layer markers, no admitted calls/callbacks, off/idle hardware and a nonzero generation greater than the previous generation. Preparation is one-use: it blocks re-retirement or a second preparation while replacement construction is pending.

Preflight failure performs no USB work and does not poison an untouched stack. Once preparation succeeds, failure constructing/opening/binding/reopening the replacement poisons the gate and leaves admission fenced. Partial initialization may already have enabled USB power; failure is quarantine, not proof the module is off. The owner must retain resources and use a separately reviewed cleanup/reset path. There is no automatic retry of partial initialization.

## USB-only entrypoint

The checked overlay now includes generated `initialization.c`, with its own pinned base hash, so the entrypoint can use the existing private BDT and `drvUSBFSInit` without exporting them or recreating clocks/PLIBs.

With global interrupts saved/disabled, it:

1. Obtains the preparation generation.
2. Calls only `DRV_USBFS_Initialize` with the existing device/full-speed controller configuration. This enables module power, but device-mode initialization leaves it detached and interrupt-masked.
3. Calls `USB_DEVICE_Initialize` with `usbDevInitData`, creating EP0/device-layer state and invoking HID common initialization.
4. Calls `USB_DEVICE_Tasks` once, in its opening state, to open the controller. It does not run the normal driver task/attach loop.
5. Binds device and HID callback routes through owning-TU helpers that are allowed only during preparation. The device binder validates both the driver-interface pointer and its event-handler function before use.
6. Validates replacement readiness and reopens admission for the exact prepared generation.
7. Publishes the new device handle only on complete success; failure leaves the caller's handle unchanged. Restores previous global interrupt state.

It does **not** call `SYS_Initialize`, NVM initialization, APP initialization, ADC/mux/clock initialization, board attach logic or auto-run/reset. It does not reset update-session/ticket/boot-epoch storage. The future owner supplies callbacks/context for its new transport generation and controls later board/driver polling, attachment and interrupt enable.

Input callback/context and handle storage belong to that trusted owner and must outlive the new instance. Do not alias handle output with live driver/device/system state. This is an internal library API, not an exposed host command.

## Readiness before admission reopens

The central gate rechecks preparation, exact generation, no poison/admitted API/callback, and these owning-layer predicates with interrupts excluded:

- HID common mutex initialization succeeded, the ledger is empty, direction counts are zero and the new app callback is bound. Descriptor/endpoints initialize later via normal host configuration after reopening.
- Device is READY/RUNNING, client open, descriptor/interface/callback routes exist, state remains DETACHED and EP0 IRPs are not pending/in progress.
- Driver is ready/open/in-use with callback route, detached/not in ISR, software endpoint queues empty, BDT present without any UOWN entries, USB enable/attach bit clear, CPU USB interrupt source masked and module not busy.

Retirement resets prior callbacks/descriptors/objects before these predicates can become true again. Readiness has no independent cryptographic/generation tag in those generated structs; the trusted builder and preparation token supply software sequencing. Arbitrary old pointers after reuse still cannot be authenticated as belonging to the previous transport generation. Full cancellation/off-idle/interrupt/task ownership must establish old references are gone before this initializer is invoked.

After reopening, the power-off/retired-layer/preparation markers clear and the recorded generation advances. Replays of reopen or preparation for an old generation fail. A subsequent restart begins a fresh fence/retirement cycle. No coordinator target ports or host-generation handshake use this automatically today.

## Reviews and fixes

Bounded initializer review found:

- Preflight rejection originally poisoned the gate before any mutation. It now returns cleanly; only post-preparation failures quarantine.
- Device callback binding dereferenced an unchecked interface member. Both interface and callable pointer are now validated.
- Handle publication happened before the final steps succeeded. The initializer now uses a local handle and publishes only after full reopen.

The follow-up core review found no retry/cycle defect under trusted serialized ownership, but identified caller-dependent layer markers and generation-specific readiness. Layer markers now accept only the individual HID/device/driver values and require retirement authorization. The readiness predicates validate freshly rebuilt private state, but hardware old-reference/generation provenance still requires target-owner qualification.

## Verification (2026-10-16)

- Owning-hook mocks test missing layer/preparation, stale/wrong generation, incomplete device/interface/callback readiness, attached hardware, successful reopen and new-cycle fencing.
- Exact overlaid initializer-body tests inject failure at preparation, controller/device construction, callback binding and reopen. Preflight is non-poisoning, partial failures quarantine, output handle remains unchanged, and successful ordering is USB-only.
- `make test` and `make test-sanitize` pass: fifteen C executables, 16 Python methods plus source, owning-hook, cancellation, HID submission and rebuild harnesses/Bun checks.
- `make usb-retirement-check` compiles the central core, owning USB files **and initialization.c** with hooks enabled. An enabled link check retaining `USB_RebuildDetached` passes at text30410/data84/bss1596. This compile-check image is **not approved for programming** and has no restart caller.
- Default `make bootloader` remains text26670/data80/bss1568 and excludes rebuild/retirement/update execution symbols. Shared MPLAB XML remains 40 C sources and includes the rebuild header.

Next implement/review the actual coordinator ports for all-path cancellation, callback-generation handoff, power-off/retire/rebuild and owner-controlled resume, then connect a **read-only** diagnostic command and qualify restart on the board. Isolated NVM/protection, application regeneration/relocation and connected uploads remain later gates. No reprogramming is requested for this inactive stage.
