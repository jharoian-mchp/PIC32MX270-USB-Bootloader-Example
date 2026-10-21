import contextlib
import io
import struct
import sys
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
        if op == diag.PREVIOUS:
            body = diag.PREVIOUS_BODY.pack(1, 1, 10020, 8, 2, 1, 4, 1, 3, 0)
        elif op == diag.POWER_PREVIOUS:
            body = self.module.power_body if self.module.power_body is not None else bytes(diag.POWER_PREVIOUS_BODY.size)
        else:
            body = diag.BODY.pack(*state)
        response = host.RESPONSE_HEADER.pack(op | 0x80, version, 0, 0, request_id) + body
        self.response = host.encode_frame(response).ljust(64, b"\0")
        return 65
    def read(self, size, timeout):
        response, self.response = self.response, b""
        return response
    def close(self):
        self.closed = True


class Module:
    def __init__(self, lost_ack=False, wrong=False, changed=False, multiple=False, no_remove=False, power_body=None):
        self.lost_ack, self.wrong, self.changed = lost_ack, wrong, changed
        self.power_body = power_body
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

    def test_previous_rejects_stale_invalid_and_reserved_evidence(self):
        for values in ((1, 1, 1, 0, 0, 0, 0, 0, 0, 0),
                       (1, 1, 0, 0, 0, 0, 32, 1, 0, 0),
                       (0, 0, 1, 0, 0, 0, 0, 0, 0, 0)):
            class Bad(Device):
                def write(self, report):
                    n = super().write(report)
                    request, _ = host.FrameDecoder().feed(report[1:])
                    op, version, rid = host.REQUEST_HEADER.unpack_from(request)
                    self.response = host.encode_frame(host.RESPONSE_HEADER.pack(op | 0x80, version, 0, 0, rid) +
                        diag.PREVIOUS_BODY.pack(*values)).ljust(64, b"\0")
                    return n
            module = Module()
            with self.assertRaisesRegex(host.HostError, "noncanonical|reserved"):
                diag.exchange(Bad(module), diag.payload(diag.PREVIOUS, 1))
            self.assertEqual(module.sent, [diag.PREVIOUS])

    def test_expanded_layout_requires_explicit_readonly_probe_profile(self):
        normal = host.CAP_BODY.pack(b"PKBL", 1, 510, 0x1D008000, 0x1D03FC00, 128, 1024)
        debug = host.CAP_BODY.pack(b"PKBL", 1, 510, 0x1D00C000, 0x1D03FC00, 128, 1024)
        self.assertEqual(host.parse_capabilities(normal)["origin"], "0x1D008000")
        with self.assertRaises(host.HostError):
            host.parse_capabilities(debug)
        with self.assertRaises(host.HostError):
            host.parse_capabilities(normal, debug_layout=True)
        self.assertEqual(host.parse_capabilities(debug, debug_layout=True)["origin"], "0x1D00C000")
        self.assertEqual(host.APP_START, 0x1D008000) # offline planner is not relocated

    def test_previous_record_is_read_only_evidence(self):
        module = Module()
        device = Device(module)
        result = diag.exchange(device, diag.payload(diag.PREVIOUS, 1))
        self.assertTrue(result["valid"])
        self.assertEqual((result["phase"], result["fault"], result["checkpoint"]), ("DRAIN", "TIMEOUT", "DRAIN"))
        self.assertEqual(module.pauses, 0)
        self.assertEqual(module.sent, [diag.PREVIOUS])

    def test_power_previous_payload_and_decode_fixtures(self):
        request = diag.payload(diag.POWER_PREVIOUS, 0x12345678)
        self.assertEqual(len(request), 6)
        self.assertEqual(host.REQUEST_HEADER.unpack(request), (diag.POWER_PREVIOUS, 1, 0x12345678))

        # No record and a valid record with zero time/no reached samples differ by identity.
        empty = diag.decode_power_previous(bytes(diag.POWER_PREVIOUS_BODY.size))
        self.assertFalse(empty["valid"])
        self.assertIsNone(empty["samples"]["pre_enable"])
        no_samples = diag.decode_power_previous(diag.POWER_PREVIOUS_BODY.pack(3, 4, 0, 0, 0, 0))
        self.assertTrue(no_samples["valid"])
        self.assertEqual((no_samples["ticket"], no_samples["generation"], no_samples["time_ms"]), (3, 4, 0))
        self.assertIsNone(no_samples["post_delta_ticks"])
        self.assertIsNone(no_samples["ready_delta_ticks"])

        pre_only = diag.decode_power_previous(
            diag.POWER_PREVIOUS_BODY.pack(5, 6, 10, 0, 0, (1 << 0) | (1 << 3) | (1 << 4)))
        self.assertEqual(pre_only["samples"]["pre_enable"], {"valid": True, "power": True, "busy": True})
        self.assertIsNone(pre_only["samples"]["post_enable"])
        self.assertIsNone(pre_only["post_delta_ticks"])

        metadata = 0x1FF  # all three valid sites, both selected bits set per site
        complete = diag.decode_power_previous(
            diag.POWER_PREVIOUS_BODY.pack(7, 8, 0, 0xFFFFFFFE, 0, metadata))
        self.assertEqual(complete["nominal_count_hz"], 24_000_000)
        self.assertEqual((complete["post_delta_ticks"], complete["ready_delta_ticks"]), (0xFFFFFFFE, 0))
        self.assertEqual(complete["samples"]["ready_power"], {"valid": True, "power": True, "busy": True})
        self.assertIn("wrap", complete["timing_note"])
        self.assertIn("non-atomic", complete["timing_note"])
        self.assertIn("logging", complete["timing_note"])

        module = Module(power_body=diag.POWER_PREVIOUS_BODY.pack(9, 10, 0, 0, 0, 0))
        result = diag.exchange(Device(module), request)
        self.assertTrue(result["valid"])
        self.assertEqual(module.sent, [diag.POWER_PREVIOUS])
        self.assertEqual(module.pauses, 0)

    def test_power_previous_rejects_invalid_records_and_shapes(self):
        fixtures = (
            (diag.POWER_PREVIOUS_BODY.pack(1, 0, 0, 0, 0, 0), "mixed-zero"),
            (diag.POWER_PREVIOUS_BODY.pack(0, 1, 0, 0, 0, 0), "mixed-zero"),
            (diag.POWER_PREVIOUS_BODY.pack(0, 0, 1, 0, 0, 0), "nonzero fields"),
            (diag.POWER_PREVIOUS_BODY.pack(1, 1, 0, 0, 0, 1 << 9), "reserved"),
            (diag.POWER_PREVIOUS_BODY.pack(1, 1, 0, 0, 0, 1 << 1), "lacks PRE_ENABLE"),
            (diag.POWER_PREVIOUS_BODY.pack(1, 1, 0, 0, 0, 1 << 2), "lacks POST_ENABLE"),
            # Invalid site power bits and invalid-site deltas must stay zero.
            (diag.POWER_PREVIOUS_BODY.pack(1, 1, 0, 0, 0, 1 << 3), "invalid pre_enable"),
            (diag.POWER_PREVIOUS_BODY.pack(1, 1, 0, 1, 0, 1), "invalid post_enable"),
            (diag.POWER_PREVIOUS_BODY.pack(1, 1, 0, 0, 1, 3), "invalid ready_power"),
        )
        for body, message in fixtures:
            with self.subTest(body=body), self.assertRaisesRegex(host.HostError, message):
                diag.decode_power_previous(body)
        with self.assertRaisesRegex(host.HostError, "response length"):
            diag.decode_power_previous(bytes(diag.POWER_PREVIOUS_BODY.size - 1))
        module = Module(power_body=bytes(diag.POWER_PREVIOUS_BODY.size - 1))
        with self.assertRaisesRegex(host.HostError, "response length"):
            diag.exchange(Device(module), diag.payload(diag.POWER_PREVIOUS, 4))

    def test_power_previous_readonly_allowlist_and_single_write(self):
        for opcode in (diag.STATUS, diag.PREVIOUS, diag.POWER_PREVIOUS):
            self.assertEqual(len(diag.payload(opcode, 1)), 6)
        for opcode in (2, 3, 5, 0x62, 0x63, 0x64, 0x70):
            with self.assertRaises(host.HostError):
                diag.payload(opcode, 1)
        with self.assertRaises(host.HostError):
            diag.payload(diag.POWER_PREVIOUS, 0)
        with self.assertRaises(host.HostError):
            diag.payload(diag.POWER_PREVIOUS, 1, 2)

        module = Module(power_body=diag.POWER_PREVIOUS_BODY.pack(1, 1, 0, 0, 0, 0))
        device = Device(module)
        writes = []
        original_write = device.write
        def recording_write(report):
            writes.append(report)
            return original_write(report)
        device.write = recording_write
        diag.exchange(device, diag.payload(diag.POWER_PREVIOUS, 1))
        self.assertEqual(len(writes), 1)
        self.assertEqual(len(writes[0]), 65)
        self.assertEqual(module.sent, [diag.POWER_PREVIOUS])

    def test_power_previous_fixed_reply_fits_one_report_at_worst_case_stuffing(self):
        # The 30-byte payload has four fixed envelope bytes, one of which is escaped;
        # request ID/body bytes may all escape, and both CRC bytes may add escapes.
        response = host.RESPONSE_HEADER.pack(
            diag.POWER_PREVIOUS | 0x80, 1, 0, 0, 0x01010101) + bytes([1]) * diag.POWER_PREVIOUS_BODY.size
        wire = host.encode_frame(response)
        payload_escapes = sum(byte in (1, 4, 16) for byte in response)
        worst_case = 2 + len(response) + 2 + payload_escapes + 2
        self.assertLessEqual(worst_case, 64)
        self.assertLessEqual(len(wire), worst_case)

    def test_power_previous_cli_command_selects_readonly_opcode(self):
        with patch.object(sys, "argv", ["usb_restart_diag.py", "power-previous"]), \
                patch.object(host, "hid_module", return_value=object()), \
                patch.object(diag, "status", return_value={"valid": False}) as query, \
                contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(diag.main(), 0)
        self.assertEqual(query.call_args.args[3], diag.POWER_PREVIOUS)

    def test_power_previous_deadline_and_trailing_padding(self):
        module = Module(power_body=diag.POWER_PREVIOUS_BODY.pack(1, 1, 0, 0, 0, 0))
        clock = Clock()
        class Slow(Device):
            def write(self, report):
                count = super().write(report)
                clock.now += 2
                return count
        with self.assertRaisesRegex(host.HostError, "deadline exceeded during write"):
            diag.exchange(Slow(module), diag.payload(diag.POWER_PREVIOUS, 1), timeout=1, clock=clock)
        self.assertEqual(module.sent, [diag.POWER_PREVIOUS])

        class ExtraPadding(Device):
            def read(self, size, timeout):
                return super().read(size, timeout)[:-1] + b"\x01"
        with self.assertRaisesRegex(host.HostError, "non-padding"):
            diag.exchange(ExtraPadding(Module(power_body=diag.POWER_PREVIOUS_BODY.pack(1, 1, 0, 0, 0, 0))),
                          diag.payload(diag.POWER_PREVIOUS, 1))

    def test_rebuild_detail_is_first_rejection_not_permission(self):
        detail = 11 | (1024 << 8) | (3 << 24)
        parsed = diag.decode_rebuild_detail(detail)
        self.assertEqual((parsed["step"], parsed["first_rejected"], parsed["observation_bits"]), ("DRIVER_READY", "usb_busy", 3))
        for stage in (0, 5, 12):
            self.assertIsNone(diag.decode_rebuild_detail(stage)["first_rejected"])
        for invalid in (13, 0xF8000000, 11 | (3 << 8), 9 | (1024 << 8),
                        0 | (1 << 8), 5 | (1 << 8), 12 | (1 << 8)):
            with self.assertRaises(host.HostError):
                diag.decode_rebuild_detail(invalid)

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
