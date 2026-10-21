#!/usr/bin/env python3
"""Flash-read-only USB restart diagnostic. PAUSE disconnects USB deliberately.

No flash/update/reset-to-app commands or retries. Requires the opt-in firmware;
this is a controlled test, not reset-safe authentication or update resumption.
"""
import argparse
import json
import secrets
import struct
import sys
import time

import pic32_hid as host

PAUSE = 0x66
STATUS = 0x67
PREVIOUS = 0x68
POWER_PREVIOUS = 0x69
BODY = struct.Struct("<IIIB")
PREVIOUS_BODY = struct.Struct("<IIIBBBBBBI")
POWER_PREVIOUS_BODY = struct.Struct("<IIIIIH")
NOMINAL_COUNT_HZ = 24_000_000
REBUILD_STAGES = ("NONE", "INPUT", "PREPARE", "DRIVER_INIT", "DEVICE_INIT", "DEVICE_OPEN",
                  "BIND_DEVICE", "BIND_HID", "REOPEN", "HID_READY", "DEVICE_READY", "DRIVER_READY", "COMPLETE")
READINESS = {
    0: (), 5: (), 12: (),
    8: ("not_preparing", "generation_mismatch"),
    9: ("not_preparing", "mutex_uninitialized", "ledger_pending", "callback_missing", "tx_count", "rx_count"),
    10: ("not_preparing", "not_ready", "task_not_running", "client_not_open", "callback_missing", "descriptor_missing",
         "interface_missing", "not_detached", "ep0_rx_pending", "ep0_tx_pending"),
    11: ("not_preparing", "not_in_use", "not_open", "not_ready", "callback_missing", "attached", "in_isr",
         "endpoint_table_missing", "bdt_missing", "irq_enabled", "usb_busy", "usb_enable", "queue_pending", "bdt_owned", "usb_power_off"),
}


def decode_rebuild_detail(detail):
    stage, rejected, observation = detail & 255, (detail >> 8) & 65535, (detail >> 24) & 7
    if detail & 0xF8000000 or stage >= len(REBUILD_STAGES):
        raise host.HostError("invalid retained rebuild detail")
    labels = READINESS.get(stage, ("call_rejected",))
    if rejected & ~((1 << len(labels)) - 1) or (rejected and rejected & (rejected - 1)):
        raise host.HostError("invalid first-rejection mask")
    return {"step": REBUILD_STAGES[stage], "first_rejected": next(
        (name for bit, name in enumerate(labels) if rejected & (1 << bit)), None),
        "rejected_mask": rejected, "observation_bits": observation,
        "note": "First failed check only; untested later conditions and unsampled registers are not passes."}

CHECKPOINTS = ("NONE", "FENCE", "CANCEL", "DRAIN", "POWER_OFF", "OFF_IDLE", "RETIRE", "REBUILD", "CONFIGURED")
PHASES = ("READY", "ACK_WAIT", "DRAIN", "OFF_WAIT", "RETIRE", "INIT", "CONFIG_WAIT", "DONE", "FAILED")
FAULTS = ("NONE", "TIMEOUT", "ACK", "FENCE", "CANCEL", "POWER_OFF", "OFF_IDLE", "DRAIN", "RETIRE", "INIT", "EXTERNAL")


def payload(opcode, request_id, generation=None):
    if type(request_id) is not int or not 1 <= request_id <= 0xFFFFFFFF:
        raise host.HostError("request ID must be 1..0xFFFFFFFF")
    if opcode in (STATUS, PREVIOUS, POWER_PREVIOUS) and generation is None:
        return host.REQUEST_HEADER.pack(opcode, 1, request_id)
    if opcode == PAUSE and type(generation) is int and 1 <= generation <= 0xFFFFFFFF:
        return host.REQUEST_HEADER.pack(opcode, 1, request_id) + struct.pack("<I", generation)
    raise host.HostError("diagnostic transport permits only read-only STATUS/PREVIOUS/POWER_PREVIOUS and generation-bound PAUSE")


def decode_power_previous(body):
    """Decode the immutable power summary captured from the previous record."""
    if len(body) != POWER_PREVIOUS_BODY.size:
        raise host.HostError("invalid previous-power response length")
    ticket, generation, time_ms, post_delta, ready_delta, metadata = POWER_PREVIOUS_BODY.unpack(body)
    if bool(ticket) != bool(generation):
        raise host.HostError("mixed-zero retained power identity")
    valid = bool(ticket and generation)
    if not valid and (time_ms or post_delta or ready_delta or metadata):
        raise host.HostError("empty retained power record has nonzero fields")
    if metadata & 0xFE00:
        raise host.HostError("reserved previous-power metadata bits are set")

    sample_sites = (
        ("pre_enable", 0, 3, 4, None),
        ("post_enable", 1, 5, 6, post_delta),
        ("ready_power", 2, 7, 8, ready_delta),
    )
    if metadata & (1 << 1) and not metadata & (1 << 0):
        raise host.HostError("POST_ENABLE sample lacks PRE_ENABLE dependency")
    if metadata & (1 << 2) and not metadata & (1 << 1):
        raise host.HostError("READY_POWER sample lacks POST_ENABLE dependency")

    samples = {}
    for name, valid_bit, power_bit, busy_bit, delta in sample_sites:
        site_valid = bool(metadata & (1 << valid_bit))
        power = bool(metadata & (1 << power_bit))
        busy = bool(metadata & (1 << busy_bit))
        if not site_valid and (power or busy or (delta is not None and delta != 0)):
            raise host.HostError(f"invalid {name} sample has data")
        samples[name] = ({"valid": True, "power": power, "busy": busy}
                         if site_valid else None)

    return {
        "valid": valid,
        "ticket": ticket,
        "generation": generation,
        "time_ms": time_ms,
        "nominal_count_hz": NOMINAL_COUNT_HZ,
        "post_delta_ticks": post_delta if metadata & (1 << 1) else None,
        "ready_delta_ticks": ready_delta if metadata & (1 << 2) else None,
        "metadata": metadata,
        "samples": samples,
        "timing_note": (
            "Raw CP0 Count ticks at the nominal profile only. Deltas may wrap, "
            "sample pairs are non-atomic, and intervening work including journal "
            "logging is included. Not power-settling time, a deadline, or authority."
        ),
        "warning": "Retained SRAM evidence only; not reset-safe identity or restart permission.",
    }


def exchange(device, request, timeout=2.0, clock=time.monotonic):
    if not 0.1 <= timeout <= 30 or len(request) not in (6, 10):
        raise host.HostError("invalid diagnostic timeout/envelope")
    op, version, request_id = host.REQUEST_HEADER.unpack_from(request)
    generation = struct.unpack_from("<I", request, 6)[0] if len(request) == 10 else None
    if version != 1 or request != payload(op, request_id, generation):
        raise host.HostError("invalid diagnostic envelope")
    deadline = clock() + timeout
    report = b"\0" + host.encode_frame(request).ljust(64, b"\0")
    if len(report) != 65:
        raise host.HostError("diagnostic request must fit one report")
    # Exactly one transmission. A write/read error may still mean PAUSE ran.
    try:
        if clock() >= deadline:
            raise host.HostError("diagnostic deadline exhausted before write; no retry")
        if device.write(report) != 65:
            raise host.HostError("short diagnostic HID write; outcome unknown")
        if clock() >= deadline:
            raise host.HostError("diagnostic deadline exceeded during write; outcome unknown")
        decoder = host.FrameDecoder()
        for _ in range(512):
            remaining = deadline - clock()
            if remaining <= 0:
                break
            data = bytes(device.read(64, max(1, min(100, int(remaining * 1000)))))
            if clock() >= deadline:
                raise host.HostError("diagnostic deadline exceeded during read; outcome unknown")
            if not data:
                continue
            if len(data) > 64:
                raise host.HostError("oversized diagnostic response report")
            response, used = decoder.feed(data)
            if response is not None:
                if any(data[used:]):
                    raise host.HostError("non-padding after diagnostic frame")
                body = host.parse_response(response, op, request_id)
                if op == PREVIOUS:
                    if len(body) != PREVIOUS_BODY.size:
                        raise host.HostError("invalid previous-record response length")
                    ticket, generation, time_ms, state, phase, fault, evidence, valid, checkpoint, detail = PREVIOUS_BODY.unpack(body)
                    if state > 8 or phase > 8 or fault >= len(FAULTS) or checkpoint >= len(CHECKPOINTS) or valid not in (0, 1) or (valid and (not ticket or not generation)):
                        raise host.HostError("invalid retained diagnostic record")
                    if evidence & ~31 or (not valid and any((ticket, generation, time_ms, state, phase, fault, evidence, checkpoint, detail))):
                        raise host.HostError("noncanonical or reserved retained diagnostic record")
                    rebuild = decode_rebuild_detail(detail)
                    if clock() >= deadline:
                        raise host.HostError("deadline exceeded decoding retained record")
                    return {"valid": bool(valid), "ticket": ticket, "generation": generation,
                            "time_ms": time_ms, "state": PHASES[state], "phase": PHASES[phase],
                            "fault": FAULTS[fault], "checkpoint": CHECKPOINTS[checkpoint], "evidence_bits": evidence,
                            "rebuild": rebuild,
                            "warning": "Retained SRAM evidence only; not reset-safe identity or restart permission."}
                if op == POWER_PREVIOUS:
                    result = decode_power_previous(body)
                    if clock() >= deadline:
                        raise host.HostError("deadline exceeded decoding retained power record")
                    return result
                if len(body) != BODY.size:
                    raise host.HostError("invalid diagnostic response length")
                ticket, generation, cycles, state = BODY.unpack(body)
                if not generation or state > 8:
                    raise host.HostError("invalid diagnostic generation/state")
                if clock() >= deadline:
                    raise host.HostError("diagnostic deadline exceeded decoding response; outcome unknown")
                return {"ticket": ticket, "generation": generation, "completed_cycles": cycles, "state": state}
    except host.HostError:
        raise
    except Exception as exc:
        raise host.HostError(f"diagnostic HID error: {exc}; outcome unknown") from exc
    raise host.HostError("diagnostic response timeout; no retry, outcome unknown")


def open_device(module, path):
    device = module.device()
    try:
        device.open_path(path)
    except Exception as exc:
        device.close()
        raise host.HostError(f"diagnostic open failed: {exc}") from exc
    return device


def status(module, path_hex=None, timeout=2.0, opcode=STATUS):
    selected = host.select_device(host.candidates(module), path_hex)
    device = open_device(module, selected["path"])
    try:
        return exchange(device, payload(opcode, secrets.randbelow(0xFFFFFFFF) + 1), timeout)
    finally:
        device.close()


def restart(module, request_id, path_hex=None, timeout=2.0, reconnect_timeout=15.0,
            clock=time.monotonic, sleep=time.sleep):
    if not 1 <= reconnect_timeout <= 30:
        raise host.HostError("reconnect timeout must be 1..30 seconds")
    # Single-board policy intentionally cannot be bypassed by --path-hex.
    listed = host.candidates(module)
    if len(listed) != 1:
        raise host.HostError("restart test requires exactly one board, even with --path-hex")
    selected = host.select_device(listed, path_hex)
    path = selected["path"]
    device = open_device(module, path)
    uncertainty = None
    try:
        before = exchange(device, payload(STATUS, secrets.randbelow(0xFFFFFFFF) + 1), timeout, clock)
        if before["state"] not in (0, 7) or any(before[key] == 0xFFFFFFFF for key in
                                                ("ticket", "generation", "completed_cycles")):
            raise host.HostError("device not ready or counters exhausted; no pause sent")
        pause = payload(PAUSE, request_id, before["generation"])
        deadline = clock() + reconnect_timeout
        try:
            remaining = deadline - clock()
            if remaining < 0.1:
                raise host.HostError("reconnect deadline exhausted before PAUSE; no resend")
            ack = exchange(device, pause, min(timeout, remaining), clock)
        except host.HostError as exc:
            # Deliberate loss of interface can race the reply. NEVER resend.
            uncertainty = str(exc)
        else:
            if ack != {"ticket": before["ticket"] + 1, "generation": before["generation"],
                        "completed_cycles": before["completed_cycles"], "state": 1}:
                raise host.HostError("pause acceptance mismatch; stop, outcome unknown")
    finally:
        device.close()
    removed = False
    for _ in range(600):
        if clock() >= deadline:
            break
        listed = host.candidates(module)
        if len(listed) > 1 or (listed and listed[0].get("path") != path):
            raise host.HostError("ambiguous or changed HID path; no automatic reselection/retry")
        if not listed:
            removed = True
        elif removed:
            # Opens/status failure is uncertainty, not a second pause attempt.
            remaining = deadline - clock()
            if remaining < 0.1:
                break
            new = open_device(module, path)
            try:
                remaining = deadline - clock()
                if remaining < 0.1:
                    raise host.HostError("reconnect deadline exhausted during open")
                after = exchange(new, payload(STATUS, secrets.randbelow(0xFFFFFFFF) + 1),
                                 min(timeout, remaining), clock)
            finally:
                new.close()
            expected = {"ticket": before["ticket"] + 1, "generation": before["generation"] + 1,
                        "completed_cycles": before["completed_cycles"] + 1, "state": 7}
            if clock() > deadline:
                raise host.HostError("reconnect deadline exceeded during status exchange")
            if after != expected:
                raise host.HostError("reconnect identity/outcome mismatch; no retry or update resume")
            return {"before": before, "after": after, "removed_observed": True,
                    "path": path.hex(), "pause_response_uncertainty": uncertainty,
                    "warning": "Controlled no-MCU-reset test only; counters are not boot authentication."}
        sleep(0.05)
    raise host.HostError("removal/reconnect not verified before deadline; no retry, outcome unknown")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("status", "previous", "power-previous", "restart"))
    parser.add_argument("--path-hex")
    parser.add_argument("--timeout", type=float, default=2)
    parser.add_argument("--reconnect-timeout", type=float, default=15)
    parser.add_argument("--request-id", type=lambda value: int(value, 0))
    parser.add_argument("--confirm-usb-disconnect", action="store_true")
    args = parser.parse_args()
    try:
        if args.command == "restart" and (not args.confirm_usb_disconnect or args.request_id is None):
            raise host.HostError("restart requires --request-id and --confirm-usb-disconnect")
        module = host.hid_module()
        opcode = {"status": STATUS, "previous": PREVIOUS,
                  "power-previous": POWER_PREVIOUS}.get(args.command)
        result = status(module, args.path_hex, args.timeout, opcode) if args.command != "restart" else restart(
            module, args.request_id, args.path_hex, args.timeout, args.reconnect_timeout)
        print(json.dumps(result, indent=2, sort_keys=True))
    except (host.HostError, OSError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
