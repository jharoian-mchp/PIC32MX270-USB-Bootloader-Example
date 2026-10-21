# Callback-mask diagnostic — initial read-only checks

The callback-mask PAUSE100 did not verify reconnect and post-list was empty at 2026-10-21 05:35. **The single-trial permission is spent: no resend, higher ID or second cycle.** The dated initial and trial instructions below are historical. After reviewing this failure report, the next bounded action is one powered SW1/MCLR recovery and read-only retained-evidence capture; no power loss, reprogramming or fault tests.

The previous `bae3050` image failed at DRIVER_READY/irq_enabled512 and was recovered with powered SW1/MCLR. This replacement defers callback registration's interrupt enables until admitted attachment, preserving the readiness guard. It has passed source tests and local artifact review; actual USB-only restart and later readiness predicates are still hardware-unqualified. Uploads, NVM writes, application handoff and protection changes remain disabled.

## Initial read-only results — 2026-10-21 05:13

The user returned all four outputs from the new `usb-callback-mask-a57fef3` directory:

- `list`: one expected Microchip USB HID Bootloader, VID04D8/PID003C/interface0/vendor usageFF00:1.
- `probe`: protocol1/max510/origin1D008000/end1D03FC00/row128/erase1024, matching the expected layout.
- STATUS: ticket0/generation1/completed_cycles0/state0.
- PREVIOUS: valid:false; ticket/generation/time/evidence zero; READY/NONE defaults; rebuild stepNONE/first_rejectednull/rejected_mask0/observation_bits0.

Initial enumeration, capabilities, live STATUS and expanded PREVIOUS decoding pass. No validated retained record is available. This absence does not identify whether earlier evidence was invalidated or rejected, verify the programmed file's hash, or qualify retention or USB-only restart. No callback-mask rebuild has been exercised by these read-only checks.

The subsequent IPE log confirms programming/verification on the second attempt, as recorded below. **Leave the board running unchanged: no PAUSE, higher ID, reset/reprogramming retry or fault test is authorised.** The earlier no-VDD report is explained by the user's later recollection below; do not reproduce it.

## IPE attempts — received 2026-10-21 05:14

The existing log names `C:\Users\c14029\Downloads\usb-callback-mask-a57fef3\usb-callback-mask.hex`, loaded successfully with DFP `PIC32MX_DFP,1.7.380,Microchip`.

- First attempt at local `2026-10-20 22:11:25 -0700` reported that the target was configured to supply its own power but no voltage was detected on VDD, then `Connection Failed`. No erase/program operation is shown for that attempt.
- Second attempt at `22:11:35 -0700` detected target voltage and found PIC32MX270F256D/revision0xA1/DeviceId0x660a053. It reported program memory `0x1D000000..0x1D007FFF`, configuration memory and boot config memory, followed by `Programming/Verify complete` and `Programming complete` at `22:11:36 -0700` (`2026-10-21 05:11:36Z`).
- `*** Hold In Reset mode is enabled ***` remains explicitly tool-reported.

Successful programming/verification of the named image and the intended program range are now tool-reported. The log does not contain an independent file hash, VDD waveform or detailed erase bounds; approximate12% usage is not the linked-image reservation audit. The later successful HID queries establish firmware running at query time, not restart qualification.

At 05:20 the user clarified: `Forgot to reattach PICKit5 prior to programming attempt`. The first attempt therefore lacked the programmer connection to the target, explaining its no-VDD report without establishing a board power fault. This recollection does not establish SRAM retention or explain PREVIOUS valid:false. Both attempts and the clarification are recorded; no retry permission follows from the successful second result. At 05:23 the user confirmed the current external supply is active and PICkit5 is disconnected from the board, without requesting any change or retest. Programming evidence, the four initial read-only results and current power/programmer state are now collected. No PAUSE is authorised; any single restart trial needs separate approval.

## One callback-mask restart trial authorised — 2026-10-21 05:28

The user explicitly approved one PAUSE on the currently running `usb-callback-mask-a57fef3` image, after successful IPE verification, initial STATUS ticket0/generation1/cycles0/state0 and confirmation of active external power/PICkit5 disconnected. This does not renew any older image's spent trial.

Proceed only with those conditions still unchanged and exactly one matching board connected. Keep PICkit5 disconnected from the board. Do not press SW1/MCLR, change power/jumpers, unplug USB or reconnect PICkit5 during the command. Using the matching package directory and existing `$PY`:

```powershell
& $PY .\usb_restart_diag.py restart --request-id 100 --confirm-usb-disconnect --timeout 2 --reconnect-timeout 15
& $PY .\pic32_hid.py list
```

**Run the restart command once only, then stop regardless of outcome.** The host reads live STATUS to bind PAUSE to the current generation and sends it once. ID100 applies to this fresh callback-mask boot; it grants no authority on older images. A timeout/lost response means an uncertain outcome, never permission to resend or increase the ID. The permission is spent when this one command is run, even if it fails.

Send the restart and post-list outputs. If exactly one expected interface is present, also send matching `usb_restart_diag.py status --timeout 2` and `pic32_hid.py probe --timeout 2`. If the interface is absent, ambiguous or a query fails, capture the output and stop. Successful qualification for this cycle requires the host's verified removal/reconnect and live ticket1/generation2/completed_cycles1/state7; HID return alone does not establish it.

**No reset, power-cycle, unplug, reprogramming or recovery before reporting.** If USB stays absent, preserve the failed state. A later powered SW1/MCLR recovery/PREVIOUS capture needs a separate decision after reviewing the output. PREVIOUS is boot-captured historical evidence, so reading it in the current boot cannot reveal the new trial's retained journal. No uploads, NVM/protection changes, destructive faults or additional cycles are authorised. Trial results are pending; no restart success is presumed.

## Failed callback-mask trial and retained recovery — 2026-10-21 05:35

The user reported the one restart command returning:

```text
Error: removal/reconnect not verified before deadline; no retry, outcome unknown
```

Post-list returned `[]`. Reconnect qualification failed; live ticket1/generation2/cycles1/state7 was not observed. This host result does not identify request acceptance, the stopped firmware phase or any failed predicate. It does not establish that the callback-mask fix passed readiness or that the older irq_enabled512 failure recurred.

After reviewing the failure report, perform **one controlled powered SW1/MCLR recovery** to attempt capture of this trial's retained record:

1. Proceed only if the existing independent external supply is still active, PICkit5 remains disconnected from the board, and USB/power/jumpers are unchanged. If uncertain, stop and report before recovery. Do not reconnect PICkit5 or change Hold In Reset settings.
2. Keep external power and USB connected. Hold SW1, toggle MCLR/reset once, wait at least100ms, then release SW1. No second reset or power-cycle.
3. From the matching `usb-callback-mask-a57fef3` directory run `pic32_hid.py list`. If exactly one expected interface returns, run `usb_restart_diag.py previous --timeout 2` first, then `usb_restart_diag.py status --timeout 2` and `pic32_hid.py probe --timeout 2`. Send all outputs.
4. If HID is absent/ambiguous or a query fails, capture the output and stop. No repeat recovery, reprogramming or recovery-HEX installation is authorised.

PREVIOUS is captured at startup; this recovery attempts to expose the last validated journal. Report it without treating it as restart authority. Valid:false leaves the firmware failure unknown. Validate any returned record against this trial's evidence; do not automatically assign an older same-schema record to the new attempt. The initial PREVIOUS was valid:false. Fresh live STATUS0/1/0/0 after recovery proves startup, not successful USB-only restart.

Recovery and retention for this image have not yet been observed. No PAUSE/resend/higher ID, uploads, NVM/protection changes or electrical fault tests are authorised. Recovery results are pending.

## Package and images

Package: `usb-callback-mask-a57fef3.zip`. Extract into a **new directory**, separate from all older packages. Use only its scripts and HEX files; do not substitute a normal MPLAB Clean/Build image.

| File | Purpose | SHA-256 |
|---|---|---|
| `usb-callback-mask.hex` | Callback-mask diagnostic | `f9dd06c6f176bf33db1c8b85639b2e3e49498b3e379d35d8e5ae00f2f35a06fd` |
| `usb-probe-recovery.hex` | Known capabilities-only recovery | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |

Firmware/host source: `a57fef3615db38a267ef467d9f5b793f8bd9bc78`. The manifest identifies the procedure commit separately. Matching Python scripts, requirements, linked-image audit, manifest and `SHA256SUMS.txt` are included.

XC32 v6.00 / DFP1.7.380 / `-Os -flto` produces text32117/data88/bss1952, fitting the unchanged 32 KB reservation with little margin. Application origin is `0x1D008000`. Schema3, the 30-byte PREVIOUS payload and the 96-byte retained journal are unchanged. The journal is NOBITS at `0xA0000000` and excluded from linked `.dinit`.

Configuration words match the default local image: `FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3`; PWP/BWP/CP remain OFF. Do not change protection to match an earlier debugger screenshot. The audit's `hardware_qualified=false` and `board_delivery_approved=false` describe artifact checks; this procedure supplies the separate limited initial-check authorisation.

## Program and start once

1. Preserve board power, jumpers and USB cable settings. Confirm the **existing independent external supply is still active** before starting and before disconnecting PICkit5. Do not change supplies or programmer power settings. If PICkit5 is the sole supply or independent power is uncertain, stop and report. Connect only one matching bootloader board to Windows.
2. From the newly extracted directory, verify the manifest source and every file in `SHA256SUMS.txt`, including both HEX files, host scripts, requirements, procedure, audit and manifest:

   ```powershell
   $Manifest = Get-Content .\PACKAGE-MANIFEST.json -Raw | ConvertFrom-Json
   $Source = 'a57fef3615db38a267ef467d9f5b793f8bd9bc78'
   if ($Manifest.firmware_source_commit -ne $Source -or $Manifest.host_source_commit -ne $Source) {
       throw 'Wrong package source; stop'
   }
   foreach ($Line in Get-Content .\SHA256SUMS.txt) {
       $Parts = $Line -split '\s+', 2
       $Actual = (Get-FileHash -LiteralPath (Join-Path $PWD $Parts[1]) -Algorithm SHA256).Hash
       if ($Actual -ne $Parts[0]) { throw "Checksum mismatch: $($Parts[1]); stop" }
   }
   Get-FileHash .\usb-callback-mask.hex -Algorithm SHA256
   Get-FileHash .\usb-probe-recovery.hex -Algorithm SHA256
   ```

   Both printed image hashes must match the table, and every checksum must pass. Inspect the manifest's schema3/30-byte PREVIOUS and initial-only scope (`pause_authorized=false`). Stop on any error or mismatch; do not combine files from different extractions.
3. In IPE select **PIC32MX270F256D** and program/verify only `usb-callback-mask.hex` with PICkit5. This installs the diagnostic through the programmer; USB uploading remains disabled. No protection changes, bulk-erase experiments or substitute images. Save the programming/verification result for reporting.
4. With the independent supply still active, disconnect PICkit5 **from the board after programming finishes**, preserving board power. Stop before disconnecting if independent power is uncertain. The previous IPE log reported Hold In Reset enabled; do not change that setting or reconnect PICkit5 during these checks. Its electrical MCLR interaction has not been measured.
5. Perform this one initial startup: hold SW1, toggle MCLR/reset once, wait at least100ms, then release SW1. Let firmware run freely; do not halt during HID requests. No further resets, unplug/power-cycle trials or programming retries are authorised.

## Four read-only checks

Use the same Python environment that passed earlier probes. From this package's directory:

```powershell
$PY = 'python.exe' # Or your existing venv python.exe
& $PY -c "import sys; assert sys.version_info >= (3,10), 'Python 3.10+ required'; print(sys.version)"
& $PY .\pic32_hid.py list
```

Require exactly one matching interface: VID04D8/PID003C/interface0/vendor usageFF00:1. If missing, duplicated, mismatched or ambiguous, capture the list and stop before any query. Only if that check passes, run:

```powershell
& $PY .\pic32_hid.py probe --timeout 2
& $PY .\usb_restart_diag.py status --timeout 2
& $PY .\usb_restart_diag.py previous --timeout 2
```

If the selected environment lacks HID, install this package's requirements using `& $PY -m pip install -r .\requirements-hid.txt`. It installs `hidapi`, imported as `hid`; do not install the unrelated PyPI package named `hid`.

Expected `list`: one VID04D8/PID003C/interface0/vendor usageFF00:1. Expected capabilities: protocol1/max510/origin1D008000/end1D03FC00/row128/erase1024. Expected live STATUS:

```json
{"ticket": 0, "generation": 1, "completed_cycles": 0, "state": 0}
```

PREVIOUS may report **valid:true with the older `bae3050` failure**, because this package keeps schema3 and some SRAM may survive programming/startup. It may also report valid:false with default fields. Send the output either way. Do not require the old record to survive or erase it to obtain valid:false. Programming and RAM-layout changes can invalidate or alter evidence; the previous observed failure is already recorded separately.

A retained ticket1/generation2/time415322/FAILED/INIT/REBUILD/DRIVER_READY/irq_enabled512 record is historical evidence from the previous trial, **not a new callback-mask restart result**. No restart has been requested during these initial checks. PREVIOUS is captured at boot, does not clear/write the journal and never grants restart permission. Zero observations can mean unsampled registers, not passing predicates. Retained SRAM is not reset-safe identity or nonvolatile persistence.

## Stop and report

Send the IPE programming/verification result and all four outputs. **Stop even if they pass. No PAUSE, higher ID, reset retry, reprogramming retry, uploads, NVM/protection changes or fault tests are authorised after this startup.** Any future evidence-producing restart needs a separate decision after reviewing these results; the older image's spent PAUSE permission does not transfer.

If programming or any query fails, capture the exact output and stop. If `list` is missing, duplicated, mismatched or ambiguous, do not run the remaining queries. Report before any further recovery action. The known recovery HEX is included for a separately confirmed recovery if needed; verify its hash before use. It implements capabilities only, not STATUS/PREVIOUS. No automatic recovery installation or second SW1/MCLR attempt is requested.

## Offline verification

Verified archive: 75724 bytes, SHA-256 `377d9e59ca67daa3c41737a8eaae8cddd1ddb4fc96b63e41153cd64bd239298b`. Its nine entries passed ZIP CRC, staged/extracted equality, all eight listed checksums and manifest file hashes/scope. Extracted host CLI and 19 test methods passed with no skips, including the mocked C dispatcher fixture. Default PREVIOUS, historical valid:true irq512 evidence and old26-byte refusal cases passed offline. No board was accessed. Packaged `START-HERE.md` is procedure commit `c9ae28417a8b3f3e0d9d20ea54ed94c00357cc9c`; this later verification record adds no board authority.

The initial-only procedure review requested manifest/all-file checksum verification and a stop before queries for any unexpected/ambiguous device list. Both changes were added and confirmed in the bounded follow-up.

Strict tests, ASan/UBSan, exact callback/attach hooks0/1 tests, generated-overlay/XC32 checks and linked ELF/HEX/configuration/startup audit passed. The unchanged MCC bases passed a five-file overlay round-trip; negative artifact regressions reject corrupt bytes and unsafe load/initialisation metadata. Bounded source/publication/attach reviews found no concrete must-fix within their completed scopes; a broader final review timed out and was not treated as approval.

The package's actual copied artifacts, archive entries/checksums and extracted host scripts are verified before attachment. Those checks do not qualify interrupt timing, later USBBUSY/USBEN/queue/BDT predicates, hardware restart, power-loss retention or arbitrary LTO instruction safety. Report this image's initial results before any further test.
