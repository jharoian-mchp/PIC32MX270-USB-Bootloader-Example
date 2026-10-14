# Enumeration-only USB diagnostic test

This build verifies **AN1 cable sensing, MCP23008 mini-B routing and USB enumeration**. It deliberately does **not** run firmware-update commands or hand off to an application. Linux builds and mocked tests pass. User screenshots on 2026-10-14 confirm connected-state ADC/mux diagnostics and Windows HID enumeration; unplug/replug and reset/cold-start behavior remain to be checked.

## What changed

User-owned `bootloader/src/board_usb_diag.c`/`.h` is added to the MPLAB X project. `main.c` calls its initialiser/tasks instead of generated SYS_Tasks. All MCC-generated files under `config/default/` remain unchanged.

The diagnostic code:

- Overrides the weak `bootloader_Trigger()` to **always stay in diagnostics**, even if old application bytes remain in flash. SW1 is not needed for this stage; physical entry is a later milestone.
- Explicitly retains RA1 as AN1 analog/input, disables its pull-ups/pull-downs and selects AD1CON2=0 (AVDD/AVSS reference and no scan). Generated ADC setup supplies ADCS=23/SAMC=16/SSRC=111.
- Uses CP0 Count at SYSCLK/2 for a main-loop timebase, so no extra timer component is needed. Unsigned elapsed arithmetic handles counter wrap when serviced regularly.
- Samples AN1 about every **10 ms** using ADC_SamplingStart and polled completion; no manually cleared SAMP trigger. Initial/recovery sampling waits 10 ms, exceeding ADC startup time.
- Uses tentative **465-count connect / 248-count disconnect** hysteresis for the measured 3.3 V reference. Three consecutive connect samples are required; a low sample detaches promptly. These thresholds distinguish measured presence, not a complete USB VBUS-compliance qualification.
- Reads expander OLAT/IODIR, clears only GP5/GP6 latch bits to establish disconnect, makes only those pins outputs, sets both high, then reads back latch/direction. **GP0–GP4 and GP7 are preserved.** I²C failures have a 100 ms deadline and one-second retry; no electrical bus-clear algorithm is claimed.
- Opens its own USB device handle. It **ignores generated POWER_DETECTED/REMOVED events** for attachment because the dedicated VBUS input is board-powered. Only qualified ADC presence plus successful mux readback enables the D+ pull-up; ADC timeout detaches.
- Calls USB_DEVICE_Tasks directly. It does **not** call generated SYS_Tasks, the bootloader protocol tasks, DATASTREAM tasks or NVM write/erase APIs. No HID report receive queue is submitted. Basic HID idle requests are supported for enumeration.

The generated HID descriptor/VID/PID are unchanged: **04D8:003C**, self-powered, 64-byte reports. A working enumeration does not prove host-update compatibility.

## Build and software verification

```sh
make test
make bootloader
```

Mocked diagnostic tests cover disconnected mux preload, readback, unrelated pins, asynchronous buffers, NACK/collision/retry/timeout, ADC cadence and timeout, hysteresis/qualification, CP0 wrap, and no attachment from dedicated power events. The test mock intentionally has no flash/update APIs. AddressSanitizer/UndefinedBehaviorSanitizer execution also passed.

The XC32 v6.00/DFP 1.7.380 -O2 diagnostic build reports **21,493 bytes text, 80 bytes data, 760 bytes BSS**. Generated-library warnings remain; user-owned diagnostic code compiles with strict warnings after excluding repeated generated-header unused-parameter diagnostics. Section bounds and HEX integrity are checked before delivery.

Linked ELF inspection confirms **bootloader command/parser and NVM erase/write routines are absent**. NVM_Initialize and the generated run_Application helper remain because generated initialization references them. The strong, tested always-true bootloader_Trigger skips that application's handoff branch; the helper's presence is not claimed to be absent. This is a production-build verification; check the host build includes board_usb_diag.c and that no manual alteration restores SYS_Tasks/update calls.

## Controlled board test

Only program the diagnostic build described here; **do not program an older generated skeleton or use the Unified Bootloader Host to send commands**. This test installs normal reset/configuration words and reserves the bootloader program region. It may erase the working standalone application depending on MPLAB programming settings. Recovery is through PICkit 5 and the immutable **v0.1.0-led-blink** application tag.

1. Pull/reopen **bootloader/USB_HID_bootloader.X** in MPLAB X. Confirm board_usb_diag.c is in the source list, XC32 v6.00, DFP 1.7.380, PICkit 5 and -Wall/-Wextra. Do not regenerate MCC just to build; if regeneration is necessary, preserve the diagnostic main integration/project entries.
2. Clean/Build and send any errors/new diagnostics. Prefer a production/Run test first; use Debug Project when inspecting variables. Avoid stepping/breakpoints repeatedly through ADC/USB tasks: halting disturbs timing and can create timeouts/disconnects.
3. Keep external board supply, PICkit target powering disabled, PIM_MCLR, J5 pins 1–2 (3.3 V), J1 pins 1–2 (0x22), and **Type-A unused**. The reviewed NCN1188 identification and no-host zero-voltage checks apply to this test.
4. With the PC USB cable disconnected, program this diagnostic image through PICkit 5. It has no LED heartbeat; the earlier D5 blink is not expected unless a prior expander state remains. Inspect the diagnostics if needed: ADC should sample near zero, mux_ready should eventually become true, and attached/configured should be false.
5. Connect a data-capable cable from the PC to **mini-B J2**. Expect raw ADC near the estimate **623–624 counts** for the reported 2.011 V/3.3 V, cable_present true after qualification, mux_ready true, then attached/configured true. Actual measured ADC counts take precedence.
6. In Windows Device Manager, check for the HID device with hardware IDs **VID_04D8&PID_003C**. No serial string is currently generated, so test one board at a time. Do not start an update utility yet.
7. Unplug J2 and confirm raw ADC returns near zero, cable_present/attached/configured become false, and detach_count increments. Replug to confirm re-enumeration. Record MCLR and cold-start behavior both with and without the USB cable.
8. Report ADC disconnected/connected values, mux readback, enumeration/VID/PID, disconnect/reconnect result and errors. If it fails, keep the attached cable/power arrangement unchanged until recorded, then stop and diagnose rather than bypassing sense thresholds or forcing attach.

## Watch `board_usb_diagnostics`

| Field | Expected use |
|---|---|
| adc_raw / adc_samples | Latest 10-bit value / successful sample count. |
| adc_min / adc_max | Extrema since boot, useful for margin checks. |
| adc_timeouts | Should remain zero in a freely running test. |
| mux_latch / mux_direction | Readback; latch bits 5–6 = 1, direction bits 5–6 = 0. Other bits depend on retained expander state. |
| mux_ready | True only after both readbacks match. |
| mux_errors | Should remain zero; covers transfer timeout/NACK/collision/readback failure. |
| cable_present | ADC-qualified presence state, not dedicated VBUS status. |
| usb_open / attached / configured | Open handle, enabled pull-up, completed host configuration. |
| suspended | USB suspend event; no remote wakeup implementation. |
| attach_count / detach_count / usb_resets / usb_errors | Event/operation counters. |
| last_fault | NONE=0, I2C=1, MUX_READBACK=2, ADC_TIMEOUT=3, USB_OPEN=4. Historical until reset; counters identify recurrence. |

The board experiment must establish the raw ADC values and cable-state behavior before thresholds are considered accepted. No firmware update, flash protection, valid-image guarantee, SW1 entry or bootloader-to-application handoff is verified in this stage.

## Observed first enumeration — 2026-10-14

User screenshots (chat attachments 93/94) show:

- ADC raw/max **0x026E = 622 counts**, consistent with the meter reading near 2.011 V at a 3.3 V reference.
- **adc_timeouts=0, mux_errors=0, usb_errors=0**, cable_present=true and usb_open=true.
- **mux_latch=0x60**: GP5/GP6 high. **mux_direction=0x9F**: those two bits are outputs.
- **attach_count=1, detach_count=0, usb_resets=2** at capture.
- Windows identifies **HID-compliant vendor-defined device**, hardware IDs **VID_04D8&PID_003C**, usage page FF00. This establishes the first connected-state enumeration milestone, not firmware-update compatibility.

The screenshot also reports historical **last_fault=USB_OPEN (4)** even though usb_open=true. Source review explains the initial transient: USB_DEVICE_Open rejects an instance until USB_DEVICE_Tasks opens its underlying driver and marks SYS_STATUS_READY. The diagnostic first tries Open before running that task, so one startup attempt can fail and the next succeeds. last_fault is historical until reset; it is not a current connection failure when usb_open is true. No code change was made to hide that diagnostic.

### Observed unplug/replug — 2026-10-14

User screenshots 95/96 show:

| Field | Unplugged | Replugged |
|---|---|---|
| adc_raw | 0 | 0x026E = 622 |
| cable_present / attached | false / false | true / true |
| attach_count / detach_count | 1 / 1 | 2 / 1 |
| mux_ready | true | true |
| adc_timeouts / mux_errors / usb_errors | 0 / 0 / 0 | 0 / 0 / 0 |
| usb_resets | 2 | 5 |

The ADC detector and pull-up detach/reattach behavior pass this observed cycle. Increased bus reset count is consistent with renewed host USB activity, not alone proof of completed configuration. The configured flag/Windows re-enumeration after this replug is not shown and still needs confirmation. The historical USB_OPEN flag remains as previously explained.

### Observed MCLR/power-cycle recovery — 2026-10-14

The user confirmed **USB removal and re-enumeration after MCLR**, and the same result after a **power cycle**. The initial enumeration-only milestone passes: connected ADC/mux/VID-PID, ADC-driven detach/reattach and host re-enumeration after reset/power cycle are observed. The earlier cable-replug screenshot did not expose configured, so it is not relabelled as a direct configured-state measurement.

Repeatability over many cycles, disconnected startup, worst-case thresholds and injected fault recovery remain unverified. Next implement/test physical SW1 entry and update-path safety; firmware updates remain inactive.

## Regeneration and later stages

Preserve board_usb_diag.c/.h and this **enumeration-only main** across MCC regeneration. The saved generator hashes still describe the original main, so review overwrite prompts. Do not make the diagnostic always-true boot trigger permanent in the eventual update bootloader; restore a reviewed physical/valid-image policy later.

After this test passes, implement/tests for physical SW1 entry and the update parser bounds/CRC/image-acceptance path are required, then relocate the application. The separate required application regeneration after H3 package updates remains open; the tagged production blink does not validate regenerated code.
