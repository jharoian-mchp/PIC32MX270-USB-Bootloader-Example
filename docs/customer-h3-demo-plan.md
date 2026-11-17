# Customer H3 bootloader demonstration

Deliver a working Harmony 3 bootloader and small application, plus one reproducible workflow that answers Chris Kepner's 11 October email.

Branch: `customer/h3-bootloader-demo`. Experimental work is preserved at `archive/experimental-nvm-paused-20261117`; see [pause checkpoint](paused-experimental-work.md).

## Customer question

The customer has a working H3 application, added bootloader support through MCC Content Manager, and asks whether MCC or cloning Harmony repositories is the preferred method. The supplied email names `PIC32MX270F512H`; John confirms the customer uses a larger variant. Its exact device details require a separate port check; the demo targets the confirmed `PIC32MX270F256D`.

## Demo decisions — 17 November 2026

- Run actual hardware tests on the PIC32MX270F256D Bluetooth Audio PIM and development board used in the earlier work. Use `docs/docling/README.md` and its linked part/PIM/carrier references first; verify ambiguous hardware details against `docs/pdf/`.
- Use USB HID as the primary update transport.
- If USB HID blocks progress or exposes problems, fall back to a stock H3 UART transport. Record the reason and verify UART pins, voltage levels, host tool and generated source dependencies before testing it. This fallback does not resume the paused custom USB/NVM project.
- Keep a fixed 32 KB bootloader reservation for both transports: application physical start `0x1D008000`, cached virtual entry `0x9D008000`. Verify reset/vector layout and bootloader fit; do not enlarge the reservation or move the application origin to accommodate USB or UART issues.

Original PDF/text are preserved outside the public repository under `/workspace/ChrisK/customer-support/` (attachment 433). Personal email addresses and telephone details are not copied into the customer plan.

## Acceptance criteria

- A direct, source-backed answer explaining the relationship between MCC configuration and H3 repositories/package content, with one recommended supported workflow and pinned versions.
- Separate minimal bootloader and application projects generated for the confirmed device. Board clock, USB route, boot button and LED adaptation are explicit.
- Application relocation includes entry/reset/vector placement, flash/RAM bounds and configuration ownership. Both USB HID and any UART fallback retain the fixed 32 KB offset (`0x1D008000` / `0x9D008000`); each linked bootloader must fit below that boundary.
- The selected host utility and image format match the stock H3 protocol. Existing custom Python v1/v2 tools are not assumed compatible.
- Offline builds and artifact checks pass, followed by separately authorised hardware installation, upload, application launch, re-entry and recovery tests.
- Customer package contains sources, MCC configuration, pinned package/tool versions, ELF/HEX/map files, compatible host instructions, hashes and documented limits. Hardware success must be observed before calling it a working board demo.

## Execution plan

1. Use the confirmed PIC32MX270F256D demo board and USB HID transport; verify current oscillator/pins/power and MPLAB/XC32/MCC/H3 versions against the saved configuration and Docling references. Record the larger customer variant separately for later porting. Reconfirm actual board/debugger state before hardware actions.
2. Check current official Microchip H3/MCC documentation and a matching stock bootloader example. Record versioned references and supported host tool; avoid combining old H2 and current H3 instructions.
3. Choose separate `customer-demo/bootloader/` and `customer-demo/application/` paths. Keep experimental sources and private USB overlays out of their source lists. Reuse the known LED board adaptation only where hardware matches.
4. Generate the minimal bootloader and application through the selected workflow. Review generated tasks, dependencies, clocks, USB descriptors, entry policy, NVM routines and host command compatibility.
5. Set and verify one bootloader/application memory contract. Review bootloader fit, application entry/vector offsets, configuration ownership and flash erase/write boundaries; do not blindly edit an application origin alone.
6. Build both images on the intended toolchain; compare Windows/Linux where useful. Run host/parser and boundary tests. Disposition existing generated-code audit findings against the actual version before enabling updates. Any required fixes are bounded to the demo and documented; the paused custom updater is not silently substituted.
7. Define the board procedure and obtain explicit approval after reconfirming actual board/session state. Test initial installation, forced entry, upload, visible application execution, update/re-entry and recovery. Interrupt/power-loss tests require separate scope.
8. Deliver a checked customer archive and concise email answer with setup steps, version pins, memory contract, observed results and limits.

## Pinned tools and content — confirmed 17 November

| Component | Version |
|---|---|
| MPLAB X | 6.35 |
| XC32 | 6.00 |
| MCC | 5.7.2.3 |
| H3 bootloader | 3.9.0 |
| H3 USB | 3.16.0 |
| H3 core | 3.17.1 |
| H3 CSP | 3.26.1 |
| PIC32MX_DFP (selected build pack) | 1.7.380 |

Record the generator DFP independently if MCC reports a different selection; do not infer it from the build pack.

## Current scope

Branch creation, requirements and workflow review are complete. Next create a fresh standalone source-managed bootloader project under `customer-demo/bootloader/`, then inspect its MCC graph and settings before generation. Existing experimental projects stay unchanged. No customer firmware build, programming, application upload or NVM operation has occurred. On 17 November John reports external power active, PICkit connected and no active debugger session. Reconfirm physical clock/jumper/board settings before hardware tests.
