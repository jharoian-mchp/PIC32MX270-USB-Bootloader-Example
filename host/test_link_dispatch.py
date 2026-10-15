"""Dynamic protocol exchanges over a C mock subprocess, never HID hardware."""
import binascii
from pathlib import Path
import struct
import subprocess
import unittest

import pic32_hid as host
import lifecycle_wire as wire


class DynamicHost(unittest.TestCase):
    def test_dynamic_tickets_image_and_lost_result(self):
        runner = Path(host.__file__).resolve().parent.parent / "build/tests/update_link_dispatch_test"
        if not runner.exists():
            self.skipTest("make test builds C mock dispatcher")
        # Small image and image crossing the 64 KiB physical address window.
        for image in (bytes(range(128)), bytes(range(256)) * 274):
            process = subprocess.Popen([str(runner), "--host"], stdin=subprocess.PIPE,
                                       stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            rpc_id = 0
            def command(text):
                process.stdin.write(text + "\n")
                process.stdin.flush()
                output = process.stdout.readline().strip()
                if not output:
                    raise AssertionError("mock process terminated: " + process.stderr.read())
                return output
            def send(opcode, epoch=0, token=0, operation_id=0, **body):
                nonlocal rpc_id
                rpc_id += 1
                payload = wire.request(opcode, rpc_id, epoch, token, operation_id, **body)
                output = command("send " + payload.hex()).split()
                self.assertNotEqual(output[0], "NONE")
                response = wire.parse_response(bytes.fromhex(output[0]), opcode, rpc_id)
                return response, tuple(map(int, output[1:]))
            try:
                hello, counts = send(wire.HELLO)
                self.assertEqual(hello["features"], 7)  # mock permission; no transport bit
                self.assertFalse(hello["transport_bound"])
                epoch = hello["epoch"]
                token = 0x100401
                operation_id = 1
                begin = dict(kind=wire.BEGIN, length=len(image), crc=binascii.crc32(image))
                stage, counts = send(wire.STAGE, epoch, token, operation_id, **begin)
                self.assertEqual(counts, (0, 0, 0))
                ticket = stage["ticket"]
                replay, again = send(wire.STAGE, epoch, token, operation_id, **begin)
                self.assertEqual(ticket, replay["ticket"])
                self.assertEqual(counts, again)
                last_counts = (0, 0)
                def execute_and_resume(operation_id, ticket):
                    nonlocal last_counts
                    accepted, info = send(wire.EXECUTE, epoch, token, operation_id, ticket=ticket)
                    self.assertEqual(accepted["result"], 0)
                    cookie, erases, writes = info
                    self.assertGreater(cookie, 0)
                    self.assertEqual((erases, writes), last_counts)
                    self.assertEqual(command(f"ack {cookie + 10000} 1"), "OK")  # stale/unmatched
                    self.assertEqual(command(f"ack {cookie} 1"), "OK")
                    pumped = command("pump").split()
                    self.assertEqual(pumped[0], "DONE")
                    last_counts = tuple(map(int, pumped[1:]))
                    # Simulate lost RESULT response: query twice, same outcome/no writes.
                    result, info = send(wire.QUERY, epoch, token, operation_id, ticket=ticket)
                    repeat, repeated = send(wire.QUERY, epoch, token, operation_id, ticket=ticket)
                    self.assertEqual(result, repeat)
                    self.assertEqual(info[1:], last_counts)
                    self.assertEqual(info, repeated)
                    self.assertEqual(result["outcome"], 1)
                    duplicate, info = send(wire.EXECUTE, epoch, token, operation_id, ticket=ticket)
                    self.assertEqual(info, (0, *last_counts))
                    wrong, info = send(wire.QUERY, epoch ^ 1, token, operation_id, ticket=ticket)
                    self.assertEqual(wrong["result"], 4)
                    resumed, info = send(wire.RESUME, epoch, token, operation_id, ticket=ticket)
                    self.assertEqual(resumed["result"], 0)
                    repeated, info = send(wire.RESUME, epoch, token, operation_id, ticket=ticket)
                    self.assertEqual(repeated["result"], 0)
                    return result
                execute_and_resume(operation_id, ticket)
                upper = None
                records = []
                for off in range(0, len(image), 128):
                    address = host.APP_START + off
                    if upper != address >> 16:
                        upper = address >> 16
                        records.append(bytes.fromhex(host.hex_record(4, 0, struct.pack(">H", upper))[1:]))
                    records.append(bytes.fromhex(host.hex_record(0, address & 65535, image[off:off + 128])[1:]))
                records.append(bytes.fromhex(host.hex_record(1, 0, b"")[1:]))
                batches = []
                batch = b""
                for record in records:
                    if len(batch) + len(record) > wire.RECORD_MAX:
                        batches.append(batch)
                        batch = b""
                    batch += record
                batches.append(batch)
                previous_ticket = ticket
                for sequence, batch in enumerate(batches):
                    operation_id += 1
                    stage, info = send(wire.STAGE, epoch, token, operation_id,
                                       kind=wire.BATCH, sequence=sequence, records=batch)
                    self.assertEqual(stage["result"], 0)
                    ticket = stage["ticket"]
                    self.assertGreater(ticket, previous_ticket)
                    previous_ticket = ticket
                    self.assertEqual(info, (0, *last_counts))
                    result = execute_and_resume(operation_id, ticket)
                    self.assertEqual(result["next_sequence"], sequence + 1)
                self.assertEqual(result["session"], 8)
                self.assertEqual(command("verify"), "VALID 1")
            finally:
                process.stdin.close()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
                error = process.stderr.read()
                process.stdout.close()
                process.stderr.close()
            self.assertEqual(process.returncode, 0, error)


if __name__ == "__main__":
    unittest.main()
