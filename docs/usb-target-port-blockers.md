# Target USB restart ports — supported-API blockers

**The target restart ports are not implemented.** Source checks and an exact-function audit show the current public API set cannot by itself establish the all-path fence/drain/retirement contract. Do not attach `usb_restart.c` to a chain of existing calls and claim a safe board restart. No firmware changes, programming or flash operations are needed for this audit stage.

## Supported versus missing operations

| Required port | Existing source/API evidence | Missing guarantee |
|---|---|---|
| Fence all submissions | Probe can stop its own reports; USB ISR/device tasks can still submit EP0 control work; USB_1_Handler calls driver handler directly | One owner must fence every HID/EP0/task/ISR submission path while still retiring old completions |
| Drain all old IRPs | Public cancel-all requests endpoint cancellation; probe knows two application transfer handles | No public all-HID-pool plus device EP0/control completion/reference snapshot |
| Disable callback route | HID EventHandlerSet installs handler/context but rejects NULL | No public callback-unregister/drained-generation retirement operation; replacing callback early can misroute a live completion |
| Power off while retaining state | Module template can clear USBPWR; driver Deinitialize bundles software-state clearing, mutex-delete call and power disable | No existing atomic two-phase API fencing task/ISR before shutdown and retaining software ownership until off/idle |
| Retire/recreate HID/device state | HID pool, instance and common mutex guard are private/static; control transfer lives in private device state | A user-owned port cannot safely clear/recount/rebind these through public setters alone |
| Reopen USB only | Driver/device Initialise APIs exist | Must first prove all old references gone and restore coherent handles/descriptor/callback/task state; whole SYS_Initialize is prohibited |

Private structures are implementation details, not a supported retirement API. Do not export/link to static globals, include private headers to poke counters, add a broad memset, or reset the entire system to simulate USB-only reinitialisation. Reset would destroy the volatile epoch/session/ticket continuity required by the proposed upload pause.

## Exact generated-function evidence

```sh
make usb-restart-audit
make test-sanitize
```

`tools/audit-usb-restart.ts` extracts five exact definitions from tracked generated source into ignored build output and records body SHA-256 hashes:

- HID GlobalInitialize, DeInitialize and EventHandlerSet.
- USB_DEVICE_Deinitialize.
- DRV_USBFS_Deinitialize.

The host harness compiles those unchanged bodies against minimal mock fields/APIs. It reproduces:

- HID global mutex creation is guarded once; subsequent calls preserve that common state.
- NULL HID callback registration is rejected; existing callback/context remain.
- HID deinit requests cancellation/endpoint disable and clears flags. Deferred mock cancellation leaves counts/references alive; synchronous mock cancellation can drain counters. **Deinit itself verifies neither outcome**. Callback state remains in both cases.
- Device deinit changes instance status but does not itself cancel functions, close the controller or clear client use.
- Driver calls OSAL_MUTEX_Delete before interrupt disable/clear and USB power disable; it supplies no drain or USBBUSY completion wait.

The **current basic OSAL mutex delete is a no-op**. This audit does not reproduce heap freeing or a use-after-free on the active runtime. It records the ordering of the delete call and the absence of ownership/off-idle guarantees. Different OSAL implementations would require a separate lifetime review. Do not overstate the mock's retained IRP as proof that the real driver always defers cancel-all; the real completion path and all references still need tracing.

The extractor covers selected definitions only, not complete translation units, actual register timing, all interrupts or the full HID/driver call graph. It fails if the selected function shape/source changes enough to invalidate extraction/expected evidence. A passing audit means these **blockers were reproduced**, not that restart is approved.

## Small explicit stack hooks needed

Before a real target binding, choose a regeneration-safe patch mechanism for the generated stack or an upstream/vendor-supported implementation. Keep the working diagnostic sources/generated state unchanged until that mechanism and test coverage are reviewed. Required semantics:

1. **Begin retirement**: atomically block all new submissions for one generation, record interrupt/task ownership, preserve existing queued buffers and callback routes. Reject reentry; do not suppress completions needed for drain.
2. **Cancel/drain snapshot**: request cancellation across control and function endpoints, then report old IRP pool/queues/control transfers/active callbacks as drained only after their ownership is released. The query must not manufacture idle from counters reset by descriptor init.
3. **Quiesce hardware**: with submissions fenced, mask relevant processing and disable USB power without discarding ownership state; wait bounded U1PWRC-only off/idle. No other USB register access while disabled-but-busy.
4. **Retire state**: only after full drain plus hardware off/idle, coherently release/reset private driver/HID/device state and callback generation. Basic OSAL and any future OSAL must have an explicit mutex strategy; do not delete the shared mutex while another instance/reference owns it.
5. **USB-only reinitialise**: restore driver/device/functions and callbacks with a fresh transport generation, independent of update/NVM/session/boot epoch storage. Stop on partial-init failure and retain quarantine state.

These hooks are a required contract, not newly available public functions. Their implementation needs full-source mocks (including real cancel callbacks/control submits), target compilation and independent ownership review. The working stack currently has one HID instance; shared global state still needs a deliberate lifetime policy rather than assuming one instance makes references safe.

## Read-only board test gate

No new board image is delivered now. Once the stack hooks/ports pass their source and mock tests, connect a **read-only** restart diagnostic with explicit host command/ACK identity and debugger traces. Prove transfer drain/off-idle/re-enumeration repeatedly, including cable/reset faults and old callback generations. It must contain no NVM write integration or uploader. Phase B in [the gated hardware plan](nvm-hardware-test-plan.md) applies.

The user-confirmed device is PIC32MX270F256D A1; protection bits remain unchanged. Do not switch PWP/BWP as part of USB restart testing. Isolated NVM/protection tests, regenerated/relocated application and connected writes stay separate gates.

## Verification and next work

Selected-function audit and full existing strict/sanitizer suites pass: fourteen C test executables plus one source-audit harness, 16 Python methods and Bun codegen self-tests. Diagnostic `make bootloader` remains unchanged; no new firmware sources were added in this audit step. Generated files are not modified. The delegated supported-API exploration timed out; parent source inspection and executable audit provide the stated facts. Independent audit review corrected two wording overclaims (counter persistence and mutex freeing); those descriptions now distinguish conditional mock outcomes and basic-OSAL no-op deletion.

Next implement/review the minimal retirement hooks under a regeneration-safe policy, then bind the inactive coordinator and deliver the read-only test image. This audit does not complete the target-port or hardware-restart checklist item.
