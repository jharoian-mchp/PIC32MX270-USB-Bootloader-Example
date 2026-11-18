# Customer demo board adapter — source and mock checks

The customer bootloader now has a minimal board adapter. Host mock checks pass; the firmware has not been compiled with XC32, linked, programmed or tested on the board.

## Integration

- `board_demo.c/.h`: single-instance, one-shot expander/ADC state machine with injected hardware operations. Persistent I2C buffers remain quarantined on timeout; no automatic retries or reinitialisation.
- `board_demo_platform.c/.h`: generated I2C1/ADC APIs, nominal Count-to-milliseconds adapter and strong `bootloader_Trigger` override. Four active-low RB2 samples over nominal 6 ms select bootloader entry; a fixed iteration bound also handles non-advancing Count. Trigger runs before peripheral initialisation, as required by generated SYS_Initialize. A released button keeps the stock application-start/blank-image policy.
- `main.c`: board adapter initialises after SYS_Initialize and polls before the unmodified SYS_Tasks loop. I2C1/ADC are already initialised and interrupts enabled at this point.
- HID datastream remains sole USB callback/attach owner. Its generated POWER_DETECTED handler now records power; DATASTREAM_Tasks attaches only after mux readback and qualified AN1 cable detection. POWER_REMOVED and board faults detach through that same owner. No competing device handler, custom protocol, USB restart or isolated NVM module is added.
- MPLAB configurations.xml includes the two new C sources. Generated makefiles must be refreshed by the IDE later before an approved build.

The expander sequence reads OLAT/IODIR, sets only GP5/GP6 latch bits and clears only their direction bits, then verifies both complete bytes. Other control pins are preserved. The adapter uses address0x22, a100ms I2C observation deadline, polled ADC with10ms deadline and10ms cadence. Three qualified samples select cable state; nominal thresholds465/248 correspond to the existing1.5V/0.8V board policy at3.3V. Supply/threshold worst-case qualification is separate.

## Verification

`make customer-board-test` passes strict C11 host compilation and mocked tests for masked expander updates/readback, retained timeout buffers, NACK, mismatched readback, ADC hysteresis/timeout, exact extracted datastream attach gate and mocked platform boot trigger (high/low/stalled/wrapping Count). `make test` also passes. These host tests do not execute the real USB driver, interrupts, I2C electrical bus or device SFRs.

Stock bootloader parser, erase/write routines and protocol are unchanged. Existing audit findings and stock close/deinitialisation behaviour still need disposition before connected writes or handoff testing. USB loss during an update has no newly implemented recovery policy. Faults stop board polling and deny attach; they do not constitute cancellation or settlement of an in-flight flash transaction.

## Regeneration and later gates

MCC regeneration can overwrite main.c and the datastream attach adaptation. Preserve/reapply only these documented narrow edits and inspect their diff; do not import experimental private USB retirement overlays. GPIO/ADC default reset-state assumptions, linked timing/code generation, startup/application handoff cleanup and complete transport callbacks require target-source/artifact review before hardware approval.

Fixed application start stays1D008000/9D008000 and reservation32768. No customer firmware build/link or board authority is granted by these source checks. The relocated demonstration application, final-image boundary review, vendor host compatibility, recovery procedure and explicit hardware test approval remain separate steps.
