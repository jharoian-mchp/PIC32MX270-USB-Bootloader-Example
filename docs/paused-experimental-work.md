# Experimental work paused — 17 November 2026

John paused the experimental USB/NVM qualification work to prioritise a customer-facing Harmony 3 bootloader and demonstration application.

## Resume position

- RAM-only debugger rehearsal passed on 5 November: hardware pre-edit/readout breakpoints, one accepted 32-bit edit and uploaded readback, one challenge consumption, RAM_COMPLETE/fault 0, intact guards/sequence 12 and matching 33-word private/published reports. Count advanced 742 ticks after consumption; no calibrated timing claim.
- Exact IDE ELF SHA-256: `09458d693083dc96786684dcc3aac814e4f2bac7b0510ed3c66ea767e2a6d709`. See [result and evidence](ram-rehearsal-result-20261105.md) and `docs/evidence/ram-rehearsal-20261105/`.
- The managed source project under `rehearsal/managed/` is now preserved in Git. Its original package instructions describe the historical delivery scope; subsequent Windows verification is recorded in the result document.
- Last reported board state, 5 November 21:57:48: halted at RAM_RehearsalReadout, external power active, PICkit attached, final export completed, no further reset/execution. Current physical state on 17 November is unknown. Reconfirm before any board procedure. Pausing development does not authorise debugger termination, disconnect, reset or power changes.
- Retain all deviations: extra erase/reprogram, repeated baseline export and five extra captured words. The first configuration-only programming operation's cause is unknown.
- Canonical USB-only restart milestone: 09da506 passed one controlled PAUSE cycle, STATUS 1/2/1/7. It is not the last reported installed image; RAM rehearsal installation superseded it.
- Isolated NVM implementation is inactive. No isolated NVM image, disposable page, protection/timing test, live uploader or connected write has been qualified. DEBUG-OFF policy, final linked write-path/startup audit, USB/DMA exclusion, bounded settlement and recovery decisions still need work.
- Application MCC regeneration/relocation and custom write-gated transport integration are unfinished. Preserve `v0.1.0-led-blink` unchanged.

## Preserved artifacts

The complete evidence archive remains at `/workspace/exports/ram-rehearsal-evidence-20261105.tar.gz`; its ELF/map and checksum inventory are retained there. Build products remain outside Git. Saved SRAM text and artifact hashes are committed here. Toolchains remain under `/workspace/ChrisK/tools/`.

`make test` passed on 17 November before the pause commit. No firmware rebuild/link or hardware operation was performed for this checkpoint.

## Customer-work separation

Use a separate customer branch and demo project paths. Existing custom manifest, dispatcher, USB restart overlays, NVM owner and RAM rehearsal are experimental reference material; they are not automatically dependencies or requirements of the stock H3 demonstration. Existing generated-parser audit findings still require explicit disposition for the selected customer path.
