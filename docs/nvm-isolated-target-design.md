# Isolated NVM target port — inactive implementation and startup requirements

`isolated_nvm_target.c/.h` and `isolated_nvm_target_hw.h` implement a finite register preflight, startup mailbox and lifecycle-aware adapter port for the [inactive owner](nvm-isolated-owner.md). Mock tests and XC32 compile-only checks pass. No main, startup binding, configuration translation unit, scratch-page default or standalone image exists. Hardware use requires the separate entrypoint, debugger workflow and final linked-image audit specified below. No deployment, programmer connection, reset, NVM/protection action or PAUSE is authorised.

The current board remains on the canonical USB diagnostic with one successful controlled cycle and its trial permission spent. The A1 powered-BUSY concern stays open; isolated testing must use the stricter USBPWR0/BUSY0 gate, never the powered-reopen policy.

## Separate source list and configuration

A future default-unbuilt target should have its own entrypoint and configuration translation unit. It must not reuse diagnostic `main`, `SYS_Initialize`, `CLK_Initialize`, generated `NVM_Initialize`, `APP_Initialize` or the USB restart/probe task machinery.

Its explicit source list should consist of:

- A small isolated main, not yet written, plus the inactive `isolated_nvm_target.c` preflight/port module.
- A separately reviewed fuse/configuration translation unit, not the generated initialization TU that also defines and constructs USB.
- `isolated_nvm.c`, `update_flash.c` and `update_nvm.c`, with the real `update_nvm_port.h` selected before mock include paths.
- XC32 reset/C-runtime support actually required by the final link; no application handoff or generated peripheral task/ISR sources.

Use canonical program placement `[0x1D000000,0x1D008000)` and the target's 64KB uncached KSEG1 data memory, preserving required reset/exception/boot configuration locations. Do not infer a correct source list from symbol absence alone: LTO can inline calls. Final reset, C-runtime hooks, exception behavior, `.dinit` and reachable code need inspection.

The current configuration is PRIPLL, HS primary oscillator, FPLLIDIV DIV_3, FPLLMUL MUL_24, FPLLODIV DIV_2, FPBDIV DIV_1, watchdog OFF, protection OFF, DEBUG OFF and ICS_PGx4. The known board crystal is marked12MHz; the nominal clock derivation is `12MHz / 3 * 24 / 2 = 48MHz`. This is a profile, not a measured oscillator result. Preserve unrelated fuse choices; configuration changes required for an approved debugger procedure must be explicit and reviewed separately.

A future build requires a page parameter with no default. Reject malformed or oversized input rather than silently narrowing it; assert physical page alignment and `1D008000 <= P <= 1D03FC00 - 1024`. The parameter is an audited compile-time constant, not a debugger request. Map/HEX assertions must exclude the page from the test's own code and require separate operator approval for losing that exact page's contents. No value of P is supplied here.

## Reset-to-main boundary

Target execution starts after programmer installation/reset, not by jumping from the running USB diagnostic. No automatic fallback, soft-reset loop or resume from an earlier mailbox is allowed.

The final linked reset path must establish normal C storage and a valid stack before isolated main, leave no USB/DMA enabler running, and invoke no NVM register-writing initializer. Main must disable interrupts immediately and keep them disabled. Verify the effective CP0 interrupt state after that operation; do not assume IEC masks alone block CPU interrupts.

Do not copy generated SYS initialization blindly. It writes PMD registers, cache/wait-state settings and initializes peripherals. The isolated design avoids PMD/cache/clock switching and peripheral control-register writes unless a later reviewed startup requirement explicitly needs them. In particular, no clearing active USB/DMA/WR to force preflight success.

Unexpected state before owner initialization must lead to a non-writing startup failure report, not owner READY. Existing generated NO_OPERATION startup is deliberately omitted: there must be no NVM register writes before valid owner arming, whether direct or through runtime/user hooks.

A synchronous exception/NMI is not prevented by masking interrupts. The final link must provide a reviewed non-resuming exception path that does not relaunch or reset the experiment. It may halt forever; it cannot claim settled NVM or a bounded hardware stop. Debug exceptions/runtime monitor behavior need separate analysis in a debug image.

## Finite read-only preflight

Use a small compile-time bounded number of observations, proposed maximum256 iterations. Each iteration samples only approved status registers/CP0 and can stop early on a matching profile. This is an instruction-count policy, not a guaranteed elapsed-time bound: a missing clock, CPU stall or debugger halt may prevent progress before or inside it. No unbounded `while(SLOCK==0)` from generated `CLK_Initialize` is permitted.

Preflight evidence should include raw oscillator state, selected configuration values, raw Count observations, IRQ state, U1PWRC, DMACON and NVM status. Do not infer measurement accuracy from nominal configuration. A startup failure has no owner erase/write attempt and must be distinguishable from FAIL_NO_LAUNCH inside the armed owner.

Read-only matching conditions:

| Input | Required condition and limit |
|---|---|
| Target/build | Compile for PIC32MX270F256D; actual operator device/revision identification remains a delivery prerequisite |
| IRQ | CPU interrupt enable clear and no enabled runtime ISR/task enabler; no later restore to enabled state |
| Oscillator | OSCCON COSC=3 (primary+PLL), OSWEN=0, CF=0, SLOCK=1; multiplier/post-divider/PB divider and fuse input-divider choices match the nominal48MHz profile |
| Count | Cause.DC=0 and at least one Count change within the finite observation loop; no Count/Compare writes or assumed debug-mode behavior |
| USB | Single U1PWRC sample has USBPWR=0 and USBBUSY=0; do not read ordinary USB registers while disabled/busy |
| Central DMA | DMACON.ON=0; no channel/software owner that can re-enable it; do not clear an unexpected active controller |
| NVM | WR=0, no WRERR/LVDERR; do not clear errors or invoke NVM startup to manufacture a clean state |
| SW1 | Correct digital RB2 input, external pull-up profile, no output-drive/PPS remapping; sampling does not arm anything |

SLOCK means PLL locked **or startup timer satisfied**, per device OSCCON definition. It is not a frequency measurement. Count increments at SYSCLK/2 when Cause.DC is clear and may be suspended in debug mode; a progress test alone does not prove24MHz or stable timing. Delivery needs appropriate physical clock/VDD evidence and an agreed debugger running mode. Do not require ULOCK as a proxy for CPU PLL readiness or enable the USB peripheral to test its PLL.

At preflight completion recheck the whole profile; latch failure if any observed unsafe state occurred rather than waiting for it to disappear and accepting a later clean sample. Only pending PLL readiness in an otherwise expected, inactive profile may be observed within the finite loop. USB/DMA activity, NVM error, invalid clock selection or unexpected interrupt state is an immediate terminal failure. No implicit bus-master shutdown.

## Allowed minimal GPIO work

The inspected generated GPIO setup only clears ANSELB bit2; the BOOT_REQUEST macros read PORTB bit2 and select TRISB input. A future minimal startup may clear ANSELB2 and set TRISB2 to input, preserving other bits. Confirm internal pulls remain off using target definitions and board profile; do not write whole registers or remap PPS.

SW1 is active-low with the board's external pull-up. The owner receives `pressed = (PORTB bit2 == 0)` and performs its own release/gesture debounce. No button state or boot-held press performs an operation without the separately reserved token and fresh valid gesture.

No LED is needed for the first target. The board LED/mux path uses additional peripheral/I2C integration; omit it rather than initialize those paths for feedback. Primary evidence is SRAM only. Existing standalone LED application and diagnostic mux/ADC behavior remain untouched.

## Target port contract

The target port/context objects must have fixed lifetime and be disjoint from owner, mailbox, row and configuration storage. No callback may reenter the owner or enable/reset peripherals.

- `count(context)`: read CP0 Count register9/select0 with the existing toolchain primitive. Never reset Count or write Compare. The validated nominal profile is24MHz; raw deltas are modular and can be defeated by unseen multiple wraps or a halted counter.
- `unsafe(context)`: always check USB power/busy, DMA activity, effective CPU IRQ state and the validated clock/Count-control profile. It must read status only and never clear a condition. The port has an explicit private adapter lifecycle: PRE_INIT, INITIALIZED or INIT_FAILED; the operator/mailbox cannot select it. In PRE_INIT, additionally require WR0/no WRERR/LVDERR as startup readiness checks. In INITIALIZED, **exclude WR, adapter pending and NVM error bits from `unsafe()`**: active WR is expected, and errors must be preserved and reported through the real backend `busy()`/`error()`/launch paths, not converted into a startup-preflight fault. INIT_FAILED is terminal and cannot return to PRE_INIT or start another initializer. Define and test the bit map and transitions; no mode is inferred from debugger-controlled report fields.
- `initialize(context, backend)`: called only after the owner's valid one-shot gesture. With lifecycle PRE_INIT and the whole preflight still valid, call `UPDATE_NvmInit` once on a new/quiescent static adapter at nominal48000000Hz. On success, switch the private port lifecycle to INITIALIZED before returning its complete backend. On rejection, latch INIT_FAILED, return false and never retry; the owner reports initializer failure without launching. Do not clear NVM errors or reset adapter state. No erase/program, generated NVM init, USB stack, peripheral resume or reporting transport. The adapter's own IRQ-excluded init/start checks remain authoritative for NVM readiness; post-initialization port checks must not hide hardware error information.

A proposed owner timing profile is5ms debounce (120000 ticks),30-second arm window (720000000 ticks) and1-second erase/row observation budget (24000000 ticks). These are future test-policy inputs to review, not an approved on-board arm window or hardware deadline. The Flash timeout is rounded up from the Count budget by the reviewed owner. Count-budget authority means precedence over a generic Flash result **only when callbacks return and elapsed Count is interpretable**; it is not preemption or a guarantee of eventual settlement/reporting.

The existing `UPDATE_NvmPortWait()` loops until Count advances by the required ticks. If Count stops after preflight, that wait can stall during LVD startup or settlement and prevent control returning to the owner. No outer one-second check, finite preflight loop or snapshot sequence can interrupt it. A Flash-fetch stall, debugger halt or multiple unseen wraps can similarly defeat the elapsed-time inference. Keep the owner/row allocated and peripherals excluded; never claim a bounded hardware stop, automatic recovery or a guaranteed terminal report in that case. No wait cancellation or watchdog/reset change is selected by this design. Verify returning-call expiry boundaries and these non-return/stopped-timebase limitations separately.

After approved minimal RB2 input setup and successful read-only startup preflight, main initializes the owner once, then samples SW1 and calls owner Tasks in a serialized loop. Preflight may not require an already-configured RB2 before that allowed GPIO setup; any required GPIO status remains read-only thereafter. No sleep/WAIT, watchdog reset, task scheduler, interrupt-driven timebase, auto-rearm or state reinitialization. After terminal failure or PASS, the only allowed action is the owner's existing settlement observation when needed; no new launch or Flash readback on failure.

## Storage and startup reports

Use ordinary zero-initialized BSS for owner, adapter, configuration context and mailbox. No reset-retained section or persistent operation token. The final map must prove that each relevant object is in `[0xA0000000,0xA0010000)` with correct alignment and no overlap with stack/heap/runtime reservations; the complete128-byte flash-owned row must be uncached and word-aligned. A static C declaration alone does not prove that placement under final LTO.

A separate fixed-word startup report is needed for preflight failures that occur before owner READY. It must be distinct from owner mailbox/token and identify startup state/profile evidence without authorizing operations. Publish with the same odd/even/aligned-word/`sync` discipline and reserved exhaustion semantics. Do not leave a zeroed owner mailbox looking like a valid READY report on failure.

The owner mailbox protocol is unchanged: operator token only while READY, target-owned payload bracketed by monotonic sequence, zero/odd/`0xFFFFFFFE` rejected, settled terminal payload immutable. At sequence exhaustion the stale payload is unusable; the later reader must explicitly report unavailable/exhausted instead of displaying the old state as current. All reports remain debugger-modifiable evidence, not authentication or resume authority.

## Debugger workflow is a separate unresolved gate

The current firmware fuse profile has DEBUG OFF. Do not assume IPE programming mode can perform the required ordered RAM token write/snapshot read while permitting free-running execution. A later procedure must establish whether the approved MPLAB/PICkit workflow can do so, or whether a separately reviewed debug-image profile is required.

If debug configuration/runtime support is necessary, inspect every resulting config/boot/debug-monitor record and document each intentional difference. Do not silently toggle DEBUG, JTAGEN, ICESEL, power settings or Hold In Reset. Keep PWP/BWP/CP unchanged. A debug executive can use extra memory or affect Count, execution and bus timing; that belongs in the exact final image/operating-method review.

Before arming, establish an approved way to run the target with IRQ masked and Count behavior understood, and to inspect READY/page/profile plus ordered sequence snapshots. A halted target may be used for a deliberate token write only under that procedure; no arbitrary source/state/error writes, register watch auto-read side effects or halting during WR/settlement. Reconnecting PICkit5, resetting, collecting RAM/readback or changing settings are not authorised by this design.

If the mailbox workflow cannot be verified, stop at source/compile work. Do not substitute automatic SW1-only destructive startup, a powered USB command or an unreviewed UART control path. No debugger command sequence is provided here.

## Final standalone image audit requirements

The existing USB restart-image audit is **not** a standalone isolated-NVM audit: it expects restart-owner symbols, a128-byte retained journal and exact diagnostic configuration/debug-return content. Do not weaken it to accept a different image. Design a separate auditor with explicit mode/profile inputs; reuse only independently checked ELF/HEX parser logic.

Required evidence from the actual final compiler/linker output:

1. **Identity and source list:** full source/build commit, XC32/DFP versions/options, exact linker input list and startup runtime hooks. Confirm no mock NVM port include, active updater, USB/DMA initializer, diagnostic/application task or handoff path. Symbol checks supplement source and disassembly review; LTO absence alone cannot prove behavior.
2. **Load spans and configuration:** every HEX byte and loadable ELF section agrees; reject overlapping/rogue loads, unsupported aliases and out-of-file data. Code fits canonical32KB, approved Boot Flash/vector/reset spans and explicit config/debug records only. No application/metadata/scratch-page load records. Require exact expected config words, unchanged protection and separately approved debug differences. Inspect programmer erase scope later; artifact spans do not certify it.
3. **Initialization and memory:** parse actual linked XC32 `.dinit` with supported operations and termination, map it to delivered HEX/runtime address, validate SRAM destinations and **pairwise non-overlap**, prove owner/mailbox/adapter/row initialized without retained token, and exclude any reserved/no-init/stack/heap/debug region. Verify correct target exceptions/vectors/stack and valid owner storage before main.
4. **NVM unlock and settlement:** inspect the reachable linked launch implementation, not only object probes. Four ordered key/WR stores with correct addresses/values and no interleaving; source sync and physical KSEG1 row conversion; mode/address before startup delay; Count-derived10µs/1µs waits and WREN disarm only after WR clear/settlement. Verify current NVM errors are preserved. Identify every reachable NVM register write, including reset/runtime hooks: no generated pre-arm NO_OPERATION or additional erase/program launcher.
5. **Control flow and ownership:** token/gesture must dominate every launch. Exact compiled page and one-page wrapper, one erase/one row attempts consumed before forwarding, no reset/error-clear/retry/USB resume. Clock preflight loop finite and no unbounded generated PLL polling; stopped-clock/Flash-stall limits documented. Full failure/settlement logic, exception non-resume behavior and hardware-register access allowlist reviewed.
6. **Negative and target-port tests:** altered page/config/source/record, startup auto-NVM/USB call, wrong geometry, load overlap, pairwise `.dinit` overlap, missing owner/mailbox/row symbols, unsafe cached or aliased storage, malformed boot/config fields and modified unlock operands/order must fail. Mock clock pending/progress/DC/debug-state, USB/DMA/IRQ failures, button initialization and initialization-before-arming paths. Before initialization WR/errors must block readiness; after success WR/pending must not poison `unsafe()`, and latched errors must reach backend error reporting without clearing or misclassification. Test INIT_FAILED cannot retry, lifecycle fields are private, and exclusion violations still fail in all lifecycle phases. Existing owner suites remain required.

The exact final linked startup and operation code must be reviewed again if flags, debug settings or instrumentation change. The compiler probe checks already available are support evidence, not final-image approval. Audit metadata must keep hardware qualification and delivery approval false until separately recorded actual outcomes; passing an audit never grants an erase command.

## Source-only implementation — 2026-10-22

The target module has no active caller. The Makefile explicitly filters it out of diagnostic `BTL_SOURCES`; generated project source lists, configuration and startup are unchanged. `make isolated-nvm-check` compiles the real register interface to an XC32 object only. `make test` and `make test-sanitize` execute host mocks, never target hardware.

The hardware interface reads OSCCON, CP0 Count/Status/Cause/Debug, U1PWRC, DMACON, NVMCON, DEVCFG0–3 and RB2 GPIO status/pulls. DFP field assertions guard selected raw mask assumptions. It writes only ANSELBCLR bit2 and TRISBSET bit2 after the initial inactive/profile check. It does not read ordinary USB registers, write Count/Compare, initialize NVM or enable/disable a bus master. SW1 may be held during GPIO setup; the owner still requires stable release, token reservation and a fresh gesture.

Preflight checks the initial sample, observes at most256 loop samples and takes a fresh final sample before READY. Only pending SLOCK/Count progress may wait within that loop. All other observed violations fail immediately and latch INIT_FAILED. The exact configuration words are `DEVCFG0=7FFFFFE3`, `DEVCFG1=FD74CE5B`, `DEVCFG2=FFF97AFA` and `DEVCFG3=FFFFFFFF`, matching the current non-debug/protection-OFF profile. These checks do not measure clock frequency, VDD or actual device revision.

The startup report uses22 aligned words: version/state/unsafe bits/observation count/first and last Count, followed by16 raw input words. States are1 checking,2 ready and3 failed. The exclusion bits are USB1, DMA2, IRQ4, clock8, Count16, debug32, NVM64, GPIO128 and state/configuration/sequence256. PRE_INIT requires clean WR/errors; INITIALIZED leaves WR/pending/errors to the backend. Initializer rejection, invalid output storage and runtime exclusion violations publish terminal failure. A repeated initializer cannot reset the adapter or launch again.

`ISOLATED_TargetSnapshot()` rejects unpublished, odd, exhausted, changed-during-copy or overlapping output snapshots without changing output. Publications use XC32 `sync` and a private expected sequence. Exhaustion or detected sequence disagreement marks the payload unavailable with `0xFFFFFFFE`, so an earlier READY payload cannot be accepted after that detected failure. Before detection, deliberate debugger mutation can still fabricate apparently coherent evidence; the mailbox is not authentication. A terminal failure permits no new initialization or publication recovery.

The trusted caller must allocate disjoint, fixed-lifetime owner/context/mailboxes/configuration/port storage, establish IRQ exclusion and call preflight once on new/quiescent storage. The API does not enforce arbitrary C pointer validity or prohibit a trusted caller from reinitializing live objects. No final SRAM placement or reset-to-main execution has been verified.

Passing verification after the final mailbox fix:

- `make isolated-nvm-check`: strict XC32v6.00/DFP1.7.380 compile-only checks for the owner and real target interface; three isolated mock suites.
- `make test` and `make test-sanitize`: full strict/ASan/UBSan suites, including30 Python methods and exact-source target snapshot cuts at all22 words and every odd publication step.
- Owner → target port → actual NVM adapter mocks: no initialization before valid arming; one erase/row/full-page verification; no-token/held-button/bad-token/expired gesture rejection; terminal pre-init/backend/post-init failures; WR/pending handling; hardware error propagation; timeout settlement without more readback or launch.
- `make nvm-codegen-check`: eight existing production/debug object modes pass ordered unlock checks. The final target object disassembly contains `sync` and read-only CP0 access. Object checks do not prove final linked startup/operation paths.
- `bun tools/audit-restart-image.ts` and `bun tools/test-restart-image-audit.ts`: existing canonical diagnostic artifact/negative audits pass without relinking it. All existing HEX hashes match their pre-test values. Canonical/recovery hashes remain `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7` and `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`.

Bounded implementation reviews found privately latched runtime failures leaving the startup report READY, incomplete configuration validation and stale READY readout after a changed-even sequence failure. Terminal report publication, exact configuration words and explicit mailbox invalidation fix those findings. Regression tests pass; a narrow final follow-up confirms the stale-readout fix. A separate GPIO comment assumed SW1 had to be released before setup; the design permits a held input and the mock suite now covers it. These reviews provide bounded source evidence only, with no exhaustive or hardware approval.

## Prior design verification and review

Inspected the actual generated oscillator/configuration, GPIO and whole-system startup, the reviewed owner API, NVM port waits/source/launch code and the current USB-specific image audit. Device DS60001168L OSCCON definitions and CPU FRM Count/debug-control text were checked against original PDF text; DFP1.7.380 defines the referenced oscillator fields. These are source/reference checks, not measured clock or voltage evidence.

Mechanical checks confirm the nominal12MHz/DIV3/MUL24/DIV2 profile gives48MHz SYSCLK/24MHz Count, and the proposed debounce/arm/operation tick values fit the reviewed modular interval. Existing inactive-owner and actual-adapter mock suites repassed. No test-page value or target backend was supplied by these checks.

A completed independent design review requested lifecycle-specific NVM `unsafe()` handling and explicit non-returning Count-wait limitations. PRE_INIT/INITIALIZED/INIT_FAILED behavior and returning-call-only budget semantics were specified above; a bounded follow-up found those clarifications closed and no new critical contradiction. It also accepted the minimal GPIO ordering clarification. This is a bounded design pass only: actual target port/startup code, debugger visibility, final linked reset/operation paths and all hardware qualifications are still unverified.

The prior design milestone changed no C source, configuration or build target. The later source-only implementation is described above. The current diagnostic and recovery artifacts remain untouched.

## Delivery gates and current scope

Before any future image is offered, complete source/port/startup review and mocks, establish the explicit debugger configuration/method, choose a separately approved disposable page and pass the dedicated final-image audit. Then obtain operator permission for the exact image, page loss, one transaction, measurements and recovery envelope. No page is selected now; abstract constants and timing values in this design are not hardware requests.

The operator must provide/approve relevant current voltage, clock/configuration and target/recovery evidence under a bounded procedure. No protection boundary or direct forbidden-prefix test belongs in the first isolated transaction. Recovery can be destructive and may lose the report; it is not automatically safe merely because a read-only HEX exists.

The next gates are an agreed debugger/mailbox operating method, separately approved exact disposable page, dedicated entrypoint/configuration and standalone final-image auditor. None is supplied by the port implementation. Keep the board on the canonical diagnostic, running/read-only with PICkit5 disconnected and external power/settings unchanged. No NVM write, programmer reconnection, reset, PAUSE, upload, protection change or fault test is authorised.
