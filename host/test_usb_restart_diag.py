import struct
import unittest
from unittest.mock import patch

import pic32_hid as host
import usb_restart_diag as diag


class Device:
    def __init__(self, module):
        self.module = module
        self.response = b""
        self.closed = False
    def open_path(self, path):
        self.path = path
    def write(self, report):
        request, _ = host.FrameDecoder().feed(report[1:])
        op, version, request_id = host.REQUEST_HEADER.unpack_from(request)
        self.module.sent.append(op)
        if op == diag.PAUSE:
            self.module.pauses += 1
            self.module.paused = True
            if self.module.lost_ack:
                raise OSError("interface disappeared after pause")
            state = (1, 1, 0, 1)
        elif self.module.reconnected:
            state = (1, 2, 1, 7) if not self.module.wrong else (1, 1, 0, 0)
        else:
            state = (0, 1, 0, 0)
        response = host.RESPONSE_HEADER.pack(op | 0x80, version, 0, 0, request_id) + diag.BODY.pack(*state)
        self.response = host.encode_frame(response).ljust(64, b"\0")
        return 65
    def read(self, size, timeout):
        response, self.response = self.response, b""
        return response
    def close(self):
        self.closed = True


class Module:
    def __init__(self, lost_ack=False, wrong=False, changed=False, multiple=False, no_remove=False):
        self.lost_ack, self.wrong, self.changed = lost_ack, wrong, changed
        self.multiple, self.no_remove = multiple, no_remove
        self.paused = self.reconnected = False
        self.pauses = self.polls = 0
        self.devices, self.sent = [], []
    def enumerate(self, vid, pid):
        if self.paused:
            self.polls += 1
            if self.polls == 1 and not self.no_remove:
                return []
            self.reconnected = True
        path = b"other" if self.changed and self.reconnected else b"board"
        one = {"path": path, "vendor_id": host.VID, "product_id": host.PID,
               "usage_page": 0xFF00, "usage": 1}
        return [one, dict(one, path=b"two")] if self.multiple else [one]
    def device(self):
        d = Device(self)
        self.devices.append(d)
        return d


class Clock:
    def __init__(self): self.now = 0
    def __call__(self): return self.now
    def sleep(self, duration): self.now += duration


class RestartDiagnostic(unittest.TestCase):
    def test_success_and_lost_ack_never_resend(self):
        for lost in (False, True):
            module = Module(lost_ack=lost)
            clock = Clock()
            result = diag.restart(module, 12, clock=clock, sleep=clock.sleep)
            self.assertEqual(result["after"], {"ticket": 1, "generation": 2, "completed_cycles": 1, "state": 7})
            self.assertEqual(module.pauses, 1)
            self.assertTrue(all(d.closed for d in module.devices))
            self.assertEqual(bool(result["pause_response_uncertainty"]), lost)
    def test_reject_ambiguity_epoch_path_and_missing_removal(self):
        for args in (dict(wrong=True), dict(changed=True), dict(multiple=True), dict(no_remove=True)):
            module = Module(**args)
            clock = Clock()
            with self.assertRaises(host.HostError):
                diag.restart(module, 12, path_hex=b"board".hex(), reconnect_timeout=1,
                             clock=clock, sleep=clock.sleep)
            self.assertLessEqual(module.pauses, 1)
            self.assertTrue(all(d.closed for d in module.devices))
    def test_blocking_call_overrun_never_reports_success(self):
        for slow_write in (True, False):
            module = Module()
            clock = Clock()
            class Slow(Device):
                def write(self, report):
                    count = super().write(report)
                    if slow_write:
                        clock.now += 2
                    return count
                def read(self, size, timeout):
                    data = super().read(size, timeout)
                    if not slow_write:
                        clock.now += 2
                    return data
            with self.assertRaisesRegex(host.HostError, "deadline exceeded"):
                diag.exchange(Slow(module), diag.payload(diag.STATUS, 1), 1, clock)
            self.assertEqual(module.sent, [diag.STATUS])

    def test_allowlist_envelope_and_deadline(self):
        for opcode in (2, 3, 5, 0x62, 0x63, 0x64, 0x70):
            with self.assertRaises(host.HostError):
                diag.payload(opcode, 1)
        for request in (b"", host.REQUEST_HEADER.pack(2, 1, 1), diag.payload(diag.STATUS, 1) + b"x"):
            with self.assertRaises(host.HostError):
                diag.exchange(Device(Module()), request)
        module = Module()
        clock = Clock()
        calls = []
        original = diag.exchange
        def measured(device, request, timeout=2, clock=clock):
            calls.append((request[0], timeout))
            return original(device, request, timeout, clock)
        with patch.object(diag, "exchange", measured):
            diag.restart(module, 12, timeout=30, reconnect_timeout=1, clock=clock, sleep=clock.sleep)
        self.assertLessEqual([timeout for op, timeout in calls if op == diag.PAUSE][0], 1)


if __name__ == "__main__":
    unittest.main()
