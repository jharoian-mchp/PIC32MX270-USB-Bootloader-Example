# Isolated NVM qualification readiness — 2026-10-22

The inactive NVM adapter is source-tested, but no isolated hardware-write test image or caller exists. The canonical32KB USB-only cycle passed once; that result does not qualify flash timing, USB-off ownership or protection. This review is offline preparation only. No NVM write, page selection, programming, protection change, reset or additional PAUSE is authorised.

## Current evidence

| Area | Established evidence | Not established |
|---|---|---|
| USB restart | One authorised48KB cycle and one authorised32KB cycle on distinct images; latest canonical STATUS1/2/1/7 and plain probe pass | Repeated cycles in one runtime, fault recovery, general old-reference provenance or NVM ownership |
| Target identity | IPE identifies PIC32MX270F256D/A1/DeviceId0x660a053; DFP1.7.380 | Independent current configuration readback and electrical measurements |
| NVM adapter | Physical bounds, error handling, source ownership and timing calculations pass mocks; target compilation succeeds | Actual erase/program/readback or timing on hardware |
| Unlock ordering | Ordered assembly; eight XC32 compile modes and relocation/order regressions pass | Complete final linked isolated image, including future LTO/instrumentation effects |
| Protection | Conditional A1 PWP32K/BWP policy documented | Protection settings applied or tested, protected/writable boundary and programmer recovery proof |
| Target port | [Inactive finite preflight/port](nvm-isolated-target-design.md), mocked owner/adapter integration and XC32 compile-only checks | Dedicated main/reset/configuration path, debugger workflow and final linked-image audit |
| Test scope | Proposed [hardware plan](nvm-hardware-test-plan.md) | Approved scratch address/contents, standalone caller/image and write/recovery procedure |

Current board state is the reconnected read-only canonical diagnostic, with the latest one-PAUSE permission spent. The full inactive adapter is not called by it. Generated startup still invokes `NVM_Initialize()` for a NO_OPERATION unlock/WR sequence; the existing read-only classification does not mean no NVM register accesses occur at startup. That startup sequence must be considered in a future isolated image audit rather than silently counted as a qualified erase/write launcher.

## Separate isolated testing from connected writes

A future cold USB-off test can omit the USB stack entirely. It need not repeat USB restart or destructive USB fault tests first if no USB transport is bound or enabled and its ownership policy enforces inactivity. It still needs its own reviewed image and explicit operator permission.

Connected writes are a different integration stage. The current powered probe would fail NVM initialization/launch checks. USB teardown, full response completion, buffer lifetime, operation settlement, session retention and reconnect ownership must be reviewed around the entire flash interval; one successful USB cycle does not establish those combined invariants.

The NVM gate remains USBPWR0/USBBUSY0 with DMA exclusion. The Harmony-powered reopen policy permits BUSY evidence while USBPWR1; it supplies no flash permission. Do not weaken the NVM guard to reuse the powered diagnostic endpoint.

## Smallest isolated test design requirements

No scratch page is selected by this report. A source-only design must define:

1. **Image placement and one-page whitelist.** Choose the isolated image's code/data placement and permitted erase span from the actual linked map. The production adapter accepts application/metadata addresses across `[1D008000,1D040000)`; that range is too broad for the proposed isolated experiment. A test-specific caller must restrict erase/write/readback to one subsequently operator-approved1024-byte page, exclude its own code, the bootloader prefix, Boot Flash/configuration and metadata, and prevent tests on an accepted bootable image without retiring it through a reviewed procedure. The earlier no-preservation permission for a debug-image overlap is not sacrificial-page approval.
2. **One bounded transaction and no automatic repeat.** Define how the operator requests or arms exactly one experiment. The initial experiment should contain an explicitly enumerated erase/verify and deterministic128-byte row/write/verify sequence, not an open-ended memory test. Do not run a destructive operation automatically merely because power, SW1 or a debugger is present. No direct-register forbidden-prefix test or error-clearing retry belongs in this first stage.
3. **Whole-operation bus ownership.** No USB initialization, attach, ISR, task or report path may enable bus activity. Keep central DMA disabled throughout erase/write, settlement, verification and timeout quarantine. The adapter checks state at init/start only; a trusted single owner must enforce the interval. Interrupt masking alone does not exclude the independent USB bus master. A timeout cannot release the row buffer, reinitialize the adapter or resume peripherals while WR or settlement remains active.
4. **Source and timing.** Retain a full128-byte word-aligned uncached KSEG1 SRAM row until the operation settles and ownership is released. Use genuine SYSCLK/2 Count, not interrupt ticks. The adapter accepts4–50MHz SYSCLK, computes10µs LVD startup and1µs settle delays (240/24 Count ticks at the48MHz profile), and rejects existing NVM errors, WR busy, USB/DMA active and low-voltage status. These conservative waits are source calculations, not hardware timing qualification. Document what to measure and how to handle stalls; typical FRC-cycle durations are not maximums.
5. **Local evidence and recovery.** Keep results accessible after settlement without auto-reenabling USB. Select a local/debugger/reporting mechanism and define which state it may inspect without altering active NVM ownership. Record exact image/hash, page preimage or explicitly disposable contents, source-buffer address, clock/VDD, launch/completion/error/readback evidence and stop behavior. Preserve the verified read-only recovery HEX and PICkit procedure. Any actual backup/readback, reset, recovery programming or voltage test requires its own approval; do not perform it to fill this plan automatically.

The startup strategy must also decide whether to omit or retain the generated NO_OPERATION `NVM_Initialize()` path. Audit the actual selected path before delivery. No generated startup modification is made here.

## Final image and hardware gates

Before offering a write procedure, test the isolated caller with mocked wrong addresses, duplicate triggers, USB/DMA activity, busy/error/low-voltage conditions, delayed completion and timeout. Then inspect the actual final ELF/HEX and linked unlock path under its real compiler/options: no forbidden Flash/config spans, exact scratch whitelist, defined code placement, uncached row location, ordered keys/WR and startup/settle behavior. Compile-probe success does not replace that final-image inspection.

The operator must approve the exact scratch page and loss of its contents, test image and bounded transaction. Obtain the necessary current voltage/clock/configuration evidence under a reviewed procedure; IPE's detected voltage and nominal clock macros alone do not supply those measurements. Preserve the board's current power/tool settings, including the reported Hold In Reset behavior, unless a later procedure explicitly changes them.

## Protection is a later, separate stage

Keep PWP/BWP/CP unchanged for this review and for any unapproved future image. The [conditional policy](flash-launch-policy.md) records the A1/256KB-Flash/64KB-SRAM applicability: erratum21 affects PWP/BWP dependency; the selected row is marked unaffected by22–24. This is a source interpretation, not hardware protection proof.

A future PWP32K+BWP ON image requires configuration/HEX review, actual readback and established PICkit/bulk-erase recovery under protection. Software rejection of the prefix cannot prove hardware PWP behavior. Forbidden-region erase experiments, first-writable-page/metadata tests and protection recovery need separate isolated scope and authorisation; none is supplied here.

Windows MCC application regeneration/relocation and write-gated uploader integration remain later work. No protection change or write integration follows from the first successful USB cycle.

## Verification and review scope

Reran `make nvm-codegen-check`: all eight O0/O1/O2/Os production/debug modes passed consecutive unlock-store checks and strict adapter compilation. Reran the existing NVM and lifecycle mock executables: bounds, Count delays, error/busy/USB/DMA gates, settlement, source retention and inactive lifecycle behavior passed. No board was accessed.

A broader delegated plan review timed out. A completed narrow review of the adapter/source contract supported offline planning and confirmed two requirements: a one-page isolated caller with whole-operation inactivity, and final linked-image/startup/port inspection before any write. It did not approve a hardware transaction or protection change.

This review changes documentation only. The subsequent [reviewed isolated-test design](nvm-isolated-test-design.md) specifies a one-page wrapper, irreversible token reservation, ordered mailbox snapshots and outer timeout/settlement handling. Bounded independent design reviews completed after resolving arming and snapshot gaps; an offline strict/sanitized Flash demonstration confirmed the need for outer post-call expiry. The later [inactive owner library](nvm-isolated-owner.md) is mock-tested, compile-checked and has completed a bounded implementation review after timeout/snapshot/alias fixes. The later [target preflight/port](nvm-isolated-target-design.md) is source-implemented, mock-tested and XC32 compile-checked; it has no startup/main binding or active firmware inclusion. No target startup/caller, standalone write image or board experiment is authorised. Keep the current canonical image running/read-only with PICkit5 disconnected and external power/settings unchanged. No PAUSE, reset, reprogramming, upload, NVM/protection action or fault test is authorised.
