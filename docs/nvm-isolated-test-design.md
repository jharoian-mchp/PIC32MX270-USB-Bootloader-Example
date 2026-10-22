# One-page isolated NVM test — source-only design

This reviewed source-only design proposes one erase and one row program on a later approved sacrificial page, with USB and DMA off throughout ownership. Bounded independent arming/containment and timeout/settlement reviews are complete; implementation and final-image verification remain separate gates. The subsequent [inactive owner implementation](nvm-isolated-owner.md) is mock-tested and compile-checked, without target startup, a standalone image or programming approval. No scratch address is selected, and no readback, erase, write, protection change, reset or new PAUSE is authorised. Keep the current canonical diagnostic running/read-only with PICkit5 disconnected and power/settings unchanged.

The [readiness review](nvm-qualification-readiness.md) identifies the inactive adapter, point-in-time exclusion checks and missing final-image/recovery evidence. One successful USB-only cycle does not qualify this test or connected flash writes.

## Separate image and startup

Use a new, default-unbuilt isolated test target with its own `main` and source list. Do not add a caller to the current USB diagnostic, updater, application or `SYS_Initialize()`.

The intended code placement is the canonical bootloader program prefix `[0x1D000000,0x1D008000)`, plus reviewed reset/vector/configuration content required for programmer installation. Use the known clock and board configuration as input, but do not blindly call generated whole-system startup. Audit actual linked placement, vectors, startup `.dinit` and configuration words before any delivery.

Startup must:

- Keep global interrupts disabled; install no USB, DMA, I2C, ADC, application or update-session task/ISR paths. A minimal GPIO setup may provide SW1 state and status LEDs, preserving the existing pin/power/mux configuration without running the mux/ADC diagnostics.
- Never initialise/enable USB or submit reports. Read only U1PWRC for USB exclusion, and refuse an arm if USBPWR or USBBUSY is set. Do not power-cycle USB to make the condition pass.
- Keep central DMA disabled and reject unexpected active DMA. Do not silently clear active DMA/USB or reconfigure another bus master. The isolated source list must contain no code that enables them.
- Omit generated `NVM_Initialize()`: it performs an unlocked NO_OPERATION/WR sequence before the test owner runs. The isolated image's startup must make no NVM register writes before explicit arming. This is a choice for the future test image only; the existing diagnostic and generated files are not changed.
- Use the reviewed oscillator/fuse profile and validate the expected clock configuration before arming. Avoid the existing generated unbounded PLL-lock loop: a minimal startup needs a bounded clock-check failure path. If clock readiness, nominal Count rate or voltage evidence is inadequate, remain non-writing and report the failure. No clock-switch recovery or reset loop.

No active bootable image selection or handoff belongs in this test. Programmer installation has its own approved bounds/erase scope and can replace the current diagnostic; it is distinct from the test's much narrower NVM API whitelist.

## Page selection and immutable manifest

Device-specific geometry is **1024-byte erase pages and128-byte program rows**. DS60001168L section5, page57 explicitly states1KB pages/128-byte rows (256/32 instruction words); generated `peripheral/nvm/plib_nvm.h` defines `NVM_FLASH_PAGESIZE=1024` and `NVM_FLASH_ROWSIZE=128`. Generic FRM examples using4KB/512-byte units do not override that device-specific definition. Implementation must compile-check these target geometry constants rather than infer sizes from another PIC32 family.

The future build must require an explicit test-page parameter with **no default**. The delivery manifest identifies the exact page, image/hash and one transaction. Neither the page parameter nor the target address is supplied by a runtime host/debugger command.

A candidate page `P` must satisfy:

```text
P % 1024 == 0
0x1D008000 <= P
P <= 0x1D03FC00 - 1024
```

Validate the subtraction-based upper bound before calculating `P+1024`; do not let a wrapping address satisfy a containment check. Reject malformed, negative or out-of-range build inputs rather than narrowing them silently to32 bits. Compile-time target assertions and host tests must enforce the same byte geometry and bounds.

Additionally prove from the final map that the page does not overlap the isolated image's own Flash load spans, the bootloader prefix, metadata, Boot Flash/configuration, or any image/data that the operator has not explicitly declared disposable. Never use an accepted bootable image's contents as scratch without a separately reviewed retirement decision. The earlier debug-overlap no-preservation approval is not scratch-page permission.

The manifest must state either the known page preimage/recovery artifact or the user's explicit approval to lose all existing contents of that exact page. Existing metadata should be treated as protected from this test. Do not issue metadata invalidation or broader erase as a convenience; the isolated image cannot hand off to an application anyway. Before restoring firmware that can hand off, page ownership/image validity must be reassessed separately.

## Narrow backend wrapper

Reuse the inactive `UPDATE_NVM` backend and `UPDATE_FLASH` verification state machine behind an isolated wrapper, not via their broad production span alone.

Initialise `UPDATE_FLASH` with `[P,P+1024)` and pass a wrapper backend that enforces all of these before forwarding to the private real backend:

| Operation | Permitted request |
|---|---|
| erase | Address exactly `P`, once |
| row program | Address exactly `P`, full128-byte row, once, only after verified erase |
| readback | Nonempty span entirely in `[P,P+1024)`, only after NVM settlement |
| busy/error | Observation/settlement only; never launch an operation |

The real backend must not be exposed through the mailbox or another public test API. Keep its callbacks private to the wrapper. Runtime address/operation fields in a report are evidence, not commands. Reject changes to the compiled page, nonphysical aliases, wrong alignment, oversized reads and a second erase/write even if the flash state could be reset.

Use non-wrapping attempt counters/latches that are set **before** forwarding a launch request. Once a launch has been attempted, a rejection or uncertain return cannot refund that attempt. Source and page validation failures before any launch also end this arm; there is no automatic corrected-address retry.

## Explicit arming and one-shot trigger

Use a debugger-observable SRAM mailbox for the first isolated experiment; no UART/USB transport is introduced. The mailbox is ordinary zero-initialised BSS, never retained SRAM. It contains a version, state, compile-time page/profile identity and a request token; target code clears the token at startup. Use aligned volatile words for debugger-visible fields: the operator writes only the token while the target is READY; the target alone owns phase/results and ignores token changes after consumption. Publish READY only after initialization. The target is the sole publisher and must bracket every target-owned payload update, including same-state updates, with an aligned volatile sequence: change an even sequence to odd before modifying fields, enforce ordering, write fields/state, then publish the next even sequence last. In the intended PIC32 implementation, locate the entire mailbox in uncached KSEG1 SRAM, use aligned32-bit sequence stores, and use an explicit `sync` with a compiler memory clobber after the odd store and before the final even store. A compiler barrier alone is not a hardware-ordering guarantee. The debugger procedure must perform ordered word reads of the sequence before/after the payload; bulk refreshes without that order cannot be accepted as coherent evidence. Readers accept a snapshot only if the sequence is even and unchanged before and after reading all fields. Prevent wrap: if another publication would wrap, fail closed without a new launch and keep the last stable snapshot; never reuse a sequence. A terminal PASS/FAIL_SETTLED/FAIL_NO_LAUNCH payload is immutable after publication. FAIL_UNSETTLED may later become FAIL_SETTLED through the same sequence protocol; it can never become PASS or READY. The operator-owned token is outside the target snapshot payload. Debugger halts/manual RAM corruption and visibility remain limitations, and the protocol needs tests against the actual target/debugger access method. These markers are consistency aids, not authentication or resume authority. Merely booting, programming, holding SW1 or attaching a debugger cannot start an erase.

A future reviewed operator procedure may deliberately set one fixed arm token in the mailbox after inspecting READY and the printed page/manifest identity. It cannot provide an address or operation selector. Zero means no attempt; on the first observed nonzero token, latch the boot's sole arming attempt as spent, copy that single word to private target state, clear the mailbox token and leave READY **before** evaluating token/preflight validity. All later token writes are ignored. No token replacement can refund or restart the private attempt.

For a valid reserved token, require a previously observed stable SW1 release since startup and reset gesture/debounce history at reservation. A press that began before reservation cannot count. If SW1 is already pressed or startup-release qualification is missing at reservation, fail terminally without NVM setup. Otherwise publish ARMED and start the bounded arm window at reservation, then require a fresh debounced press-and-release completed within that window before any backend initialisation. Wrong token, expired window, inconsistent mailbox state or failed preflight is terminal for the boot. Check expiry before accepting a gesture, so a release at or beyond the deadline cannot trigger an operation.

The arming reservation is distinct from an erase/write attempt counter; reserve each operation attempt before forwarding its backend call. There is at most one arming attempt in that boot, including rejected attempts, and no backend initialisation/launch until the valid gesture completes. A second token/gesture never starts another transaction. A reset clears BSS and returns to non-writing startup; it does not automatically resume or replay the previous experiment. No reset/rearm permission follows from the design.

The programmer/debugger connection needed for this scheme must be separately reviewed: PICkit5 has reported Hold In Reset behavior. The test image cannot alter that setting or recover itself from a halted/reset target. The later procedure must specify a verified method to run freely after deliberate arming. Debugger halts, mailbox inspection timing and attachment affect measurements and may corrupt assumptions; no such connection/test is requested now.

## Fixed transaction and expected data

One accepted arm attempts only:

1. Initialise the NVM adapter at the verified nominal48MHz SYSCLK, with USB/DMA off and no NVM WR/errors/pending settlement.
2. Initialise the wrapper and flash state machine for the compiled page.
3. Attempt one page erase at `P`.
4. Observe settlement and verify all1024 bytes are FF using the existing eight128-byte readback chunks.
5. Reset only the completed flash state (not the NVM hardware/errors or operation counters), then attempt one row write at `P`.
6. Verify the128-byte deterministic row, then check the complete page: that row matches and all remaining896 bytes stay FF. Publish PASS only after final verification and settlement.

Proposed deterministic bytes for the row are `row[i] = (uint8_t)(i ^ 0xA5)`, `i=0..127`. This supplies varied bit patterns and is a single test vector, not endurance/boundary qualification. Rows1–7 remain erased. No second pattern, metadata write, forbidden-prefix attempt or restore of the original page contents is included.

`UPDATE_FLASH` makes a private aligned128-byte row copy. Place the flash/adapter/mailbox objects in fixed-lifetime KSEG1 SRAM and prove the row's complete uncached/aligned address in the final map. No stack-temporary source, cached alias or buffer reuse after timeout. Keep all state allocated for the entire run and quarantine; never reinitialise it to make a second operation possible.

## Whole-operation ownership and timeout behavior

Hold the single main owner and global interrupt exclusion from validated arming through erase/write settlement, verification and terminal state. With no enabled USB/DMA paths or ISR/task owner, the backend cannot be raced by another software enabler. Recheck USBPWR/USBBUSY and central DMA before each step and before result acceptance; unexpected activity poisons the experiment. These point observations supplement the source-list ownership proof, not replace it.

Use the adapter's10µs startup and1µs settlement waits, with real Count at SYSCLK/2. These conservative waits remain part of the existing source contract, not measured hardware qualification. Do not change unlock keys, force-clear WR or error bits, or manually retry.

A source-design harness may use a one-second software observation budget for each erase/write phase. That value is a test policy bound, not a manufacturer maximum or hardware execution guarantee, and any delivery must approve/review it explicitly. Each phase starts at a recorded Count anchor immediately before its first launch-capable flash call, after preflight. Erase budget includes launch, settlement and all eight verification chunks; row budget includes launch, settlement, row verification and the final full-page readback. Do not reset or extend an anchor during verification.

Enforce this budget in the isolated owner, independently of `UPDATE_FlashPoll`: check elapsed Count at or beyond the limit before every launch/poll/readback call and immediately after each returns, including a returned OK, before accepting PASS or advancing to the next phase. `UPDATE_FlashPoll` only checks its timeout while the backend reports busy; a launch that stalls until completion or a slow readback can otherwise appear successful after the policy expires. An outer post-call expiry must latch failure even if the flash layer reports OK or WR is already clear. Subsequent work is limited to the settlement-observation path; no remaining verification or row launch after an expired erase phase. Do not change the generic Flash/NVM behavior to enforce this isolated policy.

Use unsigned Count differences and require an interval shorter than one counter wrap at the supported frequency. A one-second threshold does not bound a blocking call: multiple unseen counter wraps can alias a short interval. Time checks cannot preempt a CPU Flash-fetch stall; expiry may only be detected when execution returns. If Count ceases advancing or hidden wraps prevent interpretation, software cannot guarantee a deadline. State these limits in the final procedure; no watchdog/automatic reset is proposed.

On rejection, low voltage, error, mismatched readback, expired budget or unexpected peripheral activity:

- Latch failure and end the transaction. Never issue the next erase/write, clear the failure, enable USB/DMA or auto-recover.
- If launch may have occurred, preserve the row/state and classify hardware as unsettled until the backend observes WR clear and completes its conservative settle/disarm path. The flash-layer terminal timeout does not itself settle hardware.
- Continue only the existing backend busy/settlement observation after a timeout, under the same ownership. Its delayed disarm is allowed only after WR clear and the settle delay; do not write other NVM registers to cancel or diagnose a busy operation.
- If unexpected USB/DMA activity is observed, retain failure/quarantine and do not perform ordinary Flash readback or any further launch. Retain buffers regardless of later idle observations. A software interlock cannot undo hardware activity that already occurred; the whole-operation exclusion claim is invalid for that attempt.
- A failed-but-settled terminal state remains failed and cannot be rearmed. If hardware does not settle, remain quarantined indefinitely without calling it a bounded hardware stop. Only a separately approved operator recovery can end that state.

The first experiment is not a cancellation, power-interruption or electrical low-voltage fault test. No improvised disconnect, reset or brownout while WR is active.

## Evidence and reporting

Record entered phase before potentially blocking calls, accepted/attempted counters, target page/row addresses, nominal clock profile, raw Count anchors/deltas, sampled USB/DMA flags, NVM error status and readback result. Report timing as raw ticks with nominal conversions only: CPU stalls, clock variation and debugger halts can distort or hide behavior. Do not use typical FRC-cycle times as deadline proof.

The primary terminal report is a fixed SRAM mailbox snapshot, read by the debugger only under the later approved procedure. No target hardware status read or extra write is triggered by reading a mailbox. LED indication may be used as a minimal secondary hint after terminal settlement; it is not the acceptance evidence. No debugger force-write of source/state/error fields after arming.

Final result classes should distinguish READY, ARMED, RUNNING phase, PASS_SETTLED, FAIL_NO_LAUNCH, FAIL_SETTLED and FAIL_UNSETTLED/QUARANTINED. FAIL_NO_LAUNCH means no erase/program WR launch was attempted; adapter preflight may already have configured NVM mode/address, waited for LVD startup or cleared WREN on rejection. Report whether setup and launch were entered separately, and never describe every pre-launch failure as no NVM-register writes. PASS requires both operations attempted once, correct full-page readback and hardware settlement. Report a pre-call checkpoint as entered only; a stalled call cannot be presumed to have returned.

Mailbox updates are not a nonvolatile journal. Reset/power loss/programming can erase them, and a debugger can alter RAM; they are evidence, not authentication or recovery authority. Reading the report, reconnecting PICkit5 or resetting after a failure needs an explicit procedure, especially while unsettled. Do not promise that programmer recovery is nondestructive.

## Source and final-image verification gates

Before any delivery, implement and test:

- No default scratch parameter; page/span/arithmetic and exact wrapper whitelist tests. Neither boot prefix, metadata nor arbitrary application pages can reach the backend.
- Zero token is inert; first nonzero reserves the sole attempt, including wrong token. Test replacement token, expired/invalid preflight, SW1 held at startup or already pressed at reservation, stale gesture history, exact-deadline release, and duplicate launch. No rejection can return to READY or refund arming.
- One erase/one row ordering, FF verification and final mixed-pattern page verification. Page and row objects remain owned through every asynchronous/stalled/error/timeout path.
- USB/DMA activity at init, startup delay, launch, poll, settlement and readback cut points; any detected interference fails the attempt. No automatic resume/report transport or retry.
- Count wrap, expired observation window, stopped-timebase limitations, clock preflight failure and pre-call report checkpoints. Inject a launch that returns completed after expiry, a poll that returns OK after settlement crosses expiry, and a final readback that crosses expiry; all must fail before PASS or the next phase. No unbounded generated PLL loop or generated NVM/USB startup bypasses arming.
- Snapshot reader/writer interleavings across every field and same-state update: odd/changing sequences are rejected, terminal payloads are immutable, FAIL_UNSETTLED-to-FAIL_SETTLED uses the same protocol, and sequence exhaustion does not wrap or launch.
- Terminal settled versus unsettled reporting and continued settlement observation after timeout, without releasing or resetting the adapter.

Compile and audit the actual final standalone test ELF/HEX and disassembly: source list, reset/exception paths, no USB/DMA enable or broad updater/handoff caller, isolated code spans, exact page constants, row's uncached SRAM location, unchanged configuration/protection settings, bounded clock startup and the entire real unlock/WR/startup/settle path under final compiler/LTO settings. The existing eight compile probes support source verification but do not replace final linked instruction review.

The user must approve the exact page/preimage loss, image/hash, trigger, voltage/clock evidence and recovery envelope after those checks. All scratch selection, programmer readback/backup, hardware measurements, writes and recovery operations are separate future approvals. Protection qualification and connected uploads are not part of this isolated design.

## Bounded design verification and review — 2026-10-22 continuation

Earlier broad/corrected review attempts timed out. One earlier response incorrectly applied generic4KB/512-byte geometry; device datasheet DS60001168L section5/page57 and the generated PLIB establish1024/128 for this target. That incorrect finding was not adopted.

The resumed review was split into two narrow scopes:

| Finding | Resolution |
|---|---|
| First token observation did not unambiguously consume rejected arming attempts | First nonzero token is privately reserved and the boot attempt spent before validation; all later token writes ignored, every rejection terminal |
| State-last publication did not cover same-state payload changes | Odd/even sequence brackets every update, wrap refused, settled terminal payload immutable, ordered debugger reads required |
| Generic `UPDATE_FlashPoll` can accept late completed calls because it checks timeout only while busy | Isolated owner checks its phase budget before and after every launch/poll/readback, including returned OK, before PASS or next phase |
| An unchecked `P+1024` could wrap | Subtraction-based upper bound is validated before addition; malformed/oversized build inputs rejected |

Completed independent arming/containment and timeout/settlement follow-ups accepted those bounded design resolutions. The snapshot review noted that compiler barriers alone are not hardware ordering; uncached KSEG1, aligned32-bit words and explicit target `sync` ordering are now specified. Actual debugger access/visibility remains an implementation and procedure verification requirement. This is a completed narrow design review, not an exhaustive firmware review or hardware approval.

Mechanical layout/pattern/Count checks remain valid. A disposable strict and ASan/UBSan harness linked the actual `update_flash.c` with mock callbacks and demonstrated late-completed launch, settlement during poll and slow-read behavior; the outer policy rejects each at expiry before a subsequent write. Existing NVM and storage mock executables also passed. Boundary vectors reject prefix, metadata, misalignment and wrapping/oversized inputs. All addresses in these tests are abstract mock vectors, not a selected sacrificial page. The temporary harness was removed.

At this design-review cutoff no isolated mailbox, wrapper or owner had been implemented. The subsequent [inactive owner](nvm-isolated-owner.md) tests these software paths; it remains disconnected from any target startup/image. The earlier demonstrations alone did not prove caller enforcement. Implementation must test the specified trigger, snapshot interleavings, containment, timing and quarantine paths, then audit the actual final linked startup/launch image. Device geometry is fixed by the target evidence; the A1 concern and hardware timing/recovery qualifications remain open.

## Current boundary

This document records the design. The later inactive C library is described separately; no standalone image, approved address, target binding or executable board write command is supplied. The current canonical board remains running/read-only. Its PAUSE permission is spent. No reset, programmer reconnection, flash operation, protection change or fault test is authorised. The A1 powered-BUSY concern remains documented; the strict USB-off NVM contract is unchanged.
