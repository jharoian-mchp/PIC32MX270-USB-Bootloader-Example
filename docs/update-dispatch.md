# Manifest dispatcher and offline upload plan

`update_dispatch.c/.h` connects the tested image session to a versioned request/reply contract in mock tests. `host/pic32_hid.py plan` creates matching offline wire fixtures. **Neither is connected to hardware writes.** The diagnostic USB endpoint still answers CAPABILITIES only; its linked ELF contains no dispatcher/session/flash/HEX/image execution symbols.

## Payloads

All integers are little-endian. Existing framing stays unchanged: 510-byte payload limit, CRC16 appended outside the payload, SOH/EOT/DLE escaping, 64-byte HID data reports. Python output calls would include report ID 0 before those 64 bytes. The offline plan includes that extra byte; the C decoder receives only report data.

Every request starts with `opcode:u8, version:u8=1, request_id:u32` (6 bytes). Request IDs must be nonzero. CAPABILITIES remains byte-for-byte compatible with the hardware-tested endpoint.

| Opcode | Request bytes after header | Total size | Completion condition |
|---|---|---:|---|
| `60` CAPABILITIES | none | 6 | Existing layout body |
| `61` STATUS | none | 6 | Current dispatcher state |
| `62` BEGIN | session token:u32, image length:u32, CRC32:u32 | 18 | READY after metadata/application erasure |
| `63` BATCH | session token:u32, sequence:u32, binary HEX records | 15–510 | READY, or COMPLETE for final EOF batch |
| `64` ABORT | session token:u32 | 10 | ABORTED after backend settles and metadata erasure verifies |

Batches contain complete decoded Intel HEX records, including each record checksum. They carry at most **496 record bytes** after the 14-byte envelope. ASCII colons/hex digits are absent. Whole-batch validation precedes row staging. Sequence starts at zero and increments on accepted submission. The planner greedily packs normalised records without splitting them.

There is no RUN/reset opcode or legacy command dispatch. Unknown opcodes, wrong versions, zero request IDs and unusable envelopes produce no reply. Known opcodes with wrong request lengths produce BAD_REQUEST. Output capacity is checked before side effects.

Replies start with `opcode|80:u8, version:u8=1, result:u8, reserved:u8=0, request_id:u32`. CAPABILITIES keeps its existing 20-byte body. STATUS and mutation replies have this 20-byte body:

```text
session_token:u32  last_mutation_id:u32  next_batch_sequence:u32  image_length:u32
session_state:u8  flash_status:u8  hex_status:u8  updates_enabled:u8
```

Result codes: `0 OK`, `1 BAD_REQUEST`, `2 DISABLED`, `3 BUSY`, `4 TOKEN`, `5 STALE`, `6 CONFLICT`, `7 SEQUENCE`, `8 FAILED`. Session/flash/HEX state values are the enums in their headers. Session values: IDLE=0, ERASING_META=1, ERASING_APP=2, READY=3, PROGRAMMING=4, EOF=5, VERIFYING=6, COMMITTING=7, COMPLETE=8, FAILED=9, ABORTING=10, ABORTED=11. EOF is an internal transition; the controller proceeds to image verification and commit.

**OK acknowledges acceptance, not completion.** A previously accepted request can return OK while the current session state is FAILED or ABORTED. A future uploader must check both the result and state, token and sequence. COMPLETE follows image CRC verification and commit-row readback; it does not certify linker placement, executable correctness or authentication.

## Stop-and-wait and lost replies

- Only one mutation may be outstanding. Wait READY after BEGIN/nonfinal BATCH. Wait COMPLETE after EOF; inspect FAILED throughout.
- Mutation IDs strictly increase for requests which invoke the session. Failed session submissions are remembered too, so retrying them does not repeat the action. Validation/BUSY/token/sequence rejections do not consume the ID.
- The dispatcher retains the complete bytes, ID and acceptance result of the **latest** invoked mutation. An exact retry returns that result and current state without calling begin/submit/abort again.
- Reusing that ID with different valid-shaped bytes returns CONFLICT. Older IDs return STALE. Queries never consume or evict the mutation ID/cache.
- Each new BEGIN requires a nonzero token different from the current token. BATCH/ABORT require the current token. BEGIN is allowed only from IDLE, COMPLETE or ABORTED. FAILED requires ABORT/recovery first.
- IDs and sequences never wrap. After accepting mutation ID FFFFFFFF, no further increasing ID exists in that dispatcher lifetime. The offline planner rejects request-ID overflow.

The cache is volatile and covers one mutation. There is no cross-reset replay guarantee, automatic resume, persistent session token or multiple-host ownership. A future uploader must allocate fresh tokens, negotiate write permission, drain stale transport responses, and require explicit recovery after disconnect/reset. Default token/ID **1** in `plan` are deterministic test fixtures, unsuitable for live allocation.

## Backend and transport ownership

`UPDATE_DispatchInit` enables mutations only with an explicit permission flag and a valid backend. Disabled initialisation needs no backend; mutation replies say DISABLED. There is no target PLIB binding in this revision and no diagnostic caller initialises the dispatcher.

The future transport owns framing, CRC, response-buffer lifetime and errors. It must call the dispatcher only with a validated payload and avoid processing another mutation until its response/operation policy permits it. This module does not implement USB callbacks, HID response delivery, partial-report timeouts or a host inactivity deadline. Do not initialise it again while an NVM operation owns its row buffer; retain the same instance across link cancellation and keep polling.

`UPDATE_DispatchDisconnect` aborts incomplete/failed sessions. Polling waits for any active erase/write to settle, then invalidates metadata. Link loss during COMMITTING waits for the commit write and erases the record afterwards. COMPLETE images survive a subsequent disconnect. Explicit ABORT also invalidates a completed image. See [the image contract](update-image.md) for power-loss limits, metadata-last ordering and CRC checks.

## Offline use

```powershell
py -3 host\pic32_hid.py plan path\to\relocated-application.hex --output upload-plan.json
```

Optional `--session-token 0x1234 --request-base 10` sets fixture identifiers. The JSON contains the image manifest, ordered mutation payloads, escaped frames, zero-padded reports and required wait states. Output is exclusive-create. No HID module is imported, device opened, request sent or image programmed. The existing standalone application HEX fails preparation because it has not been relocated. Do not remove protected records to make it pass.

## Verification (2026-10-15)

- `make test`: nine strict C executables and 13 Python test methods pass.
- `make test-sanitize`: the same suites pass with ASan/UBSan C binaries, including Python-generated plans executed through C framing/dispatcher/mocked flash for small, multi-window and maximum **228352-byte** images.
- Retry checks span metadata erase, application erase, row writes and commit; erase/write counts and active source buffers stay unchanged. Tests reject stale/conflicting IDs, wrong tokens/sequences, busy submissions, bad manifests, malformed batches, output shortages and unsupported RUN.
- CRC failure appears in STATUS, rejected/failed submissions replay without another action, and disconnect/timeout/commit-abort tests retain buffers and erase metadata safely.
- Strict XC32 v6.00 compilation of the dispatcher passes. The diagnostic `make bootloader` passes at text 26670/data 80/bss 1568, within the 32 KB reservation; garbage collection excludes inactive update modules.

These are mock/compile results. No target NVM timing, power-cut upload, Windows uploader or relocated application was tested. Next implement/review the target NVM backend, regenerate and relocate the application, then integrate the write-gated transport and qualify recovery before enabling uploads. The working read-only firmware needs no reprogramming for this library stage.
