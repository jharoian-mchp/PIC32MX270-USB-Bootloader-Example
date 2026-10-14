# USB HID bootloader: first configuration

The standalone LED1 baseline is preserved at annotated tag **[v0.1.0-led-blink](https://github.com/jharoian-mchp/PIC32MX270-USB-Bootloader-Example/tree/v0.1.0-led-blink)**, targeting commit **dd59a291254b45d44fba65810b2085d9b6ea40df**. The user confirmed production programming, expected blinking, and recovery after both MCLR and a power cycle on 2026-10-13.

The next stage is a **separate USB device HID bootloader**, followed by a relocated version of the blink application. The initial bootloader skeleton is generated and pulled at **0e87595**. Its Linux build is blocked by missing Harmony common/configuration/task files; no bootloader binary or hardware test has passed yet.

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

The 2026-10-14 Clock Configurator screenshot confirms **UPLLIDIV DIV_3, UPLLEN ON, UFRCEN OFF**, a **4 MHz USB PLL input** and **48 MHz USB clock**. The system PLL shows input ÷3, multiplier ×24 and output ÷2 for **48 MHz SYSCLK**; the earlier calculated-frequency screen shows **48 MHz PBCLK**. These are correct screen settings for the confirmed 12 MHz crystal; generated code, peripheral enable and USB hardware behaviour still need review/testing.

The bootloader component screenshot shows USB Device HID Bootloader with an NVM dependency and firmware-trigger option. Leave firmware-triggered entry disabled for the initial physical-button design. Bootloader reservation/application start are not yet reviewed.

The updated 2026-10-14 Project Graph shows NVM, USB Device HID Bootloader, USB Device Layer/HID Function Driver, USB Full Speed Driver, I²C1 and Harmony Core, with **FreeRTOS removed**. Retain Harmony Core if the USB stack requires it; it is not itself FreeRTOS. The USB device screen shows endpoint-0 buffer size 64 and the bootloader Product ID selection; exact VID/PID and Remote Wakeup state still need verification.

The user confirmed an **external power supply powers the board**. Set **Self Powered checked**: unchecked describes bus-powered operation. This descriptor setting does not enable host VBUS sourcing and does not replace USB-host VBUS detection. Verify the actual power/USB circuit prevents back-feeding the PC and set the USB current declaration from actual bus consumption. Keep Remote Wakeup disabled for the initial bring-up unless it is deliberately implemented and tested.

The first build objective is successful **USB enumeration and a host connection**, not a firmware erase/write operation. Do not program a bootloader until generated code, memory bounds and board USB power/routing have been reviewed.

## Generated skeleton review — 2026-10-14

Saved package metadata records **bootloader v3.9.0**, **USB v3.16.0**, **Harmony core v3.17.1** and **CSP v3.26.1**. The tagged application still records CSP v3.26.0; its regeneration follow-up remains open.

Verified in generated sources:

- `bootloader_common.h`: APP_START_ADDRESS and APP_JUMP_ADDRESS use **PA_TO_KVA0(0x1D008000)**, corresponding to **0x9D008000**. The physical address shown in MCC is correct.
- `btl.ld`: program region starts at **0x9D000000**, length **32,768**, bootloader exception base **0x9D000000**; reset remains **0xBFC00000**. Actual binary fit cannot be checked until the build succeeds.
- Program `PAGE_SIZE` is **128 bytes** (row-buffer naming), ERASE_BLOCK_SIZE is **1,024 bytes**; the NVM PLIB agrees.
- Clocks/debug fuses match the plan, including UPLLEN ON, UPLLIDIV DIV_3 and ICS_PGx4.
- USB descriptors: VID/PID **04D8:003C**, endpoint-0 and HID IN/OUT packets **64 bytes**, Self Powered set, Remote Wakeup absent. Declared bus draw is currently 100 mA; actual consumption still needs review.
- Datastream code attaches on POWER_DETECTED and detaches on POWER_REMOVED. Board routing/physical VBUS sensing is not verified by that software path.

### Build blocker: Harmony Core generation

The repository contains no `configuration.h`, `tasks.c`, `system/system.h`, system interrupt/common or driver-common/OSAL generated files. Required USB/bootloader headers include these missing files. `definitions.h` still defines **SYS_Tasks() as an empty macro**, so the current main loop would not service the USB/bootloader tasks even if includes were patched manually.

Inspection of core **v3.17.1** generator scripts confirms that common/configuration/tasks generation is gated by switches which default OFF. In the **Harmony Core Service** component, keep **BareMetal/no RTOS** and enable:

- **Generate Harmony Driver Common Files** (`ENABLE_DRV_COMMON`).
- **Generate Harmony System Service Common Files** (`ENABLE_SYS_COMMON`).
- Verify **Enable System Interrupt**, **Enable OSAL** (bare-metal implementation) and **Generate Harmony Application Files** become enabled through dependencies.

Regenerate through MCC and push the result. Check for `configuration.h`, `tasks.c`, `system/` headers/sources, driver-common/OSAL headers, and a real SYS_Tasks implementation calling the USB/bootloader task functions. Review any placeholder application files produced by the generator. Do not hand-create missing generated headers or reintroduce FreeRTOS just to satisfy these dependencies.

The build failure has been reproduced with XC32 v6.00/DFP 1.7.380. The original generated files remain unchanged; no bootloader firmware is ready to program.

### Further update-path issues to resolve

The weak bootloader trigger currently returns false, and startup accepts an application based only on a non-erased first word. Physical SW1 entry and stronger valid-image/recovery policy are not implemented yet. The generated HEX parser also uses `<= FLASH_END_ADDRESS` even though flash end is exclusive, and reads a claimed record length without first checking it fits the available buffer. These need targeted review/tests before enabling host erase/program commands. Treat successful enumeration as a separate milestone, not approval of flash updating.

## Memory contract to establish

The exact device has **256 KB program flash, 3 KB boot flash, 1,024-byte erase pages and 128-byte program rows**. The HID stack cannot be assumed to fit in its 3 KB boot flash.

An upstream Harmony source review on 2026-10-13 found:

- [`bootloader_mips.py`](https://github.com/Microchip-MPLAB-Harmony/bootloader/blob/master/config/bootloader_mips.py) places the PIC32MX HID bootloader in program flash, starting at 0x9D000000, and aligns the proposed application start to flash erase size.
- [`bootloader_usb_device_hid.py`](https://github.com/Microchip-MPLAB-Harmony/bootloader/blob/master/config/bootloader_usb_device_hid.py) currently declares a **32,768-byte PIC32MX reservation**.
- [`module.py`](https://github.com/Microchip-MPLAB-Harmony/bootloader/blob/master/config/module.py) declares MEMORY and USB_DEVICE_HID dependencies for this component.

These are observations of upstream `master`, which can change. On 2026-10-14, the user reported the installed bootloader component's **Bootloader Size = 32,768 bytes**, matching the reference reservation, and firmware-triggered entry **unchecked**. This size is a reservation, not the measured binary size.

The reference generator sets **core.APP_START_ADDRESS**, so the application address belongs to **System/core**, not necessarily the bootloader component UI. Look under System's device/project settings. If absent in the UI, inspect the generated linker symbols and bootloader headers. With a bootloader origin of 0x9D000000 and a 0x8000 reservation, the calculated candidate application start is **0x9D008000**, aligned to 1 KB pages. This is provisional until the installed package's generated layout/map confirms it; it is not a directive to relocate the current standalone application now.

Save/generate the bootloader skeleton and share it for review after setting the appropriate USB device power descriptor. Generating files is safe; **do not program it yet**. Verify bootloader/application reset, exception and configuration ownership before accepting the calculated candidate address.

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
| Review generated memory and clock configuration | Physical 0x1D008000 mapped to 0x9D008000, 32 KB reservation, fuses/geometry and descriptors reviewed. Build blocked by missing Harmony common/configuration/task files. |
| Review/program USB connector routing and power | Not verified. |
| Enumerate as USB HID and connect host utility | Not tested. |
| Relocate blink application and verify firmware update | Not started. |
| Forced entry, missing/invalid image and interrupted-update recovery | Not tested. |
