# PIC32MX270 USB HID Bootloader Example

This project will build a USB HID bootloader and a simple application for the **PIC32MX270F256D-50I/PT**, using its Bluetooth Audio PIM on the PIC32 Bluetooth Audio Development Board.

The selected toolchain is the **XC32 compiler** and **standalone MPLAB Code Configurator (MCC)** with **MPLAB Harmony 3**. Exact tool and package versions, and their support for this device, still need verification. Firmware has not been generated, compiled or tested on hardware.

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

The first reference review is recorded in [Board review and first-application checklist](docs/bring-up.md). It identifies an unresolved **8 MHz versus 12 MHz crystal discrepancy** in the carrier guide and documents MCU/PIM pin mappings, flash geometry and applicable errata. Nominal **48 MHz FRCPLL with USB disabled** is proposed for the first blink application; external-crystal and USB settings await hardware confirmation.

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

All **27 source PDFs** have been converted to Markdown, with **650 separate image files** under `docs/docling/images/`. The Markdown preserves the `FRM/` structure and uses relative image links; no image data is embedded. The [conversion manifest](docs/docling/manifest.json) records source SHA-256 hashes, output paths and counts.

The original PDFs in `docs/pdf/` are unchanged. Compare ambiguous OCR, register tables, pin assignments, schematics and other hardware-critical details against those originals. Automated conversion checks do not establish hardware correctness.

## Repository layout

```text
Readme.md              Approach, progress and next steps
AGENTS.md              Instructions for work in this repository
docs/
  bring-up.md          Board findings, open checks and first-application checklist
  pdf/                 Original reference PDFs, including FRM/
  docling/             Converted Markdown, index and manifest
    FRM/               Converted family-reference chapters
    images/            Separate figures grouped by document
```

Separate `bootloader/` and `application/` directories are planned once we confirm the MCC project workflow. They do not exist yet.

## Progress

| Milestone | Status | Evidence or remaining work |
|---|---|---|
| Confirm MCU and update transport | Complete | PIC32MX270F256D-50I/PT; PC-to-board USB HID agreed. |
| Clone repository into the ChrisK workspace | Complete | Working copy: `/workspace/ChrisK/PIC32MX270-USB-Bootloader-Example`. |
| Convert and validate reference documents | Complete | 27 documents, 650 verified image links, no embedded images; source hashes checked. |
| Commit and push documentation | Complete | [Commit ce31f3e](https://github.com/jharoian-mchp/PIC32MX270-USB-Bootloader-Example/commit/ce31f3ed782e55f3993fe9f26f9ff41e315b3ce8) on `main`. |
| Establish GitHub push access | Complete | Repository-specific deploy key authenticated; SSH push verified over port 443. |
| Record approach and progress | Complete | This file; maintenance instruction in `AGENTS.md`. |
| Verify hardware configuration and tool versions | In progress | [Initial reference review](docs/bring-up.md) completed; clock discrepancy identified. Board revision, programmer, development OS and compatible package versions needed. |
| Generate, build and test standalone application | Not started | No firmware or hardware test results yet. |
| Generate and enumerate HID bootloader | Not started | Device-specific configuration and USB hardware test needed. |
| Define and test memory layout and application handoff | Not started | Bootloader size and linker contract not established. |
| Verify USB updates and recovery | Not started | Requires bootloader, relocated application and compatible host utility. |

### Next steps

- Choose the generation/build location and record the development OS, XC32 and standalone MCC versions. XC32, MCC and Java were not found in the workspace's initial tool checks.
- Record the programmer/debugger model and carrier-board/PIM revisions; confirm the installed Y1 marking to resolve the 8 MHz/12 MHz discrepancy.
- Complete the [hardware checks](docs/bring-up.md), including LED polarity, expander address/register definitions, I²C pull-ups and USB routing/power states.
- Verify the standalone MCC/Harmony workflow for this exact device, then generate the initial I²C1/LED1 blink application.

### Progress log

- **2026-10-13:** Agreed on USB HID, XC32, standalone MCC and an initial blink application; confirmed the full MCU part number and PIM availability.
- **2026-10-13:** Cloned the reference repository, converted all 27 PDFs through Docling and verified source hashes and all 650 external image links.
- **2026-10-13:** Committed and pushed the converted documentation and repository instructions. Verified repository-specific SSH access over port 443.
- **2026-10-13:** Added this approach and progress record. Firmware generation and hardware testing have not started.
- **2026-10-13:** Began hardware/toolchain review. Documented PIM pin mappings, LED-expander requirements, 1 KB flash pages/128-byte rows and relevant silicon errata. Identified conflicting carrier-crystal values and missing workspace development tools; prepared the first MCC application checklist.
