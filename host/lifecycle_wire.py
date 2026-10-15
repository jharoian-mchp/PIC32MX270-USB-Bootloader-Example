"""Offline version-2 lifecycle wire fixtures. No HID I/O or uploader.

Existing pic32_hid.py transact remains version-1 read-only. Do not transmit these
packets to deployed firmware. Epoch freshness/authentication is not generated.
"""
import struct

from pic32_hid import HostError, PAYLOAD_MAX

VERSION = 2
HELLO, STAGE, EXECUTE, QUERY, RESUME = range(0x70, 0x75)
BEGIN, BATCH, ABORT = 1, 2, 3
HEADER = struct.Struct("<BBBBIQII")
RESPONSE = struct.Struct("<BBBBI")
BODY = struct.Struct("<4sBBHQIIIIIIBBBBB3s")
RECORD_MAX = 478


def uint(value, bits, name, nonzero=False):
    if type(value) is not int or not (1 if nonzero else 0) <= value < 1 << bits:
        raise HostError(f"invalid {name}")
    return value


def request(opcode, rpc_id, epoch=0, token=0, operation_id=0, *, ticket=0,
            kind=0, length=0, crc=0, sequence=0, records=b""):
    if type(opcode) is not int or opcode not in (HELLO, STAGE, EXECUTE, QUERY, RESUME):
        raise HostError("unknown lifecycle opcode")
    for value, bits, name, required in ((rpc_id, 32, "RPC ID", True),
                                      (epoch, 64, "epoch", opcode != HELLO),
                                      (token, 32, "token", opcode != HELLO),
                                      (operation_id, 32, "operation ID", opcode != HELLO)):
        uint(value, bits, name, required)
    for value, name in ((ticket, "ticket"), (kind, "kind"), (length, "length"),
                        (crc, "CRC"), (sequence, "sequence")):
        uint(value, 32, name)
    if kind > 255:
        raise HostError("invalid stage kind")
    payload = HEADER.pack(opcode, VERSION, 0, 0, rpc_id, epoch, token, operation_id)
    if opcode == HELLO:
        if any((epoch, token, operation_id, ticket, kind, length, crc, sequence, records)):
            raise HostError("HELLO has no identity/body")
    elif opcode == STAGE:
        if ticket:
            raise HostError("stage ticket is allocated by device")
        payload += bytes((kind, 0, 0, 0))
        if kind == BEGIN:
            if not 4 <= length <= 0x37C00 or sequence or records:
                raise HostError("invalid BEGIN body")
            payload += struct.pack("<II", length, crc)
        elif kind == BATCH:
            if length or crc or not isinstance(records, bytes) or not 1 <= len(records) <= RECORD_MAX:
                raise HostError("invalid BATCH body")
            off = 0
            while off < len(records):
                count = records[off] + 5
                record = records[off:off + count]
                if len(record) != count or sum(record) & 255:
                    raise HostError("invalid binary HEX record length/checksum")
                off += count
            payload += struct.pack("<I", sequence) + records
        elif kind == ABORT:
            if any((length, crc, sequence, records)):
                raise HostError("invalid ABORT body")
        else:
            raise HostError("unknown stage kind")
    elif opcode in (EXECUTE, QUERY, RESUME):
        if not ticket or any((kind, length, crc, sequence, records)):
            raise HostError("invalid ticketed request")
        payload += struct.pack("<I", ticket)
    else:
        raise HostError("unknown lifecycle opcode")
    if len(payload) > PAYLOAD_MAX:
        raise HostError("oversized lifecycle request")
    return payload


def parse_response(payload, opcode, rpc_id, *, expected_identity=None):
    if len(payload) != 56:
        raise HostError("invalid lifecycle response length")
    op, version, result, reserved, received = RESPONSE.unpack_from(payload)
    if opcode not in (HELLO, STAGE, EXECUTE, QUERY, RESUME) or op != (opcode | 0x80) or (
        version != VERSION or reserved or received != rpc_id or not received or result > 5
    ):
        raise HostError("lifecycle response envelope mismatch")
    (magic, schema, features, maximum, epoch, token, operation_id, ticket, sequence,
     length, crc, lifecycle, outcome, session, flash, hex_status, tail) = BODY.unpack_from(payload, 8)
    if magic != b"PKL2" or schema != VERSION or maximum != PAYLOAD_MAX or not epoch or (
        features & 0xF0 or lifecycle > 6 or outcome > 3 or session > 11 or flash > 6 or
        hex_status > 6 or tail != b"\0\0\0"
    ):
        raise HostError("lifecycle response schema/state mismatch")
    identity = (epoch, token, operation_id, ticket)
    if expected_identity is not None and identity != tuple(expected_identity):
        raise HostError("boot/session/operation/ticket mismatch; no automatic resume")
    return {"result": result, "features": features, "epoch": epoch, "token": token,
            "operation_id": operation_id, "ticket": ticket, "next_sequence": sequence,
            "length": length, "crc32": crc, "lifecycle": lifecycle, "outcome": outcome,
            "session": session, "flash": flash, "hex": hex_status,
            "writes_enabled": bool(features & 4), "transport_bound": bool(features & 8)}
