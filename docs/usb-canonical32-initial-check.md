# Canonical 32 KB diagnostics — initial checks only

The user's 2026-10-22 04:44 approval covers this package, programming the supplied 32 KB diagnostic, one initial startup and **list/probe/STATUS/PREVIOUS/power-previous only**. No `restart`/PAUSE, higher ID, retry or fault test is authorised. Stop and report even if all checks pass.

The earlier 48 KB Harmony-powered image completed one controlled USB-only cycle. That result does not qualify this newly compiled 32 KB image. Data-section coalescing changes SRAM placement and startup timing; runtime source, diagnostics and safety predicates are unchanged. **The PIC32MX270 A1 powered-BUSY concern remains open:** powered readiness requires USBPWR1 and records BUSY as evidence, not a blocker. USBEN/IRQ/software/queue/BDT checks remain, and retirement/NVM retain USBPWR0/BUSY0 and their ownership gates. Uploads, NVM writes, protection changes and application handoff remain disabled.

## Extracted-file verification — 2026-10-22 04:55

The user supplied successful PowerShell checks of the expected source/profile manifest, eight-file checksum inventory, uniqueness/set equality, exact extracted-file inventory and all listed hashes. The printed diagnostic and recovery HEX hashes match the independently specified table below.

The downloaded ZIP hash comparison against the separately supplied delivery value was not included. Internal consistency and verified HEX hashes do not establish that this external archive check occurred. Confirm the existing check, or hash the retained downloaded ZIP without any board action. No repeated extraction, programming, startup or PAUSE is needed for this verification. Programming/IPE and the five initial read-only results have not yet been reported.

## Package and identity

Package: `usb-canonical32-09da506.zip`. **Before extraction, compare the downloaded ZIP SHA-256 with the separately supplied hash in the delivery chat message.** This external hash anchors the whole archive, including `SHA256SUMS.txt`, scripts and manifest; internal checksums alone are not authentication. If the independent delivery hash is unavailable or differs, stop and report.

```powershell
Get-FileHash .\usb-canonical32-09da506.zip -Algorithm SHA256
```

Only after that comparison passes, extract into a **new directory**, separate from older packages. Use only its matching scripts and prebuilt images; do not use a normal MPLAB Clean/Build output.

| File | Purpose | SHA-256 |
|---|---|---|
| `usb-canonical32.hex` | Canonical 32 KB diagnostic | `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7` |
| `usb-probe-recovery.hex` | Known default capabilities-only recovery | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |

Firmware/host/build source: `09da506e0ce228015c358f99415e08b68f6a19a0`. The manifest identifies the procedure commit separately. Scripts, requirements, actual artifact audit, manifest and `SHA256SUMS.txt` are included.

XC32 v6.00 / DFP1.7.380 / `-Os -flto -fno-data-sections` links text32284/data88/bss2052. The canonical program region uses31876 of32768 bytes; application origin is **`0x1D008000`**. Do not use `--power-debug` for this image. The normal recovery image and the historical 48 KB archive are unchanged.

Schema4 and both30-byte prior replies are unchanged. The 128-byte journal remains NOBITS at `0xA0000000`, outside linked startup clear/copy destinations. Coalescing ordinary data may change other SRAM addresses and invalidate retained evidence. Artifact flags `hardware_qualified=false` and `board_delivery_approved=false` describe offline checks; this procedure supplies the separate limited initial-check permission.

Configuration words remain `FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3`; PWP/BWP/CP remain OFF. Do not change protection to match older screenshots. Programming must stay within the diagnostic program interval `0x1D000000..0x1D007FFF`, plus supplied boot/configuration content. Do not erase unrelated application flash, select bulk erase or expand memory ranges. The HEX audit is not proof of programmer erase scope. Stop on an unexpected scope or preservation need and report the IPE log.

## Verify, program and start once

1. Preserve board power, jumpers and USB settings. Confirm the **existing independent external supply is active** before starting and before disconnecting PICkit5. If uncertain or PICkit5 is the sole supply, stop and report; do not change supplies/programmer power settings. Connect only one matching board to Windows.
2. From the new extraction, verify source/profile, exact file inventory and every checksum before running scripts or programming:

   ```powershell
   $ErrorActionPreference = 'Stop'
   $Manifest = Get-Content .\PACKAGE-MANIFEST.json -Raw | ConvertFrom-Json
   $Source = '09da506e0ce228015c358f99415e08b68f6a19a0'
   if ($Manifest.firmware_source_commit -ne $Source -or $Manifest.host_source_commit -ne $Source) {
       throw 'Wrong package source; stop'
   }
   if ($Manifest.program_reservation_bytes -ne 32768 -or $Manifest.application_origin -ne '0x1D008000' -or
       $Manifest.journal_schema -ne 4 -or $Manifest.journal_bytes -ne 128 -or
       $Manifest.scope.pause_authorized -ne $false) { throw 'Wrong profile/scope; stop' }
   $Expected = @('PACKAGE-MANIFEST.json', 'START-HERE.md', 'pic32_hid.py', 'usb_restart_diag.py',
       'requirements-hid.txt', 'restart-elf-hex-audit.json', 'usb-canonical32.hex', 'usb-probe-recovery.hex')
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
   Get-FileHash .\usb-canonical32.hex -Algorithm SHA256
   Get-FileHash .\usb-probe-recovery.hex -Algorithm SHA256
   ```

   Both printed hashes must match the table. Stop on any failed check; do not combine extractions or modify the checksum list.
3. Attach PICkit5 for programming, select **PIC32MX270F256D** in IPE and program/verify only `usb-canonical32.hex`. Permitted program range is within `0x1D000000..0x1D007FFF`, plus supplied boot/configuration content; a shorter file extent within that range is acceptable. Save the full target/DFP/memory-range/programming log. No substitute image, unrelated erase or protection change. If programming fails or reported scope differs, stop and report; no second attempt.
4. With independent external power still active, disconnect PICkit5 **from the board after programming finishes**, preserving power. Stop before disconnecting if supply status is uncertain. Hold In Reset was previously reported enabled; do not change it or reconnect PICkit5 during queries. Its electrical MCLR interaction remains unmeasured.
5. Perform one initial startup: hold SW1, toggle MCLR/reset once, wait at least100ms, then release SW1. Let firmware run freely. No halt, second reset, power-cycle or unplug test.

## Five read-only outputs

Use the same working Python3.10+ environment from this package directory. If HID is missing, install the matching requirements before the checks; this is unnecessary if the existing environment works:

```powershell
$PY = 'python.exe' # Or your existing venv python.exe
& $PY -m pip install -r .\requirements-hid.txt
if ($LASTEXITCODE -ne 0) { throw 'HID installation failed; stop and report' }
```

The dependency is `hidapi`, imported as `hid`, not the unrelated PyPI package named `hid`.

```powershell
& $PY -c "import sys; assert sys.version_info >= (3,10), 'Python 3.10+ required'; print(sys.version)"
if ($LASTEXITCODE -ne 0) { throw 'Python environment failed; stop and report' }
& $PY .\pic32_hid.py list
if ($LASTEXITCODE -ne 0) { throw 'HID list failed; stop and report' }
```

Require exactly one expected VID04D8/PID003C/interface0/vendor usageFF00:1. If absent, duplicated, mismatched or ambiguous, capture the list and stop before queries. Only if it passes, run each once:

```powershell
& $PY .\pic32_hid.py probe --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'Probe failed; stop and report' }
& $PY .\usb_restart_diag.py status --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'STATUS failed; stop and report' }
& $PY .\usb_restart_diag.py previous --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'PREVIOUS failed; stop and report' }
& $PY .\usb_restart_diag.py power-previous --timeout 2
if ($LASTEXITCODE -ne 0) { throw 'Power-PREVIOUS failed; stop and report' }
```

Use **plain probe, without `--power-debug`**. Expected capabilities: protocol1/max510/origin1D008000/end1D03FC00/row128/erase1024. A debug-profile mismatch is an error to report, not retry permission. Expected live STATUS is ticket0/generation1/completed_cycles0/state0.

PREVIOUS and power-previous may return invalid/default/null data or a valid older schema4 record, including the previous 48 KB cycle's history if SRAM survives. Send either unchanged. Do not reset/erase to force one result. A valid historical record does not establish that the canonical image ran a restart, and a valid record without samples is distinct from no record.

Prior replies are immutable boot snapshots; matching identity/time is not reset-safe authentication. Raw Count deltas can wrap, include logging and are non-atomic at a nominal24MHz profile; they are not settling deadlines. BUSY observations are not USB-off NVM permission. No new rebuild is exercised by these initial checks.

## Stop and report

Send the **IPE log and all five outputs**. Stop even if every check passes. No `restart`/PAUSE, resend ID100, higher PAUSE ID, reset/programming retry, power-cycle, uploads, NVM/protection changes or fault test is authorised. A later single trial needs a separate decision; the older successful cycle's permission is spent.

On any failure, unexpected range or preservation need, capture the output and stop. Do not catch a Python failure and continue. The normal recovery HEX is included for separately confirmed use only, not automatic installation; verify its hash before any such use. It implements capabilities only, not STATUS/PREVIOUS/power-previous. No second SW1/MCLR or reprogramming recovery is requested.

## Verification limits

Verified archive:77633 bytes, SHA-256 `54f065eded469b7a287f4254a18d6b1f7047366af9c730ced1a8b76fecb08dd3`, published separately in the delivery chat before extraction. Nine entries passed ZIP CRC, exact inventory, staged/extracted equality, all eight file checksums and manifest hashes/profile/scope. The copied canonical ELF/HEX passed its artifact audit; all four startup destinations were disjoint. Extracted host CLI and27 methods passed with no skips, including the mocked C dispatcher fixture, canonical/debug-profile rejection and empty/historical retained reply fixtures. No board was accessed.

Packaged `START-HERE.md` is procedure commit `70c33435bf2f8da8a7f7f8846d125387ed18d35e`; this later verification note changes no permission. A bounded procedure review identified that internal checksums need an independent trust anchor. The external ZIP hash check before extraction was added and the follow-up confirmed that finding closed. PowerShell was not available locally; its fail-fast and inventory checks were statically reviewed.

Canonical32KB link, strict tests/sanitizers, enabled XC32 compilation, retained128-byte startup exclusion, configuration/HEX/symbol checks and negative artifact tests passed. The four actual startup destinations were separately checked for mutual overlap. The generic audit does not enforce every pairwise initialization overlap. Actual copied artifacts, archive inventory/checksums and extracted host tests are verified before attachment.

The runtime source is unchanged, but compiled placement, startup records and timing differ. Neither offline tests nor the previous48KB hardware success qualifies this new image. The A1 concern remains documented, and this permission is limited to initial operation.
