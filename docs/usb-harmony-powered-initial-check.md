# Harmony-powered 48 KB diagnostic — initial checks only

Programming and initial response checks are complete under the user's 2026-10-22 03:13 approval. At03:46 the user separately authorised **one PAUSE on the currently running Harmony-powered image**, bounded below. No resend, higher-ID retry, second cycle, reset/recovery before reporting or fault test is authorised. Earlier initial-only instructions are historical.

## IPE programming evidence — received 2026-10-22 03:24

The user supplied the existing IPE log naming `usb-harmony-powered-f1d9e3c\usb-harmony-powered.hex`. It reports target voltage detected, PIC32MX270F256D/revision0xA1/DeviceId0x660a053, DFP1.7.380 and successful HEX loading. `Programming/Verify complete` and `Programming complete` occurred at local `2026-10-21 20:24:26 -0700` (`2026-10-22 03:24:26Z`).

The reported program range is `0x1D000000..0x1D008FFF`, plus configuration memory and boot config memory. This36KB reported extent is within the approved48KB reservation; it does not change application origin1D00C000 or establish detailed erase bounds. The log does not include an independent HEX hash. Hold In Reset is explicitly enabled; no setting change or reconnection test is requested.

Programming/verification is now tool-confirmed. The five initial read-only outputs are still pending; no initial enumeration, retention or restart qualification is inferred. Follow the already approved programmer disconnect and one-time startup below only if not yet completed; do not repeat an already completed startup. No PAUSE or programming/reset retry is authorised.

## Initial outputs and script-directory mismatch — 2026-10-22 03:28

The user returned one expected HID interface, correct debug capabilities (origin1D00C000/protocol1/max510/end1D03FC00/row128/erase1024), STATUS0/1/0/0 and both retained replies valid:false/default identity/detail/null samples. These values establish a responding initial link, not USB-only restart or retention qualification.

The PowerShell prompt shows `usb-power-debug-2de2f84`, the older extraction, rather than `usb-harmony-powered-f1d9e3c`. Comparison of the delivered archives confirms `pic32_hid.py` and requirements are byte-identical. `usb_restart_diag.py` differs only by adding DRIVER_READY `usb_power_off` for mask16384; the reported default replies are compatible with both versions. New-package script provenance is therefore not complete.

Keep the board running unchanged. Verify the new extraction's inventory/checksums if not already done, then collect only STATUS/PREVIOUS/power-previous from that directory with its matching script. No repeated IPE programming, SW1/MCLR startup, list or capabilities probe is needed. This is limited read-only provenance verification; no PAUSE, reset, reprogramming or fault test is authorised.

## Matching diagnostic replies — 2026-10-22 03:33

The user returned STATUS/PREVIOUS/power-previous from `usb-harmony-powered-f1d9e3c`, closing the earlier diagnostic-script directory gap without repeating programming or startup:

- STATUS: ticket0/generation1/completed_cycles0/state0.
- PREVIOUS: valid:false, READY/NONE and zero identity/time/evidence/detail defaults.
- Power-previous: valid:false, zero identity/time/metadata, all three samples null and POST/READY deltas null.

The initial response checks are complete, combined with the earlier expected HID/capabilities replies from the byte-identical list/probe script and IPE verification of the named Harmony-powered image. The prompts establish matching directory use; no separate user checksum output is included. Neither initial operation nor invalid retained evidence proves hardware restart, SRAM retention or the powered-BUSY rule safe. The nominal24MHz field is decoder profile information, not a measured clock.

Keep the board running with power/settings unchanged and PICkit5 disconnected. No repeated startup, reset, reprogramming, PAUSE or fault test is authorised. Any single evidence-producing trial needs a separate explicit decision after these initial results.

## One Harmony-powered trial authorised — 2026-10-22 03:46

The user explicitly authorised one current-image PAUSE after IPE verification and matching-script initial replies. Last STATUS was ticket0/generation1/cycles0/state0 and both prior-record queries invalid. This scope applies only to `usb-harmony-powered-f1d9e3c`, not any older image or spent trial.

Proceed only with PICkit5 disconnected from the board, existing independent external power active, settings unchanged and exactly one matching board connected. Do not press SW1/MCLR, change power/jumpers, unplug USB or reconnect PICkit5 while the command runs. From the matching new package directory, with existing `$PY`:

```powershell
& $PY .\usb_restart_diag.py restart --request-id 100 --confirm-usb-disconnect --timeout 2 --reconnect-timeout 15
& $PY .\pic32_hid.py list
```

Run `restart` once only, and collect post-list even if it reports an error. The host binds its single PAUSE send to live generation; ID100 is for this fresh Harmony-powered boot. An error/timeout can leave the outcome unknown and never permits automatic resend or a higher ID. Permission is spent when this one command runs, regardless of outcome.

Send both outputs. If exactly one expected interface is present, also run live STATUS and explicit debug probe once, with fail-fast checks:

```powershell
& $PY .\usb_restart_diag.py status --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'STATUS failed; stop and report' }
& $PY .\pic32_hid.py probe --power-debug --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'Debug probe failed; stop and report' }
```

A successful cycle requires verified removal/reconnect and live ticket1/generation2/completed_cycles1/state7. HID return alone is insufficient. If the interface is absent/ambiguous or a query fails, capture the output and stop; preserve any failed state.

**No reset, power-cycle, unplug, reprogramming or recovery before reporting.** A later powered SW1/MCLR recovery and both retained queries would need a separate decision after reviewing the result. The current boot's prior snapshots cannot expose this new trial's journal. No uploads, NVM/protection changes, electrical faults or additional cycles are authorised. The A1 concern remains open and results are pending.

## Experimental decision and concern

This image requires USBPWR1 for powered rebuilding and treats USBBUSY as evidence rather than a powered reopening blocker, following the user's Harmony-template direction. IRQ, software, USBEN, queue/BDT and full-stack readiness checks remain; retirement/preparation and NVM still require USBPWR0/BUSY0 with their ownership gates. No BUSY delay or software clear was added.

**PIC32MX270 A1 powered-BUSY safety remains unqualified.** Vendor templates support the sequence, not a device-specific hot-restart guarantee. This is a changed experimental readiness decision, not merely instrumentation or a manufacturer clarification. Initial enumeration cannot prove that the new reopening rule is safe or that USB-only restart works. Uploads, NVM writes, application handoff and protection changes remain disabled.

## Package, identity and layout

Package: `usb-harmony-powered-f1d9e3c.zip`. Extract into a **new directory**, separate from all older packages. Use only matching package-local scripts and prebuilt HEX files; do not substitute a normal MPLAB Clean/Build image.

| File | Purpose | SHA-256 |
|---|---|---|
| `usb-harmony-powered.hex` | Experimental 48 KB Harmony-powered diagnostic | `6e0dc7304bc6c1468be328fc97277c3cfb997b55291aff360855c84bc7f88873` |
| `usb-probe-recovery.hex` | Known normal 32 KB capabilities-only recovery | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |

Firmware/host source: `f1d9e3cfdb08d30f6768df8cf2a1930c8f4bd97b`. The manifest identifies the procedure commit separately. The package includes scripts, requirements, exact linked-image audit, manifest and `SHA256SUMS.txt`.

Diagnostic reservation:49152 bytes; application origin **`0x1D00C000`**. This may overwrite the former application interval `0x1D008000..0x1D00C000` (end exclusive). The user's renewed approval covers that interval with nothing needing preservation, **not unrelated flash or broad erase**. Stop if any new preservation need or unexpected erase/program scope appears. HEX bounds do not certify programmer erase behavior; send the IPE operation log.

XC32 v6.00 / DFP1.7.380 / `-Os -flto` gives text33237/data88/bss2052. Normal default firmware remains26670/80/1568,32KB/origin1D008000. Shrinking diagnostics back into32KB is later work; this image is temporary. Configuration words are unchanged: `FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3`; PWP/BWP/CP stay OFF. Do not change protection to match older screenshots.

Schema4,128-byte retained NOBITS journal and both30-byte retained replies are unchanged from the previous power-debug image. The journal is at `0xA0000000`, excluded from linked startup `.dinit`. Audit flags `hardware_qualified=false` and `board_delivery_approved=false` remain descriptions of artifact checks; this procedure supplies only the separate initial-check scope.

## Verify, program and start once

1. Preserve power, jumpers and USB cable settings. Confirm the **existing independent external supply is active** before starting and before disconnecting PICkit5. If uncertain or PICkit5 is the sole supply, stop and report; do not change power settings. Connect only one matching bootloader board to Windows.
2. From the new extraction, verify the manifest and every file checksum:

   ```powershell
   $ErrorActionPreference = 'Stop'
   $Manifest = Get-Content .\PACKAGE-MANIFEST.json -Raw | ConvertFrom-Json
   $Source = 'f1d9e3cfdb08d30f6768df8cf2a1930c8f4bd97b'
   if ($Manifest.firmware_source_commit -ne $Source -or $Manifest.host_source_commit -ne $Source) {
       throw 'Wrong package source; stop'
   }
   if ($Manifest.program_reservation_bytes -ne 49152 -or $Manifest.application_origin -ne '0x1D00C000' -or
       $Manifest.journal_schema -ne 4 -or $Manifest.journal_bytes -ne 128 -or
       $Manifest.scope.pause_authorized -ne $false) { throw 'Wrong diagnostic profile/scope; stop' }
   $Expected = @('PACKAGE-MANIFEST.json', 'START-HERE.md', 'pic32_hid.py', 'usb_restart_diag.py',
       'requirements-hid.txt', 'restart-elf-hex-audit.json', 'usb-harmony-powered.hex', 'usb-probe-recovery.hex')
   $Lines = @(Get-Content .\SHA256SUMS.txt)
   $Listed = @()
   foreach ($Line in $Lines) {
       $Parts = $Line -split '\s+', 2
       if ($Parts.Count -ne 2 -or $Parts[0] -notmatch '^[0-9a-fA-F]{64}$' -or
           $Expected -notcontains $Parts[1]) { throw 'Unexpected checksum entry; stop' }
       $Listed += $Parts[1]
   }
   if ($Listed.Count -ne $Expected.Count -or
       @($Listed | Sort-Object -Unique).Count -ne $Expected.Count -or
       @(Compare-Object ($Expected | Sort-Object) ($Listed | Sort-Object)).Count -ne 0) {
       throw 'Incomplete or duplicate checksum inventory; stop'
   }
   $Extracted = @(Get-ChildItem -File -Force | Where-Object { $_.Name -ne 'SHA256SUMS.txt' } | ForEach-Object { $_.Name })
   if (@(Get-ChildItem -Directory -Force).Count -ne 0 -or
       @(Compare-Object ($Expected | Sort-Object) ($Extracted | Sort-Object)).Count -ne 0) {
       throw 'Unexpected extraction contents; use a fresh directory and stop'
   }
   foreach ($Line in $Lines) {
       $Parts = $Line -split '\s+', 2
       $Actual = (Get-FileHash -LiteralPath (Join-Path $PWD $Parts[1]) -Algorithm SHA256).Hash
       if ($Actual -ne $Parts[0]) { throw "Checksum mismatch: $($Parts[1]); stop" }
   }
   Get-FileHash .\usb-harmony-powered.hex -Algorithm SHA256
   Get-FileHash .\usb-probe-recovery.hex -Algorithm SHA256
   ```

   Both printed hashes must match the table, and all checksums must pass. Stop on any error or mismatch; do not mix package files.
3. Attach PICkit5 for programming, select **PIC32MX270F256D** in IPE and program/verify only `usb-harmony-powered.hex`. The permitted program interval is within `0x1D000000..0x1D00BFFF`, plus supplied boot/configuration content; IPE may report a shorter file extent within it. No bulk erase, unrelated memory ranges, substitute images or protection changes. Stop on failure or unexpected operation scope—no second attempt. Save the complete target/DFP/memory-range/programming log.
4. With the independent external supply still active, disconnect PICkit5 **from the board after programming finishes**, preserving board power. If supply status is uncertain, stop before disconnecting. Hold In Reset was previously enabled; do not change it or reconnect PICkit5 during queries. Electrical MCLR behavior remains unmeasured.
5. Perform one initial startup: hold SW1, toggle MCLR/reset once, wait at least100ms, then release SW1. Let firmware run freely. Do not halt, repeat reset, power-cycle or unplug for diagnosis.

## Five read-only checks

Use the same existing Python3.10+ environment. If HID dependencies are missing, install the package requirements before checking:

```powershell
$PY = 'python.exe' # Or your existing venv python.exe
& $PY -m pip install -r .\requirements-hid.txt
if ($LASTEXITCODE -ne 0) { throw 'HID installation failed; stop and report' }
```

Installation is unnecessary if your existing environment already works. The requirement is `hidapi`, imported as `hid`; do not install the unrelated package named `hid`.

```powershell
& $PY -c "import sys; assert sys.version_info >= (3,10), 'Python 3.10+ required'; print(sys.version)"
if ($LASTEXITCODE -ne 0) { throw 'Python environment failed; stop and report' }
& $PY .\pic32_hid.py list
if ($LASTEXITCODE -ne 0) { throw 'HID list failed; stop and report' }
```

Require exactly one expected VID04D8/PID003C/interface0/vendor usageFF00:1. If absent, duplicated, mismatched or ambiguous, capture the list and stop before querying. If it passes:

```powershell
& $PY .\pic32_hid.py probe --power-debug --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'Debug probe failed; stop and report' }
& $PY .\usb_restart_diag.py status --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'STATUS failed; stop and report' }
& $PY .\usb_restart_diag.py previous --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'PREVIOUS failed; stop and report' }
& $PY .\usb_restart_diag.py power-previous --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'Power-PREVIOUS failed; stop and report' }
```

Run each once and stop on error; do not catch errors and continue. The capabilities probe **requires `--power-debug`**; normal probing intentionally rejects the expanded layout. Expected: protocol1/max510/origin1D00C000/end1D03FC00/row128/erase1024. Expected live STATUS is ticket0/generation1/completed_cycles0/state0.

The schema is unchanged, so retained replies may contain the **older** ticket1/generation2/time319308 BUSY failure and metadata487/power samples if SRAM survives programming. Valid:false/defaults/null samples are also possible. Send either result unchanged; do not erase or manipulate SRAM to force invalid evidence. A valid record may have no reached samples. No restart has been requested during initial checks, so retained evidence cannot establish a new Harmony-powered trial or current-candidate success.

The decoder keeps historical `usb_busy`1024 and adds `usb_power_off`16384; successful powered readiness may retain BUSY observations without a rejection. Neither means USB-off NVM permission. Paired snapshot ticket/generation/time matching is not reset-safe identity or authentication. Raw Count deltas may wrap, are non-atomic and include journal work at a nominal24MHz profile; they are not settling time or a deadline.

## Stop and report

Send the **IPE programming/verification log and all five outputs**. Stop even if all pass. **No `restart`/PAUSE, resend ID100, higher PAUSE ID, reset/programming retry, power-cycle, uploads, NVM/protection changes or fault test is authorised.** A later single trial needs a separate decision after these initial results; older trial permissions do not transfer.

If any step fails or scope differs, capture the output and stop before another action. The normal recovery HEX is included for separately confirmed use only; verify its hash first. It implements normal capabilities, not STATUS/PREVIOUS/power-previous. No automatic recovery installation or second SW1/MCLR attempt is requested.

## Verification limits

Verified archive:79133 bytes, SHA-256 `3e507dcd4079488ca05ca715a69096a6968ac2a0c4320816a4638a84258323ae`. Nine ZIP entries passed CRC, exact inventory, staged/extracted equality, all eight checksum entries and manifest file hashes/profile/scope. Actual copied ELF/HEX passed the48KB/128-byte retained-startup audit. Extracted host CLI and27 methods passed with no skips, including a mocked C dispatcher fixture. Debug-profile and old/new rejection decoding plus empty/historical retained-query fixtures passed offline; no board was accessed.

Packaged `START-HERE.md` is procedure commit `30aaf5ba36c491e7f8c22f2a6d3b575986709e7f`; this verification note changes no authority. The procedure review requested checksum coverage of the exact extraction inventory. Duplicate/missing/extra entries and files are now checked before programming, and the bounded follow-up confirmed that finding closed. PowerShell was unavailable locally; its checks were statically reviewed.

Source tests, sanitizers, enabled XC32 compilation and48KB/128-byte retained-startup image audits pass. The normal recovery hash is unchanged. Negative regressions reject unsafe load spans, corruption, undefined symbols, journal-initialization overlap and expanded-image acceptance under normal32KB rules. Bounded completed reviews found no concrete must-fix in supplied scopes; no review establishes the unresolved device-level safety or target restart behavior.

Actual copied artifacts, archive entries/checksums and extracted host scripts are verified before attachment. The A1 concern stays open, and this package's approval is limited to initial operation. Normal production/application layout remains32KB after debugging.
