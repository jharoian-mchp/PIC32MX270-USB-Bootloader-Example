# Inactive version-2 dispatcher and dynamic host fixtures

`update_link_dispatch.c/.h` connects the offline version-2 codecs to the typed lifecycle core. A Python test host exchanges packets with a C mock subprocess, consumes device-issued tickets and verifies committed mock images. **No HID handler, shutdown/reconnect implementation or hardware write path is enabled.**

## Request and response handling

`UPDATE_LinkDispatchInit` creates a fresh/quiescent instance with caller-supplied boot epoch, flash/phase deadlines and explicit permission. A target NVM backend must already satisfy the USB-off initialisation gate. Live firmware never calls this function.

`UPDATE_LinkDispatchRequest` accepts a CRC-validated payload and requires a full 56-byte response buffer, a separate length output and separate local cookie output. All input/output/controller overlap is rejected before writes or lifecycle mutation. It decodes version/shape/reserved/identity and preflights response encoding before invoking any lifecycle function. Malformed/short-buffer/overlapping requests return false with no mutation; callers must not assume output values were reset for invalid/overlapping storage.

Dispatch:

- HELLO reports current identity, state and features.
- STAGE preflights and copies the semantic BEGIN/BATCH/ABORT operation, then returns the lifecycle-issued ticket. New RPC ID with unchanged operation identity/content returns the same ticket and does not refresh leases.
- EXECUTE requires epoch/token/operation ID/ticket, enters ACK_WAIT and returns a nonzero **local completion cookie** only for the pause response needing completion. It causes no flash launch.
- QUERY requires exact current identity/ticket and reports state; it does not extend leases or execute work.
- RESUME requires successful RESULT and exact identity; it does not repeat a flash action. Cancelled/failed outcomes reject continuation.

Result values use the version-2 schema: OK/BAD_REQUEST/DISABLED/BUSY/IDENTITY/FAILED. Error classification is intentionally coarse; inspect outcome/session status as well. A query can return OK while reporting CANCELLED or FAILED. An EXECUTE replay may return OK after execution without a new completion cookie. Acceptance OK never certifies a completed write or valid executable image.

Features are schema/core plus instance write permission (03 disabled, 07 permission-enabled mocks). **The USB transport-bound bit is always zero.** No live endpoint advertises these features; a future uploader must require independently qualified transport and hardware permission rather than treating mock feature07 as usable hardware.

## Local pause-response completion

`UPDATE_LinkDispatchSent(cookie, success, now)` accepts only the current pending cookie and forwards its ticket to `UPDATE_LifecycleAck`. The monotonically increasing cookie identifies a local send generation, separately from host RPC ID and operation ticket. It is not on the wire. Wrong, duplicate, cancelled or stale callbacks cannot authorise a later operation. Cookie exhaustion fails before accepting a new EXECUTE; no wrap is permitted.

While the pause response is pending, requests are serialised: the dispatcher returns false until matching completion/cancellation. The transport remains responsible for actual IRP handle/generation, buffer retention and full-response completion. Clearing the dispatcher's cookie does not cancel a hardware transfer or release its buffer.

`UPDATE_LinkDispatchPoll` forwards the independently established bus-inactive predicate to the core. True must mean USBPWR=USBBUSY=DMA=0, with exclusive ownership retained through erase/write/readback/settlement. It cannot be fabricated from cookie completion alone. Polling with powered USB does not start flash. `UPDATE_LinkDispatchCancel` invalidates local completion authority and requests lifecycle recovery; late send completion cannot unlock execution.

Host timeout after a successful local pause send still means outcome unknown. It must query the same epoch/token/ticket after reconnect. Final commit result is authoritative once classified DONE+COMPLETE; ordinary later cancel does not erase it. Explicit ABORT is a separately staged operation. Recovery/session metadata rules remain [the core contract](update-lifecycle.md).

## Dynamic Python test host

`host/test_link_dispatch.py` starts `build/tests/update_link_dispatch_test --host`. The C process has only mocked flash/USB state and no device access. Its test-only stdin protocol supplies packet hex, local send-completion events, offline pump and stored-image verification. It is not a deployed protocol or host uploader.

The test host:

1. Reads HELLO's epoch and schema/mock permission; confirms transport-bound is false.
2. Stages a manifest and consumes the returned ticket; retries with a new RPC ID and checks no flash effects.
3. EXECUTEs the exact ticket; rejects a mismatched local completion, then completes the correct response.
4. Pumps bus-off mock execution, reads/repeats QUERY to simulate a lost result, and checks unchanged erase/write counts.
5. Repeats EXECUTE and RESUME without another flash action; rejects a changed boot epoch.
6. Builds complete-record BATCHes within the 478-byte wire bound, using newly issued tickets and next-sequence checks.
7. Completes a small 128-byte image and a 70144-byte multi-window image, then validates stored metadata and CRC.

The existing `pic32_hid.py` read-only transport refuses version-2 requests. No new production CLI or automatic reconnect/resume exists. Tests do not establish device selection among identical boards, epoch generation/authentication, host HID cancellation or USB re-enumeration behaviour.

## Verification (2026-10-15)

- Thirteen strict C executables, 16 Python test methods and Bun codegen self-tests pass via `make test`.
- The same suites pass with ASan/UBSan C binaries via `make test-sanitize`, including dynamic host exchanges.
- C dispatch tests cover no-effect staging, alias/capacity rejection, different RPC/same-ticket retry, wrong/stale cookie, pending-response serialisation, duplicate execute/query/resume, permission-disabled features, callback failure/timeout, cookie exhaustion, cancelled-result rejection and malformed payloads.
- Strict XC32 dispatcher compilation, eight-mode NVM codegen checks and diagnostic `make bootloader` pass. Shared MPLAB XML lists 37 C sources; diagnostic remains text26670/data80/bss1568 with inactive update/dispatcher symbols absent.
- Independent bounded static dispatcher review reported no concrete findings under its documented single-owner, valid-core-state and codec assumptions. Hardware predicate/USB transport binding was outside that review.

The [generated USB ownership audit and inactive restart coordinator](usb-restart-audit.md) now exist; real target ports/read-only hardware restart diagnostic remain unimplemented and must precede flash qualification. Keep [the hardware test gates](nvm-hardware-test-plan.md), Windows application regeneration/relocation and protection/readback/recovery requirements. No reprogramming is requested for this inactive stage.
