# Python PIC32 HID host — offline planning and read-only tools

`pic32_hid.py` is a **Python 3.10+ command-line tool** for offline image preparation/upload planning, HID discovery and a versioned read-only capabilities probe. **It has no upload, erase, reset, run or abort command.** The current read-only diagnostic revision now implements the capabilities handler; program it and enter with SW1 before probing. Older enumeration-only firmware will time out. See [the hardware test](../docs/usb-host-probe.md).

Python was explicitly selected for the Windows host. Offline commands use only the standard library; HID discovery/probe optionally use the PyPI **hidapi** package, imported as `hid`. Do not install the unrelated package named `hid`.

## Power-observation diagnostics — source only

The schema4 diagnostic adds `usb_restart_diag.py power-previous`, a read-only query for PRE/POST/READY power/BUSY samples and raw Count deltas captured at startup. PREVIOUS remains30 bytes; the new reply is also30 bytes. Invalid/absent samples are distinct from sampled zero bits. Timing includes logging and may wrap; these fields never authorise restart.

The temporary48KB build reports application origin `0x1D00C000`. Only `pic32_hid.py probe --power-debug` accepts that layout; normal `probe` still requires `0x1D008000`, and offline planning is unchanged. This option does not relocate an application or enable uploading. Older firmware has no `power-previous` handler. Use matching scripts only under a separately approved package/procedure; no new hardware commands are requested here. See [implementation and layout limits](../docs/usb-power-observation-implementation.md).

The experimental Harmony-powered source rule adds DRIVER_READY `usb_power_off` (mask16384), retaining legacy `usb_busy` (mask1024) decoding for historical records. Powered BUSY can be evidence on a successful readiness check; it never means USB-off NVM permission. The [concern and source change](../docs/usb-harmony-powered-readiness.md) are documented; no new board command is requested by this host change.

## Windows setup and offline check

From the repository root:

```powershell
py -3 host\pic32_hid.py selftest
```

For HID list/probe, use a project virtual environment if preferred:

```powershell
py -3 -m venv host\.venv
host\.venv\Scripts\python.exe -m pip install -r host\requirements-hid.txt
host\.venv\Scripts\python.exe host\pic32_hid.py list
```

`list` sends no USB reports. It selects vendor-defined usage page **FF00**, usage **1**, VID/PID **04D8:003C**, and lists exact paths. More than one matching interface is rejected by probe unless an explicit `--path-hex` selects one. No serial string is assumed.

The `requirements-hid.txt` version range is the API target, not a Windows-wheel compatibility result. HID calls pass mocks and the user confirmed Windows list/probe success on 2026-10-15. The exact installed Python/hidapi versions were not supplied; this is a tested environment, not proof of every version in the requirement range.

## Prepare a relocated image offline

```powershell
py -3 host\pic32_hid.py prepare path\to\relocated-application.hex --output prepared-image.zip
```

The tool requires the **bootloader-compatible relocated application** at physical **1D008000** (or matching cached/uncached aliases), with data ending before **1D03FC00**, the reserved metadata page. The existing tagged standalone blink HEX is **not** this image and should be rejected; do not hide its boot/configuration records to make preparation succeed without the agreed linker/startup change.

Input rules:

- Intel HEX types 00/01/02/04/05 only, strict lengths/checksum/EOF, no internal whitespace, 4 MiB input limit.
- Reject bootloader, boot/configuration, RAM/SFR, metadata, flash-end crossing, offset overflow and overlapping data—even identical overlaps.
- First four application bytes must be explicitly provided and not all erased.
- Any start-linear record must map to application start; missing type 05 is allowed and normalized entry is fixed to application start.
- Source ordering may vary; output is sorted physical, contiguous, 128-byte data records, with sparse gaps filled **FF**.

The exclusive-create ZIP contains:

- `application.bin`: exact declared span, including FF gaps.
- `application.hex`: canonical physical-address records for that span plus entry/EOF.
- `manifest.json`: schema/device/origin/entry, length, standard CRC32, SHA-256, metadata origin and row/erase sizes.

No device is opened, no flash is touched, and existing output files are never overwritten. The SHA/CRC are integrity checks, **not image authentication or executable correctness**. Metadata is not prewritten in this package; the device must commit after its own verification.

## Plan an upload offline

```powershell
py -3 host\pic32_hid.py plan path\to\relocated-application.hex --output upload-plan.json
```

`plan` validates and normalises the HEX using the same image rules as `prepare`. Its exclusive-create JSON includes the manifest, BEGIN/BATCH payloads, escaped frames, 65-byte report fixtures (including ID 0), and READY/COMPLETE wait conditions. Batches hold complete binary HEX records, at most 496 record bytes plus the 14-byte envelope.

No HID module is imported and no request is transmitted. Default session token/request ID 1 are deterministic offline fixtures; optional `--session-token` and `--request-base` change those fixtures. A future uploader must allocate fresh identifiers, negotiate write permission and verify progress. The current USB endpoint rejects mutation requests. The read-only `transact` function also refuses every planned mutation.

The inactive C dispatcher supports manifest-aware BEGIN/BATCH/ABORT and STATUS with exact-latest-request replay, token/sequence checks and asynchronous completion. See [the dispatcher contract](../docs/update-dispatch.md) for layouts, result codes, retry/recovery limits and tests. It has no target backend or active transport binding.

## Read-only protocol

Framing matches the bounded C layer: SOH **01**, EOT **04**, DLE **10**, reserved-byte escaping, CRC16 polynomial 1021/init 0/little-endian trailing bytes, 510-byte maximum payload, HID data reports 64 bytes. HIDAPI writes include leading **report ID 0**, making each output call 65 bytes; input reads request 64 data bytes. Windows capabilities probing and reconnect/MCLR/power-cycle smoke tests passed on 2026-10-15; multi-report upload behaviour has not been qualified.

Versioned request payload:

```text
opcode:u8  protocol_version:u8(1)  request_id:u32le(nonzero)
```

Read-only opcodes are **60 CAPABILITIES** and **61 STATUS**. Only CAPABILITIES has a live endpoint/CLI. Offline plans encode **62 BEGIN** and **63 BATCH**; **64 ABORT** exists in the inactive dispatcher and **65 RUN** remains unsupported. HID transport rejects all mutation opcodes. No legacy destructive command or fallback is sent.

Response envelope:

```text
opcode|80:u8  version:u8(1)  status:u8(0=success)  reserved:u8(0)
request_id:u32le  response_body...
```

Capabilities body (20 bytes):

```text
magic:"PKBL"  version:u16le(1)  max_payload:u16le(510)
origin:u32le(1D008000)  image_end_exclusive:u32le(1D03FC00)
row_size:u16le(128)  erase_size:u16le(1024)
```

`bootloader/src/update_wire.c/.h` implements pure C validation/response construction for this same read-only schema, and is now called by the user-owned usb_probe.c read-only endpoint. C/Python tests agree on the golden envelope vector. The inactive dispatcher defines a STATUS body and Python `parse_state` validates it. STATUS has no live handler/CLI in the diagnostic build.

```powershell
host\.venv\Scripts\python.exe host\pic32_hid.py probe --timeout 2
# Select a specific listed device if needed:
host\.venv\Scripts\python.exe host\pic32_hid.py probe --path-hex <hex-path-from-list>
```

**Current read-only firmware:** list should find it and probe should return capabilities after SW1-enabled startup/configuration. Older firmware without the endpoint will time out. Do not interpret that timeout as a mux/ADC failure if Windows enumeration works. No updater utility is enabled.

The transport checks version/opcode/request ID, length/layout, frame CRC, report size/padding and timeout; mismatches fail without legacy fallback. It bounds noise/report reads and does not automatically retry. HID device handles are closed even on failure. An OS HID write can still block beyond the user-space deadline; a later upload transport must qualify that behavior and recovery separately.

## Offline version-2 lifecycle codecs

`lifecycle_wire.py` constructs/parses separate version-2 HELLO/STAGE/EXECUTE/QUERY/RESUME fixtures with a 64-bit epoch, independent RPC/operation IDs and device-issued ticket. It has no HID I/O or CLI uploader. [Schema documentation](../docs/lifecycle-wire-v2.md) defines the 478-byte record bound and feature gating. C/Python golden vectors and exact 510-byte boundary tests pass; `pic32_hid.py transact` refuses these packets. The deployed firmware does not negotiate version 2. `test_link_dispatch.py` additionally exchanges dynamic ticket/result packets with a C mock subprocess, completes small/multi-window images and checks lost-response replay without another flash operation; see [dispatcher fixtures](../docs/link-dispatch-v2.md). It never opens HID hardware.

## USB-only restart diagnostic (hardware delivery gated)

`usb_restart_diag.py` is a separate STATUS/PAUSE verifier for the opt-in test firmware. It sends a pause once, requires a single unchanged HID path, observed removal and expected ticket/generation/cycle/DONE status after reconnect. PAUSE disconnects USB but performs no flash write. The normal hardware-tested firmware does not support it. An experimental test package now supplies the opt-in image; use only the status-first [controlled board procedure](../docs/usb-restart-board-test.md), with no hardware success assumed. See [host/image audit](../docs/usb-restart-host-audit.md). The existing `pic32_hid.py` transport still rejects pause/update commands.

## Tests

```sh
make test
# Python-only:
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s host -v
```

Twenty Python test methods cover frame splits/random chunking, CRC/rejections, HEX normalization/bounds/checksums/EOF/overlap, deterministic package output/no overwrite, request/version/ID/capability mismatches, short reports/timeouts, device selection, close-on-failure, and refusal of destructive opcodes/CLI. C wire tests validate the same capabilities vector; strict XC32 compilation and sanitizer checks pass. Planner tests check maximum-span reconstruction, request-ID overflow, report bounds/padding, no overwrite and refusal to send planned writes. `make test` also executes Python plans through the C framed dispatcher and mocked flash. `make test-sanitize` rebuilds those C tests with ASan/UBSan.

The Windows [read-only endpoint milestone](../docs/usb-host-probe.md) passed. Next implement/review the target backend, regenerate/relocate the application and integrate write-gated transport/recovery. No current artifact is a working uploader.
