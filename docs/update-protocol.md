# Bounded framing and command validation

User-owned `bootloader/src/update_protocol.c`/`.h` now provides bounded framing, CRC16, command-length checks and safe application-span validation. **It is not integrated into the diagnostic loop and performs no USB, flash or application-jump operations.** Updates remain disabled.

Reviewed: **2026-10-14**. The layer addresses the framing/dispatcher portion of the [generated-code safety audit](update-safety.md). Generated code is unchanged, so `make update-audit` still reproduces its defects; the hardened layer has separate safe-behavior tests.

## Wire format

The decoder preserves the generated AN1388-style format:

- SOH **0x01**, EOT **0x04**, DLE **0x10**.
- Reserved payload/CRC bytes escaped by a preceding DLE.
- CRC16 polynomial **0x1021**, initial value **0**, no final XOR; trailing CRC bytes are little-endian. `123456789` gives **0x31C3**.
- Maximum decoded frame: **512 bytes including two CRC bytes**, or **510 command/payload bytes**.

`UPDATE_FramePush`/`UPDATE_FrameFeed` retain frame/escape state across HID chunks, including DLE as the final byte. They validate CRC before reporting READY, bound every accumulation, discard malformed/oversized input and resynchronise at a fresh unescaped SOH. Only reserved-byte escapes are accepted; verify the selected host utility does not rely on unnecessary escapes before integration.

Feed stops at the first complete/error result and reports bytes consumed. A READY frame remains locked and further input returns BUSY until the caller explicitly resets it. This prevents silent overwrite but requires the future transport to preserve unconsumed bytes and process/resynchronise deliberately. Reset the decoder on cable loss, transfer abort or timeout; this library does not itself supply a transport timer.

Padding/noise outside a frame is ignored. An unescaped SOH restarts an unfinished frame. These behaviors need host-utility verification alongside existing byte/report conventions; no HID receive queue or session was added in this step.

## Response encoding

`UPDATE_FrameEncode` checks the exact escaped output size before writing. Insufficient output capacity returns false, leaves the output unchanged and sets `written=0`. Worst-case allocation is **1,026 bytes** for a 512-byte decoded body plus framing, though particular payloads can require less. Input and output must be separate non-overlapping buffers.

This closes unchecked escape-expansion behavior in the new encoder, not the generated response code which remains inactive. Caller buffer lifetime, USB queue completion/rejection and report padding are integration responsibilities.

## Commands: validation, not execution

The validator receives command bytes after CRC verification/stripping:

| Command | Accepted payload length and checks |
|---|---|
| READ_BOOT_INFO (1) | Exactly 1 byte. |
| ERASE_FLASH (2) | Exactly 1 byte; execution/session authorization remains elsewhere. |
| PROGRAM_FLASH (3) | Command plus complete binary HEX records; nonempty, each checksum/type-specific length valid, no truncated suffix or records after EOF. |
| READ_CRC (4) | Exactly 9 bytes: command, 32-bit little-endian address and length. Full nonempty span must be inside allowed application flash. |
| JMP_TO_APP (5) | Exactly 1 byte; image acceptance and jump policy remain elsewhere. |

Rejected input zeroes the returned command description. The validator does not execute or acknowledge any command. A syntactically valid ERASE/JUMP is not authorized, and a valid PROGRAM record structure is not proof of valid addresses or an acceptable image.

Binary HEX shape checks accept types **00, 01, 02, 04, 05**, with data count nonzero for type 00, zero count/address for EOF, 2-byte/zero-address extension records and 4-byte/zero-address start-linear records. Address extensions across packets, full destination spans, overlaps/order, row padding/flush, flash failures and final EOF/image acceptance must be checked in the next HEX/flash layer. An invalid-address extension can pass structural validation intentionally; it must never reach a write without subsequent address checks.

## Address range

`UPDATE_ApplicationSpan` accepts only these application program-flash aliases:

- Physical **0x1D008000–0x1D03FFFF**.
- Cached virtual **0x9D008000–0x9D03FFFF**.
- Uncached virtual **0xBD008000–0xBD03FFFF**.

The exclusive physical end is **0x1D040000**. The complete nonempty byte span must fit; checks use subtraction to avoid addition overflow. Returned addresses are physical. Bootloader space, boot/configuration flash, RAM, SFRs, arbitrary address segments, zero length and spans crossing the end are rejected without dereferencing memory.

This range reflects the current 32 KB reservation and **does not reserve final-image metadata yet**. Change and test it when that layout is agreed; do not silently place metadata into this allowed span and call it protected.

## Verification

`make test` includes `tests/update_protocol_test.c`, built with **-Wall -Wextra -Werror**. It covers:

- Independent generated nibble-table CRC agreement and known-vector checks.
- Every transfer split point across test frames, including split escapes/CRC, maximum frame and worst escape-heavy payload.
- Exact encoder capacity, no writes on insufficient space, malformed escapes, bad CRC/bit corruption, nested SOH resynchronisation and multi-frame consumption/BUSY behavior.
- Exact command lengths, unknown commands, truncated/oversized/type-invalid HEX records, bad checksums and EOF trailing records.
- Physical/cached/uncached ranges, start/end boundaries, empty/overflowing spans, boot/config/SFR/RAM rejection, and randomized comparison against a 64-bit span oracle.
- Deterministic **2,000 randomly chunked round trips**, **100,000 arbitrary-byte decoder steps**, and **5,000 random command buffers**.

AddressSanitizer/UndefinedBehaviorSanitizer tests pass. The source cross-compiles for PIC32MX270F256D with XC32 v6.00/DFP 1.7.380 and **-O2 -Wall -Wextra -Werror** without warning suppressions. Public MPLAB X project metadata includes it, but nothing calls it in current firmware; section garbage collection removes its routines from the linked diagnostic ELF.

Existing `make test bootloader application` passes, and the linked diagnostic image remains **21,753 text / 80 data / 768 BSS** with no update/erase/write/parser routines. No hardware reprogramming or host utility is needed for this change.

## Next layer

The [user-owned HEX/flash layers](update-storage.md) now implement full address/order validation, row padding and mocked bounds/error/readback/timeout contracts. Next add explicit asynchronous row/flash orchestration and image-completion/commit policy before target binding. Preserve generated code and the working diagnostic behavior. Define image completion/commit/recovery before restoring command execution or application handoff.
