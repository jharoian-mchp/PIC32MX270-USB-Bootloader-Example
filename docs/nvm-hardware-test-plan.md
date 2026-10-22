# Gated NVM and protection hardware test plan

**Do not run flash tests or change protection bits yet.** This is a proposed qualification sequence, not executable firmware or permission to program. Current hardware-tested firmware is read-only. One controlled48KB USB-only cycle and one controlled canonical32KB cycle on distinct images passed on2026-10-22; repeated/fault/integrated-write qualification is not established. A1 identity and IDE-displayed protection OFF are recorded; target writes and relocated application remain untested. The [current isolated-test readiness review](nvm-qualification-readiness.md) records the missing caller, scratch-page approval, final-image audit and whole-operation ownership requirements.

## Preserve recovery first

Before delivering any new diagnostic image:

- Keep annotated `v0.1.0-led-blink` fixed. Save the known-good read-only bootloader HEX and its source commit/build settings separately from a future test build.
- Confirm PICkit 5, physical MCLR/SW1 entry and recovery programming remain available. Record board power/VDD, cable connections, tool versions and explicit test-image SHA-256.
- Read actual device configuration with the programming tool and record DEVID/revision/DEVCFG0. The supplied screenshot displays 7FFFFFE0 but live-readback provenance is unknown. Do not overwrite the original/readback evidence.
- Establish the programming-tool bulk-erase/recovery procedure and protection interaction before setting BWP/PWP. Treat a failed erase/program as a stop; never improvise with an unbounded generated updater.
- Review every test HEX address, configuration record, ELF section and physical write whitelist. The test must not touch Boot Flash/configuration or the bootloader prefix through its own NVM API.

No program/erase action is performed automatically from this plan. Each hardware stage requires a separately tested image, reviewed procedure and explicit operator go-ahead.

## Phase A — read-only board evidence

No new NVM write image is needed for existing debugger/device information. Record:

- Tool-identified PIC32MX270F256D, A1 and actual configuration readback.
- SW1/USB entry and power/reset behaviour of the current diagnostic image.
- Clock/Count and voltage measurements adequate for Flash timing, without interpreting typical FRC-cycle specification values as worst-case limits.

Retain ADC/mux/I2C safety behaviour and known PICkit recovery. This phase does not establish NVM launch, protection or application handoff.

## Phase B — USB lifecycle without flash

Deliver only a reviewed, **read-only** USB restart diagnostic after pure stage/execute/lifecycle mock tests and generated driver ownership audit pass. Instrument main/task events, USB power/busy observations, callback generation, buffer ownership and re-enumeration.

Test one deliberate pause and repeated pauses under host control; verify:

- Full response transfer completes before shutdown.
- Driver/HID/IRP buffers are retained/cancelled correctly; no stale callback mutates a reused buffer.
- Bounded shutdown reaches USBPWR=0/USBBUSY=0 without forbidden register access during disabled-but-busy status.
- USB-layer reinitialisation does not call whole-system initialisation or reset the update session/NVM storage.
- Host reconnect is unambiguous on Windows and verified with epoch/token/ticket identity; uncertain/lost replies do not silently retry a mutation.
- Unplug/replug, MCLR and power cycle before/during/after the pause stop or recover according to the state machine. Record limitations in observing a brief cable disturbance.

**No NVM write is included.** Failure blocks later connected flash tests. Measure reconnect latency to decide whether operation-per-enumeration is usable. The single completed cycles do not authorise repeating this broad proposed matrix. A cold USB-off isolated NVM image can be designed separately without USB binding; connected writes still require the combined lifecycle and recovery qualification.

## Phase C — isolated target erase/row/readback, USB off

Requires a reviewed isolated test image, link/HEX whitelist, verified USB-off gate, exact page address, known snapshot and PICkit recovery. The operator must approve erasing the selected page; its previous contents will be lost.

Select a sacrificial, erase-aligned page entirely inside the reserved application interval, outside the bootloader and metadata. Choose/record its address at test-delivery time from the actual memory map; no default address or executable erase is supplied here. Do not use a page holding the standalone baseline or a valid bootable image without explicitly retiring that image first. Retire any commit record through a reviewed procedure before modifying an accepted application.

The isolated test must:

- Keep USB/DMA off for the entire erase/write/settlement/readback and timeout quarantine.
- Verify erase as 1024 bytes of FF, then write/verify approved deterministic 128-byte rows from aligned KSEG1 SRAM; check errors and source-buffer lifetime.
- Measure startup/settle/row/page/verification elapsed times with core-count logic and external timing where appropriate. Confirm the clock configuration matches the supplied timing frequency.
- Report faults locally/debugger after NVM settles, without auto-reenabling USB against a busy operation.
- Use bounded error paths; never overwrite the bootloader/configuration or silently clear/retry hardware errors.

Only offline mocks may inject low-voltage/error bits until a safe electrical fault-injection plan is approved. Do not brown out a live board or disconnect power during writes as an improvised test.

## Phase D — protection boundary and recovery

Candidate for confirmed A1: nominal PWP32K with BWP ON, subject to [the applicable errata policy](flash-launch-policy.md). Do not apply family off-by-one workarounds to this unaffected A1 row.

Before programming the candidate, inspect resulting config bits/HEX and confirm actual programmer recovery under BWP. Preserve unrelated options. Then record device readback and independently verify intended boundaries:

| Region | Intended behaviour |
|---|---|
| Program prefix [1D000000,1D008000) | Hardware protected |
| Application page 32 at 1D008000 | Writable |
| Metadata page 255 at 1D03FC00 | Writable |
| Boot Flash/configuration | BWP protection as specified; programming-tool recovery established |

A production API already rejects the protected prefix, so calling it cannot by itself prove hardware PWP. Do not add a backdoor erase to the live updater. Any direct-register forbidden-write test needs an isolated disposable image, controlled no-retry attempt, expected error handling and PICkit recovery; a mistaken boundary could erase code. Decide the safest verification method before delivering that test. No protected-region erase is authorised by this document.

Recheck first writable application page and metadata with the isolated writable-region test, then restore/reverify the agreed recovery image. Stop on any configuration/readback/protection discrepancy.

## Phase E — relocated application and connected update

Regenerate the application's own MCC configuration on Windows with current Harmony packages, preserve app_led.c/.h/main/project/ISR integration, record actual package versions and first rebuild/test it as a standalone application. The old tag's hardware result does not qualify newly generated code.

Next build/review bootloader-compatible relocated startup, vector/exception layout and linker range, excluding Boot Flash/configuration and metadata records. Run host preparation and full-span checks; review handoff state and valid-image boot selection before loading through an updater. A CRC-valid image does not prove execution correctness.

Only after previous phases pass, connect the new negotiated stage/execute transport/host. Start with the small relocated LED image. Record exact image length/CRC/commit and expected execution/reset behaviour; keep auto-run disabled until manual valid-image handoff has been independently reviewed.

## Phase F — interrupted update/recovery

Requires explicitly approved fault-injection procedure and known working recovery path. Test separately:

- Lost acceptance/EXECUTE/RESULT/RESUME responses, duplicated and changed requests.
- Unexpected cable removal, reset and power interruption during metadata invalidation, application erase, row programming, verification and commit.
- Driver restart failure, operation timeout/readback/CRC/NVM errors and changed device/boot epoch.
- Last-response loss after successful commit: host reports uncertainty and resolves stored state without rerunning commit.

Expected invariants: no application erase before metadata invalidation verifies; no successful commit before image verification; no source-buffer reuse while NVM busy; no USB reenable before settlement; no cross-reset session continuation; valid-image check governs handoff; rejected/torn/partial images remain recovery-only. Some failures can leave a previously or newly valid commit record before cleanup finishes—analyse each cut point instead of assuming all aborts persistently invalidate immediately.

Record evidence per phase: source commit, ELF/HEX hash, programmer/config readback, host/Python/hidapi/tool versions, measurements, error/status traces and pass/fail. Update README only with actual performed results. Hardware tests are not complete merely because this plan or its offline mocks pass.
