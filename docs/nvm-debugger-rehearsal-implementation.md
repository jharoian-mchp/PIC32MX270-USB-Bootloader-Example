# Inactive RAM debugger rehearsal — mock-tested implementation

`rehearsal/src/debugger_rehearsal.c/.h` implements the RAM-only challenge, guarded snapshots and one-shot inert rendezvous. `debugger_rehearsal_hw.h` supplies read-only PIC32MX270F256D register access. Strict mocks, full ASan/UBSan suites and XC32 compile-only checks pass. No main, configuration translation unit, project/startup binding, linked image or board procedure exists.

This source milestone follows the [reviewed design](nvm-debugger-rehearsal-design.md) and [offline debugger investigation](nvm-debugger-mailbox-investigation.md). The user requested the next inactive implementation step; no programming, debugger connection/settings change, reset, RAM collection, PAUSE, NVM/protection or upload operation was performed. The board stays on the canonical diagnostic, running/read-only with PICkit5 disconnected and power/settings unchanged.

## Source boundary

The entire library lives outside `bootloader/src` in `rehearsal/src`. Diagnostic source discovery and saved MPLAB projects are unchanged. The new Makefile `debugger-rehearsal-check` compiles one real hardware translation unit to an object and executes a host-only mock suite. `make test` also invokes that suite; `make test-sanitize` instruments it.

The rehearsal has no include/link dependency on isolated owner/target, `update_flash`, `update_nvm`, generated NVM startup, USB, DMA, GPIO, LED, application handoff or page geometry. Host commands link only the rehearsal test translation unit; the real compile-only command references only the rehearsal C source. No garbage-collection assumption is needed to remove a Flash launcher from its input list.

The hardware interface reads OSCCON, Count/Status/Cause/Debug CP0 registers, U1PWRC, DMACON, NVMCON and DEVCFG0–3. It writes no SFR or CP0 register, does not read ordinary USB registers and never initializes or disarms NVM. The library does not disable/restore interrupts: a future trusted main must disable once after reviewed C startup, and the library rejects observed IE1 rather than re-masking after debugger return.

The exact profile is `RAM_REHEARSAL_DEBUG_V1`: configuration words `7FFFFFE0/FD74CE5B/FFF97AFA/FFFFFFFF`, nominal primary12MHz/DIV3/MUL24/DIV2, PB DIV1, COSC/NOSC3, OSWEN0/CF0 and eventual SLOCK1, IE0/DC0/DM0 and USBPWR0/BUSY0/DMA.ON0/WR0/no NVM errors. Only DM from Debug is interpreted outside Debug mode. Static DFP assertions guard selected register-mask assumptions. Actual programmed fuses, debug executive, VDD, frequency and device identity remain delivery evidence; accepting this source profile programs nothing. The existing isolated NVM target still requires its distinct exact DEBUG-OFF word `7FFFFFE3`.

## API and storage contract

- `RAM_RehearsalInit(r, mailbox)`: initialize new/quiescent fixed-lifetime storage once per boot, perform finite read-only preflight, publish WAIT_EDIT and enter the pre-edit rendezvous once. A debugger edit at that point is left for Tasks to consume after return. Null, misaligned or overlapping context/mailbox storage is rejected without mutation.
- `RAM_RehearsalTasks(r)`: serialized polling with zero edit inert, first nonzero copy/spend/clear before sequence/guard/profile/challenge validation, fresh post-consumption Count observation and permanent terminal handling. No backend or caller callback exists.
- `RAM_RehearsalSnapshot(mailbox, out)`: copy the33 report words only when guards and valid even sequence match before/after; invalid alignment/alias/storage/sequence/guard state fails without writing output. It does not copy/modify the operator edit word or guards.
- `RAM_RehearsalPreEdit()` and `RAM_RehearsalReadout()`: production noinline inert nop/return functions. Their calls are privately gated and occur at most once each. Direct trusted-C invocation would bypass eligibility; these symbols are not a user command API. Actual linked instruction, delay slot, breakpoint and debug-return semantics need later audit.

Compile-time layout assertions pin the37-word/148-byte mailbox: guard/edit/guard/sequence, nine report header words and twelve-word before/after samples. The RAM1 challenge `0x52414D31` differs from the destructive owner token. State/fault/layout values follow the design. Wrong challenge and damaged guards still spend a nonzero attempt; sequence has validation priority, then guards, profile and challenge. Guards remain damaged on failure. Guarded snapshots reject them; the target may publish fault evidence under a still-consistent sequence without granting readout.

The caller must supply valid disjoint objects and prevent reinitialization within a boot, concurrent/reentrant APIs and arbitrary private-state writes. Init alignment/overlap checks do not authenticate C pointers, detect all backing-storage aliases or enforce hardware SRAM placement. Final KSEG1/stack/heap/executive layout and startup initialization require a linked-image review. No reset-retained session or token is used.

## Finite observations and terminal readout

Initial preflight samples once, performs at most256 loop observations, then uses fresh final and pre-edit-gate samples. Only SLOCK/Count readiness may wait; any other observed violation is terminal. The pre-edit sample is published before the rendezvous. A premature nonzero edit detected before that point is spent/cleared and rejected rather than accepted without the approved write location.

After a valid edit, Count anchors at a fresh sample taken after consumption. Post-consumption observations are capped at256; successful early progress still requires a fresh final matching profile and nonzero unsigned delta. A final sample returning to the anchor fails. Debugger delay before consumption, including a wrap, does not become the reported resume delta. Count progress/caps do not establish elapsed time,24MHz accuracy or guaranteed CPU progress.

COMPLETE/FAILED payloads are immutable. Profile or guard failure permanently denies readout even if hardware/guards later look clean. Wrong-challenge and Count-progress failures can use the conservative full-profile readout gate, provided guards/private sequence still match. Full-profile matching for failure readout is stricter than the design's minimum idle gate. No readout occurs for detected active USB/DMA/WR, invalid IRQ/clock/configuration or corrupt report. Raw errors are preserved, not cleared.

A new unsafe sample after terminal publication records a one-way private denial/reason/raw sample and does not republish the result. Detected sequence disagreement is the sole terminal mailbox-write exception: ordered `0xFFFFFFFE` marker invalidation once, leaving payload/guards/edit word unchanged. This prevents ordinary readers accepting a changed-even stale result after detection. Further mailbox writes and rendezvous are forbidden. Snapshot consistency before detection is evidence only; deliberate debugger modifications cannot be authenticated.

Readout uses a fresh complete profile, enters its inert function once and checks integrity/profile again after return. Later Tasks continue terminal integrity/status observations without another rendezvous or payload publication. They never consume a replacement edit or resume the challenge. Host breakpoint absence or delay does not grant a manual halt/reset/retry procedure.

## Verification and bounded review

Passing checks after the last source/test edits:

- `make debugger-rehearsal-check`: strict XC32v6.00/DFP1.7.380 real-interface object, mock suite and object checks. No link/HEX step.
- `make test` and `make test-sanitize`: full strict and ASan/UBSan suites, including30 Python methods and the new RAM suite.
- `make isolated-nvm-check`: existing three owner/adapter/target mock suites and real owner/target compile-only checks repassed unchanged.
- Mechanical verification: diagnostic BTL_SOURCES exactly matches baseline `9865570`; no new rehearsal file enters diagnostic/application project inputs. No changes to existing firmware C, configuration or saved MPLAB settings.
- Existing HEX hashes before/after all tests match. Canonical `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7`; recovery `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`.

The RAM mocks cover every bit of all four exact configuration words; IRQ/DC/DM/USB/DMA/WR/error/clock rejection; uninterpreted COUNTDM; finite stopped Count/pending lock; final-sample wrap/profile/guard/sequence faults; no-token waiting; wrong/destructive/premature challenges; simultaneous nonzero/guard/profile failure spending; pre-breakpoint Count delay/wrap; no IRQ re-mask after debug return; all33 field snapshot sequence/guard cuts and every odd publication; alias/alignment/output preservation; unavailable/exhausted/corrupt sequence; one-shot rendezvous; immutable terminal payload and readout denial even after activity disappears.

Test hooks are inserted only into an ignored copy of the exact source for deterministic single-thread interleavings. Production has no snapshot or rendezvous callback interface. The uninstrumented host object imports only the mocked read function; explicitly requested sanitizer builds also import ASan/UBSan runtimes. An initially over-strict import check rejected those instrumentation symbols and was corrected. A wrap-boundary fixture initially forced Count to zero rather than the original anchor; it was corrected before the passing final reruns.

The XC32 object imports only the eight allowed SFRs and C-runtime `memcpy`/`memset`. Disassembly includes `sync`/read-only `mfc0`, rejects `mtc0/di/ei/sdbbp` and shows inert `jr ra`/`nop` rendezvous blocks. Import/opcode checks supplement source inspection; they cannot prove all arbitrary instructions safe, final executable storage or an uninspected debugger executive. Final LTO/startup/install code needs its own audit.

One broad delegate attempt returned empty; a completed bounded source/test review found no must-fix logic defect. A subsequent larger follow-up timed out. A completed final narrow review confirms terminal profile denial is monotonic, sequence corruption still invalidates after denial and terminal payload remains unchanged. These are bounded source reviews only, with no exhaustive proof or hardware qualification.

## Remaining gates

A future rehearsal main/configuration translation unit, explicit default-unbuilt source list, final debug linker/auditor and non-resuming exception/debug runtime path still need separate implementation and review. The real debugger-installed executive/configuration/Boot Flash/RAM reservations and actual MPLAB Xv6.35/PICkit5 settings/erase scope require evidence. IPE's reported Hold In Reset setting does not establish IDE state.

No linked image, installation approval, programmer connection, live SRAM edit/readout or host trial is supplied. Even installing a RAM-only application can erase/program Flash; Debug mode programs configuration/Boot Flash and selected memory ranges do not bound Erase. Obtain exact image/profile/install/recovery permissions before that step.

A RAM rehearsal does not select an NVM page or qualify the gesture/owner/backend timing/settlement/protection/USB-off write interval. Those need later distinct source/final-image/page-loss/measurement/transaction/recovery approvals. Keep the current board unchanged and read-only; no reset, PAUSE, programmer reconnection, NVM/protection change, upload or fault test follows from this implementation.
