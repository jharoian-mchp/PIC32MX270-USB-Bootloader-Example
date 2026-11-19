# Draft reply to Chris

Hi Chris,

For a new Harmony 3 project, use MPLAB X with MCC and its Harmony Content Manager. Select the required H3 packages and configure/generate the bootloader and application as separate projects. The Harmony Git repositories contain the same library sources and reference examples; they are useful for version pinning and examples, but cloning alone does not configure your device. You do not need to go back to the Harmony 2 configurator workflow.

I put together and tested a UART bootloader plus a small LED application on a PIC32MX270F256D. It uses H3 bootloader3.9.0, CSP3.26.1, core3.17.1, MPLABX6.35, XC326.00 and PIC32MX_DFP1.7.380. The generated bootloader reports wire version3.7; that is separate from the package version.

The demonstration reserves32KB for the bootloader and starts the application at physical0x1D008000/cached0x9D008000. Relocation requires more than changing the application origin: its reset/startup, exception/vector placement and configuration-word ownership must agree with the bootloader. The included application linker script handles those details for the demonstrated device. Bootloader-owned BootFlash/configuration bytes are excluded from the application update image.

With the production bootloader installed and the debugger detached, the standard H3 Python UART host programmed the application, verified its CRC and reset the device. LED1 then blinked automatically with no intervention. The generated UART parser and NVM write code were left unchanged for this normal-path demonstration.

I started with USB HID, then used UART to finish the working baseline. The HID source review found a split-escape issue at report boundaries and reset/receive-state concerns, plus input/flash-error-handling limitations. Those findings are included separately; HID updating was not qualified by this test.

The package includes both source projects and MCC settings, the tested production images, the standard UART host and setup instructions. It is a demonstration for the PIC32MX270F256D board; please confirm the exact larger device in your product before porting the flash limits, package pin mappings or device configuration. The original email names PIC32MX270F512H.

The test covers a normal firmware upload, CRC and automatic application launch. Power-loss/interrupted-update recovery, malformed-input robustness, bootloader re-entry/repeated updates without reinstalling it, and your larger variant have not been tested. The stock demo's trigger defaults to false: it stays in the bootloader with a blank application, and otherwise starts the application. A product needs an explicit re-entry policy.

Regards,
John
