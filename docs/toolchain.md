# Windows generation and Linux builds

Use **XC32 v6.00** on both Windows and Linux. Generate firmware with **MCC v5.7.0 on the Windows host**; use **MPLAB X v6.35 and PICkit 5** for host builds, programming and debugging.

Recorded: **2026-10-13**. Standalone MCC v5.7.0 is the requested generator. MPLAB X's included MCC v5.7.0 is available if the standalone project workflow needs IDE integration; record the workflow actually used. No MCC firmware project has been generated yet.

## Selected tools

| Component | Windows host | Linux workspace | Verification |
|---|---|---|---|
| Compiler | XC32 v6.00, reported by the user | XC32 v6.00 installed | Linux exact-device compile, link and HEX generation passed. |
| Generator | Standalone MCC v5.7.0; IDE also includes MCC v5.7.0 | No MCC installation needed for the current split | Host generation not tested. |
| IDE | MPLAB X v6.35 | No IDE installed | Host programmer/debugger connection not tested. |
| Programmer/debugger | PICkit 5 | No hardware connection | Target identification and silicon revision not read yet. |
| Generator device pack | MCC/Harmony uses PIC32MX_DFP 1.6.369, confirmed by the user | Not needed for compilation here | Record independently from the build device pack; do not force a version match. |
| Build device pack | MPLAB X uses PIC32MX_DFP 1.7.380, confirmed by the user | PIC32MX_DFP 1.7.380 installed | Linux exact-device compiler check passed; host-generated application build not tested. |
| Harmony packages | Versions to be selected and recorded in MCC | Match host-generated package versions as required | Device-specific generation not verified. |

XC32 v6.00's MIPS release notes require MPLAB X v6.25 or later, with applicable update-centre patches. v6.35 satisfies the documented minimum version; the installed host setup still needs a real build and PICkit 5 connection check. The same release notes state that XC32 v6.00 removes licence checking for PRO optimisations and Code Coverage; no compiler activation was needed for the Linux `-O2` smoke test.

## Linux installation

Installed outside the Git repository, under persistent `/workspace/ChrisK/tools/`:

```text
/workspace/ChrisK/tools/xc32/v6.00/
/workspace/ChrisK/tools/packs/Microchip/PIC32MX_DFP/1.7.380/
```

Downloaded from Microchip over HTTPS:

| Package | Source | SHA-256 of downloaded file |
|---|---|---|
| XC32 v6.00 Linux x64 installer | [Microchip installer](https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/SoftwareTools/xc32-v6.00-full-install-linux-x64-installer.run) | `18e28f71a2ce3ff4ced7fe76cef8bf22a8e65b968ed48481c296c82adffc806d` |
| PIC32MX_DFP 1.7.380 | [Microchip device pack](https://packs.download.microchip.com/Microchip.PIC32MX_DFP.1.7.380.atpack) | `bf7e8ad4b707efb98eb3b142954405e4dc389af3fc73c67ec05e817a0ae79d04` |

These hashes record the retrieved files; they were not compared with a separately published vendor checksum. The installer required elevated privileges even with `--ModifyAll 0`; it was run with `sudo` at the selected prefix. It did not add XC32 to the global PATH. The compiler installation uses about 8.8 GB. Installer/archive downloads and temporary tests were removed after verification.

Reproduction commands, after downloading the packages:

```sh
sudo ./xc32-v6.00-full-install-linux-x64-installer.run \
  --mode unattended --unattendedmodeui none \
  --prefix /workspace/ChrisK/tools/xc32/v6.00 --ModifyAll 0

mkdir -p /workspace/ChrisK/tools/packs/Microchip/PIC32MX_DFP/1.7.380
unzip -q Microchip.PIC32MX_DFP.1.7.380.atpack \
  -d /workspace/ChrisK/tools/packs/Microchip/PIC32MX_DFP/1.7.380
```

For this XC32 v6.00 installation, `-mdfp` must point to the **pack root**, not its `xc32/` child. A trial using the child directory failed to locate device resource/startup files; the pack-root invocation passed.

## Reproducible toolchain check

From the repository root:

```sh
make toolchain-check
```

Override installation locations if needed:

```sh
make toolchain-check \
  XC32_ROOT=/path/to/xc32/v6.00 \
  PIC32MX_DFP_ROOT=/path/to/PIC32MX_DFP/1.7.380
```

The target compiles `tools/toolchain-smoke.c` for `32MX270F256D` using `-O2 -Wall -Wextra -Werror`, links with the device pack, creates a map file and converts the ELF to Intel HEX. It checks that the HEX is non-empty. Outputs go under ignored `build/toolchain-smoke/`.

The test exercises `<xc.h>`, the exact-device preprocessor symbol, startup/linker files and configuration names including `DIV_3`, `UPLLEN = ON` and `ICS_PGx4`. It is **not MCC-generated**, does not blink an LED and must **not be programmed onto the board**. It does not validate USB, debug wiring, power or any selected hardware configuration.

## Clock plan with the confirmed crystal

The user read **12.00 MHz** on the installed Y1 on 2026-10-13. This resolves the guide's 8 MHz pin-table / 12 MHz bill-of-materials discrepancy for this board. It is a confirmed marking, not a frequency measurement.

Proposed MCC clock settings for the first application:

| Setting | Proposed value | Reason |
|---|---|---|
| Primary source | 12 MHz crystal; HS oscillator mode | Match installed Y1; verify the schematic clock path and device limits. |
| CPU clock source | Primary oscillator with PLL (`PRIPLL`) | Exercise the external source used for later USB bring-up. |
| System PLL input divider | ÷3 | 12 MHz / 3 = 4 MHz. |
| System PLL multiplier | ×24 | 4 MHz × 24 = 96 MHz. |
| System PLL output divider | ÷2 | 96 MHz / 2 = 48 MHz SYSCLK, below the part's 50 MHz rating. |
| Peripheral bus clock | Proposed ÷1 = 48 MHz | Verify MCC-generated settings and peripheral limits. |
| USB PLL input divider | ÷3 | 12 MHz / 3 = required 4 MHz UPLL input. |
| USB clock | 48 MHz via USB PLL | Enable for the HID project; keep the USB peripheral disabled in the initial application. |

MCC must generate the configuration and appropriate flash wait-state/cache settings. The first application still requires physical oscillator/startup validation. A nominal 48 MHz FRCPLL configuration is a fallback for isolating a primary-clock fault, not the selected USB clock source.

## Windows first-project handoff

1. Open MPLAB X v6.35 and confirm XC32 v6.00 appears as an available compiler. Apply relevant IDE updates and confirm PICkit 5 support.
2. Confirm the **PIC32MX270F256D** target. Retain MCC/Harmony's generator DFP **1.6.369** and MPLAB X's build DFP **1.7.380**, as reported by the user. Record them separately; they do not need to match. Linux builds use build DFP 1.7.380.
3. Open standalone MCC v5.7.0 and select its **Harmony 3** workflow for the exact device. Record the Harmony package versions. If that device/workflow is unavailable, stop and report the available choices; do not substitute a different MCU.
4. Create the initial project under the host working copy's `application/` directory. Save the actual generator project/configuration and generate the supported build project. Use [the first-application checklist](bring-up.md) for I²C1, LED1, timebase and electrical checks.
5. Send a screenshot or the available device/project workflow choices if the standalone wizard does not produce a project MPLAB X can open. Use IDE-hosted MCC only as an explicitly recorded workflow change.
6. Retain the generated sources, public MPLAB X project metadata, makefiles and generator configuration. Keep private metadata and build outputs out of Git. Avoid machine-specific absolute paths in shared project files where the tools permit it.
7. Share the generated project through the repository. The assistant will check Linux portability and build it with the matching XC32/device pack. Any Harmony checkout paths or generated makefile tool paths need review; cross-platform builds are not verified yet.
8. Program and debug on Windows with PICkit 5 after the board reset/debug routing, expander address, pull-up voltage and LED polarity are checked.

## MCC configuration review

On 2026-10-13, the user confirmed generator DFP 1.6.369 and MPLAB X build DFP 1.7.380. Clock screenshots show a 12 MHz primary source, HS/PRIPLL, system PLL input ÷3, multiplier ×24 and output ÷2, with 48 MHz SYSCLK and PBCLK (bus divider ÷1). This is a screen review, not verification of generated code or hardware clocks.

The initial USB PLL setting is OFF. It can stay OFF while USB is unused; set its input divider to ÷3 and enable the PLL when preparing the HID project. Do not disable unrelated peripheral clocks wholesale before the required timebase and generated dependencies are known.

The subsequent I²C screenshots show **I²C1 Peripheral Library**, **Master** mode and **100,000 Hz**, with **SDA1 on RB9** and **SCL1 on RB8** in the pin settings. The PIM reference maps these to MCU pins 1 and 44. This is a screen review only; actual timing, generated pin setup and hardware communication have not been checked. Keep Ignore NACK disabled so an absent or incorrectly addressed expander can be detected. The I²C peripheral handles the bus pins; do not configure them as manually driven GPIO outputs. Check external pull-ups and their selected voltage before hardware testing.

Next, add a **TMR2 Peripheral Library** timebase: internal PBCLK, 16-bit periodic mode, prescaler **1:64**, period **10 ms**, with its interrupt enabled and gated/external-clock modes disabled. At 48 MHz PBCLK, the calculated count is `48,000,000 / 64 × 0.010 = 7,500`, so the period register should be **7,499**. Count 50 timer events in application logic to request each 500 ms LED state change; do not perform I²C transfers in the timer interrupt. The subsequent TMR2 screenshot shows internal peripheral clock, 1:64 prescaler, 750,000 Hz timer clock, 10 ms period and period register 7,499. Confirm its interrupt is enabled; generated code and hardware timing are not yet verified.

Schematic review confirmed carrier I²C1 pull-ups R10/R11 (3.30 kΩ each), powered through J5. Set J5 pins 1–2 for 3.3 V; pins 2–3 select 5 V. The PIM does not show its own SDA/SCL pull-up pair. See [the board findings and schematic detail](bring-up.md). Installed jumper position and resistor population still need physical confirmation.

The remaining inputs are carrier/PIM revision, other Harmony package versions and the expander/electrical checks. The MCP23008 component data sheet is also needed to verify register definitions before writing the LED-expander driver.
