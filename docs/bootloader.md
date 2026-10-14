# USB HID bootloader: first configuration

The standalone LED1 baseline is preserved at annotated tag **[v0.1.0-led-blink](https://github.com/jharoian-mchp/PIC32MX270-USB-Bootloader-Example/tree/v0.1.0-led-blink)**, targeting commit **dd59a291254b45d44fba65810b2085d9b6ea40df**. The user confirmed production programming, expected blinking, and recovery after both MCLR and a power cycle on 2026-10-13.

The next stage is a **separate USB device HID bootloader**, followed by a relocated version of the blink application. The initial skeleton was pulled at **0e87595**. Core regeneration at **556a4fa** supplied the missing common/configuration/task files; the complete Linux XC32 bootloader build now passes. First USB HID enumeration is confirmed by user screenshots; ADC disconnect/reconnect and detach/reattach are now user-confirmed; host re-enumeration after MCLR/power cycle is now user-confirmed; update tests remain unverified. The current main now runs [enumeration-only diagnostics](usb-enumeration.md), with update tasks intentionally bypassed.

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
- `btl.ld`: program region starts at **0x9D000000**, length **32,768**, bootloader exception base **0x9D000000**; reset remains **0xBFC00000**. The regenerated -O2 build fits the reservation; see the build results below.
- Program `PAGE_SIZE` is **128 bytes** (row-buffer naming), ERASE_BLOCK_SIZE is **1,024 bytes**; the NVM PLIB agrees.
- Clocks/debug fuses match the plan, including UPLLEN ON, UPLLIDIV DIV_3 and ICS_PGx4.
- USB descriptors: VID/PID **04D8:003C**, endpoint-0 and HID IN/OUT packets **64 bytes**, Self Powered set, Remote Wakeup absent. Declared bus draw is currently 100 mA; actual consumption still needs review.
- Datastream code attaches on POWER_DETECTED and detaches on POWER_REMOVED. Board routing/physical VBUS sensing is not verified by that software path.

### Resolved build blocker: Harmony Core generation

At skeleton commit **0e87595**, the repository contained no `configuration.h`, `tasks.c`, `system/system.h`, system interrupt/common or driver-common/OSAL generated files. Required USB/bootloader headers included those missing files. `definitions.h` defined **SYS_Tasks() as an empty macro**, so that main loop would not service USB/bootloader tasks even if includes were patched manually.

Inspection of core **v3.17.1** generator scripts confirms that common/configuration/tasks generation is gated by switches which default OFF. In the **Harmony Core Service** component, keep **BareMetal/no RTOS** and enable:

- **Generate Harmony Driver Common Files** (`ENABLE_DRV_COMMON`).
- **Generate Harmony System Service Common Files** (`ENABLE_SYS_COMMON`).
- Verify **Enable System Interrupt**, **Enable OSAL** (bare-metal implementation) and **Generate Harmony Application Files** become enabled through dependencies.

These settings were applied by the user and regenerated at **556a4fa**. For future regeneration, check for `configuration.h`, `tasks.c`, `system/` headers/sources, driver-common/OSAL headers, and a real SYS_Tasks implementation calling the USB/bootloader task functions. Review any placeholder application files produced by the generator. Do not hand-create missing generated headers or reintroduce FreeRTOS just to satisfy these dependencies.

The original build failure was reproduced with XC32 v6.00/DFP 1.7.380. The user's Core regeneration at **556a4fa** resolved it without hand-created headers or local edits to generated files. `configuration.h`, common/system/driver/OSAL headers, `sys_int.c`, `tasks.c` and application placeholders are present. `SYS_Tasks()` now calls `bootloader_USB_DEVICE_HID_Tasks()`, `USB_DEVICE_Tasks()` and placeholder `APP_Tasks()`; USBFS is configured for interrupt mode.

### Linux build results — 2026-10-14

Run **`make bootloader`** from the repository root. It compiles all **23 project C sources** with XC32 v6.00, DFP 1.7.380, the host project's **-O2**, generated `btl.ld`, section garbage collection and 512-byte minimum heap. Outputs go under ignored `build/bootloader/`.

- Compile, link and Intel HEX generation pass; all 1,950 HEX records have valid checksums/lengths.
- `xc32-size`: **23,837 bytes text, 224 bytes data, 1,868 bytes BSS**. This total includes separate boot/reset/configuration sections; use linker/section addresses for program-region fit.
- Loadable program-flash sections occupy **23,429 bytes**, with highest end **0x9D005DFC**, below application start **0x9D008000**. The remaining address span is **8,708 bytes**. Boot reset/configuration records are in their separate device-defined regions.
- The existing standalone application and mocked tests still pass. This does not satisfy the outstanding application-regeneration follow-up.
- With -Wall/-Wextra there are many repeated unused-parameter diagnostics in generated USB template headers, plus enum/function-pointer conversions, intentional-looking fallthrough, old-style declarations, an initializer warning and six EVIC MMIO array-bounds diagnostics at -O2. These are not dismissed as universally harmless; review nontrivial diagnostics before hardware deployment. Generated sources were not patched or warning categories suppressed.

### Windows clean-build review — 2026-10-14

The user supplied a complete Windows clean-build log (chat attachment 81). It reports **BUILD SUCCESSFUL**, XC32 v6.00, DFP 1.7.380, -O2, -Wall/-Wextra, all 23 source files and the generated `btl.ld`. Its **1,080 warning occurrences** match the Linux category counts exactly:

| Diagnostic category | Occurrences |
|---|---:|
| Unused parameter | 1,058 |
| Enum conversion | 8 |
| EVIC MMIO array bounds | 6 |
| Function-pointer cast | 3 |
| Old-style declaration | 3 |
| Missing field initializer | 1 |
| Implicit fallthrough | 1 |

There are **147 distinct warning lines**; repeated generated inline headers account for much of the volume. This comparison verifies matching diagnostic categories, not that every warning is safe or that debug/production binaries are identical.

Host commit **fd97ec3** stores -Wall and -Wextra. At the time of that log the bootloader project selected **noID** as its tool. The log has `-D__DEBUG` but no `-mdebugger`, PICkit 5 macro or debugger memory reservations. Its final IDE 'Loading code' message therefore does **not** establish programming/debugging on the board. Host commit **0274bf9**, merged at **9fdac62**, now selects **PK5Tool**; -Wall/-Wextra are retained and the project XML validates. This fixes shared tool selection but is not a hardware programming result. Building is now possible, but **do not program the skeleton yet**: the earlier skeleton lacked board routing and valid physical-entry policy. User-owned enumeration-only diagnostics now implement/test mux/ADC attachment and force diagnostic mode, but hardware verification and full update safety remain open. Program only the diagnostic image under its controlled test instructions, not the older skeleton.

### Generated AN1 ADC review — 2026-10-14

The user confirmed 16 TAD acquisition and pushed **fce3569**. ADC_Initialize is called on the bootloader path; PMD1 enables the ADC. Generated AD1CON3=0x1017, AD1CON1=0x00E0 and AD1CHS=0x00010000 match 1 µs TAD/16 µs acquisition, software-started internal-counter conversion and AN1 input. The Linux complete **24-source** -O2 build/HEX passes. `xc32-size` reports 23,893 text, 224 data and 1,868 BSS; the build still fits the 32 KB program reservation.

The API provides ADC_SamplingStart, ADC_ResultIsReady and ADC_ResultGet; the internal-counter configuration must not be driven by manually clearing SAMP. AD1CON2 and RA1 analog/input selection rely on reset defaults in the generated code; retain explicit safe configuration when implementing the detector. First connected-state ADC reading is now confirmed at 622 counts and Windows HID enumeration passed; ADC-based detach/reattach now passes a user-observed unplug/replug cycle; MCLR/power-cycle removal and re-enumeration are also user-confirmed. See [the ADC source review](usb-circuit.md).

### Enumeration-only implementation — 2026-10-14

`board_usb_diag.c` now initializes/checks MCP23008 GP5/GP6, samples AN1 with tentative hysteresis, controls USB attachment from cable sense and exposes diagnostics. Main deliberately bypasses generated SYS_Tasks/update tasks; an always-true strong bootloader_Trigger prevents startup from jumping into old app bytes. The linked production diagnostic ELF contains no bootloader command/parser or NVM erase/write routine. Linux build, strict user-code check, mocked tests and sanitizer run pass. Generated files remain unchanged; public project XML includes the new files.

The first [diagnostic connection](usb-enumeration.md) passed: Windows VID_04D8/PID_003C and connected ADC/mux diagnostics shown. ADC unplug/replug and detach/reattach pass; the user also confirmed USB removal and re-enumeration after MCLR and power cycle. The initial enumeration milestone passes. Physical SW1 entry and update-path safety review are next; firmware updates remain inactive.

### Further update-path issues to resolve

The generated weak bootloader trigger returns false and its application-jump helper accepts an application based only on a non-erased first word. The diagnostic strong override always returns true; the later functional bootloader still needs a physical/valid-image policy. SW1 startup-gated diagnostics are now implemented and software-tested, with hardware verification pending. No-button startup stays safe idle; final application handoff and stronger valid-image/recovery policy remain unimplemented. The generated HEX parser also uses `<= FLASH_END_ADDRESS` even though flash end is exclusive, and reads a claimed record length without first checking it fits the available buffer. These need targeted review/tests before enabling host erase/program commands. Treat successful enumeration as a separate milestone, not approval of flash updating.

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

**U2 identification resolved:** the user corrected its pin count to **12** and supplied the NCN1188 marking/pinout excerpt. Official `AGM` marking defines AG as the device code and M as date code; observed AG7 is consistent. The earlier identification hold is cleared. The mini-B route uses **GP5=GP6=1**, preserving other pins. See [the updated circuit findings](usb-circuit.md).

The schematic-only path is **J2 → EMI filter U3 → NCN1188 HDN/HDP → MCU D+/D−**. Component identity is now established; measured routing remains to be checked. Host readings are J2 VBUS 5.16 V and VBUS_SENSE 2.011 V; the latter is below the GPIO high guarantee at 3.3 V VDD, so ADC-based detection is under consideration. The user subsequently measured **J2 connector VBUS = 0 V** with no host, as well as **VBUS_SENSE = 0 V**. The initial disconnected voltage check passes; actual U2 identification and host-attached voltage tracking remain unresolved.

**Important:** J2 cable VBUS is divided/clamped onto **RA1/PIM pin 80**, while the carrier feeds dedicated MCU VBUS/PIM pin 54 from **5V_SW_DIG**. The generated driver uses dedicated VBUS session status, so it must not be assumed to detect PC cable presence. Before coding/programming, confirm the installed revision/U2 part and measure disconnected J2 VBUS and RA1 sense safely; see the checklist and attached schematic crops in that review.

Use the [converted PIM and carrier references](docling/README.md), verifying small schematic details against the original PDFs as necessary.

- D+ is MCU pin 8 / PIM pin 57; D− is MCU pin 9 / PIM pin 56. Do not use their alternate PGED2/PGEC2 debug function.
- NCN1188 schematic-only review identifies GP5/GP6 both high for mini-B; R5/R6 20 kΩ pull-downs yield default disconnect. **The identification hold was cleared** after the user confirmed the 12-pin NCN1188/AG7 marking. Routing code and hardware behavior remain unverified.
- Host measurements: J2 VBUS 5.16 V, RA1 sense 2.011 V. Ordinary RA1 VIH(min)=0.65×VDD=2.145 V at nominal 3.3 V, so direct GPIO detection is not guaranteed. User confirmed MCU VDD 3.3 V, same measurement point and unplugged sense near 0 V. Next configure/review RA1/AN1 ADC sensing using AVDD/AVSS references, software-started sampling and adequate acquisition time; see the proposed settings in docs/usb-circuit.md. Do not enable host VBUS sourcing or allow back-feeding the PC.
- Keep J5 pins 1–2 (3.3 V pull-ups), J1 pins 1–2 (seven-bit expander address 0x22), and reset selector PIM_MCLR.
- Force-bootloader input: **SW1/RB2, MCU pin 23/PIM pin 17**, named **BOOT_REQUEST** in the user's pin screen (attachment 97). The carrier Figure A-8/source PDF page 56 shows **R18, 20 kΩ, pull-up to 3.3 V**, with SW1 pulling the signal to ground: **released=high, pressed=low**. See the [SW1 schematic detail](images/sw1-pullup-detail.png). Use digital GPIO input, internal pull-up/down disabled. Host commit c4d27a6 provides BOOT_REQUEST_Get and ANSELB bit 2 clear. User-owned initialization explicitly sets TRISB bit 2 and disables pulls. The [SW1 startup test](usb-entry.md) builds and passes mocks; hardware verification is next. Avoid SW5/USBID sharing for this design.
- Check the supplied USB UIDLE/suspend erratum against the selected USB driver. Record the actual MCU silicon revision when PICkit reports it.

First confirm routing and power, then USB descriptors/enumeration, then HID host communication. Keep erase/program operations disabled until those prerequisites and memory bounds pass.

## SW1 diagnostic startup gate

Host commit c4d27a6 regenerated RB2/BOOT_REQUEST. User-owned diagnostics sample active-low SW1 at 5 ms intervals; four consecutive lows inside a 50 ms window latch enumeration enabled. Without the request, startup stays safe idle with no USB client/tasks/mux changes, not application handoff. The strong trigger still prevents jumping into old application bytes. Builds, mocks and sanitizers pass; follow [the controlled SW1 test](usb-entry.md) next. Updates remain inactive.

## Windows host utility

The inspected upstream package includes **UnifiedHost 1.19.3**, whose [README](https://github.com/Microchip-MPLAB-Harmony/bootloader/blob/master/tools/UnifiedHost-1.19.3/readme.txt) describes PIC32 support and requires **Java 8 with JavaFX/OpenJFX**. Use the utility and VID/PID/protocol corresponding to the selected installed bootloader package. Its readiness and device connection have not been tested.

Record generated VID/PID, report sizes, host utility version and accepted firmware format before attempting an update. Do not treat a successful generic HID enumeration as proof that the update protocol is compatible. Use development identifiers only as permitted by the vendor; do not assign arbitrary production USB identifiers.

## Stage checklist

| Stage | Current result |
|---|---|
| Preserve production-tested standalone application | Tag v0.1.0-led-blink pushed and verified at dd59a29. |
| Create bootloader MCC project and identify package versions | Generated; bootloader 3.9.0, USB 3.16.0, core 3.17.1 and CSP 3.26.1 recorded. |
| Regenerate/retest application after H3 updates | Required follow-up; not performed. Preserve the tagged baseline and user-owned LED code. |
| Review generated memory and clock configuration | Core regeneration 556a4fa resolves missing files/tasks; complete 23-source Linux -O2 build/HEX passes and fits 32 KB. Physical 0x1D008000 maps to 0x9D008000. Hardware not tested. |
| Windows skeleton clean build | Passed in supplied log with -O2/-Wall/-Wextra; warning categories match Linux. PK5Tool now selected via 0274bf9/9fdac62; hardware programming not verified. |
| Review/program USB connector routing and power | Documented mini-B HDN/HDP/GP5=GP6=1 path and RA1 cable-sense versus dedicated board VBUS reviewed. Physical voltage/revision checks and code implementation pending; see docs/usb-circuit.md. |
| Enumerate as USB HID | User screenshots confirm Windows VID_04D8/PID_003C and connected ADC 622/mux readback. ADC unplug/replug and detach/reattach confirmed; host removal/re-enumeration after MCLR and power cycle confirmed. Earlier cable-replug configured flag not captured. Update host utility unused. |
| Relocate blink application and verify firmware update | Not started. |
| Forced entry, missing/invalid image and interrupted-update recovery | Not tested. |
