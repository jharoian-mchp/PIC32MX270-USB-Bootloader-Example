# Cancellation callback tests and initializer readiness

Selected real generated cancellation/HID callback bodies now run together with the retirement admission gate in host tests. The opt-in HID callback overlay also rejects invalid pool pointers and zero-count completions before dereferencing instance/count state. **The initial aggregate guard did not establish per-IRP completion ownership.** A subsequent [per-slot ledger](usb-irp-ledger.md) claims completions within each reservation lifetime; generation-safe reuse after restart is still unimplemented. Hooks remain default-off; USB-only initialization/thaw and board restart are not implemented.

## Generated cancellation path under test

```sh
make usb-cancellation-check
make test-sanitize
```

`tools/test-usb-cancellation.ts` extracts the exact current driver `F_DRV_USBFS_DEVICE_IRPQueueFlush` and HID RX/TX callback bodies and records body hashes. It compiles them with the real retirement counter/gate core and minimal host private-field/register mocks.

The legal synchronous trace is:

1. Fence admissions with no active API/callback.
2. Flush linked RX/TX IRPs as ABORTED; the driver's callback wrapper accounts each callback.
3. The real HID callback decrements its direction count and delivers the app notification.
4. The test app observes callback accounting, retained buffer contents and fenced requeue attempts.
5. Queue becomes empty and callback counts return idle; a repeat empty flush has no duplicate notification.

This proves selected software execution, not complete driver/ISR/bus timing, EP0 cleanup, application teardown, OS cancellation or actual buffer freeing. The mock app observes release notifications; it does not implement the application's full lifetime policy. Extractor hashes are recorded, not pinned acceptance values, and its brace scanner is tailored to these known bodies rather than a complete C parser. Overlay base hashes and shape checks separately detect changes to supported generated files.

## Invalid completion quarantine and remaining duplicate risk

The opt-in `USB_RetirementHidCompletionValid` checks that the pointer is an aligned entry in the static HID pool before reading it, userData names an existing instance, IRP status is terminal/free, and the appropriate queue count is nonzero. A failure poisons the retirement gate and skips the generated callback; no app notification or count decrement occurs. Callback accounting still returns on its normal wrapper path; poisoned state prevents retirement authorization.

A fault-injected direct callback after a zero count used to underflow that count; it is now rejected. This injection is explicitly not evidence that the hardware legally delivers such a callback. Review also identified that a stale terminal pool entry can pass the guard if **another** same-direction IRP keeps the aggregate count nonzero. The regression tests record that limitation. There is no per-IRP consumed/armed state or true generation tag yet; the guard must not be used as an exactly-once or late-generation guarantee.

Before reopening admissions, track/claim each legitimately submitted IRP's completion identity atomically, cover submit failure and synchronous callbacks, and preserve active buffers until claims are settled. An old pointer reused as a new pool entry cannot be distinguished by aggregate counts. Thaw stays unavailable until a reviewed per-IRP and transport-generation policy exists.

## USB-only initialization source trace

- `DRV_USBFS_Initialize` creates its driver mutex, assigns the BDT/endpoint table and enables USB module power. For device mode, `F_DRV_USBFS_DEVICE_Initialize` clears attachment/suspension state and disables all USB interrupt sources. This is not a proof of complete high-layer readiness.
- `USB_DEVICE_Initialize` creates EP0 RX/TX control IRPs, device-layer queue/mutex state, controller handles/tasks, and invokes function-driver global initialization. Current HID global initialization preserves the one-time common mutex guard.
- `USB_DEVICE_Tasks` later opens the controller and advances to running/ready. The driver tasks can subsequently enable their interrupt sources; do not let normal board task/attach handling run prematurely.
- HID descriptor initialization and event-handler replacement are currently admission-fenced by the overlay. Descriptor initialization belongs to later host configuration, not merely initial module construction.
- Whole `SYS_Initialize` also alters GPIO/clock/NVM/boot decisions and must not be called to reconnect USB.

A safe replacement initializer must separately establish controller, EP0, function/private state and callback routes while old submissions/completions remain retired, then reopen admission at a reviewed transition before descriptor/config traffic. It must preserve update/NVM session storage and boot epoch, use a new transport generation, and quarantine partial initialization. These fields/order have not been implemented as target ports. The delegated initialization trace timed out; the facts above were inspected directly.

## Review, tests and next work

Independent bounded harness review limited the claims to selected software behavior and noted extractor/mock limits. A callback-guard review identified the aggregate-count duplicate gap, which is documented and explicitly tested. No complete initializer or target-generation approval is claimed.

`make test`/`test-sanitize`, opt-in enabled `make usb-retirement-check` and default `make bootloader` pass. Existing fourteen C test executables plus source/hook/cancellation harnesses, 16 Python methods and Bun self-tests remain green. Default diagnostic footprint and update-symbol exclusions remain unchanged; no programming is required for this source/test stage.

Per-IRP reserve/claim/release now exists and is mock/source-tested. Next implement/review a staged USB-only initializer and old-reference/generation reopen policy. Only after those pass and the actual cancel/off-idle/controller binding is reviewed can a read-only board restart diagnostic be delivered. NVM/protection/application/upload gates remain separate; uploads are disabled.
