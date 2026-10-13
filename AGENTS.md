# PIC32MX270 USB HID bootloader

## Project

- Target MCU: PIC32MX270F256D-50I/PT on the Bluetooth Audio PIM and PIC32 Bluetooth Audio Development Board.
- Bootloader transport: USB device HID, connected to a PC.
- Toolchain: XC32 v6.00 on Windows and Linux; MCC v5.7.0 generation on the Windows host with Harmony 3. MPLAB X v6.35 and PICkit 5 handle host programming/debugging. Record whether standalone or IDE-hosted MCC actually generated the project.
- MCC/Harmony generator DFP is 1.6.369; MPLAB X and Linux build DFP is 1.7.380, confirmed by the user. Record generator/build versions independently; do not ask to make them identical.
- Installed Y1 is marked 12.00 MHz, confirmed by the user. Proposed 48 MHz PRIPLL clock: input ÷3, multiplier ×24, output ÷2; USB PLL input ÷3. Hardware clock validation is still required.
- Linux tools: `/workspace/ChrisK/tools/xc32/v6.00` and `/workspace/ChrisK/tools/packs/Microchip/PIC32MX_DFP/1.7.380`. Use the pack root for `-mdfp`. `make toolchain-check` verifies compiler/linker/HEX only; never program its test image.
- User confirmed carrier J5 pins 1–2 (3.3 V I²C1 pull-ups), PIM J1 pins 1–2 (MCP23008 seven-bit address 0x22), and Timer2 interrupt enable checked. Use 0x22 for seven-bit I²C APIs; hardware bus communication/runtime interrupts are not yet tested.
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
