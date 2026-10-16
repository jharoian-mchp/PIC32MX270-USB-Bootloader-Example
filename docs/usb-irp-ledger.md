# Opt-in per-IRP completion ledger

`usb_irp_ledger.c/.h` and the checked HID overlay track each static HID IRP slot independently. They reject a second completion claim for a consumed slot even while another same-direction IRP remains pending. **Hooks remain default-off. USB-only initialization, thaw and hardware restart are still unimplemented.**

## Ownership states

Each pool slot records `armed`, `submitting`, `callback` and direction:

- Reserve atomically only a free ledger slot with terminal/free driver status. Reservation is part of pool selection, before changing IRP data/callback/count or submitting to the driver.
- `armed` means one completion is expected. A valid matching-direction callback atomically consumes it and sets `callback`.
- Callback cleanup clears `callback` on every normal C return. A nested duplicate cannot claim the same slot.
- Submit-result bookkeeping clears `submitting`. A slot is reusable only when armed/submitting/callback are all clear.
- If completion occurs before the submission API returns, callback cleanup alone cannot free the slot; `submitting` still holds it.
- Failed submit releases only an unowned armed reservation; if the driver still reports pending/in-progress, its slot and source remain owned. A completed/claimed callback is not rolled back a second time.

The HID wrapper increments its direction count before submit as before. On an error, count decrement/handle invalidation occurs only for an unowned reservation rollback; queued-but-error ownership remains visible. The current inspected lower driver normally returns success after queuing; the queued-error scenario is fault injection, not a claim that this driver routinely returns that error. OSAL unlock failures and completion races still require conservative caller handling.

## Overlay and atomic selection

The opt-in allocator now uses `USB_RetirementHidReserve(&pool[count], direction)` inside the free-slot condition. That helper rechecks status and ledger availability with interrupts excluded. A losing allocation skips all field/count/submit changes. Entry admission keeps retirement fencing from succeeding while the API is in progress.

Independent review found the first implementation sampled availability and reserved later, allowing ISR requeue to take the same slot between checks. Reservation moved into atomic selection; failure is returned rather than merely poisoning and continuing. A bounded follow-up review confirmed that specific race fix.

The callback still validates pool pointer alignment/range, instance and terminal status, then atomically claims the ledger before decrementing direction count or notifying the app. Invalid/duplicate claims poison the retirement gate and skip count/application changes. The generated callback itself remains responsible for the notification. GCC cleanup releases the ledger callback hold after that notification returns.

HID drain now requires all ledger slots free in addition to the existing pool-status, queue-count, admission and callback checks. Retirement clears the ledger only after authorized off/idle and drain verification. The shared basic-OSAL mutex policy is unchanged.

## Limits before USB reopening

This fixes **duplicates within the current reservation lifetime**, not arbitrary old callback delivery after a pool address is reused for another request. A stale pointer alone cannot identify its original generation once it names a newly armed entry. The slot ledger does not add a hardware transaction tag or change public transfer handles.

Before thaw/reinitialization, the owner must establish that every old hardware/driver/callback reference is gone, then assign and route the new transport generation coherently. No thaw API exists today. The current retirement gate remains one-shot, and no target restart command or coordinator port is installed.

The USB-only initializer requirements remain:

1. Retain admission fencing through cancellation, driver/HID/EP0 drain and off/idle retirement.
2. Restore driver/device/EP0/common state without `SYS_Initialize` or resetting update/NVM/session/boot epoch.
3. Rebind callback routes before attach/task/interrupt activity, using a reviewed generation policy.
4. Open admission at a specific validated boundary before normal HID descriptor/configuration traffic, which is itself currently fenced.
5. Quarantine partial initialization and never recycle old source buffers on timeout.

This stage supplies no replacement initializer or generation-safe reopen policy; do not treat ledger tests as board restart approval.

## Verification (2026-10-16)

- `make test`: fifteen strict C executables, 16 Python methods, exact-source/hook/cancellation/submission harnesses and Bun checks pass.
- `make test-sanitize`: the same host C tests pass with ASan/UBSan.
- Pure ledger tests cover atomic-reservation ownership semantics, wrong direction, duplicates with another pending IRP, callback-before-submit-return, failed unowned rollback, driver-owned failure retention and misuse rejection.
- Extracted owning-hook tests verify a competing reserve cannot claim the same slot and that callbacks/submission holds block reuse/drain.
- Exact generated queue flush and HID callback tests reject an old consumed slot while another actual ledger slot is armed; correct cancellation still delivers its callback.
- `tools/test-usb-hid-submission.ts` compiles exact overlaid HID ReportSend/ReportReceive and callback bodies with a mocked lower driver. It verifies rejection rollback, immediate completion and queued-error ownership retention. These are selected bodies/private mocks, not complete stack/hardware timing tests.
- `make usb-retirement-check` compiles opt-in target files with XC32; the default diagnostic build remains text26670/data80/bss1568 and excludes ledger/retirement/update execution symbols. Shared MPLAB XML lists 40 C sources.

The delegated submission-error trace timed out; parent inspected the exact driver path. Independent ledger/overlay review identified the reservation race, fixed and confirmed as above. No complete initializer/reopen/hardware qualification is claimed.

The [opt-in detached USB-only initializer](usb-rebuild-detached.md) now supplies preparation/readiness/reopen checks with source/mock tests. Next implement/review the target cancellation/old-reference/generation handoff and coordinator binding before a board restart. Continue to keep [NVM/protection hardware tests](nvm-hardware-test-plan.md) and application regeneration/relocation as separate gates. No reprogramming is requested.
