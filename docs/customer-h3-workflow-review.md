# H3 workflow and demo baseline — 17 November 2026

Use MCC's Harmony 3 configuration workflow for the demo, with pinned H3 package content and a stock USB HID example as reference. Git repositories provide package sources/examples; cloning them alone does not generate a device-specific application.

## Verified upstream sources

Retrieved official Microchip repositories on 17 November; branch heads below identify the inspected content, not a promise of installed Windows versions.

| Repository | Inspected master commit | Finding |
|---|---|---|
| [bootloader](https://github.com/Microchip-MPLAB-Harmony/bootloader) | `8262d3df2ca612a60c2afe580e34a6fdb814b976` | Release v3.9.0; configuration scripts/templates and UnifiedHost 1.19.3 |
| [bootloader_apps_usb](https://github.com/Microchip-MPLAB-Harmony/bootloader_apps_usb) | `a4b11a4fa3d2ccf733bcf8bccbef1fc53fe3d607` | Release v3.5.0; USB HID examples include PIC32MX1XX/2XX/5XX family |
| [bootloader_apps_uart](https://github.com/Microchip-MPLAB-Harmony/bootloader_apps_uart) | `c3183f8f2e2a11a439c327b0805a3eb84d633a26` | UART fallback reference repository |

The HID example includes `pic32mx_125_sk`, `pic32mx470_curiosity` and `pic32mx_eth_sk2` projects. None is automatically the PIC32MX270F256D PIM/carrier. Use the small-MX-family project as a reference and generate for the actual demo device; verify its exact device and peripheral settings before reuse.

The library's `config/bootloader_usb_device_hid.py` sets the PIC32MX maximum bootloader size to 32768 bytes and invokes the MIPS configuration script. This supports the chosen 32 KB reservation; linked fit still requires measurement.

Official documentation:

- [USB Device HID Bootloader](https://onlinedocs.microchip.com/oxy/GUID-90F6CA10-E41C-4317-A3AA-CB049A76D4F0-en-US-5/GUID-53BA9F83-0A43-476E-A6A7-1D20B0E311CB.html): uses Unified Bootloader Host, application HEX and Product ID 0x3C; handles incoming data with bootloader_Tasks().
- [MIPS application configuration](https://onlinedocs.microchip.com/oxy/GUID-90F6CA10-E41C-4317-A3AA-CB049A76D4F0-en-US-5/GUID-FAE7062B-4942-4EB8-A515-490BE1DD11F6.html): application reset must match bootloader start; linker entry/vector/configuration ownership and optional RAM trigger reservations matter. Example numerical settings are device-specific and must not be copied directly to this small MX.
- [HID library configuration entry](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=mcc_h3_usb_bootloader_configurations&redirect=true).

## Intended generation path

1. Use a separate MPLAB X project for PIC32MX270F256D and initialise its MCC Harmony 3 configuration. Select pinned bootloader/core/CSP/USB content through Content Manager; record installed versions before generation.
2. Select the USB Device HID Bootloader and resolve its generated dependencies. Adapt the confirmed board clocks, routing, power and entry button. Do not copy the experimental USB retirement overlays or custom manifest protocol.
3. Generate the bootloader and a separate simple application. Set application start 0x1D008000 / cached entry 0x9D008000 and review the resulting linker/vector/reset contract. The application update image must not replace bootloader-owned configuration or Boot Flash.
4. Review generated files, build logs and matching stock host behaviour offline before any programming or upload.

This path replaces mixed H2/H3 instructions with one MCC-driven demo workflow. Exact current IDE screenshots and component compatibility are still needed before issuing generation instructions.

## Host choice and limits

Candidate HID host: `bootloader/tools/UnifiedHost-1.19.3/UnifiedHost-1.19.3.jar`. The README contains Java-version and UI caveats, especially for UART COM-port Apply. Confirm the actual Windows Java version and launch behaviour offline. Do not prescribe obsolete Java installation without a separate security/tooling decision.

Earlier [host inspection](host-compatibility.md) found stock erase has no custom manifest and a checksum-command mismatch. The customer demo will use the actual stock generated protocol; the custom Python uploader and its manifest rules are not substitutions. Recheck checksum/erase/program/reset packet compatibility and the [generated parser audit](update-safety.md) against the selected version before enabling writes. This review does not approve UnifiedHost erase/program operations.

## Saved board and tool baseline

Reference source: `docs/docling/README.md`, device/PIM/carrier PDFs and existing generated MCC configuration.

- PIC32MX270F256D Bluetooth Audio PIM/carrier; historical reported carrier revision R7 and 12 MHz Y1.
- Planned/previously generated system PLL DIV3/MUL24/DIV2 = 48 MHz; USB PLL DIV3. Measured clock frequency is unqualified.
- MCP23008 seven-bit I2C address 0x22; LED and USB mux need board-specific expander handling. Mini-B mux GP5/GP6 high; cable sense RA1/AN1, SW1 RB2 active low.
- Saved bootloader packages: bootloader3.9.0, core3.17.1, CSP3.26.1, USB3.16.0. Earlier application CSP3.26.0 needs a deliberate generation/version alignment decision.
- Last reported host: MPLAB X6.35, XC326.00, MCC5.7.0, build DFP1.7.380; older generator DFP1.6.369 recorded separately. Upstream release notes list older tested tools (IDE6.30/XC325.00/MCC5.6.4), so version compatibility must be checked rather than assumed.
- Last board session report is 5 November's RAM readout halt. Current debugger, external-power and PICkit state must be reconfirmed.

### Current host report — 17 November 18:32

John reports MPLAB X6.35, XC326.00 and MCC5.7.2.3, with all packs updated. The board has external power and PICkit connected, with no active debugger session. This supersedes the 5 November active-halt report; intervening reset/programming history is unknown. Screenshot434 shows Content Manager, but does not establish every exact selected bootloader/USB/core/DFP version. Exported content selections or expanded package rows are required before generation. BSP version must not be substituted for core version.

USB HID remains primary. If blocked, use stock H3 UART while retaining the 32 KB offset. UART pin routing, levels and compatible host have not been selected or hardware-tested.

## Verification performed

Read-only official HTTP retrieval, upstream source inspection and saved YAML version extraction. Research copies live outside Git under `/workspace/ChrisK/customer-support/research-20261117/`. No firmware build/link, MCC generation, host transaction or board operation occurred. The web-search wrapper failed because its local turndown dependency was missing; direct official-site retrieval succeeded.
