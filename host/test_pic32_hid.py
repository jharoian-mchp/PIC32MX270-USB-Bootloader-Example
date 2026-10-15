import binascii
import json
from pathlib import Path
import random
import struct
import subprocess
import sys
import tempfile
import unittest
import zipfile

import pic32_hid as host


def image_text(address=host.APP_START, data=b"\x01\x02\x03\x04", extra=()):
    return "\n".join([
        host.hex_record(4, 0, struct.pack(">H", address >> 16)),
        host.hex_record(0, address & 65535, data), *extra,
        host.hex_record(1, 0, b""),
    ]) + "\n"


class Frames(unittest.TestCase):
    def test_vectors_and_splits(self):
        self.assertEqual(host.crc16(b"123456789"), 0x31C3)
        self.assertEqual(host.encode_frame(b"\x01").hex(), "01100121101004")
        for payload in (b"\x01", bytes(range(256)), b"\x10" * 510):
            wire = host.encode_frame(payload)
            for split in range(len(wire) + 1):
                decoder = host.FrameDecoder()
                first, used = decoder.feed(wire[:split])
                self.assertEqual(used, split)
                result = first if first is not None else decoder.feed(wire[split:])[0]
                self.assertEqual(result, payload)
                with self.assertRaises(host.HostError):
                    decoder.feed(b"\0")

    def test_rejections(self):
        for bad in (b"", bytes(511)):
            with self.assertRaises(host.HostError):
                host.encode_frame(bad)
        for bad in (b"\x01\x04", b"\x01\x10\x55", b"\x01\x02\x55\x55\x04",
                    b"\x01" + b"U" * 513, b"bad-noise"):
            with self.assertRaises(host.HostError):
                host.FrameDecoder().feed(bad)
        decoder = host.FrameDecoder()
        decoder.feed(b"\x01\x10")
        decoder.reset()
        self.assertEqual(decoder.feed(b"\0\0"), (None, 2))

    def test_random_chunking(self):
        rng = random.Random(143)
        for _ in range(300):
            payload = rng.randbytes(rng.randrange(1, 511))
            wire = host.encode_frame(payload)
            decoder = host.FrameDecoder()
            pos = 0
            result = None
            while pos < len(wire):
                chunk = wire[pos:pos + rng.randrange(1, 65)]
                result, used = decoder.feed(chunk)
                self.assertEqual(used, len(chunk))
                pos += used
            self.assertEqual(result, payload)


class Images(unittest.TestCase):
    def test_valid_aliases_and_package(self):
        prepared = host.prepare_hex(image_text(extra=(host.hex_record(0, 0x8010, b"xyz"),)))
        self.assertEqual(prepared.binary, b"\x01\x02\x03\x04" + b"\xFF" * 12 + b"xyz")
        self.assertEqual(prepared.manifest["length"], 19)
        self.assertEqual(int(prepared.manifest["crc32"], 16), binascii.crc32(prepared.binary))
        self.assertEqual(host.prepare_hex(prepared.normalized_hex).binary, prepared.binary)
        for alias in (0x9D008000, 0xBD008000):
            self.assertEqual(host.prepare_hex(image_text(alias)).binary, b"\x01\x02\x03\x04")
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "image.zip"
            host.write_package(prepared, path)
            with zipfile.ZipFile(path) as archive:
                self.assertEqual(set(archive.namelist()), {"application.bin", "application.hex", "manifest.json"})
                self.assertEqual(archive.read("application.bin"), prepared.binary)
                self.assertEqual(json.loads(archive.read("manifest.json"))["length"], 19)
            before = path.read_bytes()
            with self.assertRaises(FileExistsError):
                host.write_package(prepared, path)
            self.assertEqual(path.read_bytes(), before)
            second = Path(directory) / "second.zip"
            host.write_package(prepared, second)
            self.assertEqual(path.read_bytes(), second.read_bytes())

    def test_reject_unsafe_input(self):
        for address in (0x1D000000, host.META_START, host.FLASH_END, 0x1FC00BF0, 0xA0000000, 0xFFFFFFFF):
            with self.assertRaises(host.HostError):
                host.prepare_hex(image_text(address))
        with self.assertRaises(host.HostError):
            host.prepare_hex(image_text(host.META_START - 2))
        for text in (
            image_text(data=b"\xFF" * 4), image_text(data=b"\x01\x02\x03"),
            image_text().rsplit(":", 1)[0], image_text() + host.hex_record(1, 0, b""),
            image_text(extra=(host.hex_record(0, 0x8000, b"\x01"),)),
            image_text(extra=(host.hex_record(5, 0, struct.pack(">I", host.APP_START + 4)),)),
            image_text(extra=(host.hex_record(3, 0, b"\0\0\0\0"),)),
            image_text().replace(":048000000102030472", ":048000000102030473"),
            ":FF00000000\n", ":00 000001FF\n", "not hex",
        ):
            with self.subTest(text=text[:40]):
                with self.assertRaises(host.HostError):
                    host.prepare_hex(text)

    def test_out_of_order_normalizes_and_window(self):
        lines = [host.hex_record(4, 0, b"\x1D\x00"), host.hex_record(0, 0x8010, b"z"),
                 host.hex_record(0, 0x8000, b"abcd"), host.hex_record(1, 0, b"")]
        self.assertEqual(host.prepare_hex("\n".join(lines)).binary, b"abcd" + b"\xFF" * 12 + b"z")
        data = bytes(range(256)) * 300
        source = []
        upper = None
        for off in range(0, len(data), 128):
            addr = host.APP_START + off
            if addr >> 16 != upper:
                upper = addr >> 16
                source.append(host.hex_record(4, 0, struct.pack(">H", upper)))
            source.append(host.hex_record(0, addr & 65535, data[off:off + 128]))
        source.append(host.hex_record(1, 0, b""))
        image = host.prepare_hex("\n".join(source))
        self.assertEqual(image.binary, data)
        self.assertEqual(host.prepare_hex(image.normalized_hex).binary, data)


class FakeDevice:
    def __init__(self, response=b"", short=False):
        self.response = [response[i:i + 64] for i in range(0, len(response), 64)]
        self.short = short
        self.writes = []
        self.closed = False
        self.path = None

    def write(self, data):
        self.writes.append(data)
        return len(data) - 1 if self.short else len(data)

    def read(self, length, timeout):
        return list(self.response.pop(0)) if self.response else []

    def open_path(self, path):
        self.path = path

    def close(self):
        self.closed = True


class WireAndHID(unittest.TestCase):
    def test_envelopes_and_readonly(self):
        request = host.request_payload(host.CAPABILITIES, 1)
        self.assertEqual(request, b"\x60\x01\x01\0\0\0")
        body = host.CAP_BODY.pack(b"PKBL", 1, 510, host.APP_START, host.META_START, 128, 1024)
        response = host.RESPONSE_HEADER.pack(0xE0, 1, 0, 0, 1) + body
        self.assertEqual(response.hex(), "e001000001000000504b424c0100fe010080001d00fc031d80000004")
        self.assertEqual(host.parse_response(response, host.CAPABILITIES, 1), body)
        self.assertEqual(host.parse_capabilities(body)["max_payload"], 510)
        for op in (2, 3, 5, 0x62, 0x63, 0x64, 0x65):
            with self.assertRaises(host.HostError):
                host.request_payload(op, 1)
            device = FakeDevice()
            with self.assertRaises(host.HostError):
                host.transact(device, struct.pack("<BBI", op, 1, 1))
            self.assertFalse(device.writes)
        for corrupt in (host.RESPONSE_HEADER.pack(0xE0, 2, 0, 0, 1),
                        host.RESPONSE_HEADER.pack(0xE0, 1, 1, 0, 1),
                        host.RESPONSE_HEADER.pack(0xE0, 1, 0, 0, 2),
                        host.RESPONSE_HEADER.pack(0xE0, 1, 0, 1, 1), b"short"):
            with self.assertRaises(host.HostError):
                host.parse_response(corrupt, 0x60, 1)
        with self.assertRaises(host.HostError):
            host.parse_capabilities(body[:-1])

    def test_transaction_selection_timeout(self):
        body = host.CAP_BODY.pack(b"PKBL", 1, 510, host.APP_START, host.META_START, 128, 1024)
        response = host.encode_frame(host.RESPONSE_HEADER.pack(0xE0, 1, 0, 0, 7) + body)
        device = FakeDevice(response.ljust(64, b"\0"))
        self.assertEqual(host.transact(device, host.request_payload(0x60, 7)), body)
        self.assertEqual(len(device.writes), 1)
        self.assertEqual(len(device.writes[0]), 65)
        self.assertEqual(device.writes[0][0], 0)
        for dev in (FakeDevice(short=True), FakeDevice(), FakeDevice(response + b"unexpected")):
            with self.assertRaises(host.HostError):
                host.transact(dev, host.request_payload(0x60, 7), clock=lambda: 0)
        wrong = FakeDevice(host.encode_frame(host.RESPONSE_HEADER.pack(0xE0, 1, 0, 0, 8) + body))
        with self.assertRaises(host.HostError):
            host.transact(wrong, host.request_payload(0x60, 7))
        for devices in ([], [{"path": b"a"}, {"path": b"b"}]):
            with self.assertRaises(host.HostError):
                host.select_device(devices)
        self.assertEqual(host.select_device([{"path": b"a"}, {"path": b"b"}], "62")["path"], b"b")
        with self.assertRaises(host.HostError):
            host.select_device([{"path": b"a"}], "not-hex")
        class Module:
            def __init__(self): self.dev = FakeDevice()
            def enumerate(self, *unused):
                return [{"vendor_id": host.VID, "product_id": host.PID, "usage_page": 0xFF00, "usage": 1, "path": b"a"}]
            def device(self): return self.dev
        module = Module()
        with self.assertRaises(host.HostError):
            host.probe(module, None, 0.1)
        self.assertTrue(module.dev.closed)
        class Broken(FakeDevice):
            def write(self, data): raise OSError("disconnected")
        with self.assertRaises(host.HostError):
            host.transact(Broken(), host.request_payload(0x60, 7))
        class BrokenRead(FakeDevice):
            def read(self, *unused): raise OSError("unplugged")
        with self.assertRaises(host.HostError):
            host.transact(BrokenRead(), host.request_payload(0x60, 7))

    def test_cli_no_hid_for_offline(self):
        result = subprocess.run([sys.executable, str(Path(host.__file__)), "selftest"], capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        result = subprocess.run([sys.executable, str(Path(host.__file__)), "upload"], capture_output=True)
        self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
