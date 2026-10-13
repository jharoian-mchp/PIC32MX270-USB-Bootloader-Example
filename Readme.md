# PIC32MX270 USB HID Bootloader Example

This project will build a USB HID bootloader and a simple application for the **PIC32MX270F256D-50I/PT**, using its Bluetooth Audio PIM on the PIC32 Bluetooth Audio Development Board.

Use **XC32 v6.00** on the Windows host and in this Linux workspace. Generate firmware on Windows with **standalone MCC v5.7.0** and Harmony 3; use **MPLAB X v6.35 with PICkit 5** for host programming/debugging. The IDE also includes MCC v5.7.0. Linux XC32 and **PIC32MX_DFP 1.7.380** passed an exact-device compiler/linker/HEX smoke test. MCC/Harmony uses generator **DFP 1.6.369**; MPLAB X uses build **DFP 1.7.380**, as confirmed by the user. These versions are recorded independently. The initial MCC application is generated; saved components record **CSP v3.26.0**. User-owned LED1 code now builds with XC32 here and passes mocked tests. The user confirmed a successful Windows PICkit 5 debug build/load and LED1 blinking. Production/reset/cold-start tests are still needed; the USB bootloader has not been generated.

Last updated: **2026-10-13**. Update this file when project decisions, setup requirements or verified progress change.

## Recommended approach

Use two separate firmware projects:

| Component | Purpose |
|---|---|
| USB HID bootloader | Enumerate as a USB device, receive application firmware from a PC, write application flash and start a valid application. |
| Simple application | Blink a board LED to demonstrate execution before and after a USB firmware update. |
| PC host utility | Transfer application firmware using a protocol compatible with the selected bootloader. |
| ICSP programmer/debugger | Install the bootloader initially and recover the board if USB updating fails. |

Start with Microchip's [Harmony USB HID bootloader examples](https://github.com/Microchip-MPLAB-Harmony/bootloader_apps_usb/tree/master/apps/usb_device_hid_bootloader) as a reference. Adapt the configuration to this MCU and board; example-board pins, clocks and linker layouts must be checked before reuse. Confirm the host utility and accepted firmware format against the chosen bootloader version.

USB HID is the agreed transport: a PC connects directly to the board. USB-flash-drive updates are outside the initial scope. Bluetooth and audio functionality will wait until the basic application and firmware-update path work.

### Bring-up sequence

1. **Verify hardware and tool support.** Review the device, PIM and carrier-board references. Confirm the installed board revision, oscillator, USB routing and power circuitry, button inputs, LED control and ICSP connections. Record the development OS, programmer/debugger and compatible XC32, MCC, device-pack and Harmony versions.
2. **Generate a standalone blink application.** Configure clocks and board peripherals in MCC, build with XC32 and program through ICSP at the normal reset location. Verify visible blinking and repeatable reset behaviour.
3. **Bring up the USB HID bootloader.** Adapt the Harmony reference, configure the required 48 MHz USB clock, choose a button to force bootloader entry and stay in bootloader mode when no valid application exists. Verify USB enumeration before testing flash writes.
4. **Define the bootloader/application memory contract.** Measure the generated bootloader and confirm the device flash erase/program geometry. Reserve bootloader flash and define the application start address, reset entry, exception/vector placement, configuration-bit ownership and valid-image criteria. Restrict updates to the application region.
5. **Build and update the relocated application.** Link the blink application for the agreed memory layout, transfer it with the compatible HID host utility and verify that the bootloader starts it correctly.
6. **Exercise recovery.** Test a missing application, forced bootloader entry, repeated updates and an interrupted update. Record actual results and any limitations before adding features.

The application start address and bootloader reservation are **not chosen yet**. USB enumeration, flash writes and application handoff all require hardware verification.

### Board-specific consideration

The [PIM information sheet](docs/docling/PIC32MX270F256D-Bluetooth-Audio-PIM.md) states that the PIM's **MCP23008 I²C expander controls the board LEDs and charge-management control lines**. A board-LED blink application therefore needs expander setup and the correct I²C mapping; it cannot assume a directly connected GPIO LED. Verify the expander address, LED polarity and safe control-line states against the schematics before coding.

The [Board review and first-application checklist](docs/bring-up.md) records MCU/PIM pin mappings, flash geometry and applicable errata. The user confirmed installed **Y1 is marked 12.00 MHz**, resolving the guide's conflicting crystal values for this board. Proposed clock settings are **12 MHz PRIPLL, input ÷3, multiplier ×24, output ÷2 = 48 MHz SYSCLK**. USB also needs a **÷3 UPLL input divider**; keep the USB peripheral disabled in the first blink application. These settings are confirmed in generated code; hardware clock verification is still needed.

## Documentation

Use **`docs/docling/` first** for device, PIM and carrier-board references. Start with its [document index](docs/docling/README.md).

| Reference | Use |
|---|---|
| [Device data sheet](docs/docling/PIC32MX1XX2XX283644-PIN_Datasheet_DS60001168L.md) | Device features, memory, pins, configuration and electrical requirements. |
| [Device errata](docs/docling/PIC32MX1XX-2XX-28-36-44-pin-Family-Silicon-Errata-DS80000531.md) | Silicon limitations and workarounds to review against the actual device revision. |
| [PIM information sheet](docs/docling/PIC32MX270F256D-Bluetooth-Audio-PIM.md) | Device-to-carrier pin mapping and PIM schematics. |
| [Development-board reference guide](docs/docling/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide.md) | Carrier-board connections, switches, power and schematics. |
| [USB family reference](docs/docling/FRM/FRM-27-USB.md) | USB module operation. |
| [Oscillator family reference](docs/docling/FRM/FRM-06-Oscillators.md) | Clock sources and PLL configuration. |
| [Flash-programming family reference](docs/docling/FRM/FRM-05-Flash-Programming.md) | Flash erase/program operations. |
| [Programming specification](docs/docling/PIC32-Flash-Programming-Specification-DS60001145.md) | Device programming details. |

All **28 reference PDFs**, including the added [MCP23008 component data sheet](docs/docling/MCP23008-MCP23S08-Data-Sheet-DS21919E.md), have been converted to Markdown, with **675 separate image files** under `docs/docling/images/`. The Markdown preserves the `FRM/` structure and uses relative image links; no image data is embedded. The [conversion manifest](docs/docling/manifest.json) records source SHA-256 hashes, output paths and counts.

The original PDFs in `docs/pdf/` are unchanged. Compare ambiguous OCR, register tables, pin assignments, schematics and other hardware-critical details against those originals. Automated conversion checks do not establish hardware correctness.

## Toolchain check

The Linux compiler and device pack live outside the repository under `/workspace/ChrisK/tools/`. Run **`make toolchain-check`** to compile/link a test for the exact MCU and generate HEX. This test is not MCC-generated and must not be programmed onto the board. See [Windows generation and Linux builds](docs/toolchain.md) for installation versions, hashes, clock settings and the host-project handoff.

## Application build and test

Run **`make test`** for the mocked LED state-machine tests and **`make application`** for the XC32 firmware build. Open **`application/app.X`** on Windows, pull the changes and Clean/Build before programming with PICkit 5. Expected behaviour: **LED1 (D5) changes state about every 500 ms**. See [LED1 application: build and board test](docs/application.md) for safety, diagnostics, warnings and regeneration notes. LED1 blink was confirmed by the user in a Windows debug session; production/reset/cold-start behaviour has not been verified.

## Repository layout

```text
Readme.md              Approach, progress and next steps
AGENTS.md              Instructions for work in this repository
Makefile               XC32 application build, mocked tests and compiler smoke test
application/
  app.X/               MPLAB X project and saved MCC configuration
  src/                 Generated PLIBs, main and user-owned LED application
tests/                 Mocked application tests
tools/
  toolchain-smoke.c    Exact-device compile/link test; not application firmware
docs/
  application.md       LED logic, build instructions and first hardware test
  toolchain.md         Windows/Linux tools, clock plan and generation handoff
  bring-up.md          Board findings, open checks and first-application checklist
  pdf/                 Original reference PDFs, including FRM/
  docling/             Converted Markdown, index and manifest
    FRM/               Converted family-reference chapters
    images/            Separate figures grouped by document
```

The application project is present. A separate `bootloader/` directory will follow the first passing board test.

## Progress

| Milestone | Status | Evidence or remaining work |
|---|---|---|
| Confirm MCU and update transport | Complete | PIC32MX270F256D-50I/PT; PC-to-board USB HID agreed. |
| Clone repository into the ChrisK workspace | Complete | Working copy: `/workspace/ChrisK/PIC32MX270-USB-Bootloader-Example`. |
| Convert and validate reference documents | Complete | 28 documents and 675 referenced images after adding the MCP23008 data sheet; source hashes and links checked. |
| Commit and push documentation | Complete | [Commit ce31f3e](https://github.com/jharoian-mchp/PIC32MX270-USB-Bootloader-Example/commit/ce31f3ed782e55f3993fe9f26f9ff41e315b3ce8) on `main`. |
| Establish GitHub push access | Complete | Repository-specific deploy key authenticated; SSH push verified over port 443. |
| Record approach and progress | Complete | This file; maintenance instruction in `AGENTS.md`. |
| Confirm host tools, programmer and crystal | Complete | Windows, XC32 v6.00, MCC v5.7.0, MPLAB X v6.35, PICkit 5; Y1 marked 12.00 MHz. |
| Install and verify Linux compiler/device pack | Complete | XC32 v6.00 + PIC32MX_DFP 1.7.380; `make toolchain-check` passes compile, link and HEX generation. |
| Review MCC device-pack split and clock configuration | Generated-code review complete | Generator DFP 1.6.369 / build DFP 1.7.380; CSP v3.26.0; generated PRIPLL/48 MHz settings confirmed. Hardware clocks not measured. |
| Verify remaining hardware configuration | In progress | Board/PIM revision and bus/clock measurements still needed; expander register definitions and LED active-low schematic reviewed. |
| Review I²C1 configuration and pin assignment | Screen review complete | I²C1 master at 100 kHz; SDA1 RB9 and SCL1 RB8. Generated code and hardware transfers not verified. |
| Review Timer2 timebase and I²C pull-up schematic | Screen/schematic review complete | TMR2 internal clock, 1:64, 10 ms, PR2 7,499 shown. Carrier R10/R11 are 3.3 kΩ pull-ups via J5; select pins 1–2 for 3.3 V. User confirmed Timer2 interrupt enable, J5 pins 1–2 (3.3 V) and PIM J1 pins 1–2 (seven-bit address 0x22). Runtime interrupts and bus communication not tested. |
| Review reset and development configuration bits | User/screen review complete | PIM_MCLR confirmed; FWDTEN/JTAGEN/PWP/BWP/CP OFF shown. User and generated code confirm ICS_PGx4. Debug/programming not tested. |
| Generate MCC skeleton and add LED1 application | Complete | User generated with no notifications; `app_led.c` uses address 0x22 and preserves GP1–GP7. Generated PLIBs unchanged. |
| Linux build and application logic tests | Passed | XC32 compile/link/HEX, strict user-code compile, mocked tests and sanitizer run passed. Expanded Linux build includes all 12 project C sources; -Wall/-Wextra reports eight unused-parameter warnings in generated support code. |
| Windows debug build/load and board blink | Passed, user-reported | Clean debug build/load with XC32 v6.00, DFP 1.7.380 and PICkit 5; LED1/D5 blinks as expected. Shared project selects PK5Tool (48faaa9). |
| Production, reset and cold-start recovery | Not tested | Check standalone operation after debugger disconnect and after reset/power cycle. |
| Generate and enumerate HID bootloader | Not started | Device-specific configuration and USB hardware test needed. |
| Define and test memory layout and application handoff | Not started | Bootloader size and linker contract not established. |
| Verify USB updates and recovery | Not started | Requires bootloader, relocated application and compatible host utility. |

### Next steps

- Add `-Wall -Wextra` to host compiler additional options as agreed; generated support code may report unused parameters. They are diagnostic differences, not a failed build.
- Check production/Run operation after disconnecting the debugger, then reset and cold power-cycle recovery. Report whether LED1 resumes blinking.
- Prepare the separate USB HID bootloader project after these checks; preserve the working application baseline.
- If needed, inspect `app_led_diagnostics` for readiness, NACK/collision/timeout faults and transition count.
- Record carrier-board/PIM revisions and remaining hardware measurements; review USB routing/power before bootloader generation.
- Keep generator DFP 1.6.369 and build DFP 1.7.380 separate, and preserve the user-owned LED code during MCC regeneration.

### Progress log

- **2026-10-13:** Agreed on USB HID, XC32, standalone MCC and an initial blink application; confirmed the full MCU part number and PIM availability.
- **2026-10-13:** Cloned the reference repository, converted all 27 PDFs through Docling and verified source hashes and all 650 external image links.
- **2026-10-13:** Committed and pushed the converted documentation and repository instructions. Verified repository-specific SSH access over port 443.
- **2026-10-13:** Added this approach and progress record. Firmware generation and hardware testing have not started.
- **2026-10-13:** Began hardware/toolchain review. Documented PIM pin mappings, LED-expander requirements, 1 KB flash pages/128-byte rows and relevant silicon errata. Identified conflicting carrier-crystal values and missing workspace development tools; prepared the first MCC application checklist.
- **2026-10-13:** Confirmed Windows host generation with MCC v5.7.0, XC32 v6.00, MPLAB X v6.35 and PICkit 5. User read 12.00 MHz on installed Y1.
- **2026-10-13:** Installed Linux XC32 v6.00 and PIC32MX_DFP 1.7.380 outside the repository. Exact-device compile/link/HEX generation passed; added the reproducible `make toolchain-check` target and Windows/Linux setup notes.
- **2026-10-13:** User confirmed MCC/Harmony generator DFP 1.6.369 and MPLAB X build DFP 1.7.380. Reviewed clock screenshots showing the planned 12 MHz PRIPLL path and 48 MHz SYSCLK/PBCLK; I²C1 configuration is next.
- **2026-10-13:** Reviewed I²C1 master/100 kHz and SDA1 RB9/SCL1 RB8 pin screenshots. Proposed a Timer2 10 ms timebase, prescaler 1:64 and PR2 7,499, with application logic counting 50 events per LED transition. Firmware generation and hardware tests have not run.
- **2026-10-13:** Reviewed the matching Timer2 screen. Verified carrier I²C1 R10/R11 3.30 kΩ pull-ups and J5 voltage-selection connections against original PDF page 53; recommend J5 pins 1–2 for 3.3 V. Added a readable schematic crop. Hardware population/jumper state and timer interrupt were unverified at that review.
- **2026-10-13:** User confirmed carrier J5 pins 1–2 (3.3 V), PIM J1 pins 1–2 (MCP23008 seven-bit address 0x22), and Timer2 interrupt enable checked. Watchdog/debugger configuration review is next; no generated firmware or runtime tests yet.
- **2026-10-13:** User confirmed PIM_MCLR reset selection. Configuration screenshot shows watchdog, JTAG and code/write protection disabled. ICESEL requires full-value verification in generated code; MCC skeleton generation/handoff is next.
- **2026-10-13:** User confirmed ICS_PGx4, generated with no notifications and pushed the initial MCC project. Pulled and reviewed the generated settings; CSP v3.26.0 recorded.
- **2026-10-13:** Added user-owned LED1 state-machine logic, bounded I²C retry/timeout handling and debugger diagnostics. Linux application build, strict user-code compile, mocked tests and sanitizer run passed; Windows/hardware tests await the user. Added and converted the MCP23008 data sheet; reference inventory is now 28 PDFs and 675 images.
- **2026-10-13:** User confirmed LED1/D5 blinking and supplied a successful Windows PICkit 5 debug build/load log without extra warning flags. Pulled host PK5Tool configuration commit 48faaa9. Aligned the Linux source list with all 12 Windows project sources and added section garbage collection; full Linux build/tests pass. Production/reset/cold-start recovery is not yet reported.
