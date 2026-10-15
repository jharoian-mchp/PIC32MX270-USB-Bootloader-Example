# Proposed USB-off update session

The existing version-1 dispatcher cannot drive the target backend while HID is powered. This proposal separates **staging** from **execution**, keeps USB off for each accepted flash operation, and reconnects to report its result. **The USB/wire contract is not implemented or hardware-qualified.** A typed inactive lifecycle core now tests these boundaries; see [the core API](update-lifecycle.md). It is not a supported wire protocol. The deployed endpoint and Python transport remain read-only.

## Why version 1 cannot be connected directly

- `UPDATE_DispatchRequest(BEGIN)` calls `UPDATE_SessionBegin`, which submits metadata erase immediately. It cannot send an acceptance reply first and power USB off afterwards.
- BATCH stages bytes, but polling can submit row writes, verify the complete image and commit. Those tasks require an explicit USB-off execution phase.
- ABORT can submit a metadata erase after outstanding work settles. Abort/recovery must also pass through USB-off execution.
- `UPDATE_DispatchDisconnect` aborts any incomplete session. Intentional USB removal needs a separate bounded lifecycle state; globally suppressing disconnect abort would weaken recovery.
- `UPDATE_NvmInit` rejects powered/busy USB. Creating an enabled dispatcher after enumeration therefore also fails. Initial NVM/session storage must be established in a confirmed USB-off, quiescent phase; USB-layer reinitialisation must not reinitialise that storage.

Keep version-1 offline plans and the live CAPABILITIES vector unchanged. A future stage/execute/resume protocol requires explicit version/feature negotiation, new bounded schemas and matching C/Python fixtures. Do not reuse version-1 mutation payloads with different execution semantics. Opcode assignments and encoding are not selected here.

## Proposed operation cycle

Names below describe semantics, not implemented commands.

1. **READY_LINK**: return protocol/features, a fresh nonzero boot-instance epoch, session token, last accepted/executed mutation, batch sequence and operation state. The epoch changes at reset; state is not persisted as a resumable update across reset. Identify the exact HID interface; the current device has no serial string.
2. **STAGE**: validate the complete manifest, binary HEX batch, or abort request; copy it into bounded persistent staging storage. No flash operation occurs. Only one operation may be staged. Respond with epoch, token, request ID, batch sequence and a nonzero one-use operation ticket. Exact repeats return the same ticket; changed bytes under that identity are rejected. Queries do not evict the slot.
3. **EXECUTE**: require the exact current epoch/token/ticket/request/sequence. Freeze the staging slot and stop accepting further mutations. Respond `PAUSE_ACCEPTED` with the operation identity and bounded reconnect expectations. This is acceptance only; it is not verified execution or commit.
4. **QUIESCE**: only after successful **local HID transfer completion** of the full PAUSE_ACCEPTED response, deliberately detach/stop the transport, retain/cancel outstanding buffers and establish driver/module shutdown. Require USBPWR=0, USBBUSY=0 and DMA off. If response delivery cannot be confirmed, do not start flash; cancel safely and expose uncertainty/failure when link service resumes. HID send-completion confirms transfer ownership/status, not application receipt. This local completion is the execution-authorisation boundary; a host timeout after EXECUTE has outcome **unknown** and must query the same ticket. It cannot cancel or replay the mutation based on timeout alone.
5. **EXECUTE_OFF**: invoke the remembered operation once, using the persistent session/NVM context. Poll through verified erase/write completion. Stop after the staged operation has drained to READY, COMPLETE, ABORTED or FAILED. Keep USB and DMA off for the entire operation, verification and timeout quarantine. EOF may verify the image and commit in this same offline phase. No auto-run/reset follows completion.
6. **RECONNECT**: with NVM idle, settled and owned write buffers released, recreate/reconfigure the USB layers through a reviewed driver lifecycle and resume enumeration. Keep the boot epoch, update session, mutation cache, operation ticket/result and image state intact. New USB transfer generations invalidate old callback handles; do not recycle pending buffers.
7. **RESULT/RESUME**: the host opens the uniquely selected interface and queries the operation identity/state. It sends RESUME with the same epoch/token/ticket/last operation before another STAGE is accepted. Require matching sequence and verified outcome. A different epoch, wrong identity, unknown ticket, FAILED or ABORTED stops upload; do not silently begin another erase.

A second EXECUTE for an operation already executing/executed returns its state and never invokes flash again. A stale STAGE/EXECUTE/ticket is rejected. Mutation IDs and sequences do not wrap. After confirmed RESUME, keep a latest-completed replay record so lost RESUME replies can be retried without an extra operation. Cache eviction and query behaviour need tests before schema finalisation.

The boot epoch is a freshness discriminator, **not authentication**. Its generator, collision/restart policy and identity representation are not implemented. A 32-bit host token alone is insufficient evidence of continuity across reset. With no serial number, safe automatic reconnect among multiple same-VID/PID boards is also unresolved; fail on ambiguity and qualify a stable OS path/physical-port or explicit operator selection policy.

## Disconnect, timeout and abort rules

- **Before EXECUTE acceptance**: an unexpected reset/disconnect cancels the staging slot. If no NVM work has occurred in that session, the previous committed image may remain valid; do not claim every cancellation already erased metadata.
- **After PAUSE_ACCEPTED completion**: only main-task ownership may enter intentional QUIESCE. The bounded pause covers the one identified operation and reconnect/resume window, not arbitrary later disconnects.
- **Unexpected cable/USB reset before planned shutdown**, ADC cable loss during/offline after the operation, reconnect failure or resume expiry: mark the update aborted and stop normal work. Cancel the paused transaction; do not automatically resume the next batch.
- **Abort cleanup**: wait for any NVM operation to settle, then establish/retain USB-off status before verified metadata invalidation. If a timeout leaves hardware busy, retain source storage and forbid USB reinitialisation/reuse until completion; no timer cancels hardware. If USB cannot be made inactive, do not launch an erase merely to complete an abort. Remain non-runnable/recovery-only in that runtime and record that metadata may still be present until cleanup succeeds.
- **COMMITTING abort**: after an in-flight commit settles, invalidate metadata if the session was cancelled before declaring operation completion. Report cancellation pending/unknown until invalidation verifies; never report ABORTED while that commit may remain valid. Once the controller classifies the verified operation as DONE+COMPLETE, normal cancel is too late and an explicit staged ABORT is needed. An image might otherwise have a valid commit record; runtime recovery mode cannot replace persistent invalidation across reset. Analyse/test this boundary explicitly.
- **COMPLETE operation**: a committed image survives ordinary later disconnect; explicit user abort follows the documented invalidate-completed-image policy, through a new offline cleanup operation. Do not treat a lost final RESULT reply as proof that commit failed or rerun it.
- **Reset/power loss**: boot-instance epoch changes, volatile operation tickets disappear and host continuation is refused. Stored-image validation controls later boot acceptance; there is no cross-reset update resume.

Deadlines for staging, response, shutdown, NVM progress, reconnect and resume must be explicit and bounded. They are not assigned measured values here. Core Count provides elapsed time across IRQ-disabled intervals; USB/DMA must stay off while NVM owns buffers. A short cable disturbance hidden by CPU stall/ADC sampling cannot be inferred as detected—qualify observable failure behaviour on hardware.

## Driver lifecycle requirements

Inspected source locations:

- `usb_probe.c`: persistent RX/TX handles and buffers; reset/disconnect uses `USB_DEVICE_IRPCancelAll`. Callbacks can be synchronous. Pending buffers are quarantined until release.
- `drv_usbfs_device.c`: UOWN transfers BDT ownership to USB; head `IRPCancel` can defer completion; `IRPCancelAll` clears endpoint BDT flags and flushes software IRPs. Endpoint cancellation is not a module-idle guarantee.
- `USB_DEVICE_Close` clears client use/open state. `USB_DEVICE_Deinitialize` only marks the instance uninitialised; it does not independently release all driver resources.
- `DRV_USBFS_Deinitialize` clears instance/open/callback state, deletes its mutex, disables/clears USB interrupts, then disables module power. It does not supply a complete high-layer HID teardown/cancel contract or a USBBUSY timeout wait.
- Initialisation creates driver mutex/BDT state and device-layer state. Calling `SYS_Initialize` to reconnect would also touch clocks, GPIO, generated NVM initialisation and boot/handoff decisions; **do not use it as a USB-only restart**.

Before implementation, trace every device/HID/IRP close, callback and instance allocation in the generated stack. Define cancel/close/device deinit/driver deinit ordering, final pending-transfer acknowledgements, polling ownership and when mutex/buffer storage may be released. Once USBPWR=0/USBBUSY=1, avoid other USB register access until idle. The owner must block `BOARD_USB_DIAG_Tasks` attach/reconfiguration and relevant interrupts while restarting; normal diagnostics currently keep servicing USB. A generation/handle design must reject late callbacks after reinitialisation without dropping completion needed to release a live buffer.

Teardown and reconnect must be independently source-reviewed and measured on target. This document does not assert the listed APIs alone establish a safe restart.

## Bounded storage and throughput

Each STAGE stores one manifest/abort or at most 496 HEX record bytes, matching the existing planner's record bound. Session state, three row copies and active flash source survive USB shutdown. Whole-image RAM staging is unavailable: maximum application span is 228352 bytes, SRAM is 65536 bytes before runtime overhead.

At three 128-byte data records per 496-byte batch, a maximum image needs roughly 600 operation cycles with the present record format. Re-enumeration per cycle may be impractically slow. Measure realistic latency before choosing this as a usable uploader. Increasing batch storage needs a separate memory/throughput review; shortening bus-exclusion requirements without evidence is unacceptable. A documented arbitration-based live-USB policy might prove preferable, but is not qualified by this proposal.

## Acceptance tests before code/hardware delivery

Offline C/Python tests must prove:

- STAGE, rejection, query and lost replies cause no flash effects; output-capacity failures precede side effects.
- EXECUTE requires exact epoch/token/ticket/sequence, confirmed response transfer, shutdown and inactive module status before the first NVM register/flash call.
- Duplicate EXECUTE, RESULT and RESUME never repeat erase/write/commit; changed/stale identities and ambiguous device selection fail.
- Shutdown/send/cancel/reconnect/lease failures enter bounded recovery; active buffers survive timeout and late callbacks.
- Unexpected cable/reset events retain ordinary abort semantics outside a single identified pause.
- Metadata invalidate-first, EOF hash/commit-last and commit-abort boundaries work across power-loss snapshots.
- USB-layer generations reset only USB state, preserving session/NVM context. Mock traces forbid USB reenable while hardware remains busy.

The inactive typed `update_lifecycle.c/.h` now supplies staging/ticket/ACK/off/result/resume transitions with mocked session/flash tests. A bounded independent contract review identified the local-send and commit-cancellation linearisation issues; the definitions above resolve them. A full controller code-review attempt timed out; a subsequent fragment review found the declared completion/cancellation boundary consistent under its single-owner/invoked assumptions. A subsequent full core review found ticket-output aliasing, now fixed/tested. Separate offline [version-2 codecs](lifecycle-wire-v2.md) define packet/feature/identity layouts; there is no request dispatcher, epoch generator or generated USB teardown binding yet. The live endpoint remains CAPABILITIES-only; version-1 offline plans are unchanged. Follow [the gated hardware test plan](nvm-hardware-test-plan.md) for the separate board qualification sequence.
