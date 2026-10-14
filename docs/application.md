# LED1 application: build and board test

The MCC-generated application now includes user-owned MCP23008 LED1 logic. **Linux compilation and mocked tests pass. The user confirmed successful Windows PICkit 5 debug and production builds/programming, LED1 blinking, and recovery after MCLR and a power cycle on 2026-10-13.**

Reviewed: **2026-10-13**. The generated baseline was pulled from the user's first-generation commit `5226a6d` through merge `c8bb0cb`. The user reported no MCC notifications and confirmed `ICS_PGx4`.

## Generated configuration

The saved project is `application/app.X`, with generator configuration `application/app.X/app_default/mcc-config.mc4` and component YAML files. The component metadata records **Harmony CSP v3.26.0**, generator core **5.9.0**, and SPA host **1.3.0**. These are generator-internal/package versions; the Windows MCC application version remains the user-reported v5.7.0. Generator DFP 1.6.369 and compiler DFP 1.7.380 remain separate.

Code review confirmed:

- `initialization.c`: PRIPLL, HS, system PLL ÷3/×24/÷2, PBCLK ÷1, USB PLL OFF with input ÷3, `ICESEL = ICS_PGx4`, WDT/JTAG/protection OFF.
- I²C1: BRG 235; asynchronous `I2C1_Write()` and `I2C1_WriteRead()` shift the seven-bit address internally and handle NACK/collision errors.
- Timer2: internal clock, 16-bit mode, prescaler 1:64, PR2 7,499 and interrupt enabled. **MCC initialises it stopped**; the application registers a callback and calls `TMR2_Start()`.
- EVIC: multivector mode; Timer2 and I²C1 handlers use priority 1, subpriority 0. The application does not submit I²C operations in either interrupt.
- GPIO initialisation is empty because the selected I²C pins use their fixed peripheral functions; no directly connected LED GPIO is assumed.

This review verifies source settings, not oscillator frequency, physical pin behaviour or working hardware.

## LED behaviour

User-owned code is in `application/src/app_led.c` and `app_led.h`; `main.c` calls its initialiser and task function. `nbproject/configurations.xml` includes the new files for MPLAB X. Files under `application/src/config/default/` were left unchanged.

LED1 maps to MCP23008 **GP0**, seven-bit address **0x22**. The carrier's Figure A-8 (source PDF page 56) shows the LED1 active-low sink path. The [MCP23008 data sheet](docling/MCP23008-MCP23S08-Data-Sheet-DS21919E.md), registers IODIR (0x00) and OLAT (0x0A), defines input/output direction and latch access.

Initialisation performs four asynchronous transactions:

1. Read OLAT, preserving GP1–GP7.
2. Write the same latch with GP0 high, preloading LED1 OFF.
3. Read IODIR, preserving other pins' direction.
4. Clear only IODIR bit 0, enabling GP0 as an output.

Once ready, the main loop changes only OLAT bit 0 after 50 Timer2 ticks: **approximately 500 ms per state change**, or one second per full blink cycle. No other code writes the expander, so the preserved latch/direction values remain valid. Do not add another expander writer without coordinating its state.

Transfers use persistent buffers. A NACK or collision records a fault and enters a one-second retry delay. An active transfer or rejected start has a 100 ms deadline, then resets the I²C PLIB atomically and retries. Recovery re-reads the expander before changing LED1. This is bounded software retry, **not** an electrical bus-clear sequence: permanently held-low SDA/SCL still needs hardware diagnosis. Timeouts depend on a functioning Timer2 interrupt; they do not recover a stopped CPU clock or disabled global interrupts.

## Linux build and tests

From the repository root:

```sh
make test
make application
```

`make application` builds the exact PIC32MX270F256D with XC32 v6.00 and DFP 1.7.380, using the MCC-provided linker script and 512-byte minimum heap. Outputs are ignored:

- `build/application/app.elf`
- `build/application/app.hex`
- `build/application/app.map`

The map places the normal reset entry at `0xBFC00000` and exception memory at the generated `0x9D03F000` region. This is an **ordinary application without a USB bootloader or bootloader offset**. With all 12 project C sources and section garbage collection enabled, `xc32-size` reports 6,100 bytes text and 104 bytes BSS; map-based region accounting and runtime stack use are separate checks.

`make test` compiles the application with mocked I²C/timer interfaces and `-Wall -Wextra -Werror`. Tests cover safe OFF-before-output initialisation, preservation of GP1–GP7 latch/direction, the 500 ms cadence, asynchronous buffer lifetime, NACK, collision, transfer timeout, rejected starts and retry behaviour. A separate AddressSanitizer/UndefinedBehaviorSanitizer run also passed. Mocks do not validate the generated PLIB or physical bus.

The user-owned application also compiled separately with XC32 `-Wall -Wextra -Werror`. The complete Linux build with `-Wall -Wextra` reports eight unused-parameter warnings: `data` in generated `SYS_Initialize`, `forcedWrite` in generated I²C transfer setup, and six parameters in the generated `read`/`write` console stubs. The earlier two-warning report omitted three support sources from the Linux build; the target now includes every C source listed in the Windows build. Generated code was not patched.

## Windows build and first board test

1. Pull the repository using the agreed merge-based workflow. If the project is already open in MPLAB X, reload/reopen it so `app_led.c` is included.
2. Open **`application/app.X`** in MPLAB X v6.35. Confirm XC32 v6.00, PIC32MX_DFP 1.7.380 and PICkit 5.
3. **Clean and Build Project**. MPLAB X recreates its ignored machine-generated makefiles. No MCC regeneration is needed just to build the added LED code.
4. Confirm `app_led.c` appears in the compilation output. If it is absent, stop and report the project view/build output rather than programming a skeleton without the application.
5. Confirm the supplied hardware settings: Y1 marked 12 MHz, reset selector PIM_MCLR, carrier J5 pins 1–2 (3.3 V), PIM J1 pins 1–2 (0x22). Use the board's documented power arrangement and check the PICkit target-voltage setting before connecting/programming.
6. Program the **application project**, not `tools/toolchain-smoke.c` or its HEX. Expect **LED1 (D5)** to change state about every 500 ms after initialisation.
7. Record the PICkit target/device revision, LED behaviour and reset recovery. Test from a fresh board power-on as well as a reset; the expander may retain state across an MCU-only reset.
8. If it does not blink, use **Debug Project** and inspect `app_led_diagnostics`. Set a breakpoint in main-context `APP_LED_Tasks()` rather than pausing repeatedly inside an I²C/timer ISR; halting the CPU can disturb peripheral timing.

### Debugger diagnostics

| Field | Meaning |
|---|---|
| `ready` | Successful expander initialisation. |
| `led_on` | Last successfully written LED1 state; may be stale after a fault. |
| `last_fault` | NONE=0, NACK=1, BUS_COLLISION=2, TIMEOUT=3. Cleared on successful reinitialisation. |
| `errors` | Cumulative faults since application start. |
| `transitions` | Successfully completed blink state changes. |

If TIMEOUT is reported, check pull-ups, address/jumpers, IRQ enable and SDA/SCL activity. If the CPU remains in generated clock initialisation, investigate oscillator/PLL startup before I²C. Neither case has been observed on hardware yet.

## MCC regeneration

Keep `app_led.c`/`.h` user-owned. After regeneration, verify the two calls in `main.c` and the source/header project entries remain present; MCC's saved file hashes still describe the original skeleton, so review any proposed overwrite. Do not accept overwrite blindly. Re-run Linux and Windows builds after configuration changes.

## Observed Windows/board result

On 2026-10-13 the user supplied a clean Windows debug build log showing XC32 v6.00, DFP 1.7.380, `app_led.c`, `-D__DEBUG`, `-D__MPLAB_DEBUGGER_PK5=1`, successful linking/loading, and reported LED1 blinking as expected. The schematic identifies LED1 as **D5**. The provided commands did not enable `-Wall -Wextra`, explaining why those diagnostics were absent. The user intends to add these warning options.

Host commit `48faaa9` selects `PK5Tool` in the shared project configuration and has been pulled here. The corrected Linux target includes generated `exceptions.c`, `stdio/xc32_monitor.c` and `plib_i2c_smbus_common.c`, and uses `-ffunction-sections -fdata-sections` plus `--gc-sections` like the Windows build. It remains a production build; Windows debug reservations/options differ, so binary identity is not claimed.

### Required regeneration after H3 updates

On 2026-10-14 the user reported H3 content updates caused by adding the bootloader library. **After those updates settle, regenerate this application's own MCC configuration too.** Preserve the user-owned LED code, main calls, public project entries, PICkit 5 and compiler options; keep the standalone linker layout unchanged during this refresh. Record the actual new package versions, review API/configuration diffs, rebuild on Linux and Windows, and repeat production blink/MCLR/power-cycle tests. This regeneration is not yet performed; the existing hardware result applies to the tagged baseline only. See [the regeneration checklist](bootloader.md).

## Production-tested milestone

The user pushed host compiler -Wall/-Wextra settings in commit **dd59a291254b45d44fba65810b2085d9b6ea40df**, then confirmed that this version was production-built and programmed and that blinking survives both **MCLR** and a **power cycle**. Annotated tag **[v0.1.0-led-blink](https://github.com/jharoian-mchp/PIC32MX270-USB-Bootloader-Example/tree/v0.1.0-led-blink)** marks that exact tested commit. The tag was pushed and checked against the remote.

Measured blink period, target silicon revision and injected bus-fault recovery remain unverified. Next create the [separate USB HID bootloader project](bootloader.md), preserving this application baseline.
