# Instrumented USB diagnostic — initial read-only check

This package installs retained-SRAM phase/checkpoint reporting for a future USB restart investigation. **Only programming and initial `list`, `probe`, `status` and `previous` checks are authorised here. Do not run `restart`/PAUSE.** The failed PAUSE on the old `2c91a8e` image remains unresolved. Uploads, application handoff and protection changes remain disabled.

## Initial checks passed; one instrumented trial authorised — 2026-10-21

The user supplied successful instrumented `list`, capabilities `probe` and STATUS (ticket0/generation1/completed_cycles0/state0). PREVIOUS returned valid:false with zero/default fields, as expected when no validated prior record exists. This establishes the initial host link and PREVIOUS response parsing, not MCLR retention or successful restart.

This dated confirmation supersedes the initial-only restriction below for **one** evidence-collection PAUSE on the currently running instrumented image:

```powershell
& $PY .\usb_restart_diag.py restart --request-id 100 --confirm-usb-disconnect --timeout 2 --reconnect-timeout 15
& $PY .\pic32_hid.py list
```

Keep PICkit5 disconnected from the board and do not change power, USB cable, SW1 or MCLR while the command runs. Send restart output and list, plus STATUS/probe if the interface is present. Stop after this one request regardless of outcome; no resend/higher-ID retry or second cycle. **Do not reset/power-cycle/reprogram before reporting.** If USB remains absent, a subsequent controlled MCLR recovery (without power loss or reprogramming) and PREVIOUS query can attempt to read the retained failure checkpoint; that recovery sequence will be confirmed after observing the result. PREVIOUS is a boot-captured snapshot and is not updated by the current operation until a new startup captures it.

The failed old image is not authorised for another PAUSE. No uploads, NVM/protection changes, destructive electrical faults or production qualification are authorised by this diagnostic trial.

## Instrumented failure located — 2026-10-21

The instrumented PAUSE100 timed out and post-list was empty. After the requested powered SW1/MCLR recovery, the user returned the expected HID interface and live STATUS ticket0/generation1/completed_cycles0/state0. PREVIOUS returned a validated retained record:

```text
ticket=1  attempted generation=2  time_ms=1351925
state=FAILED  phase=INIT  fault=INIT  checkpoint=REBUILD  evidence_bits=0
valid=true
```

This is positive hardware evidence that this journal survived the reported recovery sequence and was validated/decoded afterwards. It is not proof of preservation under power loss, programming or every reset. Live generation1 is a fresh startup; retained generation2 denotes the failed replacement attempt, not successful reopening.

The coordinator reached REBUILD and its USB-only initialization callback returned false. Prior coordinator drain/off-idle/retire gates progressed far enough to enter that callback, but the record does not identify whether preparation, driver/device construction, binding or a readiness predicate failed. The zero blocker bitmap is an earlier wait observation, not all initializer checks passing. No timeout occurred at this recorded failure; the time value is elapsed boot milliseconds, not operation duration.

**The single-trial permission above is spent. Do not repeat PAUSE, use a higher ID, reset again or reprogram for diagnosis without confirmation.** Keep the board recovered. Next investigate/instrument the exact rebuild sub-step and readiness inputs; do not weaken the USB-off NVM guard or reinterpret successful startup as successful USB-only restart. No firmware/protection changes are requested by this result.

## Images

Use these prebuilt files from this package; do not use a normal Clean/Build output or mix scripts/HEX files from older packages.

| File | Purpose | SHA-256 |
|---|---|---|
| `usb-restart-instrumented.hex` | Initial phase-reporting diagnostic | `c69c3f3a2a8ad152be0f83e3a4e09475d9e0af20b4ac0ac10816d9f595dfdaac` |
| `usb-probe-recovery.hex` | Default CAPABILITIES-only recovery | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |

Firmware source is `9e061a8`. It uses XC32 v6.00, DFP1.7.380, opt-in `-Os -flto` and the same 32KB reservation/application origin1D008000. The instrumented link is text31341/data88/bss1936. Configuration words match the default local build: FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3. PWP/BWP/CP remain OFF; do not change them to match an earlier debugger screenshot.

The package includes matching host scripts, requirements, image audit JSON, package manifest and `SHA256SUMS.txt`. The audit JSON records hardware_qualified=false and board_delivery_approved=false because artifact checks are not hardware approval; this procedure defines the limited initial evaluation scope.

## Program and start

1. Keep the confirmed board power/jumper/USB cable settings and PICkit5 recovery available. Connect only one matching HID bootloader board to Windows.
2. Extract into a **new directory**, separate from `usb-restart-evaluation-2c91a8e` and older packages.
3. From that directory, verify:

   ```powershell
   Get-FileHash .\usb-restart-instrumented.hex -Algorithm SHA256
   ```

   The hash must match the table and checksums file. Do not program a mismatched image.
4. Select PIC32MX270F256D in IPE and program **`usb-restart-instrumented.hex`** with PICkit5. Confirm programming/verification completes successfully. This explicitly programs the diagnostic bootloader/boot/configuration via the programmer; it is not USB firmware uploading.
5. Disconnect PICkit5 **from the board after programming finishes**, preserving the board's existing power. Reconnecting it previously caused device removal and may hold MCLR; electrical/tool-setting cause is not confirmed.
6. Hold SW1 while toggling MCLR/reset, wait at least100ms, then release. Let firmware run freely; do not halt during HID requests. The SW1 input still gates entry.

## Run the four read-only checks

Use the same Python environment that previously passed the recovery/experimental probes. In the newly extracted package directory:

```powershell
$PY = 'python.exe' # Or your existing venv python.exe
& $PY -c "import sys; assert sys.version_info >= (3,10), 'Python 3.10+ required'; print(sys.version)"
& $PY .\pic32_hid.py list
& $PY .\pic32_hid.py probe --timeout 2
& $PY .\usb_restart_diag.py status --timeout 2
& $PY .\usb_restart_diag.py previous --timeout 2
```

Use these package-local scripts: old versions do not understand the new `previous` response. If HID is missing, install this package's `requirements-hid.txt` with the selected `$PY`; don't install the unrelated package called `hid`.

Expected initial live STATUS:

```json
{"ticket": 0, "generation": 1, "completed_cycles": 0, "state": 0}
```

`list` should find04D8:003C/interface0/vendor usageFF00:1. The probe must return the previously verified protocol1/max510/origin1D008000/end1D03FC00/row128/erase1024 layout.

`previous` will usually report `valid:false` on first installation. A valid record may be older retained SRAM; send it without interpreting it as current restart success. It reports historical ticket/generation/time/state/phase/fault/checkpoint/evidence, captured at startup. It is not restart permission or boot authentication.

**This image cannot reconstruct the old failed PAUSE:** that firmware had no journal, and programming/power/reset may invalidate RAM. The initial check validates the new endpoint, not MCLR preservation. Hardware retention is untested until a separately authorised evidence-producing cycle is performed.

## Stop and report

Send all four outputs and the IPE programming result. **Stop here even if they all pass. Do not run `usb_restart_diag.py restart`, resend ID100, use a higher PAUSE ID, or perform unplug/MCLR/power-cycle fault tests.** Another diagnostic cycle requires separate confirmation after these results are reviewed.

If enumeration/probe/status/previous fails, capture the error and stop. First try the known controlled SW1/MCLR recovery with PICkit5 disconnected, then read-only checks. If it still fails, first run `Get-FileHash .\usb-probe-recovery.hex -Algorithm SHA256` and require the recovery hash in the table/checksums file. Then program the included `usb-probe-recovery.hex`, disconnect PICkit5 from the board after programming, repeat SW1/MCLR and run only the normal capabilities probe. Recovery firmware does not implement `status` or `previous`. No protection/bulk-erase experiment or ad-hoc brownout test is authorised.

## What was verified before delivery

- Independent bounded owner/journal/PREVIOUS review found no concrete must-fix for this initial read-only scope. Earlier stale pre-port recording/request-race findings were fixed and tested.
- 21 strict C executables and 21 Python methods plus source/hook/submission/rebuild harnesses pass; ASan/UBSan and enabled target compilation pass.
- The canonical32KB LTO link and ELF/HEX/configuration/symbol audit pass; corruption/initialization-overlap regressions reject unsafe artifacts.
- Retained journal is96 bytes in `.persist` NOBITS SRAM, excluded from the linked startup `.dinit` clear/copy destinations. Those facts do **not** establish electrical SRAM retention, actual USB restart or all LTO instruction effects on the board.

Only this initial experimental evaluation is being delivered. The underlying restart failure is not fixed or hardware-qualified; safety predicates are unchanged. Future evidence collection, NVM/protection work, application relocation and firmware upload remain gated separately.
