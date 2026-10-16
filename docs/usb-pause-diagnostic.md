# Opt-in USB pause request and completion authority

The opt-in restart diagnostic now accepts a compact **USB-only pause** packet and status query. The default firmware still handles CAPABILITIES only. No erase/program/update session endpoint is connected, and no board image is delivered in this stage; matching [host reconnect checks/image audit](usb-restart-host-audit.md) now exist; final independent review and controlled board qualification remain open.

## Packets

Existing bounded CRC16/SOH/EOT/DLE framing and 64-byte reports remain. All integers are little-endian:

| Opcode | Request payload | Bytes |
|---|---|---:|
| `66` PAUSE | opcode:u8, version:u8=1, request ID:u32 nonzero, current transport generation:u32 | 10 |
| `67` STATUS | opcode:u8, version:u8=1, request ID:u32 nonzero | 6 |

These opcodes are allowed only when `USB_RESTART_DIAGNOSTIC=1` and retirement hooks are enabled. They do not reset the MCU, erase/program flash or run an application. A pause request deliberately removes/re-enumerates USB, so it has an operational side effect despite being flash-read-only.

Response payload is 21 bytes:

```text
opcode|80:u8  version:u8=1  result:u8=0  reserved:u8=0  request_id:u32
ticket:u32  current_transport_generation:u32  completed_cycles:u32  restart_state:u8
```

A response is framed and padded into one 64-byte report. Rejected/malformed/version/generation/stale requests receive no reply in this diagnostic contract. The host must not interpret timeout as successful pause or automatically resend a mutation.

Generation starts at 1 for this owner lifetime and advances on USB-only reconstruction. STATUS reports the latest pause ticket and actual current state. The generation is not a boot-unique epoch or authentication: it restarts at MCU reset. This diagnostic does **not** safely resume updates across reset. A future host test must fail on uncertainty/ambiguous same-VID/PID devices and combine returned ticket/generation/cycle evidence with a controlled no-reset test.

## Replay and completion

Accepted PAUSE IDs increase without wrap; exact latest ID+generation replay returns its ticket/current status but creates **no new send-completion authority**. Changed identity or older IDs are rejected. The cache survives USB-only rebuild, not MCU reset.

Only the newly accepted pause binds `pause_completion_ticket` to its response. Main consumes that ticket once when the exact queued transfer completes locally. Success requires OK status, full64-byte length, active send timer, elapsed time below1000ms, no cancellation attempt and no pending probe reset. Short/failed/cancelled/expired/reset completion rejects restart authorization.

Completion processing and `USB_RestartOwnerAck` share nested **global interrupt exclusion**. Owner rechecks recorded reset/error flags before fencing/cancelling; callbacks record only transfer/event evidence. Host application receipt is not proved by local HID completion. A host timeout may race a successful restart; status/identity must resolve it, not a different pause resend.

The successful ACK can fence/detach within probe tasks. The probe now rechecks active/reset before queuing another RX, and the owner rechecks phase before device tasks. This prevents same-main-pass EP0/report submissions after fencing. Owner snapshot/application/scheduling also run under one global interrupt fence, with a pending synchronous-event recheck, closing the reviewed old-CONFIGURED/new-RESET race. Actual interrupt latency remains a hardware qualification item.

## Reviews and tests

A full pause/probe-owner review attempt timed out. Bounded ACK/replay review raised execution-boundary/reset-race questions, resolved by explicit main/global-critical-section checks. A later source-pair review identified the event-snapshot scheduling race; fixed by extending the main critical section and confirmed by follow-up review. These are source-level checks, not board approval.

`make test` and `make test-sanitize` pass nineteen C executables, 16 Python methods plus the existing generated-source/hook/submission/rebuild harnesses. Pause tests cover full/short/late send, reset/cancel/timeout failure, malformed commands and STATUS without restart authority. The combined real probe+owner test runs framed pause through full completion, verifies no same-task submission after fence, then mocked off/retire/rebuild and new-generation status. Target callback/driver timing is still mocked in that test.

## Build evidence and delivery gate

The new command exceeded the previous integrated `-Os` margin. The opt-in `make bootloader-restart-check` now uses **`-Os -flto`** while keeping the original 32KB linker/application origin. It links at text29041/data84/bss1736 for the checked source; final results must be regenerated after any change. Default `-O2` build retains text26670/data80/bss1568 and its old read-only endpoint behavior. No reservation/protection bits changed.

LTO changes final code generation; object-only checks do not qualify all optimized integrated paths. No update/NVM backend is linked into this diagnostic. The host verifier and linked artifact audit now exist. Before delivering a HEX, complete final independent owner/image review and controlled programming instructions, verifying linked symbols/sections/HEX/configuration/hash and exact owner/transport behavior under LTO. Run the controlled Windows/PICkit read-only restart plan, including cable/MCLR/power-cycle faults, only with explicit test instructions and a tested recovery image.

The final review fixes and hashed experimental HEX/recovery package are now covered by [the initial board evaluation procedure](usb-restart-board-test.md). No hardware qualification is claimed. Uploads remain disabled. See [single-owner integration](usb-owner-integration-size.md), [retirement contracts](usb-retirement-hooks.md) and [hardware gates](nvm-hardware-test-plan.md).
