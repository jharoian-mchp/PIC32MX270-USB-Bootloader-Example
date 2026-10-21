# Temporary 48 KB power-observation diagnostic — initial checks

The power-debug PAUSE100 did not verify reconnect and post-list was empty at 2026-10-21 20:28. **The single-trial permission is spent: no resend, higher ID or second cycle.** After reviewing the failure report, one controlled powered SW1/MCLR recovery and read-only retained capture is instructed below. No power loss, reprogramming or fault test. Earlier initial/trial instructions are historical.

The previous callback-mask trial failed at DRIVER_READY/usb_busy1024 with USBPWR and USBBUSY sampled set. This image adds three one-shot observations; it adds no wait or readiness correction. USB-only restart and BUSY semantics remain hardware-unqualified. Uploads, NVM writes, application handoff and protection changes are disabled.

## Programming and initial results — 2026-10-21 20:24

The user supplied the existing IPE log for `usb-power-debug-2de2f84\usb-power-debug.hex` and all five read-only outputs:

- IPE: target voltage detected, PIC32MX270F256D/revision0xA1/DeviceId0x660a053, DFP1.7.380, HEX loaded successfully. `Programming/Verify complete` and `Programming complete` at local `2026-10-21 13:22:23 -0700` (`2026-10-21 20:22:23Z`). Hold In Reset remains enabled.
- Reported program memory: `0x1D000000..0x1D008FFF`, plus configuration memory and boot config memory. This36KB reported extent lies inside the approved48KB reservation; it does not relocate the application origin or reduce the linker reservation. The log does not establish detailed erase bounds or an independent HEX hash.
- Python3.14.8; list found exactly one expected VID04D8/PID003C/interface0/vendor usageFF00:1 HID.
- Explicit debug probe: protocol1/max510/origin1D00C000/end1D03FC00/row128/erase1024.
- STATUS: ticket0/generation1/completed_cycles0/state0.
- PREVIOUS: valid:false, READY/NONE and zero identity/time/evidence/detail defaults.
- Power-previous: valid:false, zero identity/time/metadata, all three samples null and POST/READY deltas null. Nominal Count24MHz is decoder profile information, not a measured board clock.

Programming/verification, initial link, debug-layout reporting and both retained query decoders pass. No validated prior record or reached sample is available. This does not establish why older SRAM evidence is absent, retention under this image, power-enable timing or successful USB-only restart. No rebuild observation has been produced by these initial checks.

Initial-only evidence collection is complete. Keep external power/settings unchanged and PICkit5 disconnected. **No PAUSE, higher ID, reset/programming retry or fault test is authorised.** A single evidence-producing trial requires a separate decision; no new command is granted by these results.

## One power-debug evidence trial authorised — 2026-10-21 20:26

The user explicitly authorised one PAUSE on the currently running `usb-power-debug-2de2f84` image after IPE verification and the five initial checks. The last live STATUS was ticket0/generation1/cycles0/state0 and both previous-record queries were invalid. Older images' spent trials are not renewed.

Proceed only with PICkit5 disconnected from the board, the existing independent external supply active, settings unchanged and exactly one matching board connected. Do not press SW1/MCLR, change power/jumpers, unplug USB or reconnect PICkit5 while the command runs. From the matching package directory, with the existing `$PY`:

```powershell
& $PY .\usb_restart_diag.py restart --request-id 100 --confirm-usb-disconnect --timeout 2 --reconnect-timeout 15
& $PY .\pic32_hid.py list
```

Run the restart command **once only**. The host reads live STATUS and binds its single PAUSE transmission to that generation. ID100 applies to this fresh power-debug boot only. A timeout/lost response leaves the outcome uncertain; it never authorises a resend or higher ID. Still collect post-list after a restart error, then stop. Permission is spent when this one restart command is run, regardless of outcome.

Send both outputs. If exactly one expected interface returns, also collect live STATUS and the explicit debug capabilities probe once:

```powershell
& $PY .\usb_restart_diag.py status --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'STATUS failed; stop and report' }
& $PY .\pic32_hid.py probe --power-debug --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'Debug probe failed; stop and report' }
```

A successful cycle requires verified removal/reconnect plus live ticket1/generation2/completed_cycles1/state7. HID return alone is insufficient. If HID is absent/ambiguous or a query fails, report that output and preserve the state.

**No reset, power-cycle, unplug, reprogramming or recovery before reporting.** A later powered SW1/MCLR recovery and PREVIOUS/power-previous capture needs a separate decision after the result. Both retained queries are boot snapshots: in the current boot they cannot expose this new trial's journal. This trial grants no uploads, NVM/protection changes, electrical faults or additional cycle. Results are pending; no restart success is assumed.

## Failed trial and powered retained capture — 2026-10-21 20:28

The user reported the one PAUSE100 command returning `Error: removal/reconnect not verified before deadline; no retry, outcome unknown`, then list returned `[]`. Reconnect qualification failed; expected live ticket1/generation2/cycles1/state7 was not observed. The host output does not identify acceptance, stopped firmware phase, first rejection or power samples. Do not substitute earlier images' failures for this missing record.

After reviewing the report, perform **one controlled powered SW1/MCLR recovery** to attempt capture of both retained snapshots:

1. Proceed only with the existing independent external supply active, PICkit5 disconnected from the board, and USB/power/jumpers unchanged. If uncertain, stop and report before recovery. Do not reconnect PICkit5 or change Hold In Reset settings.
2. Keep external power and USB connected. Hold SW1, toggle MCLR/reset once, wait at least100ms, then release SW1. No second reset or power-cycle.
3. From `usb-power-debug-2de2f84`, run list. Only if exactly one expected interface returns, run PREVIOUS then power-previous, followed by STATUS and `probe --power-debug`. Check each Python exit code and stop on error. Send every output.
4. If HID is absent/ambiguous or a query fails, stop and report. No repeated recovery, programmer operation or recovery-HEX installation is authorised.

Both queries capture the immutable previous snapshot at boot. Compare their ticket/generation/time when reporting; matching fields are not reset-safe identity. Invalid records or missing samples leave corresponding firmware behavior unknown. Initial PREVIOUS and power-previous were invalid. Fresh STATUS0/1/0/0 after recovery establishes startup, not successful USB-only restart. Raw Count deltas include logging, may wrap and cannot prove a settling deadline.

Recovery results are pending. No PAUSE, higher ID, reset retry, reprogramming, upload/NVM/protection change or fault test is authorised beyond this one bounded recovery and its read-only queries.

## Package and temporary layout

Package: `usb-power-debug-2de2f84.zip`. Extract into a **new directory**, separate from older packages. Use this package's scripts and prebuilt images only; do not substitute a normal MPLAB Clean/Build output.

| File | Purpose | SHA-256 |
|---|---|---|
| `usb-power-debug.hex` | Temporary 48 KB diagnostic | `310fc0ded96480df67e95b3b3c8dd4946792c31e9a468098bac116f125a0d35f` |
| `usb-probe-recovery.hex` | Known normal 32 KB capabilities-only recovery | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |

Firmware/host source: `2de2f84b87177dd8f8d158e64a1f7ae2097712f1`. The manifest identifies the procedure commit separately. Matching Python scripts, requirements, linked-image audit, manifest and `SHA256SUMS.txt` are included.

The diagnostic reserves49152 bytes; its application origin is **`0x1D00C000`**, not the normal `0x1D008000`. Programming it may overwrite the former application interval `0x1D008000..0x1D00C000` (end exclusive); the user's no-preservation confirmation covers that interval. It does not approve erasing unrelated application flash above it. Do not select bulk erase or expand memory ranges. Review IPE's selected memory areas; stop on an unexpected range or erase prompt. The HEX audit checks artifact spans, not the programmer's erase behavior. Send the operation log so the actual reported scope can be reviewed.

XC32 v6.00 / DFP1.7.380 / `-Os -flto` produces text33241/data88/bss2052. Normal firmware remains26670/80/1568 with a 32 KB reservation. Returning diagnostics to32KB is a later task; this image is temporary. Configuration words match the normal local image: `FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3`. PWP/BWP/CP stay OFF; do not change protection to match an earlier screenshot.

Schema4 uses a 128-byte retained NOBITS journal at `0xA0000000`, excluded from linked startup `.dinit`. Older schemas are rejected, and programming/RAM-layout changes can invalidate SRAM evidence. Installing this image cannot reconstruct missing observations from the old trial. Both PREVIOUS and power-previous replies contain30-byte payloads. The artifact audit's `hardware_qualified=false` and `board_delivery_approved=false` remain true descriptions of its scope; this procedure supplies the separate limited initial-check permission.

## Verify, program and start once

1. Preserve board power, jumpers and USB cable settings. Confirm the **existing independent external supply is active** before starting and before disconnecting PICkit5. If PICkit5 is the sole source or independent power is uncertain, stop and report; do not change supplies or programmer power settings. Connect only one matching bootloader board to Windows.
2. From the new extraction, verify the manifest source and all checksum entries, including images, scripts, requirements, audit, manifest and procedure:

   ```powershell
   $ErrorActionPreference = 'Stop'
   $Manifest = Get-Content .\PACKAGE-MANIFEST.json -Raw | ConvertFrom-Json
   $Source = '2de2f84b87177dd8f8d158e64a1f7ae2097712f1'
   if ($Manifest.firmware_source_commit -ne $Source -or $Manifest.host_source_commit -ne $Source) {
       throw 'Wrong package source; stop'
   }
   if ($Manifest.program_reservation_bytes -ne 49152 -or $Manifest.application_origin -ne '0x1D00C000' -or
       $Manifest.journal_schema -ne 4 -or $Manifest.journal_bytes -ne 128 -or
       $Manifest.scope.pause_authorized -ne $false) { throw 'Wrong diagnostic profile/scope; stop' }
   foreach ($Line in Get-Content .\SHA256SUMS.txt) {
       $Parts = $Line -split '\s+', 2
       $Actual = (Get-FileHash -LiteralPath (Join-Path $PWD $Parts[1]) -Algorithm SHA256).Hash
       if ($Actual -ne $Parts[0]) { throw "Checksum mismatch: $($Parts[1]); stop" }
   }
   Get-FileHash .\usb-power-debug.hex -Algorithm SHA256
   Get-FileHash .\usb-probe-recovery.hex -Algorithm SHA256
   ```

   Both printed image hashes must match the table; all checks must pass. Stop on any error or mismatch.
3. With PICkit5 attached for this programming operation, select **PIC32MX270F256D** in IPE and program/verify only `usb-power-debug.hex`. The intended program region is `0x1D000000..0x1D00BFFF`, plus the supplied boot/configuration content. Do not select broad erase, unrelated memory ranges, substitute images or protection changes. Save the programming/verification log, including target/DFP and memory ranges. If IPE reports a failure or unexpected scope, stop and report; no automatic second attempt is authorised.
4. With the independent external supply still active, disconnect PICkit5 **from the board after programming finishes**, preserving power. Stop before disconnecting if independent power is uncertain. Hold In Reset was previously reported enabled; do not change that setting or reconnect PICkit5 during queries. Its electrical MCLR interaction has not been measured.
5. Perform one initial startup: hold SW1, toggle MCLR/reset once, wait at least100ms, then release SW1. Let firmware run freely. Do not halt, repeat reset, power-cycle or unplug for diagnosis.

## Five read-only checks

Use the existing Python environment (Python3.10+) from this package's directory:

```powershell
$PY = 'python.exe' # Or your existing venv python.exe
& $PY -c "import sys; assert sys.version_info >= (3,10), 'Python 3.10+ required'; print(sys.version)"
if ($LASTEXITCODE -ne 0) { throw 'Python environment check failed; stop and report' }
& $PY .\pic32_hid.py list
if ($LASTEXITCODE -ne 0) { throw 'HID list failed; stop and report' }
```

Require exactly one expected VID04D8/PID003C/interface0/vendor usageFF00:1. If absent, duplicated, mismatched or ambiguous, capture the list and stop before querying. If it passes, run each query once, stopping on any error:

```powershell
& $PY .\pic32_hid.py probe --power-debug --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'Probe failed; stop and report' }
& $PY .\usb_restart_diag.py status --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'STATUS failed; stop and report' }
& $PY .\usb_restart_diag.py previous --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'PREVIOUS failed; stop and report' }
& $PY .\usb_restart_diag.py power-previous --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'Power-PREVIOUS failed; stop and report' }
```

The probe **requires `--power-debug`**. Normal probing intentionally rejects this layout; do not interpret that rejection as permission to retry or use older scripts. Expected capabilities: protocol1/max510/origin**1D00C000**/end1D03FC00/row128/erase1024. The explicit flag affects read-only probing only; the offline uploader/planner remains unconnected and uses the normal layout.

If the environment lacks HID, install this package's requirements before the checks:

```powershell
& $PY -m pip install -r .\requirements-hid.txt
if ($LASTEXITCODE -ne 0) { throw 'HID installation failed; stop and report' }
```

It installs `hidapi`, imported as `hid`; do not install the unrelated package named `hid`.

Expected live STATUS:

```json
{"ticket": 0, "generation": 1, "completed_cycles": 0, "state": 0}
```

PREVIOUS will usually report valid:false/default fields because schema4 rejects older records. Power-previous should then report valid:false, zero identity/time/metadata, null samples and null deltas. A valid schema4 record is also possible if compatible evidence survives startup; send it unchanged. Valid identity with no reached samples is distinct from no record. Do not erase or manipulate memory to force either outcome.

PREVIOUS and power-previous are immutable boot snapshots. Compare their ticket/generation/time when reporting, but matching fields do not establish reset-safe identity or authenticate a trial. A mismatch/error must be reported without auto-retry. No restart has occurred during these checks, so they cannot determine the old failure's power timing or prove this image's restart success.

Power observations describe the first reached PRE_ENABLE/POST_ENABLE/READY_POWER sites. Invalid samples mean unsampled, not power/BUSY clear. Raw CP0 tick deltas may wrap, use a nominal24MHz profile and include journal overhead; they are not wall time, settling deadlines or proof of transient versus stuck BUSY. Neither query clears/writes the journal or authorises PAUSE.

## Stop and report

Send the **IPE programming/verification log and all five outputs**. Stop even if all pass. **No `restart`/PAUSE, resend ID100, higher PAUSE ID, reset/programming retry, power-cycle, upload/NVM/protection change or fault test is authorised after startup.** A later evidence-producing trial needs a separate decision after these initial results.

If programming, enumeration or a query fails, capture the exact output and stop before another action. The normal recovery HEX is included for a separately confirmed recovery, not automatic installation. Its hash must match before use; it implements normal-layout capabilities only, with no STATUS/PREVIOUS/power-previous. No second SW1/MCLR or reprogramming recovery is requested now.

## Offline verification scope

Verified archive: 78734 bytes, SHA-256 `ab75b8043c3dd33fbe7dbe756591e8f0f279d361b020b40cd2f498460cd3600f`. All nine ZIP entries passed CRC, staged/extracted byte equality, all eight checksum entries and manifest file hashes/layout/scope. The actual copied ELF/HEX passed the48KB/128-byte startup audit. Extracted host CLI plus27 methods passed with no skips, including a mocked C dispatcher fixture; debug/normal capability rejection and empty/valid power snapshots passed offline. No board was accessed.

Packaged `START-HERE.md` is procedure commit `41fdc0575b611a0262ee8c0a5199346d4278ebcb`; this later verification note changes no permission. The procedure review requested immediate `$LASTEXITCODE` checks because Windows PowerShell does not automatically stop after a failed Python invocation. These checks were added and confirmed closed in the bounded follow-up. PowerShell itself was not available locally; the procedure was statically checked and reviewed.

Source tests/sanitizers, enabled XC32 compilation, the separate48KB link and ELF/HEX/configuration/128-byte retained-startup audit passed. Negative audits reject corrupt bytes, unsafe loads, undefined symbols, journal-initialization overlap and expanded-image acceptance under normal32KB rules. The normal recovery hash is unchanged. Bounded source reviews found no concrete must-fix within completed scopes; no review established target timing or hardware restart.

Actual copied artifacts, archive entries/checksums and extracted host scripts are verified before attachment. This package's programming approval does not establish a working USB-only restart or permit any additional cycle. The normal production/application layout must remain32KB when debugging is complete.
