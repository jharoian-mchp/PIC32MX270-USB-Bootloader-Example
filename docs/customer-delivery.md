# Customer UART demonstration delivery

The delivery contains the exact production UART bootloader and application binary used in the successful19November target test. It includes source/MCC projects and a draft answer. No new firmware build or hardware operation is needed to assemble it.

## Reproduction

Use MPLABX6.35/XC326.00/MCC5.7.2.3, H3bootloader3.9.0/core3.17.1/CSP3.26.1 and buildDFP1.7.380. GeneratorDFP1.6.369 was recorded independently. USB3.16.0 applies to the preserved HID project, which is not a tested updater in this delivery.

The archive includes `customer-demo/uart-bootloader/u-bl.X` and `customer-demo/application/app.X` with sibling source folders. Open them as source-managed Application Projects. UART stock source and settings remain unchanged; the LED main integration and application linker selection are user additions. Do not regenerate over these without inspecting/reapplying documented changes. Build outputs may vary with flags/host; validate newly built artifacts instead of assuming hash identity.

Install the production `images/uart-bootloader.hex` through ICSP only after checking the exact device, power arrangement and desired erase scope. Tested procedure used PICkit5/toolpowerOFF/externalpower, EraseAll/preservationOFF and successfulverify, then detachedPICkit. This loses existing application/configuration/BootFlash contents. A blank application keeps the stockloader active.

UART2 is115200/8N1/no flow control. On the demonstrated carrier, FTDI TTL-232R-3V3 blackGND→J8pin2, orangeTX→J8pin3/RC8RX, yellowRX→J8pin5/RC4TX; red5V/CTS/RTS disconnected. Confirm actual board revision/orientation/shared circuits and voltage levels before wiring. Do not infer these pins for the larger customer device or another board.

Prepare a local Python environment (WindowsPowerShell):

```powershell
uv venv .venv
uv pip install --python .\.venv\Scripts\python.exe pyserial==3.5
```

For the tested FTDI COM41:

```powershell
.\.venv\Scripts\python.exe .\host\btl_host.py -v -i COM41 -d PIC32MX -a 0x9D008000 -p 1024 -f .\images\application.bin
```

This command writes222applicationpages, verifiesCRC and resets the MCU. ChangeCOM41 only to the verified adapter port. Do not use bootloader-write, fuse-write, bankswap or autotune options for this baseline. Stop on failure rather than automatically resending or powercycling. The host's response retries are repeated reads, not retransmission.

## Layout and artifacts

Fixed application entry9D008000/physical1D008000; stockbootloader reserves32KB. Application binary spans physical1D008000..1D03F7FF and is227328bytes, paddedFFthrough sparse gaps. Exception page at1D03F000 is included. No application configuration orBootFlash data are in the update binary.

- TestedUARTHEX:4e74d3ba9efe69886dbd2ffdb37e34239bf0058d514c4c133d411e564d0ad2a7.
- TestedapplicationBIN:c13edd9338e2a720abd84baf574efd65b7e78be66b1192b9acdd7d43acffdacd.
- Stockhost:964dde1b8c9be0a006ebd284d422c275520b8703fc5f36f0b762e738a3b9af84.

SHA256SUMS in the archive covers its complete delivered file inventory. Preserve vendor copyright/licence notices in source and host. The draft reply has not been sent to the customer.

## Observed results and limits

See customer-uart-target-result.md: productioninstall, version3.7, normalupload100%, CRCsuccess, hostreset and autonomousLEDblink passed with no debugger. Package version3.9.0 and wireversion3.7 are distinct.

The HID source findings and stockUART input/error limitations are retained in customer-upload-review.md and customer-uart-review.md. No pretest UART hardening was applied, at John's direction. Normal success is not interrupted-write/protection/malformed-input qualification.

Re-entry with a valid app, coldpower recovery, fault tests and the larger customer variant are untested. Stocktriggerfalse requires a product-specific re-entry decision; the package does not implement SW1re-entry. Preserve an ICSP recovery path.
