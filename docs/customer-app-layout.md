# Customer LED application and 32 KB layout

The customer application has LED1 source integration and a dedicated PIC32MX270F256D linker script. Host mocks and source-layout checks pass. No target compile, link, HEX generation or hardware test has run.

## LED source

`customer-demo/application/src/app_led.c/.h` uses generated I2C1 and Timer2 APIs. LED1 is active-low MCP23008 GP0 at seven-bit address0x22. Startup reads OLAT/IODIR, sets only GP0 high (LED off), changes only GP0 to output, and verifies direction readback. All GP1–GP7 latch/direction bits, including USB routing/control pins, are preserved. The Timer2 callback increments a10ms tick only; main-loop tasks perform I2C and toggle after50ticks/500ms.

Buffers are static and retained through asynchronous transfer completion. NACK, rejected start, readback mismatch and100ms timeout produce terminal failure without abort/retry or uncertain buffer reuse. APP_LED_Initialize is a startup-only API; its trusted main caller invokes it once. Reinitialising while a transfer is retained is outside the API contract. These checks are host mocks, not electrical I2C or interrupt tests.

## Linker preparation

`src/config/default/app_32k.ld` is derived from the generated exact-device script; the original `p32MX270F256D.ld` is unchanged. MPLAB project metadata selects the new script and enables the vendor startup code, since no private startup source exists in this application.

| Region | Prepared address/span |
|---|---|
| Application entry/reset | `0x9D008000` |
| Reset/startup/BEV reservation | `0x9D008000..0x9D00848F` (0x490 bytes) |
| Ordinary program memory | origin `0x9D008490`, length `0x36B70`, ending before `0x9D03F000` |
| EBase/exception page | `0x9D03F000..0x9D03FFFF` (preserves generated small-MX EBase/vector scheme) |
| RAM | generated `0xA0000000`, length0x10000 |

BEV handler relocates to`0x9D008380`. The prepared script retains the generated NOLOAD debug reservations; debugger/executive behaviour is not qualified. Application configuration inputs `.config_*` and debug-exception inputs are discarded. The bootloader owns programmed fuses/Boot Flash; the application's generated USB PLL OFF setting is excluded from the delivered update configuration image. Final ELF/HEX checks must reject every file-backed Boot Flash/configuration byte and every program byte below physical`0x1D008000`.

Installed XC32 DFP CRT loads `_ebase_address` into CP0 EBase. This supports retaining the generated exception page for this MCU; PIC32MZ example vector offsets were not copied. Actual startup fit, orphan sections, reset code, constructors/data initialisation, ISR relocation and application jump need final linked disassembly/ELF/HEX review. The0x490 reservation is a candidate source layout; overflow must stop the build, never move the fixed32KB entry.

No custom updater metadata page is reserved: stock H3 demo application flash extends to physical`0x1D03FFFF`, subject to exact selected stock parser/write-range audit. The paused updater's last-page metadata contract is not imported.

## Reproducible checks and later gates

- `make customer-app-test`: strict host LED tests and explicit source-layout/project/peripheral checks.
- `make customer-board-test`: existing customer bootloader board mocks.
- `git diff --check`: whitespace verification.

MCC regeneration can overwrite main integration/project linker selection; preserve these reviewed edits. Keep generated clocks/peripherals/version records and the original linker source for comparison.

Next requires separate approval for offline target builds of both projects, then linked-image bounds/configuration/reset/vector/startup checks and disposition of stock host/parser/handoff concerns. This source milestone includes no program, USB command, reset, debugger session or NVM action.
