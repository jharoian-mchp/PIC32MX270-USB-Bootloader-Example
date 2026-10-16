# Experimental USB-only restart board test

This procedure evaluates the first opt-in USB restart image on the reported **PIC32MX270F256D A1** board. The image performs USB teardown/re-enumeration only; **firmware upload, erase/program commands, application handoff and protection changes remain disabled**. Hardware restart is untested until you report these results.

## Delivered images and recovery

Use the supplied **prebuilt experimental HEX**, not a normal Clean/Build: the normal MPLAB project keeps restart flags OFF. Both images use XC32 v6.00, DFP1.7.380 and the unchanged 32KB reservation/application origin1D008000.

| File | Purpose | SHA-256 |
|---|---|---|
| `usb-restart-experimental.hex` | Opt-in USB-only pause/status evaluation | `a9e473bac44c031876c3a1f9c5e755d51cabcbc4f0629380a33cc49eed8c813a` |
| `usb-probe-recovery.hex` | Default CAPABILITIES-only diagnostic recovery | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |

The package includes exact ELF/HEX audit evidence, both HEX files, host Python scripts, HID requirements, this procedure and checksums. Keep it separate from old firmware exports. Neither image is an application updater or the standalone LED application.

The experimental linked image is text29249/data88/bss1736; the canonical linker passes. Both images have identical local configuration words:

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

## 3. One controlled USB-only pause

Only after step2 passes, with no cable/power/MCLR changes. **Do not press SW1 or assert MCLR during either clean PAUSE cycle.**

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
