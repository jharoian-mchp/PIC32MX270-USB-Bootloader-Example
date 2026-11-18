# H3 UART fallback review — 18 November 2026

Stock H3 UART uses a simpler binary transport and avoids HID report escaping, but has independent packet-validation and flash-error issues. This review is source-only; no UART project, build or hardware test has run.

HID findings are preserved in [customer-upload-review.md](customer-upload-review.md), commit c3a27a3, with active customer source paths and normal-frame versus malformed-input distinctions.

## Pinned references and host

- H3 bootloader v3.9.0 commit8262d3df2ca612a60c2afe580e34a6fdb814b976: `templates/src/optimized/bootloader_uart_mips.c.ftl`, `config/bootloader_uart.py`, `tools/btl_host.py`.
- UART examples commitc3183f8f2e2a11a439c327b0805a3eb84d633a26: `apps/uart_bootloader/bootloader/firmware/src/config/pic32mx_125_sk/bootloader/bootloader_uart.c` and its UART2 PLIB. This board/device is a reference, not the actual demo.
- Host is Python `btl_host.py`, default115200baud, binary input. Protocol is little-endian guard0x5048434D,32bit payload size,8bit command,payload. CommandsA0unlock/A1data/A2CRCverify/A3reset/A6version. It is different from UnifiedHost's HID/IntelHEX path.

## Concrete source findings

The reference generated `bootloader_uart.c` gives these directly inspectable locations; the current v3.9.0 template has the same relevant patterns.

- Lines164–193 parse arbitrary header size and fill `byte_buf[ptr]` until that size. No cap against the allocated input buffer was found. Command-specific length checks are absent. Oversized/truncated packets need regression coverage and rejection before mutation.
- Lines219–231 unlock checks end>begin and end<=flashend, but does not require begin>=APP_START_ADDRESS. Therefore a received unlock can authorise erasing bootloader addresses. A normal correctly addressed host file is narrower, but device-side containment is missing.
- Lines237–250 validate only the starting data address inside unlock; full page span and `data_size-4` underflow/row geometry are unchecked. It copies an erase-page-sized buffer regardless of received payload size.
- Lines305–339 discard PageErase/RowWrite return values, wait for busy without deadline, and finally sendOK. Flash failure reporting remains a blocker for the UART path, as for HID.
- CRC verification uses the unlocked span; reset waits for transmit completion then requests hardware reset. Neither establishes whole-image commit/power-loss validity. These observations require exact generated PIC32MX270 regression and codegen review before any write test.

The HID-specific split-DLE/reset receive-queue issues do not transfer directly to the polled UART parser. UART does not establish safe flash operation merely by dropping USB.

## Image and hardware contract

Retain the32KB application start physical1D008000/cached9D008000. Binary conversion must preserve the sparse application exception page at1D03F000, fill holes explicitly, and exclude bootloader/config/BootFlash. The delivered application HEX is currently sparse; do not strip HEX syntax or use a generic conversion without checking origin and padding.

The PIM Docling mapping lists UART1RX RC6/PIM77, UART1TX RC7/PIM78, UART2RX RC8/PIM49, UART2TX RC4/PIM50. The carrier guide describes a Bluetooth UART1 path on J3RX/TX. Availability, jumper routing and PPS-supported mapping must be verified against original schematic/device sources before choosing a UART. No UART pins or external adapter are selected yet. Use correct MCU logic levels, not raw RS232 signals; electrical wiring requires a separately stated procedure.

## Recommended next scope

Preserve the HID project and create a separate stock H3 UART customer project. Inspect its generated exact-device source and choose the physical serial route. Add bounded packet-size/command/address-span validation and truthful NVM failure reporting as narrow demo fixes, with offline mocks and matching standard host protocol. Do not import the paused customv2/manifest/USBrestart architecture. Regeneration, source changes, new builds and hardware writes need their respective explicit scope; this review performs none of them.

Research source copies remain outside Git at `/workspace/ChrisK/customer-support/research-uart-20261118/`.
