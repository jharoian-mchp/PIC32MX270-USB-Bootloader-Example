# Python PIC32 HID host — preparation and read-only tools

`pic32_hid.py` is a **Python 3.10+ command-line tool** for image preparation, HID discovery and a versioned read-only capabilities probe. **It has no upload, erase, reset, run or abort command.** The current read-only diagnostic revision now implements the capabilities handler; program it and enter with SW1 before probing. Older enumeration-only firmware will time out. See [the hardware test](../docs/usb-host-probe.md).

Python was explicitly selected for the Windows host. Offline commands use only the standard library; HID discovery/probe optionally use the PyPI **hidapi** package, imported as `hid`. Do not install the unrelated package named `hid`.

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

## Read-only protocol

Framing matches the bounded C layer: SOH **01**, EOT **04**, DLE **10**, reserved-byte escaping, CRC16 polynomial 1021/init 0/little-endian trailing bytes, 510-byte maximum payload, HID data reports 64 bytes. HIDAPI writes include leading **report ID 0**, making each output call 65 bytes; input reads request 64 data bytes. The actual platform behavior still needs a controlled Windows test.

Versioned request payload:

```text
opcode:u8  protocol_version:u8(1)  request_id:u32le(nonzero)
```

Read-only opcodes are **60 CAPABILITIES** and **61 STATUS**. Values **62–65** are reserved for later destructive/session requests but are rejected by this tool. No legacy destructive command or fallback is sent.

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

`bootloader/src/update_wire.c/.h` implements pure C validation/response construction for this same read-only schema, and is now called by the user-owned usb_probe.c read-only endpoint. C/Python tests agree on the golden envelope vector. STATUS response schema and CLI are not implemented; a later dispatcher can use that reserved query after its data contract is agreed.

```powershell
host\.venv\Scripts\python.exe host\pic32_hid.py probe --timeout 2
# Select a specific listed device if needed:
host\.venv\Scripts\python.exe host\pic32_hid.py probe --path-hex <hex-path-from-list>
```

**Current read-only firmware:** list should find it and probe should return capabilities after SW1-enabled startup/configuration. Older firmware without the endpoint will time out. Do not interpret that timeout as a mux/ADC failure if Windows enumeration works. No updater utility is enabled.

The transport checks version/opcode/request ID, length/layout, frame CRC, report size/padding and timeout; mismatches fail without legacy fallback. It bounds noise/report reads and does not automatically retry. HID device handles are closed even on failure. An OS HID write can still block beyond the user-space deadline; a later upload transport must qualify that behavior and recovery separately.

## Tests

```sh
make test
# Python-only:
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s host -v
```

Nine Python test groups cover frame splits/random chunking, CRC/rejections, HEX normalization/bounds/checksums/EOF/overlap, deterministic package output/no overwrite, request/version/ID/capability mismatches, short reports/timeouts, device selection, close-on-failure, and refusal of destructive opcodes/CLI. C wire tests validate the same capabilities vector; strict XC32 compilation and sanitizer checks pass. Existing firmware builds/tests remain unchanged.

Next follow [the Windows read-only endpoint test](../docs/usb-host-probe.md). Then define a negotiated BEGIN_IMAGE length/CRC and replay-safe update dispatcher, target backend and relocated application. No current artifact is a working uploader.
