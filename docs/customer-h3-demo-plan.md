# Customer H3 bootloader demonstration

Deliver a working Harmony 3 bootloader and small application, plus one reproducible workflow that answers Chris Kepner's 11 October email.

Branch: `customer/h3-bootloader-demo`. Experimental work is preserved at `archive/experimental-nvm-paused-20261117`; see [pause checkpoint](paused-experimental-work.md).

## Customer question

The customer has a working H3 application, added bootloader support through MCC Content Manager, and asks whether MCC or cloning Harmony repositories is the preferred method. He expects a 32 KB application offset. The supplied email names `PIC32MX270F512H`; our board is `PIC32MX270F256D`. Confirm the actual orderable device before using a pack, linker script or flash geometry. USB HID was agreed for our earlier board work; the email itself specifies no update transport.

Original PDF/text are preserved outside the public repository under `/workspace/ChrisK/customer-support/` (attachment 433). Personal email addresses and telephone details are not copied into the customer plan.

## Acceptance criteria

- A direct, source-backed answer explaining the relationship between MCC configuration and H3 repositories/package content, with one recommended supported workflow and pinned versions.
- Separate minimal bootloader and application projects generated for the confirmed device. Board clock, USB route, boot button and LED adaptation are explicit.
- Application relocation includes entry/reset/vector placement, flash/RAM bounds and configuration ownership. A 32 KB offset is accepted only after matching the bootloader configuration and linked size.
- The selected host utility and image format match the stock H3 protocol. Existing custom Python v1/v2 tools are not assumed compatible.
- Offline builds and artifact checks pass, followed by separately authorised hardware installation, upload, application launch, re-entry and recovery tests.
- Customer package contains sources, MCC configuration, pinned package/tool versions, ELF/HEX/map files, compatible host instructions, hashes and documented limits. Hardware success must be observed before calling it a working board demo.

## Execution plan

1. Confirm exact customer MCU, customer board or demonstration-board choice, oscillator/pins/power, update transport, current MPLAB/XC32/MCC/H3 versions and whether the customer's project can be supplied.
2. Check current official Microchip H3/MCC documentation and a matching stock bootloader example. Record versioned references and supported host tool; avoid combining old H2 and current H3 instructions.
3. Choose separate `customer-demo/bootloader/` and `customer-demo/application/` paths. Keep experimental sources and private USB overlays out of their source lists. Reuse the known LED board adaptation only where hardware matches.
4. Generate the minimal bootloader and application through the selected workflow. Review generated tasks, dependencies, clocks, USB descriptors, entry policy, NVM routines and host command compatibility.
5. Set and verify one bootloader/application memory contract. Review bootloader fit, application entry/vector offsets, configuration ownership and flash erase/write boundaries; do not blindly edit an application origin alone.
6. Build both images on the intended toolchain; compare Windows/Linux where useful. Run host/parser and boundary tests. Disposition existing generated-code audit findings against the actual version before enabling updates. Any required fixes are bounded to the demo and documented; the paused custom updater is not silently substituted.
7. Define the board procedure and obtain explicit approval after reconfirming actual board/session state. Test initial installation, forced entry, upload, visible application execution, update/re-entry and recovery. Interrupt/power-loss tests require separate scope.
8. Deliver a checked customer archive and concise email answer with setup steps, version pins, memory contract, observed results and limits.

## Current scope

Branch creation, requirements and planning are complete. Demo implementation, new firmware builds, programming, debugger session end/reset, application upload and NVM writes have not been performed for this task. Last-known experimental hardware state is dated 5 November and must be reconfirmed.
