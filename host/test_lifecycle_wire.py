import pathlib
import struct
import subprocess
import unittest

import pic32_hid as host
import lifecycle_wire as wire


class LifecycleWire(unittest.TestCase):
    def test_c_python_vectors_and_identity(self):
        payload = wire.request(wire.STAGE, 1, 7, 1, 1, kind=wire.BEGIN, length=128, crc=0xCBF43926)
        self.assertEqual(len(payload), 36)
        response = wire.RESPONSE.pack(0xF1, 2, 0, 0, 1) + wire.BODY.pack(
            b"PKL2", 2, 3, 510, 7, 1, 1, 1, 0, 128, 0xCBF43926, 1, 0, 0, 0, 0, b"\0\0\0")
        self.assertEqual(len(response), 56)
        parsed = wire.parse_response(response, wire.STAGE, 1, expected_identity=(7, 1, 1, 1))
        self.assertFalse(parsed["writes_enabled"] or parsed["transport_bound"])
        with self.assertRaises(host.HostError):
            wire.parse_response(response, wire.STAGE, 1, expected_identity=(8, 1, 1, 1))
        runner = pathlib.Path(host.__file__).resolve().parent.parent / "build/tests/update_lifecycle_wire_test"
        if runner.exists():
            result = subprocess.run([str(runner), "--vectors"], capture_output=True, check=True)
            self.assertEqual(result.stdout.decode().splitlines(), [payload.hex(), response.hex()])
        for offset in (0, 1, 3, 4, 8, 12, 14, 53):
            corrupt = bytearray(response)
            corrupt[offset] ^= 0x80
            with self.assertRaises(host.HostError):
                wire.parse_response(bytes(corrupt), wire.STAGE, 1)
        for bad in (b"", response[:-1], response + b"\0"):
            with self.assertRaises(host.HostError):
                wire.parse_response(bad, wire.STAGE, 1)

    def test_requests_bounds_and_readonly_guard(self):
        records = bytes.fromhex(host.hex_record(0, 0x8000, bytes(range(128)))[1:])
        payloads = [wire.request(wire.HELLO, 1)]
        for op in (wire.EXECUTE, wire.QUERY, wire.RESUME):
            payloads.append(wire.request(op, 2, 0x100000007, 1, 1, ticket=3))
        payloads.append(wire.request(wire.STAGE, 3, 7, 1, 1, kind=wire.BATCH, records=records * 3))
        payloads.append(wire.request(wire.STAGE, 4, 7, 1, 1, kind=wire.ABORT))
        boundary = bytes.fromhex(host.hex_record(0, 0x8000, bytes(255))[1:]) + bytes.fromhex(
            host.hex_record(0, 0x80FF, bytes(213))[1:])
        maximum = wire.request(wire.STAGE, 5, 7, 1, 1, kind=wire.BATCH, records=boundary)
        self.assertEqual(len(maximum), 510)
        payloads.append(maximum)
        for payload in payloads:
            self.assertLessEqual(len(payload), 510)
            self.assertEqual(host.FrameDecoder().feed(host.encode_frame(payload))[0], payload)
            class Device:
                def write(self, report): raise AssertionError("offline schema must not transmit")
            with self.assertRaises(host.HostError):
                host.transact(Device(), payload)
        invalid = [dict(opcode=256, rpc_id=1), dict(opcode=-1, rpc_id=1),
                   dict(opcode=wire.STAGE, rpc_id=1, epoch=7, token=1, operation_id=1, kind=256),
                   dict(opcode=wire.HELLO, rpc_id=0), dict(opcode=wire.HELLO, rpc_id=1, epoch=1),
                   dict(opcode=wire.STAGE, rpc_id=1, epoch=7, token=1, operation_id=1, kind=1, length=3),
                   dict(opcode=wire.STAGE, rpc_id=1, epoch=7, token=1, operation_id=1, kind=2, records=records * 4),
                   dict(opcode=wire.STAGE, rpc_id=1, epoch=7, token=1, operation_id=1, kind=2, records=b"bad"),
                   dict(opcode=wire.EXECUTE, rpc_id=1, epoch=7, token=1, operation_id=1, ticket=0),
                   dict(opcode=wire.QUERY, rpc_id=1, epoch=1 << 64, token=1, operation_id=1, ticket=1)]
        for args in invalid:
            with self.assertRaises(host.HostError):
                wire.request(**args)


if __name__ == "__main__":
    unittest.main()
