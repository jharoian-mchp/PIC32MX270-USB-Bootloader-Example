# Docling reference documents

All 27 PDFs in `../pdf/` were converted to Markdown using Docling Serve with EasyOCR and accurate table extraction. The original PDFs are unchanged.

Images are separate files under `images/`, grouped by document. Markdown uses relative image links; no image data is embedded. The `FRM/` directory structure is preserved.

`manifest.json` records source SHA-256 hashes, output paths and conversion counts. Automated checks verified the source inventory, non-empty outputs and all image links. OCR text, tables and schematics may need comparison with the source PDF before use in firmware design.

## Documents

| Source PDF | Markdown | Images |
|---|---|---:|
| [FRM/FRM-01-Intro.pdf](../pdf/FRM/FRM-01-Intro.pdf) | [Read](FRM/FRM-01-Intro.md) | 3 |
| [FRM/FRM-02-CPU.pdf](../pdf/FRM/FRM-02-CPU.pdf) | [Read](FRM/FRM-02-CPU.md) | 15 |
| [FRM/FRM-05-Flash-Programming.pdf](../pdf/FRM/FRM-05-Flash-Programming.pdf) | [Read](FRM/FRM-05-Flash-Programming.md) | 2 |
| [FRM/FRM-06-Oscillators.pdf](../pdf/FRM/FRM-06-Oscillators.pdf) | [Read](FRM/FRM-06-Oscillators.md) | 10 |
| [FRM/FRM-09-Watchdog.pdf](../pdf/FRM/FRM-09-Watchdog.pdf) | [Read](FRM/FRM-09-Watchdog.md) | 7 |
| [FRM/FRM-10-Power-Saving-Modes.pdf](../pdf/FRM/FRM-10-Power-Saving-Modes.pdf) | [Read](FRM/FRM-10-Power-Saving-Modes.md) | 3 |
| [FRM/FRM-12-GPIO.pdf](../pdf/FRM/FRM-12-GPIO.pdf) | [Read](FRM/FRM-12-GPIO.md) | 7 |
| [FRM/FRM-13-PMP.pdf](../pdf/FRM/FRM-13-PMP.pdf) | [Read](FRM/FRM-13-PMP.md) | 48 |
| [FRM/FRM-14-Timers.pdf](../pdf/FRM/FRM-14-Timers.pdf) | [Read](FRM/FRM-14-Timers.md) | 7 |
| [FRM/FRM-15-Input-Capture.pdf](../pdf/FRM/FRM-15-Input-Capture.pdf) | [Read](FRM/FRM-15-Input-Capture.md) | 10 |
| [FRM/FRM-16-Output-Compare.pdf](../pdf/FRM/FRM-16-Output-Compare.pdf) | [Read](FRM/FRM-16-Output-Compare.md) | 23 |
| [FRM/FRM-17-ADC.pdf](../pdf/FRM/FRM-17-ADC.pdf) | [Read](FRM/FRM-17-ADC.md) | 24 |
| [FRM/FRM-19-Comparator.pdf](../pdf/FRM/FRM-19-Comparator.pdf) | [Read](FRM/FRM-19-Comparator.md) | 7 |
| [FRM/FRM-20-Comparator-Voltage-Ref.pdf](../pdf/FRM/FRM-20-Comparator-Voltage-Ref.pdf) | [Read](FRM/FRM-20-Comparator-Voltage-Ref.md) | 6 |
| [FRM/FRM-21-UART.pdf](../pdf/FRM/FRM-21-UART.pdf) | [Read](FRM/FRM-21-UART.md) | 23 |
| [FRM/FRM-23-SPI.pdf](../pdf/FRM/FRM-23-SPI.pdf) | [Read](FRM/FRM-23-SPI.md) | 33 |
| [FRM/FRM-24-I2C.pdf](../pdf/FRM/FRM-24-I2C.pdf) | [Read](FRM/FRM-24-I2C.md) | 32 |
| [FRM/FRM-27-USB.pdf](../pdf/FRM/FRM-27-USB.pdf) | [Read](FRM/FRM-27-USB.md) | 12 |
| [FRM/FRM-29-RTCC.pdf](../pdf/FRM/FRM-29-RTCC.pdf) | [Read](FRM/FRM-29-RTCC.md) | 12 |
| [FRM/FRM-31-DMA.pdf](../pdf/FRM/FRM-31-DMA.pdf) | [Read](FRM/FRM-31-DMA.md) | 10 |
| [FRM/FRM-33-Programming.pdf](../pdf/FRM/FRM-33-Programming.pdf) | [Read](FRM/FRM-33-Programming.md) | 11 |
| [FRM/FRM-37-CTMU.pdf](../pdf/FRM/FRM-37-CTMU.pdf) | [Read](FRM/FRM-37-CTMU.md) | 7 |
| [PIC32 Bluetooth Audio Dev Kit Reference Guide.pdf](../pdf/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide.pdf) | [Read](PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide.md) | 37 |
| [PIC32-Flash-Programming-Specification-DS60001145.pdf](../pdf/PIC32-Flash-Programming-Specification-DS60001145.pdf) | [Read](PIC32-Flash-Programming-Specification-DS60001145.md) | 158 |
| [PIC32MX1XX-2XX-28-36-44-pin-Family-Silicon-Errata-DS80000531.pdf](../pdf/PIC32MX1XX-2XX-28-36-44-pin-Family-Silicon-Errata-DS80000531.pdf) | [Read](PIC32MX1XX-2XX-28-36-44-pin-Family-Silicon-Errata-DS80000531.md) | 1 |
| [PIC32MX1XX2XX283644-PIN_Datasheet_DS60001168L.pdf](../pdf/PIC32MX1XX2XX283644-PIN_Datasheet_DS60001168L.pdf) | [Read](PIC32MX1XX2XX283644-PIN_Datasheet_DS60001168L.md) | 135 |
| [PIC32MX270F256D-Bluetooth-Audio-PIM.pdf](../pdf/PIC32MX270F256D-Bluetooth-Audio-PIM.pdf) | [Read](PIC32MX270F256D-Bluetooth-Audio-PIM.md) | 7 |

Total: 27 PDFs, 27 converted documents, 650 image files, 650 verified image references.
