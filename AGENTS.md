# PIC32MX270 USB HID bootloader

## Project

- Target MCU: PIC32MX270F256D-50I/PT on the Bluetooth Audio PIM and PIC32 Bluetooth Audio Development Board.
- Bootloader transport: USB device HID, connected to a PC.
- Toolchain: XC32 compiler and standalone MCC with MPLAB Harmony 3.
- Start with a simple application before adding Bluetooth or audio functionality.

## Documentation

- Use `docs/docling/` first for device, PIM and carrier-board documentation.
- Start with `docs/docling/README.md` to find the converted documents.
- Figures are separate files under `docs/docling/images/`, linked from Markdown. Keep image links relative and do not embed image data.
- `docs/docling/manifest.json` records source PDF hashes and conversion results.
- Use the original PDFs under `docs/pdf/` to verify ambiguous OCR, register tables, pin assignments, schematics and other hardware-critical details.
- Do not assume a Harmony example board's pins, clock settings or memory layout match this board.

## Work practices

- Read the relevant documentation and existing files before changing code.
- Test or verify changes before declaring them complete. State when hardware testing has not been performed.
- Keep credentials, private SSH keys, transient conversion files and build outputs out of the repository.
- Use merge-based Git pulls; do not rebase unless requested.
