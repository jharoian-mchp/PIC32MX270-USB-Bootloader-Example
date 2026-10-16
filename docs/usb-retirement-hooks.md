# Opt-in USB stack retirement hooks

A checked overlay adds private-state hooks inside the generated files that own HID, device and USBFS storage. **`USB_RETIREMENT_HOOKS` defaults to 0.** The live diagnostic behaviour, footprint and read-only handler remain unchanged. No target coordinator port, cancel sequence, USB-only initializer, thaw or board restart command is connected.

## Regeneration overlay

```sh
bun tools/usb-retirement-overlay.ts --check
# After reviewing regenerated sources that match the supported base:
bun tools/usb-retirement-overlay.ts --apply
make usb-retirement-check
```

The overlay pins SHA-256 hashes of the four supported generated base files. It rejects changed generator output until the new base and insertion/callback patterns are reviewed. Rechecking an applied overlay reconstructs its unpatched base and compares the complete intended content, rather than accepting a marker alone. All files/patterns are validated before writes; application is repeatable, but filesystem write failures are not an atomic multi-file transaction.

Files: `usb_device_hid.c`, `usb_device.c`, `drv_usbfs.c`, `drv_usbfs_device.c`. Generated modifications consist only of the marked include/guards/callback instrumentation and appended opt-in hooks. Owning-TU access avoids exporting static globals or importing private structs into the board module. Preserve these marked changes during MCC regeneration or reapply the checked overlay before build.

## Admission and callbacks

The central `usb_retirement.c/.h` gate has a one-shot fence, admitted-API count and callback count. IRQ exclusion protects counter changes. Fencing fails while an API or callback is active. Counter underflow/overflow poisons the gate; no silent wrap/reset occurs.

Guards wrap HID send/receive/event-handler replacement/descriptor initialization, device ControlSend/Receive/Status, and driver IRPSubmit. GCC cleanup scopes release admission on every normal C return. Lower IRPSubmit is also fenced, so callbacks cannot queue another transfer after fencing. Direct/private callers and unrelated stack mutation paths still need target-owner review; these guards are not a general multithread lock.

Thirteen driver IRP/event callback dispatch sites are counted, including flush/abort paths. Existing callbacks are **not dropped merely because the fence is set**: cancellation must still complete ownership release. Callback count blocks drain/retirement until return. A callback wrapper is accounting, not a late-generation routing gate. The future transport owner must also prevent reset/task/reconfiguration and ensure old callback sources are quiescent before retirement; there is no new callback-dispatch cancellation policy here.

A subsequent opt-in HID completion guard rejects invalid/misaligned pool pointers, invalid instances/status and zero direction counts, poisoning the gate before the generated callback can underflow. It does not provide per-IRP exactly-once or generation identity: a stale pool entry can pass when another same-direction transfer remains. [Cancellation/initializer readiness tests](usb-cancellation-readiness.md) record that limitation; thaw stays unavailable.

The trusted owning driver sets a power-off authorization latch only after IRQ-excluded full drain checks and module disable. Retirement requires that latch, the fence, no admitted APIs/callbacks and off/idle status. Each final drain check plus private reset is IRQ-excluded. A subsequent [detached rebuild entrypoint](usb-rebuild-detached.md) adds guarded preparation/readiness/reopen after all layers retire. It remains opt-in/unconnected; no unconditional thaw exists. Do not bypass the gate to make current enumeration work after retirement.

## Drain and retirement hooks

- HID: all shared IRP pool entries terminal/free (`<=COMPLETED_SHORT`) and TX/RX counters zero. Retire only after authorization; reset retired IRP/instance storage, preserving the shared basic-OSAL mutex and its one-time guard.
- Device: both EP0 RX/TX IRPs are not pending/in progress (`<=SETUP`). The current generated `USB_DEVICE_OBJ` has no third “other-speed descriptor IRP”; that earlier assumption was corrected during enabled compilation. Logical control state is discarded only after hardware/IRP retirement, not used to fabricate drain.
- Driver: endpoint queues empty, not in interrupt context, BDT present. Ordinary UOWN descriptors block drain. UOWN|STALL may deliberately persist with no IRP; do not clear it before off/idle. Power-off retains buffers/BDT/object/mutex state, masks USB IRQ ownership and disables USB.
- Driver retire: only authorized off/idle/drained state calls generated deinit, clears BDT and endpoint/object state. BDT pointer is saved before deinit. Current configuration has one device-mode instance; shared/multiple-controller lifetime and other OSAL variants need fresh review before extension.

These private resets discard complete instance fields, including callbacks/descriptors/configuration state. They are permissible only after the reviewed predicates and must later be reconstructed coherently. There is no claim that current initialization APIs already supply that reconstruction under the fence. Shared HID mutex preservation is for the current basic OSAL; no live mutex is freed while its references remain.

## Review and limits

Initial bounded review found entry-only fence sampling could miss an in-flight API, retire checks lacked a power-off prerequisite/atomic reset, and a later thaw could reopen incompletely reconstructed state. Fixes add admission scopes, the power-off latch, IRQ-excluded reset and no thaw; BDT pointer/ownership checks were tightened too.

Follow-up review noted callbacks still execute while fenced and final checks need IRQ exclusion. Accounting intentionally lets old cancellation callbacks retire ownership; the atomic final checks are IRQ-excluded. **No full target late-callback/generation guarantee is claimed.** More source/port review is required before these experimental hooks are enabled on a board.

The gate/core and owning-hook bodies pass mock tests for admitted-API fence rejection, cleanup on early/normal return, callback drain, HID/EP0/endpoint/BDT refusal, retirement without power-off refusal, powered-down-but-busy retention, off/idle private reset, and poisoned-counter quarantine. The tests extract owning hook bodies but use minimal mock private fields; they are not whole generated translation-unit host execution or actual ISR/USB hardware tests.

`make usb-retirement-check` compiles all five enabled target files with XC32 and runs those mocks. `make test`/`test-sanitize` add hook checks, alongside fourteen existing C executables, exact-source audit and 16 Python methods. Default `make bootloader` stays text26670/data80/bss1568; inactive hook/controller/update symbols are absent. Shared MPLAB XML lists 39 C sources. No hardware programming performed.

Selected generated queue-flush/HID callbacks now have executable tests. A subsequent [per-IRP ledger](usb-irp-ledger.md) adds reserve/claim/release tracking and selected real-HID API tests. Next review full late-event/old-reference paths and implement replacement USB-only initialization without reopening admission prematurely, then bind a **read-only** restart diagnostic and qualify it on target. NVM/protection/application/update gates remain separate. This step does not complete the target restart checklist item.
