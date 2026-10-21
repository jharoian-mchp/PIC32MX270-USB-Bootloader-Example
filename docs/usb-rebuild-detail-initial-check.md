# Rebuild-detail diagnostic — initial read-only checks

Only programming the supplied schema-3 diagnostic and the initial `list`, `probe`, `status` and `previous` checks are authorised. **Do not run `restart`/PAUSE, resend ID100, use a higher PAUSE ID or perform fault tests.** The approval follows the user's 2026-10-21 04:01 confirmation. Stop and report after the four checks, even if all pass.

The previous 18384b2 PAUSE100 failed at INIT/REBUILD. Powered SW1/MCLR recovery restored HID and recovered its retained record. The exact failing sub-step is unknown. This replacement adds evidence recording without changing the restart safety predicates. It has not fixed or qualified USB-only restart. Uploads, NVM writes, application handoff and protection changes stay disabled.

## Package and compatibility

Package: `usb-rebuild-detail-bae3050.zip`. Extract into a **new directory**, separate from both older experimental/instrumented packages. Use only this package's scripts and HEX files; do not substitute a normal MPLAB Clean/Build image.

| File | Purpose | SHA-256 |
|---|---|---|
| `usb-rebuild-detail.hex` | Schema-3 rebuild evidence diagnostic | `7dbcc93f3a2414707913a752d7f99761833c2a65aa175999c776c8b9054f44a2` |
| `usb-probe-recovery.hex` | Known default capabilities-only recovery | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |

Firmware/host source: `bae305099bc46797926477bd98d77b5c0db12464`. The package manifest identifies the procedure commit separately. The package includes matching scripts, `requirements-hid.txt`, the linked-image audit, a manifest and `SHA256SUMS.txt`.

Schema 3 keeps the 96-byte two-slot retained journal but rejects schema-1/2 records. PREVIOUS expands from 26 to 30 payload bytes. **The older failed record cannot be reconstructed or enhanced by installing this image.** Programming/reset/power changes can also invalidate retained SRAM. Keep the reported old record as historical evidence; do not try to preserve it by changing programmer settings.

XC32 v6.00 / DFP1.7.380 / `-Os -flto` produces text32041/data88/bss1952, within the unchanged 32 KB reservation with little margin. Application origin stays `0x1D008000`. The 96-byte journal is NOBITS at `0xA0000000`, excluded from linked startup `.dinit`. Configuration words match the default local image: `FFFFFFFF / FFF97AFA / FD74CE5B / 7FFFFFE3`; PWP/BWP/CP remain OFF. Do not change protection settings to match an earlier debugger screenshot. Artifact audit flags remain `hardware_qualified=false` and `board_delivery_approved=false`; this procedure provides the separate limited initial-check authorisation.

## Program and start once

1. Preserve the confirmed board power, jumpers and USB cable settings. The board was previously confirmed externally powered. Confirm that its existing external board supply is active and independent of PICkit5 before starting; do not change supplies or programmer power settings. If PICkit5 is the sole supply, or independent power is uncertain, stop and report. The user confirmed the external supply active during package preparation on 2026-10-21. Keep PICkit5 recovery available and connect only one matching HID bootloader board to Windows.
2. Extract the ZIP into a new directory. From that directory, verify **both** images before programming:

   ```powershell
   Get-FileHash .\usb-rebuild-detail.hex -Algorithm SHA256
   Get-FileHash .\usb-probe-recovery.hex -Algorithm SHA256
   ```

   Both hashes must match the table and checksum file. Stop on any mismatch.
3. In IPE select **PIC32MX270F256D**, then program/verify only `usb-rebuild-detail.hex` with PICkit5. This programmer operation installs the diagnostic; USB upload remains disabled. Do not change protection, use bulk-erase experiments or substitute other HEX files. Report the programming/verification result.
4. With the existing independent external supply still active, disconnect PICkit5 **from the board after programming finishes**, preserving board power. If that supply cannot be confirmed active, stop and report before disconnecting. Its suspected MCLR interaction is electrically unconfirmed; do not reconnect it during these checks.
5. For this initial startup, hold SW1 while toggling MCLR/reset, wait at least100ms, then release SW1. Let firmware run freely; do not halt during HID requests. Do not perform further reset/unplug/power-cycle trials.

## Run the four read-only checks

Use the same Python environment that passed the prior probes. From this newly extracted directory:

```powershell
$PY = 'python.exe' # Or your existing venv python.exe
& $PY -c "import sys; assert sys.version_info >= (3,10), 'Python 3.10+ required'; print(sys.version)"
& $PY .\pic32_hid.py list
& $PY .\pic32_hid.py probe --timeout 2
& $PY .\usb_restart_diag.py status --timeout 2
& $PY .\usb_restart_diag.py previous --timeout 2
```

If the selected Python environment lacks HID, install this package's `requirements-hid.txt` with `& $PY -m pip install -r .\requirements-hid.txt`. It installs `hidapi`, imported as `hid`; do not install the unrelated package named `hid`.

Expected `list`: VID04D8/PID003C/interface0/vendor usageFF00:1. Expected capabilities: protocol1/max510/origin1D008000/end1D03FC00/row128/erase1024. Expected live STATUS:

```json
{"ticket": 0, "generation": 1, "completed_cycles": 0, "state": 0}
```

PREVIOUS should usually report `valid:false` with zero/default fields and `rebuild.step`=`NONE`, `first_rejected`=null, masks/observations zero. A valid record can be older schema-3 SRAM evidence; send it as returned without treating it as current restart success, authentication or permission. Live tickets/generations start fresh at boot.

The `rebuild` fields name an entered sub-step, its first rejected check and ordered register observations. No new restart has occurred during these checks, so they cannot identify the old failure. Unchecked later predicates or unsampled registers are not passes. PREVIOUS is a boot-captured historical snapshot and does not clear/write the journal or authorise a restart.

## Stop and report

Send the programming/verification result and all four outputs. **Stop even if they pass. No PAUSE, higher request ID, reset retry, reprogramming retry, uploads, NVM/protection changes or fault tests are authorised after this initial startup.** A future evidence-producing restart requires a separate decision after these results.

If programming, enumeration or any query fails, capture the exact output and stop; do not run the remaining queries if HID is absent. Report before taking another recovery action. The known `usb-probe-recovery.hex` is included for a separately confirmed recovery if needed: its hash must match before use, and it implements capabilities only, not STATUS/PREVIOUS. No ad-hoc brownout, unplug or MCLR diagnosis is requested.

## Verified before delivery

Strict tests, ASan/UBSan, generated overlay/XC32 checks and the linked ELF/HEX/configuration/startup audit passed; negative artifact regressions reject corrupt/missing bytes and unsafe load/initialisation metadata. A bounded owner/journal/observer/parser review found no concrete must-fix. Offline compatibility checks reject valid-CRC older-schema records and the old26-byte PREVIOUS reply. Matching extracted scripts and archive checksums are verified before attachment.

These checks do not establish actual USB-only restart, electrical SRAM retention through power loss/programming or every reset, or arbitrary LTO instruction safety. Existing initial enumeration and powered-MCLR retention evidence applies to the previous image; report this image's initial results before any further test.
