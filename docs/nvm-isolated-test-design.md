# One-page isolated NVM test — source-only design

This draft design proposes one erase and one row program on a later approved sacrificial page, with USB and DMA off throughout ownership. Independent arming/containment/settlement review is incomplete. It is not implemented or approved for programming. No scratch address is selected, and no readback, erase, write, protection change, reset or new PAUSE is authorised. Keep the current canonical diagnostic running/read-only with PICkit5 disconnected and power/settings unchanged.

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
P + 1024 <= 0x1D03FC00
```

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

Use a debugger-observable SRAM mailbox for the first isolated experiment; no UART/USB transport is introduced. The mailbox is ordinary zero-initialised BSS, never retained SRAM. It contains a version, state, compile-time page/profile identity and a request token; target code clears the token at startup. Use aligned volatile words for debugger-visible fields: the operator writes only the token while the target is READY; the target alone owns phase/results and ignores token changes after consumption. Publish READY only after initialization, and publish each checkpoint/terminal state last after its associated fields with compiler ordering. A debugger snapshot can tear across updates; the later procedure must read a terminal snapshot or require matching state/sequence before and after a running snapshot. These markers are consistency aids, not authentication or resume authority. Merely booting, programming, holding SW1 or attaching a debugger cannot start an erase.

A future reviewed operator procedure may deliberately set one fixed arm token in the mailbox after inspecting READY and the printed page/manifest identity. It cannot provide an address or operation selector. Before consuming it, target code must require stable released SW1 since startup, then a fresh debounced press-and-release after the token is present, within a bounded arm window. Holding SW1 through reset does not count as that gesture. Wrong token, expired window or inconsistent mailbox state ends arming without writes.

Consume/clear the token and latch the arm as spent **before** any backend initialisation or launch. There is at most one arm in that boot, including an arm that fails preflight. A second token/gesture never starts another transaction. A reset clears BSS and returns to non-writing startup; it does not automatically resume or replay the previous experiment. No reset/rearm permission follows from the design.

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

A source-design harness may use a conservative one-second software observation budget for each erase/write phase. That value is a test policy bound, not a manufacturer maximum or hardware execution guarantee, and any delivery must approve/review it explicitly. Use unsigned Count/time differences and require an interval shorter than one counter wrap at the supported frequency. Time checks cannot preempt a CPU Flash-fetch stall; expiry may only be detected when execution returns. If Count ceases advancing, software cannot guarantee a deadline. State these limits in the final procedure; no watchdog/automatic reset is proposed.

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
- Wrong/duplicate/expired token, SW1 held at startup, missing fresh gesture and duplicate launch tests; arm/attempt consumed before the backend call and not refunded on failure.
- One erase/one row ordering, FF verification and final mixed-pattern page verification. Page and row objects remain owned through every asynchronous/stalled/error/timeout path.
- USB/DMA activity at init, startup delay, launch, poll, settlement and readback cut points; any detected interference fails the attempt. No automatic resume/report transport or retry.
- Count wrap, expired observation window, stopped-timebase limitations, clock preflight failure and pre-call report checkpoints. No unbounded generated PLL loop or generated NVM/USB startup bypasses arming.
- Terminal settled versus unsettled reporting and continued settlement observation after timeout, without releasing or resetting the adapter.

Compile and audit the actual final standalone test ELF/HEX and disassembly: source list, reset/exception paths, no USB/DMA enable or broad updater/handoff caller, isolated code spans, exact page constants, row's uncached SRAM location, unchanged configuration/protection settings, bounded clock startup and the entire real unlock/WR/startup/settle path under final compiler/LTO settings. The existing eight compile probes support source verification but do not replace final linked instruction review.

The user must approve the exact page/preimage loss, image/hash, trigger, voltage/clock evidence and recovery envelope after those checks. All scratch selection, programmer readback/backup, hardware measurements, writes and recovery operations are separate future approvals. Protection qualification and connected uploads are not part of this isolated design.

## Draft verification and review status

Mechanical checks passed for1024/128=8 rows per page,896 bytes remaining after the one programmed row, the128 distinct bytes in `i ^ 0xA5`, and the nominal one-second observation policy being below one24MHz Count wrap. These checks supply no selected address or hardware deadline guarantee. Existing NVM/Flash mock tests pass for ownership, settlement, readback and error handling; they do not test this proposed mailbox/caller, which does not exist yet.

The broader independent design review timed out. A narrow response incorrectly applied generic4KB/512-byte Flash geometry; direct inspection of device datasheet DS60001168L section5/page57 and generated PLIB disproved that finding, and the device-specific citation was added above. The corrected boundary review also timed out. No completed independent pass for the arming/containment/settlement design is claimed.

Before implementation, complete a bounded review of the draft's one-shot trigger, exact-page wrapper, stopped-clock/CPU-stall limitations and late-settlement behavior against `update_flash.c`/`update_nvm.c`. Reconcile any changes in a source-only design revision. Device geometry is fixed by the target evidence; it is not an unresolved family-size guess.

## Current boundary

This document is design only; no new C module, standalone image, address default or executable write command is supplied. The current canonical board remains running/read-only. Its PAUSE permission is spent. No reset, programmer reconnection, flash operation, protection change or fault test is authorised. The A1 powered-BUSY concern remains documented; the strict USB-off NVM contract is unchanged.
