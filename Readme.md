# PIC32MX270 USB HID Bootloader Example

This project will build a USB HID bootloader and a simple application for the **PIC32MX270F256D-50I/PT**, using its Bluetooth Audio PIM on the PIC32 Bluetooth Audio Development Board.

Use **XC32 v6.00** on the Windows host and in this Linux workspace. Generate firmware on Windows with **standalone MCC v5.7.0** and Harmony 3; use **MPLAB X v6.35 with PICkit 5** for host programming/debugging. The IDE also includes MCC v5.7.0. Linux XC32 and **PIC32MX_DFP 1.7.380** passed an exact-device compiler/linker/HEX smoke test. MCC/Harmony uses generator **DFP 1.6.369**; MPLAB X uses build **DFP 1.7.380**, as confirmed by the user. These versions are recorded independently. The initial MCC application is generated; saved components record **CSP v3.26.0**. User-owned LED1 code now builds with XC32 here and passes mocked tests. The user confirmed a successful Windows PICkit 5 debug build/load and LED1 blinking. The user also confirmed production operation and blinking after MCLR and power-cycle resets. The exact tested commit is tagged **v0.1.0-led-blink**. A tested **enumeration-only diagnostic build** now adds AN1 sensing and safe mini-B mux setup; the first connected-state ADC/mux and Windows HID enumeration test is confirmed. ADC unplug/replug detection and detach/reattach are now confirmed; host re-enumeration after MCLR and power cycle is now user-confirmed. Firmware-update commands stay inactive.

Last updated: **2026-10-15**. Update this file when project decisions, setup requirements or verified progress change.

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

Run **`make test`** for the mocked LED state-machine tests and **`make application`** for the XC32 firmware build. Open **`application/app.X`** on Windows, pull the changes and Clean/Build before programming with PICkit 5. Expected behaviour: **LED1 (D5) changes state about every 500 ms**. See [LED1 application: build and board test](docs/application.md) for safety, diagnostics, warnings and regeneration notes. LED1 blink, production operation and recovery after MCLR and a power cycle were confirmed by the user.

## Working baseline and next stage

Annotated tag **[v0.1.0-led-blink](https://github.com/jharoian-mchp/PIC32MX270-USB-Bootloader-Example/tree/v0.1.0-led-blink)** points to user-tested commit **dd59a29**, including the host -Wall/-Wextra and PICkit 5 settings. Production blinking survives MCLR and a power cycle.

Create a separate **bootloader/** MCC project next; leave application/ intact. See [USB HID bootloader configuration](docs/bootloader.md) for components, USB clock settings, memory/routing checks and the next host screenshot. No USB bootloader is generated or programmed yet.

## Repository layout

```text
Readme.md              Approach, progress and next steps
AGENTS.md              Instructions for work in this repository
Makefile               XC32 application build, mocked tests and compiler smoke test
application/
  app.X/               MPLAB X project and saved MCC configuration
  src/                 Generated PLIBs, main and user-owned LED application
host/                  Python image preparation and read-only HID discovery/probe
tests/                 Mocked application/update tests
tools/
  toolchain-smoke.c    Exact-device compile/link test; not application firmware
docs/
  host-compatibility.md Vendor host packet findings and recommended manifest extension
  update-image.md      Metadata-last commit and bounded async session; not active in firmware
  update-storage.md    HEX row/session and flash backend contracts; not active in firmware
  update-protocol.md   Tested bounded framing/command validation; not active in firmware
  update-safety.md     Host-only generated-parser audit and hardening requirements
  usb-entry.md         SW1-held startup enumeration and safe-idle test
  usb-enumeration.md   Diagnostic-only ADC/mux/enumeration board test
  usb-circuit.md       Mini-B mux/VBUS schematic findings and physical checks
  bootloader.md        USB HID project setup and pre-programming checks
  application.md       LED logic, build instructions and first hardware test
  toolchain.md         Windows/Linux tools, clock plan and generation handoff
  bring-up.md          Board findings, open checks and first-application checklist
  pdf/                 Original reference PDFs, including FRM/
  docling/             Converted Markdown, index and manifest
    FRM/               Converted family-reference chapters
    images/            Separate figures grouped by document
```

The application project is present. A separate `bootloader/` directory is the next host project; it is not yet generated.

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
| Production, reset and cold-start recovery | Passed, user-reported | Production-built app blinks and recovers after MCLR and power cycle. Exact commit dd59a29 tagged v0.1.0-led-blink. |
| Regenerate application after H3 content updates | Required follow-up | Adding the bootloader library updated host H3 content. After versions settle, regenerate the application, preserve user-owned LED code and repeat Linux/Windows builds plus production blink/MCLR/power-cycle tests. Not yet regenerated. |
| Generate/build HID bootloader | Linux build passed | Core regeneration 556a4fa supplies missing files/tasks; make bootloader compiles 23 C sources at -O2 and generates HEX. Program sections end at 0x9D005DFC, within 32 KB reservation. Warnings reviewed preliminarily; USB routing/VBUS/entry still need work. |
| Windows HID skeleton clean build | Passed | Supplied -O2/-Wall/-Wextra build log succeeds; 1,080 warning occurrences match Linux categories. Host fd97ec3 stores options; 0274bf9/9fdac62 now selects PK5Tool. Programming remains unverified. |
| Enumeration-only diagnostic code | Software checks passed | User-owned ADC hysteresis/mux/readback/attach code, always-diagnostic entry, no update-task calls. Linux build/mocks/sanitizers pass; erase/write/parser routines absent from linked ELF. |
| Enumerate HID bootloader | First connection passed, user screenshots | Windows VID_04D8/PID_003C; ADC 622 counts, mux latch 0x60/direction 0x9F, zero ADC/mux/USB error counters. Historical USB_OPEN startup flag explained; ADC unplug/replug and attach counts confirmed; MCLR and power cycle both remove/re-enumerate USB, confirmed by user. Earlier cable-replug configured flag not captured. No update commands. |
| Define and test memory layout and application handoff | Not started | Bootloader size and linker contract not established. |
| SW1 diagnostic startup selection | Passed, user-reported | User confirmed 51a78c5 works as described: held-at-reset enumeration, latched entry, normal safe idle and no late entry. No application handoff. |
| Generated update-path audit | Defects reproduced, host-only | 12 unsafe behaviors confirmed with mocked flash/datastream and sanitizers; generated code untouched. See docs/update-safety.md. |
| Bounded framing/command validation | Software checks passed, not integrated | User-owned update_protocol handles split escapes, CRC, buffer limits, exact command lengths and safe READ_CRC spans. Strict host/XC32 tests and sanitizers pass. No command execution. |
| HEX rows and flash-operation adapter | Software checks passed, not integrated | Full-span/order/batch validation, row padding and mocked flash bounds/error/readback/timeout tests pass. No target backend binding or command execution. |
| Image commit and async session | Software checks passed, not integrated | Metadata-last CRC32 commit, exact image-span bounds, copied-row queue and abort/busy quarantine tested with mocks/sanitizers. Last 1 KB page reserved provisionally; no target writes or handoff. |
| Host compatibility review | Offline review complete | UnifiedHost 1.19.3 bare erase lacks manifest; inspected checksum class uses command 8 rather than generated READ_CRC 4. Captured frame-vector tests pass. Project host extension recommended, not implemented. |
| Python host preparation/read-only tools | Software tests passed | host/pic32_hid.py validates/packages relocated HEX, lists HID and probes versioned capabilities. Optional hidapi; no upload/erase/reset command. Current firmware lacks query handler; probe expected to time out. |
| Verify USB updates and recovery | Disabled/not tested | Requires active manifest/schema handler, target backend/dispatcher, relocated app/linker and controlled hardware qualification. |

### Next steps

- Initial [diagnostic enumeration](docs/usb-enumeration.md) passes, including MCLR/power-cycle re-enumeration. Generated SW1/BOOT_REQUEST settings are verified. User-owned startup debounce now gates enumeration: hold SW1 across reset for diagnostic mode; no button yields safe idle, not application handoff. The user confirmed [SW1 works as described](docs/usb-entry.md); physical-entry diagnostics pass. Update commands remain inactive.
- The [host-only update audit](docs/update-safety.md) reproduces 12 unsafe behaviors in generated framing/HEX/flash handling. Run `make update-audit` to reproduce them safely with mocks; a passing audit is not update approval.
- [Bounded framing and command validation](docs/update-protocol.md) now pass tests, but remain uncalled in firmware. [HEX session/row and flash adapter](docs/update-storage.md) now pass strict tests too. [Image commit and asynchronous sessions](docs/update-image.md) now pass mocked tests too. The [vendor host inspection](docs/host-compatibility.md) confirms bare erase has no manifest and the inspected checksum class differs from generated command 4. A [Python host](host/README.md) now implements offline image preparation and read-only discovery/probe, with a matching uncalled C capabilities schema. Next add/test its firmware read-only endpoint on Windows before implementing manifest-aware upload/target binding.
- U2 is identified as **12-pin NCN1188**, marking AG7; the user corrected the initial ten-pin count. Mini-B mux selection is **GP5=GP6=1**, preserving other bits. Disconnected readings are 0 V; connected readings are J2 VBUS 5.16 V and VBUS_SENSE 2.011 V. User confirmed MCU VDD=3.3 V, same measurement point and return-to-zero on unplug. RA1 GPIO VIH requires 2.145 V, so add/review AN1 ADC sensing using AVDD/AVSS instead of GPIO-high detection; proposed setup is in docs/usb-circuit.md. See [USB circuit checks](docs/usb-circuit.md).
- User-owned mux/ADC/attach diagnostic logic is implemented and software-tested. Capture hardware unplug/replug/reset results before proceeding. Physical SW1 diagnostic entry is now user-confirmed; update-safety logic remains next.
- **Regenerate the application against the updated H3 content**, preserving app_led.c/.h, main integration, project entries, PICkit 5 and warning options. Rebuild on Linux/Windows and repeat production blink/MCLR/power-cycle tests. Keep v0.1.0-led-blink unchanged.
- Continue the separate bootloader MCC project for PIC32MX270F256D.
- Keep the reviewed bootloader clocks: USB PLL ON, input ÷3, UFRCEN OFF, 48 MHz USB clock; system PLL ÷3/×24/÷2 for 48 MHz SYSCLK/PBCLK. Finish USB Device HID Bootloader dependency connections.
- FreeRTOS is removed; keep required Harmony Core/USB dependencies. Set **Self Powered checked** for the externally powered board; verify USB VBUS sensing and prevent back-feeding.
- Generated APP_START/JUMP uses PA_TO_KVA0(0x1D008000), confirming virtual address 0x9D008000; the -O2 binary now fits the 32 KB reservation. Do not program yet.
- Resolve the carrier USB mux and VBUS/power path before connecting/attempting USB updates.
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
- **2026-10-13:** User confirmed LED1/D5 blinking and supplied a successful Windows PICkit 5 debug build/load log without extra warning flags. Pulled host PK5Tool configuration commit 48faaa9. Aligned the Linux source list with all 12 Windows project sources and added section garbage collection; full Linux build/tests pass. Production/reset/cold-start recovery was not yet reported at that point.
- **2026-10-13:** Pulled host compiler-option commit dd59a29. User confirmed production programming and blinking after both MCLR and power cycle. Re-ran Linux build/tests and pushed annotated tag v0.1.0-led-blink at that exact commit. Prepared separate USB HID bootloader configuration instructions.
- **2026-10-14:** User reported that adding the bootloader library updated H3 content. Added a required application-regeneration and regression-test follow-up after package versions settle. No application regeneration or new hardware verification has occurred yet.
- **2026-10-14:** Reviewed bootloader Clock Configurator: USB PLL input ÷3/4 MHz, UPLLEN ON, UFRCEN OFF and 48 MHz USB clock; system PLL ÷3/×24/÷2. USB HID bootloader/NVM component is visible; full dependency graph and memory settings remain to be reviewed.
- **2026-10-14:** User removed FreeRTOS, confirmed external board supply, reported 32,768-byte bootloader reservation and firmware trigger OFF. Corrected Self Powered to checked for this power arrangement; core owns APP_START_ADDRESS. Candidate 0x9D008000 remains subject to generated layout/map review. Bootloader skeleton generation/handoff was next at that point; programming was not authorised.
- **2026-10-14:** User confirmed Self Powered and physical application address 0x1D008000, generated and pushed skeleton 0e87595. Source review confirms the 0x9D008000 mapping, 32 KB linker region, USB clock fuses, 128-byte rows/1 KB erase pages and 04D8:003C self-powered HID descriptors. Build fails because Harmony common/configuration/task files are missing and SYS_Tasks remains empty. Requested MCC generation-switch correction; no bootloader hardware test yet.
- **2026-10-14:** Pulled Core regeneration 556a4fa with no user-reported notifications. Verified generated SYS_Tasks and common files. Added make bootloader; all 23 sources compile/link at -O2 and produce valid HEX within 32 KB. Standalone tests/build still pass; USB board routing/VBUS, physical entry and update-safety review remain.
- **2026-10-14:** Reviewed the user's full Windows bootloader build log: success, no errors and warning-category counts matching Linux (1,080 occurrences, mostly repeated unused-parameter diagnostics). Pulled fd97ec3 saving -Wall/-Wextra. Tool remains noID; IDE loading text does not verify hardware programming. USB routing/entry and nontrivial-warning review remain next.

- **2026-10-14:** Pulled host PICkit 5 selection commit 0274bf9 through merge 9fdac62; verified PK5Tool, retained -Wall/-Wextra and valid XML. USB routing/VBUS and entry review are next; no bootloader hardware test yet.
- **2026-10-14:** Traced mini-B J2 through U3 and NCN1188 HDN/HDP; component truth table selects this with GP5/GP6 high. Identified cable VBUS divider/clamp to RA1 versus dedicated VBUS fed from board 5V_SW_DIG. Added schematic crops and pre-PC voltage/revision checks; no hardware measurements or bootloader changes yet at that review.
- **2026-10-14:** User reported board revision R7, ten-pin U2 marked AG7 and disconnected VBUS_SENSE 0 V. The supplied NCN1188 schematic has twelve pins; suspended the earlier mux control setting pending actual part/revision identification. J2 connector VBUS reading remains outstanding.

- **2026-10-14:** User confirmed J2 connector VBUS also measures 0 V with no host attached, matching the earlier 0 V VBUS_SENSE reading. Initial disconnected voltage check passes; mux-part identification remains the hold point.

- **2026-10-14:** User corrected U2 pin count to 12 and provided NCN1188 marking/pinout excerpt; AG device code plus date character explains AG7. Cleared the identification hold and restored planned GP5=GP6=1 mini-B route. Connected-state VBUS_SENSE measurements and routing/attach logic remained next at that point; no bootloader programming yet.
- **2026-10-14:** User measured connected J2 VBUS 5.16 V and VBUS_SENSE 2.011 V. Data-sheet review finds ordinary I/O VIH(min)=0.65×VDD, 2.145 V at nominal 3.3 V, so direct RA1 GPIO cable detection is not guaranteed. Record actual VDD and unplug return-to-zero; assess ADC-based AN1 sensing without changing board components.

- **2026-10-14:** User confirmed MCU VDD 3.3 V, VBUS_SENSE returns near 0 V on unplug and the same measurement point. Proposed AN1 ADC setup: AVDD/AVSS references, 1 µs TAD, 16 µs acquisition and software-started sampling. Candidate 1.5 V/0.8 V hysteresis awaits ADC readings and tests; no firmware change yet.

- **2026-10-14:** Initial ADC screenshot shows correct AN1/reference/result choices, but TAD 125 ns and software-cleared SAMP trigger. Requested TAD 1,000 ns, internal-counter conversion trigger and 16 TAD acquisition, retaining software sampling start/polled completion. Revised ADC settings not yet generated.

- **2026-10-14:** Updated ADC screenshots confirm 1,000 ns TAD, internal-counter conversion trigger and RA1/AN1 analog input with no pulls. SAMC=16 remains to be exposed/confirmed; ADC generation and hardware readings are not yet available.

- **2026-10-14:** User confirmed Auto-Sample Time 16 TAD. ADC screen settings are ready for generation (1 µs TAD, 16 µs acquisition); generated code/build and hardware readings remain unverified.

- **2026-10-14:** Pulled generated AN1 ADC commit fce3569. Verified ADCS=23/SAMC=16, internal-counter software-started sampling and AN1/VREFL selection; complete 24-source Linux bootloader rebuild/HEX passes. Hardware ADC readings and detector/mux integration remain next.

- **2026-10-14:** Added enumeration-only board diagnostics: ADC cadence/threshold qualification, mux disconnect/select/readback, dedicated-VBUS-independent attach/detach and debugger counters. Always stays diagnostic; update tasks/parser and NVM erase/write absent from linked production ELF. Linux builds, strict user-code compile, mocked tests and sanitizer run pass; controlled hardware test is next.

- **2026-10-14:** User screenshots confirm first Windows vendor-defined HID enumeration at VID_04D8/PID_003C, ADC raw 622, mux latch 0x60/direction 0x9F and zero ADC/mux/USB error counters. Historical USB_OPEN flag explained by device-layer startup readiness. Unplug/replug/reset tests are next; update commands remain inactive.

- **2026-10-14:** User screenshots show unplug ADC0/cable absent/detached with detach_count=1, then replug ADC622/cable present/attached with attach_count=2. ADC timeouts/mux/USB errors remain zero. Confirm host configured state/re-enumeration after replug and reset/cold-start next; updates inactive.

- **2026-10-14:** User confirmed USB removal/re-enumeration after MCLR and power cycle. Initial enumeration-only milestone passes; physical SW1 entry and update-path safety tests are next without enabling host update commands.

- **2026-10-14:** Reviewed SW1/RB2 BOOT_REQUEST pin screen and carrier Figure A-8; R18 is a 20 kΩ pull-up to 3.3 V, button shorts to ground, active-low. Internal pulls can stay off. Generated GPIO digital/input settings and entry behavior remain next; updates inactive.

- **2026-10-14:** Pulled c4d27a6 SW1 generation, verified AN4 disabled and BOOT_REQUEST_Get. Added startup-button debounce: held SW1 enables enumeration, normal startup safe-idles without application handoff. Builds/mocks/sanitizers pass; SW1 hardware test next. Updates inactive.

- **2026-10-14:** User confirmed SW1 works as described for diagnostic startup-gated version 51a78c5. Physical entry milestone passes; normal start remains safe idle, with no application handoff or update commands. Parser/flash/image-safety tests are next.

- **2026-10-14:** Added host-only audit against unmodified generated update code. Reproduced 12 unsafe behaviors (eight non-crashing cases, four sanitizer failures) with mocked flash/datastream; valid baselines pass. Diagnostic/application builds and tests still pass, and update/erase/write code remains absent from diagnostic ELF. Next harden framing/command validation and HEX/flash/image contracts before enabling updates.

- **2026-10-14:** Added user-owned bounded update framing/command validation, not connected to active firmware. Split-frame/CRC/size/resync, exact command lengths, HEX shape and application-span tests pass with sanitizers and strict XC32 compile. Generated audit still reproduces original defects; diagnostic firmware/update safety unchanged. Next HEX session/row writer and flash/image contracts.

- **2026-10-14:** Added user-owned bounded HEX session/row layer and backend flash-operation adapter. Full batch preflight/range/order, padding/abort/failure, readback and timeout-buffer retention tests pass, including strict XC32 and sanitizers. Neither layer is bound to target NVM or connected to active firmware; diagnostic binary unchanged. Async orchestration and image commit/recovery remain next.

- **2026-10-14:** Added exact length/CRC32 image acceptance and metadata-last commit at reserved last 1 KB page, plus bounded async session orchestration. Mocked/sanitizer tests cover invalidation ordering, torn metadata, CRC, abort during row/commit, timeout quarantine and 70 KB image. Strict XC32 compile and existing builds/tests pass; active diagnostic unchanged. Host manifest transport/target backend/dispatcher/app layout still required before any hardware update.

- **2026-10-14:** Inspected exact bootloader v3.9.0 UnifiedHost1.19.3 Java packet classes offline. Bare command2 erase has no manifest; inspected checksum class uses ID8 versus generated READ_CRC4. Captured info/erase/reset frame vectors pass validator tests. Recommend a small negotiated manifest-aware project host; no uploader or target update command enabled.

- **2026-10-15:** User requested Python HID host. Added Python3.10+ offline HEX/image-package preparation, optional hidapi discovery and read-only versioned capabilities probe; no destructive CLI. Matching inert C wire schema/golden tests added. Nine Python test groups plus C/mocks/builds pass; standalone app HEX correctly rejected. Active firmware has no query handler yet, so real probe will time out; upload and hardware Python HID validation remain future work.
