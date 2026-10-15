#!/usr/bin/env python3
"""PIC32MX270 HID host: offline image preparation/planning and read-only probe.

Python 3.10+. stdlib only for prepare/plan/selftest; optional PyPI 'hidapi'
exposes module 'hid' for list/probe. NO uploader or hardware-write transport.
"""
from __future__ import annotations

import argparse
import binascii
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import secrets
import struct
import sys
import time
import zipfile

VID = 0x04D8
PID = 0x003C
APP_START = 0x1D008000
META_START = 0x1D03FC00
FLASH_END = 0x1D040000
ROW_SIZE = 128
ERASE_SIZE = 1024
PAYLOAD_MAX = 510
REPORT_SIZE = 64
PROTOCOL_VERSION = 1
CAPABILITIES = 0x60
STATUS = 0x61
BEGIN = 0x62
BATCH = 0x63
ABORT = 0x64
BATCH_BYTES_MAX = PAYLOAD_MAX - 14
STATE_BODY = struct.Struct("<IIIIBBBB")
BEGIN_BODY = struct.Struct("<BBIIII")
BATCH_HEADER = struct.Struct("<BBIII")
# Mutation payloads are offline fixtures ONLY; transact refuses them. No RUN.
REQUEST_HEADER = struct.Struct("<BBI")
RESPONSE_HEADER = struct.Struct("<BBBBI")
CAP_BODY = struct.Struct("<4sHHIIHH")
MAX_HEX_BYTES = 4 * 1024 * 1024


class HostError(ValueError):
    """Safe validation, selection, or protocol failure; no automatic retries."""


def crc16(data: bytes) -> int:
    return binascii.crc_hqx(data, 0)


def encode_frame(payload: bytes) -> bytes:
    if not 1 <= len(payload) <= PAYLOAD_MAX:
        raise HostError("payload must contain 1..510 bytes")
    body = payload + struct.pack("<H", crc16(payload))
    return b"\x01" + b"".join(
        (b"\x10" + bytes([b])) if b in (1, 4, 16) else bytes([b])
        for b in body
    ) + b"\x04"


class FrameDecoder:
    """State survives reports/escapes. Returns payload and consumed byte count.

    READY locks the decoder until reset. Error resets and raises; caller must
    terminate this transaction rather than accept a later unrelated frame.
    """
    def __init__(self) -> None:
        self.reset()

    def reset(self) -> None:
        self.body = bytearray()
        self.receiving = False
        self.escaped = False
        self.ready = False

    def _fail(self, message: str) -> None:
        self.reset()
        raise HostError(message)

    def feed(self, data: bytes) -> tuple[bytes | None, int]:
        if self.ready:
            raise HostError("completed frame not reset")
        for index, byte in enumerate(data):
            if self.escaped:
                self.escaped = False
                if byte not in (1, 4, 16):
                    self._fail("invalid escape")
            elif byte == 1:
                self.reset()
                self.receiving = True
                continue
            elif not self.receiving:
                if byte != 0:
                    self._fail("non-padding data outside response frame")
                continue
            elif byte == 16:
                self.escaped = True
                continue
            elif byte == 4:
                if len(self.body) < 3:
                    self._fail("short frame")
                payload = bytes(self.body[:-2])
                expected = struct.unpack("<H", self.body[-2:])[0]
                if crc16(payload) != expected:
                    self._fail("frame CRC mismatch")
                self.ready = True
                self.receiving = False
                return payload, index + 1
            if len(self.body) >= 512:
                self._fail("frame exceeds 512 decoded bytes")
            self.body.append(byte)
        return None, len(data)


def physical_address(address: int, length: int) -> int:
    if 0x1D000000 <= address < FLASH_END:
        result = address
    elif 0x9D000000 <= address < 0x9D040000:
        result = address - 0x80000000
    elif 0xBD000000 <= address < 0xBD040000:
        result = address - 0xA0000000
    else:
        raise HostError(f"address 0x{address:08X} is not application flash")
    if length <= 0 or result < APP_START or result >= META_START or length > META_START - result:
        raise HostError(f"span 0x{address:08X}+{length} overlaps bootloader/metadata or flash end")
    return result


def hex_record(kind: int, offset: int, data: bytes) -> str:
    if not 0 <= len(data) <= 255 or not 0 <= offset <= 0xFFFF:
        raise HostError("invalid normalized HEX record")
    body = bytes([len(data)]) + struct.pack(">H", offset) + bytes([kind]) + data
    return ":" + (body + bytes([(-sum(body)) & 255])).hex().upper()


@dataclass(frozen=True)
class PreparedImage:
    binary: bytes
    normalized_hex: str
    manifest: dict


def prepare_hex(text: str) -> PreparedImage:
    """Validate before normalization. Reject boot/config/metadata; never discard.

    Accept physical/KSEG0/KSEG1 aliases and out-of-order source records, but reject
    overlaps even if identical. Output sorted physical records with erased gaps.
    """
    if len(text.encode("utf-8")) > MAX_HEX_BYTES:
        raise HostError("HEX input exceeds 4 MiB limit")
    memory: dict[int, int] = {}
    extension = 0
    eof = False
    entry: int | None = None
    records = 0
    for number, raw in enumerate(text.splitlines(), 1):
        line = raw.strip()
        if not line:
            continue
        if eof:
            raise HostError(f"line {number}: record after EOF")
        if not line.startswith(":") or len(line) < 11 or (len(line) - 1) % 2 or len(line) > 521:
            raise HostError(f"line {number}: invalid record syntax/size")
        try:
            body = bytes.fromhex(line[1:])
        except ValueError as exc:
            raise HostError(f"line {number}: non-hexadecimal record") from exc
        # bytes.fromhex tolerates internal whitespace: disallow it explicitly.
        if any(ch not in "0123456789abcdefABCDEF" for ch in line[1:]):
            raise HostError(f"line {number}: whitespace/non-hex inside record")
        if len(body) != body[0] + 5 or sum(body) & 255:
            raise HostError(f"line {number}: length/checksum mismatch")
        size = body[0]
        offset = int.from_bytes(body[1:3], "big")
        kind = body[3]
        data = body[4:-1]
        records += 1
        if kind == 0:
            if size == 0 or offset + size > 0x10000 or extension + offset + size > 0x100000000:
                raise HostError(f"line {number}: empty/overflowing data record")
            address = physical_address(extension + offset, size)
            for index, byte in enumerate(data):
                if address + index in memory:
                    raise HostError(f"line {number}: overlapping data at 0x{address + index:08X}")
                memory[address + index] = byte
        elif kind in (2, 4):
            if size != 2 or offset != 0:
                raise HostError(f"line {number}: invalid address extension")
            extension = int.from_bytes(data, "big") << (4 if kind == 2 else 16)
        elif kind == 1:
            if size != 0 or offset != 0:
                raise HostError(f"line {number}: malformed EOF")
            eof = True
        elif kind == 5:
            if size != 4 or offset != 0 or entry is not None:
                raise HostError(f"line {number}: invalid/duplicate entry record")
            entry = physical_address(int.from_bytes(data, "big"), 1)
            if entry != APP_START:
                raise HostError(f"line {number}: entry is not 0x{APP_START:08X}")
        else:
            raise HostError(f"line {number}: unsupported record type {kind:02X}")
    if not eof or not memory:
        raise HostError("HEX requires data and one final EOF")
    if any(APP_START + i not in memory for i in range(4)):
        raise HostError("first four application bytes must be explicitly provided; regenerate a relocated image")
    end = max(memory) + 1
    binary = bytearray(b"\xFF" * (end - APP_START))
    for address, byte in memory.items():
        binary[address - APP_START] = byte
    if binary[:4] == b"\xFF" * 4:
        raise HostError("application reset word is erased")
    # Canonical contiguous image, including sparse 0xFF gaps, 128-byte records.
    lines = []
    previous_upper = None
    for offset in range(0, len(binary), ROW_SIZE):
        address = APP_START + offset
        upper = address >> 16
        if upper != previous_upper:
            lines.append(hex_record(4, 0, struct.pack(">H", upper)))
            previous_upper = upper
        count = min(ROW_SIZE, len(binary) - offset, 0x10000 - (address & 0xFFFF))
        lines.append(hex_record(0, address & 0xFFFF, bytes(binary[offset:offset + count])))
    lines.append(hex_record(5, 0, struct.pack(">I", APP_START)))
    lines.append(hex_record(1, 0, b""))
    data = bytes(binary)
    return PreparedImage(data, "\n".join(lines) + "\n", {
        "schema": "pic32mx270-hid-image/1", "device": "PIC32MX270F256D",
        "origin": f"0x{APP_START:08X}", "entry": f"0x{APP_START:08X}",
        "length": len(data), "crc32": f"0x{binascii.crc32(data) & 0xFFFFFFFF:08X}",
        "sha256": hashlib.sha256(data).hexdigest(),
        "metadata_origin": f"0x{META_START:08X}", "row_size": ROW_SIZE,
        "erase_size": ERASE_SIZE, "source_records": records,
        "gap_fill": "0xFF", "authentication": "none",
    })


def write_package(image: PreparedImage, path: Path) -> None:
    # Exclusive create: never overwrite a previous artifact or user file.
    created = False
    try:
        with path.open("xb") as output:
            created = True
            with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as archive:
                for name, data in (
                    ("application.bin", image.binary),
                    ("application.hex", image.normalized_hex.encode("ascii")),
                    ("manifest.json", (json.dumps(image.manifest, indent=2, sort_keys=True) + "\n").encode()),
                ):
                    info = zipfile.ZipInfo(name, (1980, 1, 1, 0, 0, 0))
                    info.compress_type = zipfile.ZIP_DEFLATED
                    info.external_attr = 0o100644 << 16
                    archive.writestr(info, data)
    except Exception:
        if created:
            path.unlink(missing_ok=True)
        raise


def upload_plan(text: str, token: int = 1, request_base: int = 1) -> dict:
    """Offline fixtures only: deterministic IDs are NOT live session allocation.

    Future uploader must probe update permission, allocate a fresh session token,
    wait for each operation, check status/token/sequence and never replay older
    mutations. No plan is passed to the strictly read-only HID transport.
    """
    for name, value in (("session token", token), ("request base", request_base)):
        if type(value) is not int or not 1 <= value <= 0xFFFFFFFF:
            raise HostError(f"invalid {name}: must be 1..0xFFFFFFFF")
    image = prepare_hex(text)
    batches: list[bytes] = []
    batch = bytearray()
    for line in image.normalized_hex.splitlines():
        record = bytes.fromhex(line[1:])
        if len(batch) + len(record) > BATCH_BYTES_MAX:
            batches.append(bytes(batch))
            batch.clear()
        batch.extend(record)
    if batch:
        batches.append(bytes(batch))
    if request_base + len(batches) > 0xFFFFFFFF:
        raise HostError("mutation request IDs would overflow; no wrapping allowed")
    operations = []

    def operation(payload: bytes, wait: str, sequence: int | None) -> None:
        wire = encode_frame(payload)
        reports = [b"\0" + wire[off:off + REPORT_SIZE].ljust(REPORT_SIZE, b"\0")
                   for off in range(0, len(wire), REPORT_SIZE)]
        operations.append({
            "opcode": payload[0], "request_id": int.from_bytes(payload[2:6], "little"),
            "batch_sequence": sequence, "wait_for": wait,
            "payload_hex": payload.hex(), "framed_hex": wire.hex(),
            "hid_reports_hex": [report.hex() for report in reports],
        })

    operation(BEGIN_BODY.pack(BEGIN, PROTOCOL_VERSION, request_base, token,
                              len(image.binary), int(image.manifest["crc32"], 16)), "READY", None)
    for sequence, records in enumerate(batches):
        payload = BATCH_HEADER.pack(BATCH, PROTOCOL_VERSION, request_base + 1 + sequence,
                                    token, sequence) + records
        operation(payload, "COMPLETE" if sequence == len(batches) - 1 else "READY", sequence)
    return {
        "schema": "pic32mx270-hid-plan/1", "offline_only": True,
        "hardware_writes_enabled": False, "session_token": token,
        "image": image.manifest, "batch_records_max": BATCH_BYTES_MAX,
        "operations": operations,
        "retry_policy": "stop-and-wait; exact latest accepted mutation only; never replay earlier IDs",
        "warning": "Offline fixture; no device opened. Deployed firmware rejects mutations. "
                   "No uploader, automatic retry/resume, reset or application handoff is implemented.",
    }


def write_plan(plan: dict, path: Path) -> None:
    created = False
    try:
        with path.open("x", encoding="ascii") as output:
            created = True
            json.dump(plan, output, indent=2, sort_keys=True)
            output.write("\n")
    except Exception:
        if created:
            path.unlink(missing_ok=True)
        raise


def parse_state(body: bytes) -> dict:
    """Decode inactive dispatcher state; no live STATUS CLI is connected."""
    if len(body) != STATE_BODY.size:
        raise HostError("invalid state response length")
    token, last_id, sequence, length, state, flash, hex_status, enabled = STATE_BODY.unpack(body)
    if state > 11 or flash > 6 or hex_status > 6 or enabled not in (0, 1):
        raise HostError("invalid state response fields")
    return {"session_token": token, "last_mutation_id": last_id,
            "next_batch_sequence": sequence, "length": length, "state": state,
            "flash_status": flash, "hex_status": hex_status, "updates_enabled": bool(enabled)}


def request_payload(opcode: int, request_id: int) -> bytes:
    if opcode not in (CAPABILITIES, STATUS):
        raise HostError("this host permits only read-only CAPABILITIES/STATUS requests")
    if not 1 <= request_id <= 0xFFFFFFFF:
        raise HostError("invalid request ID")
    return REQUEST_HEADER.pack(opcode, PROTOCOL_VERSION, request_id)


def parse_response(payload: bytes, opcode: int, request_id: int) -> bytes:
    if len(payload) < RESPONSE_HEADER.size:
        raise HostError("short response envelope")
    op, version, status, reserved, received_id = RESPONSE_HEADER.unpack_from(payload)
    if op != (opcode | 0x80) or version != PROTOCOL_VERSION or reserved != 0 or received_id != request_id:
        raise HostError("protocol/version/request-ID mismatch; no legacy fallback")
    if status != 0:
        raise HostError(f"device rejected read-only request (status {status})")
    return payload[RESPONSE_HEADER.size:]


def parse_capabilities(body: bytes) -> dict:
    if len(body) != CAP_BODY.size:
        raise HostError("invalid capabilities length")
    magic, protocol, maximum, origin, end, row, erase = CAP_BODY.unpack(body)
    if magic != b"PKBL" or protocol != PROTOCOL_VERSION or maximum != PAYLOAD_MAX or (
        origin, end, row, erase
    ) != (APP_START, META_START, ROW_SIZE, ERASE_SIZE):
        raise HostError("device capabilities/layout do not match this host")
    return {"protocol": protocol, "max_payload": maximum,
            "origin": f"0x{origin:08X}", "image_end": f"0x{end:08X}",
            "row_size": row, "erase_size": erase}


def hid_module():
    try:
        import hid  # PyPI hidapi, not the unrelated package named 'hid'.
    except ImportError as exc:
        raise HostError("HID commands require: py -m pip install -r host/requirements-hid.txt") from exc
    if not hasattr(hid, "device") or not hasattr(hid, "enumerate"):
        raise HostError("unexpected hid API; install the PyPI 'hidapi' package")
    return hid


def candidates(module) -> list[dict]:
    return [device for device in module.enumerate(VID, PID)
            if device.get("vendor_id") == VID and device.get("product_id") == PID
            and device.get("usage_page") == 0xFF00 and device.get("usage") == 1]


def select_device(devices: list[dict], path_hex: str | None = None) -> dict:
    if path_hex is not None:
        try:
            requested = bytes.fromhex(path_hex)
        except ValueError as exc:
            raise HostError("invalid --path-hex") from exc
        devices = [device for device in devices if device.get("path") == requested]
    if len(devices) != 1:
        raise HostError(f"expected exactly one vendor HID interface, found {len(devices)}; list and select --path-hex")
    return devices[0]


def transact(device, payload: bytes, timeout: float = 2.0, clock=time.monotonic) -> bytes:
    if not 0.1 <= timeout <= 30.0:
        raise HostError("timeout must be 0.1..30 seconds")
    # Defense in depth: no caller can use this read-only transport for old/new writes.
    if len(payload) != REQUEST_HEADER.size or payload[0] not in (CAPABILITIES, STATUS):
        raise HostError("refusing non-read-only transport request")
    opcode, version, request_id = REQUEST_HEADER.unpack(payload)
    if version != PROTOCOL_VERSION or not request_id:
        raise HostError("invalid read-only envelope")
    deadline = clock() + timeout
    wire = encode_frame(payload)
    # No automatic resend: future writes need explicit replay/idempotence rules.
    for start in range(0, len(wire), REPORT_SIZE):
        if clock() >= deadline:
            raise HostError("HID transaction timeout")
        report = b"\x00" + wire[start:start + REPORT_SIZE].ljust(REPORT_SIZE, b"\x00")
        try:
            count = device.write(report)
        except Exception as exc:
            raise HostError(f"HID write failed: {exc}") from exc
        if count != len(report):
            raise HostError("short HID write")
    decoder = FrameDecoder()
    for _ in range(64):  # Bound unsolicited/noise work as well as elapsed time.
        remaining = deadline - clock()
        if remaining <= 0:
            break
        try:
            report = bytes(device.read(REPORT_SIZE, max(1, min(100, int(remaining * 1000)))))
        except Exception as exc:
            raise HostError(f"HID read failed: {exc}") from exc
        if not report:
            continue
        if len(report) > REPORT_SIZE:
            raise HostError("unexpected HID input report length")
        result, used = decoder.feed(report)
        if result is not None:
            if any(report[used:]):
                raise HostError("non-padding bytes after response")
            return parse_response(result, opcode, request_id)
    raise HostError("no matching response; check SW1 entry, firmware revision and USB connection")


def probe(module, path_hex: str | None, timeout: float) -> dict:
    selected = select_device(candidates(module), path_hex)
    device = module.device()
    try:
        try:
            device.open_path(selected["path"])
        except Exception as exc:
            raise HostError(f"HID open failed: {exc}") from exc
        body = transact(device, request_payload(CAPABILITIES, secrets.randbelow(0xFFFFFFFF) + 1), timeout)
        return parse_capabilities(body)
    finally:
        device.close()


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    prepare = commands.add_parser("prepare", help="offline: validate relocated HEX and create image package")
    prepare.add_argument("hex", type=Path)
    prepare.add_argument("--output", type=Path, required=True)
    plan = commands.add_parser("plan", help="offline: create manifest/batch wire fixtures; sends no reports")
    plan.add_argument("hex", type=Path)
    plan.add_argument("--output", type=Path, required=True)
    plan.add_argument("--session-token", type=lambda value: int(value, 0), default=1)
    plan.add_argument("--request-base", type=lambda value: int(value, 0), default=1)
    commands.add_parser("list", help="enumerate matching HID interfaces; sends no reports")
    query = commands.add_parser("probe", help="send read-only versioned capabilities request; no legacy fallback")
    query.add_argument("--path-hex")
    query.add_argument("--timeout", type=float, default=2.0)
    commands.add_parser("selftest", help="offline known CRC/frame tests; no HID dependency")
    args = parser.parse_args(argv)
    try:
        if args.command == "prepare":
            if args.hex.stat().st_size > MAX_HEX_BYTES:
                raise HostError("HEX input exceeds 4 MiB limit")
            image = prepare_hex(args.hex.read_text(encoding="ascii"))
            write_package(image, args.output)
            print(json.dumps(image.manifest, indent=2, sort_keys=True))
            print(f"Prepared {args.output}; no device opened or programmed.")
        elif args.command == "plan":
            if args.hex.stat().st_size > MAX_HEX_BYTES:
                raise HostError("HEX input exceeds 4 MiB limit")
            result = upload_plan(args.hex.read_text(encoding="ascii"), args.session_token, args.request_base)
            write_plan(result, args.output)
            print(f"Planned {len(result['operations'])} mutations in {args.output}; offline only, no HID I/O.")
        elif args.command == "list":
            listed = candidates(hid_module())
            print(json.dumps([{key: (value.hex() if isinstance(value, bytes) else value)
                               for key, value in item.items()}
                              for item in listed], indent=2, sort_keys=True))
        elif args.command == "probe":
            print(json.dumps(probe(hid_module(), args.path_hex, args.timeout), indent=2, sort_keys=True))
        else:
            if crc16(b"123456789") != 0x31C3 or binascii.crc32(b"123456789") != 0xCBF43926:
                raise HostError("CRC selftest failed")
            payload = bytes(range(256))
            decoder = FrameDecoder()
            result, _ = decoder.feed(encode_frame(payload))
            if result != payload:
                raise HostError("frame selftest failed")
            print("PASS: offline CRC16/CRC32/frame checks; no HID I/O.")
    except (HostError, OSError, UnicodeError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
