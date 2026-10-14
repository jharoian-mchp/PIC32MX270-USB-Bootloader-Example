# USB HID bootloader: first configuration

The standalone LED1 baseline is preserved at annotated tag **[v0.1.0-led-blink](https://github.com/jharoian-mchp/PIC32MX270-USB-Bootloader-Example/tree/v0.1.0-led-blink)**, targeting commit **dd59a291254b45d44fba65810b2085d9b6ea40df**. The user confirmed production programming, expected blinking, and recovery after both MCLR and a power cycle on 2026-10-13.

The next stage is a **separate USB device HID bootloader**, followed by a relocated version of the blink application. No bootloader firmware has been generated, built or tested yet.

## Preserve the working application

- Keep `application/` as the working standalone application during USB bring-up.
- Create the bootloader project separately under **`bootloader/`**. Do not convert the existing application's MCC configuration into a bootloader project.
- Do not overwrite or move the milestone tag. It marks the exact user-tested code and host compiler-option configuration, before later planning/documentation changes.
- Recover through PICkit 5 using the tagged application if a bootloader build does not work. The current standalone application is not yet suitable for installation through the future bootloader: its startup/linker/configuration layout needs adaptation first.

## H3 content update follow-up — 2026-10-14

The user reported that adding the bootloader library caused updates to host Harmony content. **Regenerate the application too, after the package updates settle.** The package-selection screenshots are not proof of the final installed versions; record versions from the resulting saved configurations. The tagged baseline used CSP v3.26.0; do not silently relabel it with the new versions.

Application regeneration checklist:

1. Save/commit the current work before regeneration and leave **v0.1.0-led-blink** unchanged.
2. Reopen the application's own MCC configuration and regenerate using the updated relevant H3 packages. Do not copy bootloader configuration into the standalone application or change its linker offset in this regeneration step.
3. Review generated diffs and retain `app_led.c`/`.h`, `APP_LED_Initialize()`/`APP_LED_Tasks()` integration, project source entries, PK5Tool, XC32/DFP selections and -Wall/-Wextra options. Review any MCC overwrite prompt rather than accepting it blindly.
4. Record package versions separately for application and bootloader. Check clock/I²C/timer configuration and generated API changes before adapting user-owned code.
5. Run Linux `make test application`, Windows Clean/Build, then repeat production LED blink, MCLR and power-cycle tests. The old hardware pass does not verify regenerated firmware.
6. Commit regenerated output and the new test results. This follow-up is **required and not yet performed**.

## Host MCC setup: next action

1. Create a new standalone MCC/Harmony project for **PIC32MX270F256D** under `bootloader/`, using XC32 v6.00 and the existing host workflow. Record the selected bootloader and USB package versions; retain generator DFP 1.6.369 and build DFP 1.7.380 unless the installed package requires a documented change.
2. Reuse the reviewed clock settings: **12 MHz primary crystal, HS, PRIPLL, system PLL input ÷3, multiplier ×24, output ÷2, 48 MHz SYSCLK/PBCLK**.
3. For USB, set **UPLLIDIV = DIV_3** and **UPLLEN = ON**. The USB PLL input is 4 MHz from the 12 MHz crystal; the peripheral needs its resulting 48 MHz USB clock.
4. Keep WDT and development code/write protection disabled, JTAG disabled, and **ICESEL = ICS_PGx4**. Revisit protection after the memory contract and update path are verified.
5. Add the **USB Device HID Bootloader** component from the installed bootloader package. Let MCC connect its required **MEMORY/NVM** and **USB Device HID** dependencies. Do not substitute a USB host/MSD bootloader or an arbitrary HID demonstration protocol.
6. Keep/add **I²C1 master at 100 kHz**, SDA RB9 and SCL RB8, for the PIM's MCP23008 board-control outputs. It may be needed to establish USB connector routing before enumeration; the existing LED module deliberately preserves GP5/GP6 and does not select a USB path.
7. Send the **Project Graph and bootloader component settings** next, including its package version, bootloader reservation and proposed application address. Review these before generating or programming. If the component is unavailable for the exact device, send the available components/package versions and stop rather than selecting another MCU.

The 2026-10-14 bootloader clock screenshot shows **48 MHz system/CPU/peripheral bus clocks**. Its USB Clock Configuration section is collapsed, so USB PLL enable/input divider and the 48 MHz USB clock have not yet been certified from that screen. Review the expanded USB section or Clock Configurator next.

The first build objective is successful **USB enumeration and a host connection**, not a firmware erase/write operation. Do not program a bootloader until generated code, memory bounds and board USB power/routing have been reviewed.

## Memory contract to establish

The exact device has **256 KB program flash, 3 KB boot flash, 1,024-byte erase pages and 128-byte program rows**. The HID stack cannot be assumed to fit in its 3 KB boot flash.

An upstream Harmony source review on 2026-10-13 found:

- [`bootloader_mips.py`](https://github.com/Microchip-MPLAB-Harmony/bootloader/blob/master/config/bootloader_mips.py) places the PIC32MX HID bootloader in program flash, starting at 0x9D000000, and aligns the proposed application start to flash erase size.
- [`bootloader_usb_device_hid.py`](https://github.com/Microchip-MPLAB-Harmony/bootloader/blob/master/config/bootloader_usb_device_hid.py) currently declares a **32,768-byte PIC32MX reservation**.
- [`module.py`](https://github.com/Microchip-MPLAB-Harmony/bootloader/blob/master/config/module.py) declares MEMORY and USB_DEVICE_HID dependencies for this component.

These are observations of upstream `master`, which can change. They are **not verified values from the user's installed bootloader package**. Do not adopt the reservation or an application start address until its package/configuration and generated map are reviewed.

Before flash updates, agree on:

- Bootloader program region and actual build size, aligned to 1 KB erase pages.
- Application reset entry, exception/vector region and linker layout; no application section may overlap bootloader or configuration storage.
- Configuration-bit ownership and restrictions on host writes to boot/configuration areas.
- Accurate 1 KB erase-page and 128-byte row handling, including partial last-row padding and range checks.
- App image acceptance criteria and behaviour after an interrupted update. A non-erased reset word is not evidence of a complete/valid image.
- Handoff cleanup: interrupt sources, USB state, stack/startup and exception-base requirements of the relocated application.

Do not enable host configuration-bit programming or live update for this first bring-up. RAM-triggered entry is unnecessary initially; use a physical button once its mapping is verified.

## Board USB checks before enumeration

Use the [converted PIM and carrier references](docling/README.md), verifying small schematic details against the original PDFs as necessary.

- D+ is MCU pin 8 / PIM pin 57; D− is MCU pin 9 / PIM pin 56. Do not use their alternate PGED2/PGEC2 debug function.
- The carrier has a USB multiplexer/charge-management path. Resolve how **MCP23008 GP5 (PIM 89)** and **GP6 (PIM 90)** select the mini-B device connector and how their default states affect routing. The precise output byte/truth table is not yet approved.
- Review VBUS sensing on RA1 and the dedicated VBUS pin's actual circuit. Do not enable host VBUS sourcing or allow back-feeding the PC.
- Keep J5 pins 1–2 (3.3 V pull-ups), J1 pins 1–2 (seven-bit expander address 0x22), and reset selector PIM_MCLR.
- Proposed force-bootloader input: **SW1/RB2, MCU pin 23/PIM pin 17**. Verify active level and make its analog-capable pin digital before use. Avoid SW5/USBID sharing for this first design.
- Check the supplied USB UIDLE/suspend erratum against the selected USB driver. Record the actual MCU silicon revision when PICkit reports it.

First confirm routing and power, then USB descriptors/enumeration, then HID host communication. Keep erase/program operations disabled until those prerequisites and memory bounds pass.

## Windows host utility

The inspected upstream package includes **UnifiedHost 1.19.3**, whose [README](https://github.com/Microchip-MPLAB-Harmony/bootloader/blob/master/tools/UnifiedHost-1.19.3/readme.txt) describes PIC32 support and requires **Java 8 with JavaFX/OpenJFX**. Use the utility and VID/PID/protocol corresponding to the selected installed bootloader package. Its readiness and device connection have not been tested.

Record generated VID/PID, report sizes, host utility version and accepted firmware format before attempting an update. Do not treat a successful generic HID enumeration as proof that the update protocol is compatible. Use development identifiers only as permitted by the vendor; do not assign arbitrary production USB identifiers.

## Stage checklist

| Stage | Current result |
|---|---|
| Preserve production-tested standalone application | Tag v0.1.0-led-blink pushed and verified at dd59a29. |
| Create bootloader MCC project and identify package versions | Bootloader library added; host content updates reported. Final installed versions not recorded in the repository yet. |
| Regenerate/retest application after H3 updates | Required follow-up; not performed. Preserve the tagged baseline and user-owned LED code. |
| Review generated memory and clock configuration | Not generated. |
| Review/program USB connector routing and power | Not verified. |
| Enumerate as USB HID and connect host utility | Not tested. |
| Relocate blink application and verify firmware update | Not started. |
| Forced entry, missing/invalid image and interrupted-update recovery | Not tested. |
