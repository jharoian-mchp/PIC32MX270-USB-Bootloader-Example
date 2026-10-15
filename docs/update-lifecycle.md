# Inactive ticketed update lifecycle core

`update_lifecycle.c/.h` implements a typed, mock-tested staging/execute/ACK/USB-off/result/resume controller around `UPDATE_SESSION`. **No wire schema, USB callback binding or target caller is connected.** Hardware writes stay disabled; no reprogramming is needed for this library stage.

## API and ownership

- `UPDATE_LifecycleInit`: initialise new/quiescent storage with a caller-supplied nonzero 64-bit boot epoch, flash timeout, phase timeout and explicit permission. A target backend must already be USB-off to initialise. Never overwrite a live session/source buffer.
- `UPDATE_LifecycleStage`: validate/copy one BEGIN manifest, BATCH (up to 496 binary HEX record bytes) or ABORT. No flash effects, including malformed HEX rejection. BEGIN requires valid length/new token and an idle/completed/aborted session. BATCH requires current token/sequence/READY and whole-batch preflight. IDs increase; exact latest semantic operation retries return the same ticket without refreshing leases.
- `UPDATE_LifecycleExecute`: require exact epoch/token/ID/ticket, then wait for the pause-response acknowledgement. Replays expose the existing acceptance state and never start a second operation.
- `UPDATE_LifecycleAck`: main/transport owner confirms successful **local full HID transfer completion** for the current ticket. Wrong, duplicate, failed or expired completions cannot start NVM. Successful completion enters OFF_WAIT; it does not establish host application receipt.
- `UPDATE_LifecyclePoll`: caller supplies bus inactivity only when USBPWR=USBBUSY=DMA=0 and exclusive ownership is retained. Session begin/submit/abort and polling occur only in offline execution/recovery. The module neither toggles USB nor certifies hardware quiescence.
- `UPDATE_LifecycleResume`: require current identity and successful RESULT; return to LINK. Exact last resume retries are accepted without renewing idle leases. A cancelled/failed result cannot resume; no automatic recovery/rebegin command is implemented.
- `UPDATE_LifecycleCancel`: unexpected disconnect/reset event or timeout cancels incomplete work. Ordinary cancellation after DONE+COMPLETE is too late; explicit staged ABORT invalidates a completed image through another offline operation.

`UPDATE_STAGED_OPERATION` has epoch, token, ID, sequence, length, CRC, kind, size and copied records. BEGIN/ABORT have no records; BATCH has zero manifest fields; ABORT has zero sequence. The controller compares those declared fields and used record bytes for exact replay; structure padding and unused record tail are not wire data. Future wire parsing must enforce canonical lengths/reserved fields separately. Tickets increase without wrapping; sequence/ID limits reject new stages. Caller-provided epoch freshness is not implemented or authenticated.

## State and result boundaries

```text
LINK -> STAGED -> ACK_WAIT -> OFF_WAIT -> OFF_RUNNING -> RESULT -> LINK
                             cancel/timeout/failure -> RECOVERY -> RESULT
```

RECOVERY may persist indefinitely if hardware stays busy, bus exclusion is unavailable or cleanup cannot erase/verify metadata. This is safe quarantine, not a bounded hardware-cancellation guarantee. Phase deadlines bound staging, acknowledgement, shutdown wait, operation, reconnect/resume and resumed READY-link inactivity; duplicate requests do not refresh them. Time comparisons use unsigned subtraction with timeouts <=7FFFFFFF. Hardware/host deadline values still need qualification.

STAGE/ACK_WAIT/OFF_WAIT cause no normal flash execution. A failed ACK or unexpected disconnect before a new BEGIN invokes no erase; if the prior session was IDLE/COMPLETE, its image may remain valid. Cancellation of an already READY/incomplete session still requires offline metadata cleanup. The typed CANCELLED outcome alone never means persistent ABORTED; inspect session status and prior-operation context.

OFF_RUNNING invokes the staged operation once, polls its verified completion and returns DONE with session READY/COMPLETE/ABORTED. BATCH sequence increments on accepted session submission, not staging. After session failure, recovery invalidates metadata if possible and reports FAILED; no normal next mutation is accepted.

A successful local pause-response send authorises later offline execution even if the host times out. The host outcome is unknown until the same epoch/token/ticket result is queried; it must not infer cancellation or resend a different mutation. This core does not send or parse those host messages.

Cancellation during COMMITTING waits for its active source to settle and metadata erase/readback. Before that erase verifies, reset may still observe a valid commit; the controller does not report session ABORTED. Once it classifies verified COMPLETE as DONE, ordinary cancellation cannot invalidate the image; explicit staged ABORT is required. This defines the cancellation linearisation point. There is no persistent cancellation-intent record or cross-reset update resume.

The caller must not reenable USB during any active operation, timeout buffer retention, cleanup or readback. An offline-running poll receiving `buses_inactive=false` enters RECOVERY without polling/submitting new flash work. It retains the active buffer and awaits exclusion again. The controller's boolean argument cannot prevent another owner from changing hardware; the target NVM port independently checks USB/DMA at launch, and the future owner/interlock must enforce whole-operation exclusion.

## Verification (2026-10-15)

- `make test`: eleven strict C executables and 13 Python methods pass.
- `make test-sanitize`: the same mocked C tests pass with ASan/UBSan.
- Lifecycle tests cover no-effect stage/preflight, immutable copies, ticket/epoch/stale/conflict/sequence checks, duplicate EXECUTE/RESUME, failed/late ACK, response-before-off ordering, phase expiry/wrap, READY-link inactivity, 70 KB multi-window image and stored-image validation.
- Failure tests cover NVM timeout source retention, unexpected bus reenable, backend failure staying recovery-only, explicit abort of COMPLETE and commit-cancel metadata invalidation. They assert mock writes/readback never occur with USB on.
- Strict XC32 lifecycle compilation and `make bootloader` pass. Diagnostic remains text26670/data80/bss1568 with inactive lifecycle/update execution symbols absent. Shared MPLAB XML lists all 35 sources.
- `make nvm-codegen-check` passes unchanged eight-mode target launch checks. No new host command or mutation transport exists.

A bounded independent contract review found two issues (local send authorisation and commit-cancel reporting), which were clarified here and in the proposal. Full controller code review timed out; a later narrow completion/cancel fragment review found the declared semantics consistent under its assumptions. Full independent code review is still open. Tests do not validate generated USB teardown, actual board NVM timing, epoch generation, transport identity, power-cut recovery or application handoff.

Next finalise/implement the negotiated wire schema and identity representation with offline C/Python fixtures, then independently review the full core and generated USB lifecycle before live binding. Preserve the [USB-off proposal](usb-off-update-session.md), [hardware gates](nvm-hardware-test-plan.md) and physical SW1/PICkit recovery. No hardware programming is requested for this stage.
