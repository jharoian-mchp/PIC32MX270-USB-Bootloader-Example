# Stock UART offline candidate — 18 November 2026

The exact generated UART project at6c44d8e compiles, links and produces HEX without modifying its parser/NVM code. No serial transaction or board operation occurred.

`bun tools/build-customer-demo.ts --uart-only` builds12project-listed C sources with XC326.00/DFP1.7.380/O2/vendorCRT. `bun tools/audit-customer-demo.ts --uart-only` verifies flash ELFsection/HEX equality, reset, configuration and32KB bounds.

- text3720/data4/bss2084; entryBFC00000.
- PFM bytes1D000000..1D000CEF, within fixed32KB reservation.
- Configuration3..0:FFFFFFFF/FFF9FAFA/FD74CE5B/7FFFFFE3 (USBPLL off, PGx4).
- ELF SHA256:d2d50f150217cfd169d10342e1a75e271c18a39d5491a77649c5fb13e556c6a8
- HEX SHA256:4e74d3ba9efe69886dbd2ffdb37e34239bf0058d514c4c133d411e564d0ad2a7
- Stock UART source SHA256:b9cef7a74626e65fb371b4753ab9209dc2d873e5eb1a89a3d5a1d1144ff0c0da

## Application binary input

Prepared an offline binary from the previously checked app HEX, filling sparse holes with FF and padding to1024byte pages. Originphysical1D008000; exclusiveend1D03F800;227328bytes/222pages. This includes the high exception page and excludes all BootFlash/configuration. A future reproducible package must retain conversion/checksums/CRC and verify every binary address against HEX before host use.

The stock generated UART protocol compares addresses in its configured KSEG0 range; btl_host.py sends the supplied address directly. Use cached address9D008000 in the eventual host command, not a blindly substituted physical address. PIC32MX host options require explicit interface/device/address/sector size; inspect complete command before running. No bootloader-write option, fuse-write option, autotune or implicit reset command is approved here. btl_host normally uploads, verifies CRC and resets; these effects must be included in later hardware scope.

## Board route cross-check

Original carrier PDF TableA-1 and PIM original PDF map J8pin3 GPIO_18/UART2RX→PIM49→MCURC8, and J8pin5 GPIO_19/UART2TX→PIM50→MCURC4. The carrier table's RX PIM cell is missing; the PIM schematic/table supplies the connection. PPSsourceU2RXR6/RPC4R2 matches chosen RC8RX/RC4TX. Header orientation, a ground pin and whether other circuitry drives these nets must be checked before wiring. No ground-pin number is established by this review. User reports FTDI3.3VTTL, modelunknown; keep VCClead disconnected.

## Remaining target-test preparation

Preserve stock parser/NVM per user's test-first direction. The weak trigger is false: blank app enters UART loader; a valid app runs automatically and SW1re-entry requires a separate board-trigger adaptation or ICSP-controlled test procedure. This is an entry-policy choice, not permission to rewrite the stock UART write path.

Before target testing: validate reproducible binary/hostcommand, current board/session/power, exact erase/install scope, verified header ground/orientation/shared-net isolation and ICSP recovery. Windowsbuild equivalence and optional bootbutton adaptation remain untested. No malformedinput or failure-injection test is included. Known sourceissues remain in customer-uart-review.md; normal target success will qualify only the observed sequence.
