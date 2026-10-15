# Offline lifecycle wire schema, version 2

`update_lifecycle_wire.c/.h` and `host/lifecycle_wire.py` define matching offline stage/execute/query/resume packets. **They do not dispatch requests, send HID reports or enable hardware writes.** The deployed endpoint remains version-1 CAPABILITIES-only; existing Python `probe`/`plan` are unchanged.

## Request envelope

Integers are little-endian. Existing bounded CRC16/SOH/EOT/DLE framing stays outside the payload. Payload limit remains 510 bytes.

```text
opcode:u8  version:u8=2  reserved:u16=0  rpc_id:u32(nonzero)
boot_epoch:u64  session_token:u32  operation_id:u32
```

The header is 24 bytes. RPC ID correlates one response; operation ID identifies the semantic staged mutation across queries/retries. A retry may use a new RPC ID with the same operation identity/content. Boot epoch is caller-supplied freshness, not authentication; no generator or cross-reset resume is implemented.

| Opcode | Name | Body after header | Total bytes |
|---|---|---|---:|
| `70` | HELLO | none; epoch/token/operation ID must be zero | 24 |
| `71` | STAGE | kind:u8, reserved:3 zero bytes, kind-specific data | see below |
| `72` | EXECUTE | device-issued ticket:u32 nonzero | 28 |
| `73` | QUERY | ticket:u32 nonzero | 28 |
| `74` | RESUME | ticket:u32 nonzero | 28 |

Non-HELLO requests require nonzero epoch/token/operation ID. EXECUTE/QUERY/RESUME require the exact known operation ticket. STAGE ticket is allocated by the typed core, not supplied by host.

STAGE bodies:

- BEGIN kind=1: length:u32 (4..37C00), CRC32:u32; total 36 bytes.
- BATCH kind=2: sequence:u32, one or more complete binary HEX records with their checksums; total 33..510 bytes. Complete-record validation makes the smallest usable batch 5 record bytes, total 37 bytes.
- ABORT kind=3: no further data; total 28 bytes.

The larger version-2 header leaves **478 record bytes** after the 32-byte BATCH envelope. The typed core's 496-byte maximum is an internal API bound; version-2 packets enforce 478. Version-1 offline planner still uses its original 496-byte wire bound and is not a version-2 uploader.

C decoding validates exact header/body lengths, reserved bytes, IDs/version, manifest length, and batch record shape/checksum before producing a typed request. Address/order/EOF semantics are checked by `UPDATE_LifecycleStage` against session state. Failed decode leaves the output request unchanged. Decoder/encoder storage must be separate from inputs; these pure codec APIs document that non-overlap precondition.

## Response envelope and negotiation

Each response is exactly 56 payload bytes:

```text
opcode|80:u8  version:u8=2  result:u8  reserved:u8=0  rpc_id:u32
magic:4bytes="PKL2"  schema:u8=2  features:u8  max_payload:u16=510
boot_epoch:u64  token:u32  operation_id:u32  ticket:u32
next_sequence:u32  image_length:u32  expected_crc32:u32
lifecycle:u8  outcome:u8  session:u8  flash:u8  hex:u8  reserved:3zero
```

Result assignments: 0 OK, 1 BAD_REQUEST, 2 DISABLED, 3 BUSY, 4 IDENTITY, 5 FAILED. The encoder supports those values; no request dispatcher yet maps them to core transitions.

Feature bits: bit0 schema implemented, bit1 typed core implemented, bit2 hardware-write permission, bit3 USB pause/reconnect transport bound. Bits4..7 are reserved. Offline vectors use **03** (schema/core only); no live firmware returns this response. A future uploader must negotiate the required version/layout, both write/transport bits and target memory layout before mutation; HELLO alone does not promise a working update path.

Lifecycle/outcome enums are in `update_lifecycle.h`; session/flash/HEX enums remain their respective headers. Acceptance/result OK is not operation completion. Inspect outcome/state and exact identity. Python can require a known `(epoch,token,operation_id,ticket)` when parsing results and rejects mismatches without automatic continuation. HELLO discovers identity; later responses must be bound to it. Memory layout remains available through existing version-1 CAPABILITIES, but no automatic legacy write fallback is allowed.

The encoder checks enum ranges/reserved feature bits/output capacity before writing payload bytes. Python validates version, opcode/RPC ID, magic/schema/maximum, epoch, enum values and reserved bytes. There is no RUN/reset opcode, epoch authentication, ticket persistence or reconnect device-selection solution.

## Full core review and regressions

A bounded full static review of `update_lifecycle.c/.h` completed. It found a concrete aliasing issue: writing zero to the output ticket before validation could overwrite `c.ticket` and permit ticket reuse. The core now rejects any ticket-output overlap with the controller or input operation before writing even a failure value. Tests verify `&c.ticket`, `&op.id` and staged-operation aliases leave counters/content intact. Output storage must be distinct; delayed ACK tickets cannot be reset by those aliases.

The same review found no other concrete transition defect under the documented exclusive-owner/fresh-epoch/new-storage assumptions. Codec review found Python packed an unvalidated out-of-range opcode; it now validates opcode/type and stage-kind range before struct/byte construction. Follow-up bounded review confirmed both specific fixes. This is static code review, not live transport/hardware approval.

## Verification (2026-10-15)

- `make test`: twelve strict C executables, 15 Python methods and Bun checker self-tests pass.
- `make test-sanitize`: the same C suites pass with ASan/UBSan, including packet boundaries and lifecycle alias regressions.
- C/Python compare golden 36-byte BEGIN/56-byte response vectors; tests reject malformed envelopes/reserved/version/identity/length/checksum and test exactly 478 record bytes/510 payload bytes.
- All version-2 packets remain refused by `pic32_hid.py transact`; no HID dependency is needed for codecs and no new live CLI command exists.
- Strict XC32 lifecycle/codec compilation, eight-mode NVM code-generation checks and diagnostic `make bootloader` pass. Shared MPLAB XML lists 36 sources. Diagnostic footprint remains text26670/data80/bss1568; inactive codec/lifecycle/update symbols are absent.

Next implement/review the inactive version-2 request dispatcher and dynamic host fixtures (tickets are device-issued, so static offline plans cannot predetermine a complete upload). Then audit/bind the generated USB lifecycle and run [gated hardware tests](nvm-hardware-test-plan.md). Application regeneration/relocation and protection recovery still precede hardware uploads. No reprogramming is needed for these library/codec changes.
