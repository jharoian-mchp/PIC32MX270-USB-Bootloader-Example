# Board review and first-application checklist

The first firmware milestone is an MCC-generated application that blinks LED1 through the PIM's MCP23008 expander. USB stays disabled for this milestone. The MCC skeleton and user-owned LED code now build on Linux, with mocked logic tests passing. Windows programming and hardware tests have not run.

Reviewed: **2026-10-13**. Pin mappings below are documented values, not measurements of the installed board. Confirm the board revision and inspect its schematics before programming.

## Reference findings

Sources:

- [PIM information sheet](docling/PIC32MX270F256D-Bluetooth-Audio-PIM.md): Table 1, lines 15–72; Figure 3 and LED/control mapping, lines 81–94.
- [Carrier-board guide](docling/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide.md): kit identity and PIM selection, lines 393–403; features, lines 428–458; pin table, lines 1100–1215; bill of materials, line 1425; reset-selector instructions, lines 1512 and 1526.
- [Device data sheet](docling/PIC32MX1XX2XX283644-PIN_Datasheet_DS60001168L.md): USB family feature table near the start; Flash Programming chapter, line 2594; configuration word 2, lines 12146–12155.
- [Oscillator family reference](docling/FRM/FRM-06-Oscillators.md): FRCPLL operation, line 738; USB clock generation, lines 796–820. Device-specific configuration takes precedence over family-level descriptions.
- [Device errata](docling/PIC32MX1XX-2XX-28-36-44-pin-Family-Silicon-Errata-DS80000531.md): Table 3, lines 79–88; oscillator, I²C and USB issues, lines 350–408.

### Device and board

| Item | Documented value | Remaining check |
|---|---|---|
| MCU | PIC32MX270F256D-50I/PT, confirmed by the user | Read device/revision ID with the programmer. |
| Carrier | DM320032; complete kit DV320032 | Confirm installed board revision. |
| Memory | 256 KB program Flash, 3 KB boot Flash, 64 KB RAM | Verify selected DFP and linker memory regions against the exact device. |
| Flash geometry | 1,024-byte erase page; 128-byte program row | Use these device-specific sizes, not geometry copied from a larger PIC32MX example. |
| Documented device ID | 0x0660A053; errata lists A1/A2 revision IDs 0x1/0x2 | Record the actual silicon revision; do not treat the device ID as the full unmasked DEVID register value. |
| Reset selection | Guide requires PIM_MCLR when using a PIM | User confirmed selector set to PIM_MCLR on 2026-10-13; reset/debug operation not tested. |

### MCU-to-board pin mapping

These are **44-pin MCU numbers** and **100-pin PIM connector numbers**; they are not interchangeable.

| Function | MCU signal | MCU pin | PIM pin | Intended use |
|---|---|---:|---|---|
| I²C1 SDA | RB9 / SDA1 | 1 | 59, 67 | MCP23008 communication. |
| I²C1 SCL | RB8 / SCL1 | 44 | 58, 66 | MCP23008 communication. |
| SW1 | RB2 | 23 | 17 | Proposed force-bootloader button; confirm active level and pull-up. |
| SW2 | RB3 | 24 | 38 | Available documented switch input. |
| SW3 | RC5 | 38 | 29 | Available documented switch input. |
| SW4 | RC9 | 5 | 41 | Available documented switch input. |
| SW5 | RB5 / USBID | 41 | 42 | Shares USBID; do not assume it can serve both functions simultaneously. |
| USB D+ | RB10 / D+ | 8 | 57 | Dedicated USB data path; also shares PGED2. |
| USB D− | RB11 / D− | 9 | 56 | Dedicated USB data path; also shares PGEC2. |
| Board VBUS sense | RA1 | 20 | 80 | Distinct from the dedicated VBUS pin. |
| Dedicated VBUS | VBUS | 42 | 54 | PIM table labels this 3.3V/VBUS; verify the actual circuit before USB configuration. |
| Primary oscillator | OSC1 / RA2 | 30 | 63 | External crystal input; installed Y1 marked 12.00 MHz, confirmed by user. |
| Primary oscillator | OSC2 / RA3 | 31 | 64 | External crystal output. |
| Reset | MCLR | 18 | 13 | Selected PIM reset path. |
| PIM programming data | RA10 / PGED, labelled PGD4 | 12 | 27 | Trace to the ICSP connector and confirm the DFP debugger setting. |
| PIM programming clock | RA7 / PGEC, labelled PGC4 | 13 | 26 | Trace to the ICSP connector and confirm the DFP debugger setting. |

SW6 is not assigned to an MCU GPIO in the PIM information sheet's Table 1. Do not copy the carrier's soldered-down MCU assignment into this PIM configuration.

### MCP23008 and LED control

| Expander signal | Expander pin | PIM pin | Function |
|---|---:|---:|---|
| GP0 | 12 | 60 | LED1 — proposed first blink output. |
| GP1 | 13 | 61 | LED2. |
| GP2 | 14 | 91 | LED3. |
| GP3 | 15 | 92 | LED4. |
| GP4 | 16 | 28 | LED5. |
| GP5 | 17 | 89 | USB_SEL/OE#. |
| GP6 | 18 | 90 | USB_SEL/S. |
| GP7 | 19 | 84 | GPIO_32. |

The carrier's Figure A-8 (source PDF page 56) confirms the LED1/D5 active-low sink path, consistent with its `LED1#` label. User-owned code preloads GP0 high (OFF) before making it an output; runtime LED behaviour is not yet tested.

The PIM documentation states that the expander address is changeable. Its address-strap circuit is in [Figure 3](docling/images/PIC32MX270F256D-Bluetooth-Audio-PIM/image_000003_e49099fffd484b4bfb1282e7ed87ff9ecf4bf0919d9c13f6277f650855be46c5.png). On 2026-10-13, the user confirmed **J1 pins 1–2, selecting seven-bit address 0x22**. Pass **0x22** to an I²C API that accepts seven-bit addresses; the corresponding on-wire address bytes are 0x44 for write and 0x45 for read. Do not shift the address twice. The [MCP23008 component data sheet](docling/MCP23008-MCP23S08-Data-Sheet-DS21919E.md) is now included. Its register definitions confirm IODIR 0x00 (1=input, 0=output) and OLAT 0x0A (output latch). The application reads/preserves GP1–GP7 before changing GP0.

The carrier's **Figure A-5 (source PDF page 53)** shows **R10 and R11, each 3.30 kΩ**, pulling up the I²C1 SDA/SCL nets through **J5 (I2C1 PWR Conf)**. **J5 pins 1–2 select +3.3V_DIG; pins 2–3 select +5V_SW_DIG.** Both pull-ups use the common centre pin. J6 is the separate I²C2 selector. See the [high-resolution schematic crop](images/i2c1-pullups-detail.png), rendered from the original PDF to verify small schematic labels.

The PIM Figure 2/3 schematics route SDA/SCL to the MCP23008 without showing dedicated bus pull-ups; the resistors around its address pins are address-selection circuitry. Use the carrier pull-ups and set **J5 to pins 1–2 (3.3 V)** for this bring-up. The carrier bill of materials lists J5/J6 shunts at +5V, so do not assume their as-shipped positions are suitable. On 2026-10-13, the user confirmed **J5 is fitted on pins 1–2 (3.3 V)**. Resistor population and bus idle voltage/rise time have not been measured. Do not add another pull-up pair unless the existing circuitry or measured rise time requires it.

For the first application, initialise only the required LED output, preload its verified OFF latch state before enabling output, and avoid whole-port writes that unintentionally change GP5–GP7. Preserve the reviewed reset/default behaviour of unused control lines.

### Clock discrepancy

The carrier guide contains conflicting values:

- Pin table, lines 1174–1175: **XTAL (8 MHz)** on OSC1/OSC2.
- Bill of materials, line 1425: **Y1, 12 MHz**, TXC part `7A-12.000MAAJ-T`.

On 2026-10-13, the user confirmed that installed **Y1 is marked 12.00 MHz**. Use 12 MHz as the primary-source design input for this board. Board revision and clock routing still need confirmation; the marking is not a measured oscillator frequency.

The **proposed first-blink configuration** is primary oscillator with PLL (`PRIPLL`) at **48 MHz SYSCLK**: `12 / 3 × 24 / 2 = 48 MHz`. Verify MCC-generated configuration, flash wait states and peripheral clock before programming. USB remains disabled in the initial application. Nominal 48 MHz FRCPLL is a fallback for primary-clock fault isolation. See [the toolchain and clock plan](toolchain.md).

For USB, the primary source must provide a suitable accurate clock and the USB PLL requires a **4 MHz input** to produce its internally divided **48 MHz USB clock**:

| Installed primary source, if confirmed | Candidate UPLL input divider | Resulting UPLL input |
|---|---|---|
| 8 MHz | ÷2 | 4 MHz |
| 12 MHz | ÷3 | 4 MHz |

For the confirmed 12 MHz marking, select a **÷3 USB PLL input divider** in the proposed MCC configuration. This is a calculated setting, not a hardware-verified clock. Use the DFP/MCC symbolic USB-PLL-enable setting. The device data sheet states that the raw UPLLEN bit enables the PLL when **0**, while the family chapter's prose says to set it; do not hand-copy a raw bit value from the generic description.

### USB routing and power

The carrier has a 3:1 USB multiplexer selecting Type-A, mini-B or an off-board path. The guide describes the mini-B path for a USB-host source and the Type-A path for a USB-device source. The PC-to-board HID design will use the reviewed device-side path, expected to be mini-B.

Before USB bring-up:

- Trace the MCU D+/D− path through the multiplexer to the mini-B connector for the installed revision.
- Resolve GP5/GP6 enable/select truth table and any additional charge-management selection signals. The guide and PIM use different signal labels; do not invent an output-port byte.
- Verify VBUS sensing, the dedicated VBUS pin connection, board power selection and protection against back-feeding the PC. Do not enable host VBUS sourcing for the HID device.
- Confirm the ICSP/debug path does not contend with USB D+/D− on PGEC2/PGED2.

Review the carrier's USB/power schematic figures in the [converted guide](docling/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide.md), then compare unresolved details with the source PDF and actual board.

### Silicon errata

For the 256 KB Flash / 64 KB RAM devices, the supplied errata marks both A1 and A2 for the primary-oscillator/FSCM issue, the reserved-address I²C client issue and USB idle handling.

- A primary-clock failure can leave the CPU on FRC despite later software clock-switch requests; the supplied errata gives no workaround. Review the clock-failure recovery policy before USB operation.
- The I²C issue concerns client mode at reserved address 0x78. The LED application will use I²C master mode; this issue does not justify an arbitrary master-mode workaround.
- USB UIDLE may not reassert if cleared while the bus stays idle. Review the selected Harmony USB driver's suspend/resume handling against the documented workaround. Do not claim that a package version handles it without checking.

## Development tools

The workspace runs Linux x86_64. On 2026-10-13, PATH checks did not find `xc32-gcc`, `xc32-bin2hex`, `mcc`, `java`, `pic32prog` or `ipecmd`; a limited search of `/opt`, `/usr/local` and the project directory found no XC32/MCC/Harmony installations. This does not inspect the user's development computer.

After the initial review, **XC32 v6.00 and PIC32MX_DFP 1.7.380 were installed** under `/workspace/ChrisK/tools/`. An exact-device compile/link/HEX smoke test passed; see [toolchain details](toolchain.md). Java and MCC are not installed here: generation will run on the user's Windows host with MCC v5.7.0. Host programming/debugging will use MPLAB X v6.35 and PICkit 5. The saved MCC project and generated PLIBs are now present under `application/`; user-owned LED1 logic and the complete firmware build passed Linux checks. Component metadata records CSP v3.26.0. Hardware tests are next.

The current upstream `pic32mx_125_sk` HID example is **not this device**: its project XML selects PIC32MX570F512L, XC32 4.60 and PIC32MX_DFP 1.6.369. Those versions are reference metadata, not a verified toolchain for PIC32MX270F256D. Record the selected Harmony package versions and commit/tag once compatibility is checked.

Reference inspected: [upstream project configuration](https://github.com/Microchip-MPLAB-Harmony/bootloader_apps_usb/blob/master/apps/usb_device_hid_bootloader/bootloader/firmware/pic32mx_125_sk.X/nbproject/configurations.xml), retrieved 2026-10-13. The URL tracks upstream `master` and can change.

## First application: MCC checklist

Complete the tool and electrical checks before applying these proposed settings. Use the actual standalone MCC workflow; do not hand-author a configuration file and label it MCC-generated.

1. Select **PIC32MX270F256D**. Keep generator DFP 1.6.369 and build DFP 1.7.380 as confirmed by the user; record the remaining Harmony package versions and generated project/build format.
2. Create the initial application under `application/`, using the normal device startup and linker layout, with no bootloader offset.
3. Configure the reviewed clock source. With the confirmed 12 MHz Y1 marking, propose PRIPLL with input ÷3, multiplier ×24 and output ÷2 for 48 MHz SYSCLK; USB remains disabled. Record SYSCLK and PBCLK explicitly.
4. Configure **I²C1 master**, initially targeting **100 kHz**; confirm the generated SDA/SCL mapping is RB9/RB8. Enable only the peripherals required for the first application.
5. Proposed timebase: **TMR2**, internal PBCLK, 16-bit periodic mode, **1:64 prescaler**, **10 ms** interrupt period. At 48 MHz PBCLK, 7,500 counts require **PR2 = 7,499**. Count 50 events in application logic to request an LED state change every **500 ms**. Keep I²C transactions in the application state machine, not a timer ISR. This gives an approximately one-second complete blink cycle; verify the timer screen and generated code before use.
6. Use the confirmed expander address and component register definitions. Initialise LED1 safely, retain the other control lines' reviewed states and handle I²C NACK/timeouts without an infinite blocking loop.
7. Disable the watchdog for the first bring-up unless a deliberate servicing policy is implemented. Configure analog/digital modes and the debugger channel for the actual pin use; check shared functions before disabling JTAG or other facilities.
8. Generate code with MCC, retain the generator configuration and version records, then build with XC32 through the supported standalone build flow. Review warnings, the map file, memory use and configuration words.
9. Program through ICSP with the board set for the PIM. Record the MCU revision, successful LED1 blink, repeated reset behaviour and any I²C fault observations.
10. Only after the standalone application passes, create the HID bootloader project and define the flash/linker contract from the generated build.

### Acceptance record

| Check | Expected result | Current result |
|---|---|---|
| Generated configuration | Correct device and recorded package/tool versions | Generated and reviewed; CSP v3.26.0, exact MCU and ICS_PGx4 confirmed. |
| XC32 build | Successful generated-application build; warnings reviewed | Linux complete application passed; user code warning-free under -Werror. Two unused-parameter warnings in generated SYS_Initialize and I²C transfer setup. Windows build not yet tested. |
| Board reset/debug path | Programmer identifies the target PIM | Not tested. |
| LED control | LED1 changes state about every 500 ms | Not tested. |
| Unused expander signals | No unintended USB/control-line changes | Not tested. |
| Reset recovery | Application restarts consistently | Not tested. |

## Confirmed setup and remaining inputs

Confirmed by the user on 2026-10-13:

- Windows host; MCC generation on the host and XC32 builds also available in this Linux workspace.
- XC32 v6.00 and standalone MCC v5.7.0.
- MPLAB X v6.35, also including MCC v5.7.0, for the host IDE.
- PICkit 5 programmer/debugger.
- Installed Y1 marked 12.00 MHz.
- Carrier J5 on pins 1–2, selecting 3.3 V I²C1 pull-ups.
- PIM J1 on pins 1–2, selecting seven-bit MCP23008 address 0x22.
- Timer2 interrupt enable checked in MCC. Runtime interrupts are not yet tested.
- Reset selector set to PIM_MCLR.
- Generated configuration confirms FWDTEN/JTAGEN/PWP/BWP/CP OFF and ICS_PGx4; user also confirmed ICESEL.
- User generated with no MCC notifications and pushed the saved project. Linux build and mocked LED tests passed.

Still needed:

- Carrier-board/PIM revisions.
- Windows build/programming result and board blink/reset test. Generator DFP 1.6.369, build DFP 1.7.380 and CSP v3.26.0 are recorded; generated clock configuration is reviewed, but hardware clocks are not measured.
- Remaining bus/clock/USB electrical checks; expander register definitions and LED1 polarity have been reviewed.

Follow [the application build and board-test instructions](application.md) next. Do not start the USB bootloader until the standalone board application passes.
