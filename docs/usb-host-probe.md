# Read-only Python-to-firmware probe test

The SW1-gated diagnostic firmware now answers the project's **read-only CAPABILITIES** request. The Python host validates its version, request ID and memory layout. **No erase/program/reset/session command is executed.** Linux builds, C/Python/mocked transport tests and sanitizers pass. The user confirmed the Windows Python HID list/capabilities probe succeeds on 2026-10-15.

## Firmware integration

User-owned `usb_probe.c/.h` receives EP1 OUT reports and sends EP1 IN responses after device configuration. `board_usb_diag.c` forwards HID report events, tells the endpoint when the device configures/resets/detaches, and polls it from the existing main loop. Generated drivers/descriptors and diagnostic main remain unchanged.

- Accept only valid CRC-protected **opcode 60**, version 1, nonzero request ID and exact six-byte query envelope.
- Construct the existing matching 28-byte capabilities payload (response envelope plus PKBL layout fields); frame it and zero-pad one **64-byte HID report**.
- Reject STATUS (not yet implemented), unknown/legacy/destructive commands, wrong version/ID/length, malformed CRC/escape/overflow and non-padding data after a query. No legacy fallback or update/session dispatch.
- Preserve split-frame/escape state across input reports; partial frames time out after one second.
- Submit no second input request while a response is queued/sending. Keep persistent RX/TX buffers until completion; check actual transfer length/handle/status.
- Retry queue rejection with bounded timing. A failure returning a live transfer handle quarantines its buffer instead of falsely treating the operation as unsubmitted.
- On USB reset/deconfiguration/ADC detach, cancel endpoint queues and reset framing only after pending buffers are released. The generated single-transfer cancel can defer queue-head completion until ISR; this endpoint uses **USB_DEVICE_IRPCancelAll** on EP1 OUT/IN to flush the queues. Cancellation failures retain ownership and block reuse.
- Response submission/completion has a one-second timeout. This does not qualify every possible USB-stack failure; hardware reconnect tests are required.

Only the pure frame/read-only wire functions are newly linked. Target update-session/HEX/flash/NVM erase/write and generated update tasks remain absent from the production diagnostic ELF. NVM initialization and generated application-jump helper can remain; the existing strong diagnostic trigger still prevents handoff.

## Windows test

1. Pull, reopen **bootloader/USB_HID_bootloader.X**, Clean/Build and program **this read-only diagnostic revision** with PICkit 5. Verify `usb_probe.c` is compiled. Preserve all usual board power/jumper settings; Type-A unused.
2. **Hold SW1 through reset for at least 100 ms**, then release. Confirm the existing HID device enumerates at VID_04D8/PID_003C. Normal reset without SW1 still safe-idles and should not answer the probe.
3. Install/use Python **3.10+** and optional HIDAPI from the repository root:

```powershell
py -3 host\pic32_hid.py selftest
py -3 -m venv host\.venv
host\.venv\Scripts\python.exe -m pip install -r host\requirements-hid.txt
host\.venv\Scripts\python.exe host\pic32_hid.py list
host\.venv\Scripts\python.exe host\pic32_hid.py probe --timeout 2
```

If multiple matching interfaces are listed, select the exact path with `--path-hex` as explained in [the host guide](../host/README.md). Do not install the unrelated package named `hid`; this tool expects PyPI **hidapi**.

Expected successful JSON:

```json
{
  "protocol": 1,
  "max_payload": 510,
  "origin": "0x1D008000",
  "image_end": "0x1D03FC00",
  "row_size": 128,
  "erase_size": 1024
}
```

4. Run probe repeatedly while firmware runs freely, then unplug/replug J2 and repeat. The diagnostics should show accepted query/response counts increasing. Check MCLR/power-cycle recovery holding SW1 and that reset without SW1 no longer enumerates.
5. Send `list`/`probe` output and debugger counters if it fails. Check `board_usb_diagnostics` still has valid ADC/mux/attachment and `usb_probe_diagnostics.active` is true after configuration. Avoid repeated breakpoint stops during reports; halting can cause host/firmware timeouts.

This is an approved **read-only query test**, not approval to run a vendor updater or program an application through USB. The Python CLI still contains no destructive operation.

## Windows stale source-list build failure — 2026-10-15

User build log attachment 111 fails at link time on USB_PROBE_Configure/Event/Initialize/Tasks. The link command omits **usb_probe.o** and **update_wire.o**. Comparison with the saved project XML shows **32 C source entries but only 30 compiled** in that log; both missing sources are present in Git/XML and resolve locally. A repeated local XC32 build links all four USB_PROBE definitions successfully. No source-code fix or generated-file edit is needed on that evidence; stale IDE/generated-makefile state is the likely cause.

Recovery:

1. Close the bootloader project in MPLAB X, confirm the latest Git files are present, reopen `bootloader/USB_HID_bootloader.X`, then Clean/Build. Check output contains compilation/link objects for both **usb_probe.c** and **update_wire.c**.
2. If the files still appear absent in the IDE source tree, add those existing files through **Source Files → Add Existing Item**, save and rebuild; do not create duplicate source copies.
3. If project entries are present but the makefile remains stale, close MPLAB X and remove only ignored `nbproject/Makefile-default.mk`, `nbproject/Makefile-impl.mk` and `nbproject/Makefile-variables.mk`, then reopen/build so the IDE recreates them. **Keep configurations.xml, project.xml, the root project Makefile and nbproject/private/**; do not delete all project metadata or private programmer settings.

**Resolved, user-confirmed 2026-10-15:** both source files were visible in the project, but deleting/recreating the generated makefiles fixed compilation/linking. The program is now built on Windows. This confirms stale generated makefile state as the cause; no C-source fix was required. Programming this revision and the read-only Python probe remain to be reported.

## Windows hardware result — 2026-10-15

User terminal output confirms `py -3 host\pic32_hid.py list` finds one interface with VID **04D8**, PID **003C**, interface 0, usage page **FF00**, usage 1 and product string **USB HID Bootloader**. Serial is empty, as expected.

`probe --timeout 2` returns protocol **1**, max payload **510**, origin **0x1D008000**, exclusive image end **0x1D03FC00**, row size **128** and erase size **1024**. The host validated the matching frame CRC, version/opcode/request ID and capabilities layout, so the first end-to-end Python/HID/firmware read-only transaction passes. The observed Windows hidapi API/report conventions work in this environment; no Python/hidapi version number was supplied.

The user subsequently confirmed **all requested exercises passed**: repeated probes, USB unplug/replug, MCLR while holding SW1, and power cycle while holding SW1. This completes the read-only host-link/lifecycle smoke test in the user's Windows environment. No per-cycle counters, counts of repetitions or interrupted mid-report fault-injection results were supplied, so this is not exhaustive cancellation qualification.

No flash write occurred through the host tool and update/erase/reset commands remain disabled. Next implement/test manifest/upload schema and target integration, with the application regeneration/relocation follow-up still open.

## Endpoint diagnostics

Watch **usb_probe_diagnostics**:

| Field | Meaning |
|---|---|
| active | Device currently configured for queries. |
| queries / responses | Accepted capabilities queries / successful report completions. |
| rejected / frame_errors | Invalid/unimplemented queries / framing errors. |
| rx_pending / tx_pending | Report buffers still owned by the HID stack. |
| rx_errors / tx_errors | Submission/completion failures; may include transient queue results. |
| timeouts | Partial-frame or response timeout count. |
| cancel_errors | Queue cancellation failed; buffers are retained until released. |

The existing historical board USB_OPEN diagnostic is separate from endpoint errors. Successful read-only probe should not change any flash/application image or reset the board.

## Software evidence and remaining work

Tests use mocked HID transfers and cover split queries, padding/CRC, all 255 non-CAPABILITIES opcodes rejected, version/ID failure, queue retries, stale/wrong completion handle, partial/send timeout, cancellation, reconfiguration and failed submit with live handle/buffer retention. Sanitizers and strict user-code XC32 compile pass; repeated generated-header unused-parameter warnings are excluded for that strict compile only, not globally suppressed.

The active read-only build fits the **32 KB bootloader reservation**, but its larger footprint is not the final updater size. Combined target flash/session/dispatcher fit remains unverified. Existing application/SW1/validation/session tests and the generated-defect audit still pass.

After this read-only link passes on Windows, define the manifest/start/status and replay-safe upload schema, bind the target backend and regenerate/relocate the application. Firmware update and power-cut recovery remain future tests.
