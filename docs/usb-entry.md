# SW1 startup-entry test

This diagnostic build **enumerates only when SW1 is held during startup**. No button selects **safe idle**, not application handoff. Firmware-update commands remain inactive; the current revision additionally serves [read-only capabilities queries](usb-host-probe.md). Linux builds, mocked tests and sanitizer checks pass. On 2026-10-14 the user confirmed **SW1 works as described** for this diagnostic startup-entry test.

## Configuration and behavior

The user's MCC commit **c4d27a6** adds `BOOT_REQUEST_Get()` on **RB2, MCU pin 23/PIM pin 17**, and clears **ANSELB bit 2**, disabling AN4. Carrier **R18, 20 kΩ**, pulls the signal to 3.3 V; SW1 pulls it to ground. Entry is **active-low**. User-owned initialization explicitly sets RB2 as input and disables its internal pulls; generated code is unchanged locally.

After SYS_Initialize, the diagnostic loop samples the button at 5 ms intervals. **Four consecutive low samples**, within a **50 ms startup window**, latch diagnostic enumeration on. Hold SW1 across MCLR release or power-on and for at least 100 ms afterward to give margin. This is main-loop debounce, not busy waiting in an interrupt.

- **SW1 held at startup:** run the previously tested ADC/mux/USB diagnostics. Once entered, releasing SW1 does not exit or detach USB.
- **SW1 released at startup:** decide safe idle after 50 ms; do not open the USB client, run USB tasks, submit ADC samples or change expander/mux state.
- **Press after the window:** remain idle until the next reset. There is no runtime-entry shortcut in this test.
- **Button bounce:** release samples reset the consecutive-low count. A last-moment press that cannot complete four samples before the window closes is rejected.

The strong `bootloader_Trigger()` deliberately still returns true to block the generated application's unsafe first-word-only jump path. It **does not mean the button is pressed**. The subsequent user-owned startup decision gates USB enumeration. We will integrate SW1 into the final trigger/valid-image/handoff policy only after update safety and the relocated application are tested.

The initial USB enumeration/reset/cable tests in `docs/usb-enumeration.md` apply to the preceding always-enumerating diagnostic version. The user subsequently confirmed this SW1-gated revision works as described; no firmware-update or application-handoff result is implied.

## Software checks

Run `make test` and `make bootloader`. Tests cover four-sample qualification, bounce reset, latched release behavior, normal safe idle, late/boundary press rejection, CP0 counter wrap and the previous ADC/mux/attach error handling. Normal idle tests assert no I²C submissions, ADC sampling or USB tasks/attachment and unchanged expander registers.

XC32 v6.00/DFP 1.7.380 -O2 builds this diagnostic image within the 32 KB reservation. `xc32-size`: **21,753 bytes text, 80 bytes data, 768 bytes BSS**. Linked-symbol checks verify the strong trigger and absence of the update-task/parser and NVM erase/write routines. Existing generated-library diagnostics remain; no generated sources were patched.

## Controlled hardware test

1. Pull the repo, reopen `bootloader/USB_HID_bootloader.X`, Clean/Build and program **this SW1-entry diagnostic version** through PICkit 5. Keep external power, PICkit target power disabled, PIM_MCLR, J5 at 3.3 V, J1 at 0x22 and Type-A unused. Do not use the firmware-update utility.
2. With the USB cable connected, reset **without holding SW1**. Expect the HID device absent. In the debugger expect `startup_decided=true`, `diagnostic_enabled=false`, `attached=false`, and `boot_request_active=false` while the button is released.
3. Press SW1 without resetting. `boot_request_active` should become true, but the device should **not** enumerate; release it and confirm the field becomes false.
4. **Hold SW1, press/release MCLR, keep SW1 held for at least 100 ms, then release SW1.** Expect enumeration at **VID_04D8/PID_003C**. Watch `startup_decided=true`, `diagnostic_enabled=true`; `boot_request_active` follows the live button and can be false after release. ADC raw should be around the prior **622 counts**, mux_ready/attached/configured true and error counters stable.
5. Unplug/replug J2 after entering diagnostics. Expect the previous ADC-based detach/reattach behavior; releasing SW1 must not prevent reconnect in the same boot.
6. Reset without SW1 again: expect removal and safe idle. Power-cycle once without SW1 and once holding it through startup; verify both selection paths.

For timing-sensitive behavior, run freely rather than repeatedly single-stepping through startup or USB tasks. Debugger halts can exceed the startup window; hold SW1 before starting execution. There is no LED heartbeat or normal application in this build.

Report the normal-start and held-SW1 enumeration results, the three new diagnostic fields, and any build/programming error. These results establish physical entry only; they do not establish firmware update, flash protection, valid-image recovery or application handoff.

## Observed hardware result — 2026-10-14

The user reported **“SW1 works as described”** after delivery of startup-gated diagnostic commit **51a78c5**. Record the intended selection behavior as passed, user-reported: normal reset does not enumerate, held SW1 during reset enables diagnostic enumeration, release after entry stays in diagnostics, and a late press does not enter without reset. No additional timing measurements or per-case screenshots were supplied. This pass does not verify a functional update bootloader or application handoff.

## Next work

Keep update commands inactive. Review and test host packet/HEX lengths, exclusive flash bounds, NVM errors, disconnect/interrupted-update handling and image acceptance before restoring functional update tasks. Regenerate/retest the separate application after the recorded H3 package updates, then prepare a bootloader-compatible linker layout. Preserve immutable tag `v0.1.0-led-blink` for standalone recovery.
