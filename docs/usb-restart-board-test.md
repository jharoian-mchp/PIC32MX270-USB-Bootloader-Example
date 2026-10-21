# Experimental USB-only restart board test

This procedure evaluates the first opt-in USB restart image on the reported **PIC32MX270F256D A1** board. The image performs USB teardown/re-enumeration only; **firmware upload, erase/program commands, application handoff and protection changes remain disabled**. Hardware restart is untested until you report these results.

## Replacement after initial enumeration failure — 2026-10-17

The original `usb-restart-evaluation-fba5883` package is **superseded**. The user confirmed IPE programming and SW1 held during power cycling, but `list` returned no device. Source inspection and a new readiness-dependent regression reproduced a startup deadlock: the opt-in board path tried Open before the device layer became READY and never ran its required opening Tasks pass.

The replacement runs device-layer bootstrap tasks only while the handle is invalid, after the SW1 gate. After Open/owner initialization, the owner retains exclusive USB scheduling. Tests now cover delayed readiness and owner-init failure; hardware enumeration is still unverified. **On this replacement, stop after initial list/probe/STATUS and send those results. Do not run PAUSE until separately confirmed.** The later pause instructions remain reference for a subsequent authorised phase.

## Recovery control and PICkit5 observation — 2026-10-20

The user reported no current HID interface with either image while PICkit5 was connected to the board. Historical Windows entries alone did not establish enumeration. After removing PICkit5 from the board and repeating SW1/MCLR, Windows and HIDAPI both found the **recovery** firmware at `04D8:003C`; its capabilities probe passed with protocol1/max510/origin1D008000/end1D03FC00/128row/1024erase.

The user then reported that reconnecting PICkit5 to the board caused the Windows removal sound and appeared to assert MCLR. This is consistent with programmer/reset interaction, but MCLR voltage and the exact IPE tool setting have not been measured/confirmed. Do not treat the sound alone as proof of electrical reset assertion.

For the pending experimental check, program the supplied `2c91a8e` HEX through IPE, then disconnect PICkit5 **from the board** after programming finishes, preserving board power and jumpers. Repeat SW1/MCLR and run initial list/probe/STATUS only. Do not change programmer reset options speculatively or run PAUSE before those outputs are reviewed. Recovery success does not qualify experimental startup or USB-only restart.

## Experimental initial checks passed — 2026-10-20

The user supplied successful `list`, capabilities `probe` and diagnostic `status` output for the replacement experimental setup after the recovery control and PICkit5-disconnection steps. HIDAPI reports `04D8:003C`, interface0, usageFF00/1. Capabilities match protocol1/max510/origin1D008000/end1D03FC00/row128/erase1024; STATUS is ticket0/generation1/completed_cycles0/state0.

The initial-output gate is satisfied. The next authorised step is **one clean USB-only PAUSE with request ID100**, using the currently running image and keeping PICkit5 disconnected from the board. Do not touch SW1, MCLR, power or the USB cable during that cycle. Report the restart and post-probe/STATUS outputs before another cycle or fault scenario. USB restart itself is not yet hardware-verified; no NVM/protection tests or updates are authorised.

## First PAUSE failed — 2026-10-20

The user sent the authorised PAUSE ID100. The host returned `removal/reconnect not verified before deadline; no retry, outcome unknown`. Windows made a removal sound; subsequent list was empty and STATUS could not find the interface. Initial startup/probing passed, but USB-only reconnection **failed this evaluation**. The failed phase and whether full-response success or an error/timeout caused detachment are not observable from these host outputs.

**No further PAUSE or higher-ID retry is authorised.** The current firmware has no host-visible off-phase diagnostics. The completed transaction and read-only checks permit recovery only: keep PICkit5 disconnected, hold SW1 through a controlled MCLR/reset, release after at least100ms, then list/probe/STATUS. If startup does not return, use the supplied recovery HEX with PICkit5 and disconnect it from the board after programming. Record results. Firmware uploads, protection changes, second-cycle and interrupted-pause fault tests remain blocked pending diagnosis. Do not weaken drain/off-idle checks based on this symptom alone.

## Startup recovery passed after failed PAUSE — 2026-10-20

After the requested SW1/MCLR recovery, the user supplied successful list/probe/STATUS again: the expected HID interface, unchanged capabilities and ticket0/generation1/completed_cycles0/state0. Startup recovery passes for this run. These reset counters do **not** establish that the failed USB-only rebuild completed; the volatile stopped-phase evidence has been lost.

Do not repeat PAUSE on the current image. The [retained phase/failure instrumentation](usb-restart-phase-evidence.md) now has a separately reviewed [initial read-only instrumented package](usb-instrumented-initial-check.md). Its programming/list/probe/STATUS/PREVIOUS checks are authorised; PAUSE is still suspended. This old procedure does not authorise another PAUSE or supply matching new host scripts. No new firmware, protection setting or hardware fault test is requested by this recovery result.

## Delivered images and recovery

Use the supplied **prebuilt experimental HEX**, not a normal Clean/Build: the normal MPLAB project keeps restart flags OFF. Both images use XC32 v6.00, DFP1.7.380 and the unchanged 32KB reservation/application origin1D008000.

| File | Purpose | SHA-256 |
|---|---|---|
| `usb-restart-experimental.hex` | Opt-in USB-only pause/status evaluation | `7dc4de0fbc53dec3f24da64e3e96a742427e51ade0b6890181379b409ed9e535` |
| `usb-probe-recovery.hex` | Default CAPABILITIES-only diagnostic recovery | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |

The package includes exact ELF/HEX audit evidence, both HEX files, host Python scripts, HID requirements, this procedure and checksums. Keep it separate from old firmware exports. Neither image is an application updater or the standalone LED application.

The replacement experimental linked image is text29265/data88/bss1736; the canonical linker passes. Both images have identical local configuration words:

```text
DEVCFG3 FFFFFFFF   DEVCFG2 FFF97AFA
DEVCFG1 FD74CE5B   DEVCFG0 7FFFFFE3
```

PWP/BWP/CP are OFF. These are production build artifacts, not your prior IDE screenshot/readback (7FFFFFE0); debugger-related low bits can differ. Do not manually change configuration options to make them match a screenshot. Record actual programmer readback separately. No larger bootloader reservation or relocated application is used.

## 1. Preparation — no PAUSE yet

- Keep the board's confirmed power/jumper/cable settings unchanged and PICkit5 recovery available. Connect only **one** 04D8:003C matching HID board to this host.
- Save the previously working image; the package's recovery HEX provides the default read-only probe build too. Do not touch the fixed standalone baseline tag.
- From the **extracted package directory** containing the HEX files, verify the experimental checksum with `Get-FileHash .\usb-restart-experimental.hex -Algorithm SHA256`.
- Pull the accompanying source revision if running from the repository. Use Python3.10+ and the existing PyPI `hidapi` environment; do not install the unrelated `hid` package.
- Do not run the vendor UnifiedHost utility, `prepare/plan` vectors, version-2 codecs, a firmware uploader or a flash/protection test.

## 2. Program and enumerate

Program **only `usb-restart-experimental.hex`** with PICkit5, for example by selecting PIC32MX270F256D and loading the HEX in MPLAB IPE. This step deliberately programs the bootloader/boot/config regions through the programming tool; it is not USB flashing. Do not select a stale build output or the test-only ELF/HEX from another check directory.

Hold SW1 through MCLR/reset for at least100ms, release and let the firmware run freely. Do not halt at breakpoints during HID transfers; that can force host/firmware timeouts. Keep USB cable/power fixed throughout the first restart cycle.

Choose the existing Python/venv executable that owns hidapi. Set $PY to that executable and verify **Python 3.10+** before continuing:

```powershell
# Replace with your existing venv Python path if needed.
$PY = 'python.exe'
& $PY -c "import sys; assert sys.version_info >= (3,10), 'Python 3.10+ required'; print(sys.version)"
```

From the **extracted package directory** containing scripts and HEX files:

```powershell
& $PY .\pic32_hid.py selftest
& $PY .\pic32_hid.py list
& $PY .\pic32_hid.py probe --timeout 2
& $PY .\usb_restart_diag.py status --timeout 2
```

Alternatively, from the **repository root** use:

```powershell
& $PY .\host\pic32_hid.py selftest
& $PY .\host\pic32_hid.py list
& $PY .\host\pic32_hid.py probe --timeout 2
& $PY .\host\usb_restart_diag.py status --timeout 2
```

If HID is missing, install requirements-hid.txt with the chosen $PY (package root or host\ in the repository). Remaining examples below use package-root paths; add host\ from the repository root.

Expected initial status after reset/configuration:

```json
{"ticket": 0, "generation": 1, "completed_cycles": 0, "state": 0}
```

The regular probe must return the previously agreed protocol1/max510/origin1D008000/end1D03FC00/128row/1024erase layout. **Stop here if enumeration, probe or STATUS fails.** Capture terminal output and diagnostics; do not force PAUSE by retrying IDs.

## 3. One controlled USB-only pause — suspended after first failure

**This phase and the second-cycle phase below are no longer authorised for the current evaluation image after the 2026-10-20 failure. Keep them as historical procedure; use recovery/read-only checks only pending confirmation.**

**Do not run this phase on the replacement until the initial outputs have been reviewed and you receive confirmation.** Then, only after step2 passes, with no cable/power/MCLR changes. **Do not press SW1 or assert MCLR during either clean PAUSE cycle.**

```powershell
& $PY .\usb_restart_diag.py restart --request-id 100 --confirm-usb-disconnect --timeout 2 --reconnect-timeout 15
```

The USB interface should disappear and return. The tool sends PAUSE once; even a lost acceptance reply is not resent. Successful output requires:

```text
removed_observed = true
before: ticket0 generation1 completed_cycles0 state0
after:  ticket1 generation2 completed_cycles1 state7
```

`pause_response_uncertainty` may contain a HID error/timeout if removal raced the reply, but success still requires observed removal, identical unambiguous path and exact post-state. A missing removal, different path or mismatched state is a **test failure/uncertain outcome**, not permission to force a second pause. The host may fail conservatively if enumeration is too fast to observe; capture the evidence rather than weakening the verifier.

After success:

```powershell
& $PY .\pic32_hid.py probe --timeout 2
& $PY .\usb_restart_diag.py status --timeout 2
```

Send the `list`, `probe`, initial `status`, `restart` and post-status outputs. Include Python/hidapi versions if convenient.

## 4. Second cycle only if the first passed

Use request ID101 without resetting the board:

```powershell
& $PY .\usb_restart_diag.py restart --request-id 101 --confirm-usb-disconnect --reconnect-timeout 15
```

Expected after-state: ticket2/generation3/completed_cycles2/state7. Verify the probe again. Do not reuse100 for a new cycle; exact latest retries never grant another completion authority. Resetting the MCU restarts the counters, so this is not reset-safe identity/authentication.

## Failure and recovery

Stop on any failure. Do not send another PAUSE with a higher ID while an outcome is uncertain. Record `usb_restart_owner_diagnostics`, `board_usb_diagnostics` and `usb_probe_diagnostics` if the debugger can inspect them **after** the host transaction has stopped. State8 is FAILED/quarantine. Pending buffers are intentionally retained until actual completion/reset; it does not imply the peripheral is safely freed or powered off.

First try a controlled MCLR/reset holding SW1, then status/probe only. If that does not recover, program `usb-probe-recovery.hex` with PICkit5, reset holding SW1 and verify the standard probe. Never erase a protected/application page or change PWP/BWP as USB recovery. Preserve board power and cable safety; do not improvise brownout injection.

Cable loss/reset during an active pause is a later fault-qualification test, **not** part of the first cycle. After two clean cycles, stop and report results. **No unplug/MCLR/power-cycle fault testing is authorised without separate approval.** Recovery reset/reprogramming after a failed transaction has stopped is not an interrupted-pause fault test. No isolated NVM writes or firmware upload testing is authorised by this procedure.

## Evidence and limits

Final owner source review found no remaining concrete owner issue after failed-state detach/quarantine fixes. Artifact-review findings were corrected and negatively tested: PT_LOAD bounds, zero-file/nonzero-memory spans, defined required symbols, supported MIPS segment conversion and host post-I/O deadlines. Static/source/mocks and LTO link/audit pass; they do not establish hardware timing, USB bus quiescence, actual readback or all inlined instruction effects.

This is an **initial experimental evaluation image with a recovery path**, not production-ready firmware or successful hardware verification. Update the project milestone only after the board test reports actual results.
