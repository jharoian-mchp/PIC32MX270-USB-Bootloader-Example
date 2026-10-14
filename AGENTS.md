# PIC32MX270 USB HID bootloader

## Project

- Target MCU: PIC32MX270F256D-50I/PT on the Bluetooth Audio PIM and PIC32 Bluetooth Audio Development Board.
- Bootloader transport: USB device HID, connected to a PC.
- Toolchain: XC32 v6.00 on Windows and Linux; MCC v5.7.0 generation on the Windows host with Harmony 3. MPLAB X v6.35 and PICkit 5 handle host programming/debugging. Record whether standalone or IDE-hosted MCC actually generated the project.
- MCC/Harmony generator DFP is 1.6.369; MPLAB X and Linux build DFP is 1.7.380, confirmed by the user. Record generator/build versions independently; do not ask to make them identical.
- Installed Y1 is marked 12.00 MHz, confirmed by the user. Proposed 48 MHz PRIPLL clock: input ÷3, multiplier ×24, output ÷2; USB PLL input ÷3. Hardware clock validation is still required.
- Linux tools: `/workspace/ChrisK/tools/xc32/v6.00` and `/workspace/ChrisK/tools/packs/Microchip/PIC32MX_DFP/1.7.380`. Use the pack root for `-mdfp`. `make toolchain-check` verifies compiler/linker/HEX only; never program its test image.
- User confirmed reset selector PIM_MCLR, carrier J5 pins 1–2 (3.3 V I²C1 pull-ups), PIM J1 pins 1–2 (MCP23008 seven-bit address 0x22), and Timer2 interrupt enable checked. Use 0x22 for seven-bit I²C APIs; hardware bus communication/runtime interrupts are not yet tested.
- Initial MCC application is present at `application/app.X`; saved components record CSP v3.26.0. Generated code confirms ICS_PGx4. Linux `make application` and `make test` pass; User confirmed Windows PICkit 5 debug build/load and LED1 blink; production operation and MCLR/power-cycle recovery are now user-confirmed. Shared project selects PK5Tool (host commit 48faaa9).
- Production-tested standalone baseline is annotated tag v0.1.0-led-blink at dd59a291254b45d44fba65810b2085d9b6ea40df, including host -Wall/-Wextra settings. Never move this milestone tag. Create the HID bootloader as a separate project; see docs/bootloader.md.
- REQUIRED FOLLOW-UP (2026-10-14): adding the bootloader library updated host H3 content. After package versions settle, regenerate the application's own MCC configuration too, preserving LED code/integration and standalone memory layout. Record actual versions, rebuild both platforms and repeat production blink/MCLR/power-cycle tests. Not yet regenerated; old hardware verification applies only to the tagged baseline.
- User-owned LED logic is `application/src/app_led.c`/`.h`, called from main.c. Preserve it and its public project entries during MCC regeneration; generated PLIBs remain unchanged. See `docs/application.md` for diagnostics and board tests.
- Bootloader configuration (2026-10-14): FreeRTOS removed, 32 KB reservation reported, firmware trigger OFF. Board is externally powered: USB Self Powered should be checked. Generated APP_START/JUMP is PA_TO_KVA0(0x1D008000), confirming 0x9D008000; linker reserves 32 KB. Core regeneration 556a4fa resolves the original missing-file/task blocker. `make bootloader` passes the complete 23-source -O2 build and generates HEX within 32 KB; loadable program sections end at 0x9D005DFC. Generated code still lacks board USB-mux setup/physical entry; nontrivial compiler diagnostics and update parser safety need review. Windows clean-build attachment 81 also succeeds with identical warning-category counts; fd97ec3 stores -Wall/-Wextra. Host commit 0274bf9 (merged at 9fdac62) now selects PK5Tool with -Wall/-Wextra retained. The earlier log lacked PICkit 5 flags; bootloader hardware programming is still not verified; see docs/bootloader.md. Bootloader v3.9.0, USB v3.16.0, core v3.17.1, CSP v3.26.1 recorded. Do not program before build/USB mux/VBUS and memory review.
- USB circuit identification resolved (2026-10-14): user corrected U2 to TWELVE-PIN NCN1188 and supplied datasheet excerpt; AG=device code, M=date code, so AG7 is consistent. Board revision reported R7. Mini-B selects GP5/GP6 both HIGH, preserving other bits. User confirmed disconnected J2 VBUS and VBUS_SENSE both 0 V. Connected readings now reported: J2 VBUS 5.16 V, VBUS_SENSE 2.011 V. RA1 GPIO VIH=0.65×VDD (2.145 V at 3.3 V), so direct digital detection is not guaranteed; user confirmed actual VDD=3.3 V, same measurement point and unplugged sense near 0 V. Next configure AN1 ADC (AVDD/AVSS, not external VREF− shared on RA1); review proposed clock/acquisition/thresholds in docs/usb-circuit.md before implementation. Route/attach-detach code and enumeration remain unverified. Cable sense goes to RA1/PIM80; dedicated VBUS/PIM54 is board 5V_SW_DIG and generated driver senses that dedicated VBUS. See docs/usb-circuit.md. Do not program current skeleton yet.
- Generated AN1 ADC commit fce3569 reviewed/build passed: AD1CON3=0x1017 (ADCS23/SAMC16), AD1CON1=0xE0 (software sample start/internal counter conversion), AD1CHS=0x10000. Use ADC_SamplingStart/ResultIsReady/ResultGet, not manual ConversionStart. 24-source bootloader builds; actual ADC readings/detector and mux integration not yet verified.
- Start with a simple application before adding Bluetooth or audio functionality.

## Documentation

- Use `docs/docling/` first for device, PIM and carrier-board documentation.
- Start with `docs/docling/README.md` to find the converted documents.
- Figures are separate files under `docs/docling/images/`, linked from Markdown. Keep image links relative and do not embed image data.
- `docs/docling/manifest.json` records source PDF hashes and conversion results.
- Use the original PDFs under `docs/pdf/` to verify ambiguous OCR, register tables, pin assignments, schematics and other hardware-critical details.
- Do not assume a Harmony example board's pins, clock settings or memory layout match this board.

## Work practices

- Keep the root `Readme.md` updated when project decisions, setup requirements, verified milestones or next steps change. Update its last-updated date and progress log; distinguish completed work from planned or untested work.
- Read the relevant documentation and existing files before changing code.
- Test or verify changes before declaring them complete. State when hardware testing has not been performed.
- Keep credentials, private SSH keys, transient conversion files and build outputs out of the repository.
- Use merge-based Git pulls; do not rebase unless requested.
