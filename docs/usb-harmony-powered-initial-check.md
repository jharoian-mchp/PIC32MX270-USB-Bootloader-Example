# Harmony-powered 48 KB diagnostic — initial checks only

The user's 2026-10-22 03:13 approval covers preparing/programming this temporary 48 KB image and the five initial read-only checks, with the same no-preservation overlap scope as before. **No `restart`/PAUSE, higher PAUSE ID, retry or fault test is authorised.** Stop and report even if every initial check passes.

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

Source tests, sanitizers, enabled XC32 compilation and48KB/128-byte retained-startup image audits pass. The normal recovery hash is unchanged. Negative regressions reject unsafe load spans, corruption, undefined symbols, journal-initialization overlap and expanded-image acceptance under normal32KB rules. Bounded completed reviews found no concrete must-fix in supplied scopes; no review establishes the unresolved device-level safety or target restart behavior.

Actual copied artifacts, archive entries/checksums and extracted host scripts are verified before attachment. The A1 concern stays open, and this package's approval is limited to initial operation. Normal production/application layout remains32KB after debugging.
