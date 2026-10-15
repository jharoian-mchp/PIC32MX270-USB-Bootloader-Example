# Image commit and asynchronous session contract

User-owned **update_image.c/.h** and **update_session.c/.h** now define a CRC-checked image record and a bounded asynchronous update controller. Strict host/XC32 builds and mocked/sanitizer tests pass. **No target backend, command dispatcher or application handoff is connected. The working SW1 diagnostic firmware is unchanged and updates remain inactive.**

Reviewed: **2026-10-14**. This follows [bounded framing](update-protocol.md) and [HEX/flash contracts](update-storage.md).

## Provisional image layout

| Region | Physical addresses |
|---|---|
| Bootloader reservation | 0x1D000000–0x1D007FFF |
| Application image | 0x1D008000–0x1D03FBFF |
| Reserved commit/metadata erase page | **0x1D03FC00–0x1D03FFFF** |
| Exclusive flash end | 0x1D040000 |

The last **1 KB erase page** is reserved for metadata. Maximum declared image span is **228,352 bytes** (0x37C00), with minimum length 4. This is a software contract proposal, not a completed application linker change. The future application must exclude this page and boot/configuration areas; framing's broad application-range helper still includes it, so final execution must use the session's stricter bounds.

The host must declare **image length and expected CRC32** before starting. CRC covers exactly that span starting at application entry, including sparse gaps as 0xFF. The initial reset word must not be erased. A type-05 HEX entry, if supplied, must map to application start. These checks do not prove code authenticity, vector correctness or a runnable application.

The [offline vendor host review](host-compatibility.md) confirms legacy ERASE_FLASH carries no manifest and finds a checksum-class mismatch. Recommend a small project host with explicit negotiated length/CRC manifest. Its wire schema/uploader is not yet implemented; do not claim drop-in UnifiedHost compatibility or bypass manifest validation.

## Update sequence

`UPDATE_SessionBegin` accepts a new/quiescent instance and a valid declared span:

1. **Erase and read-verify the metadata page first.** Existing image remains intact until its old acceptance record is invalidated. If power is lost before invalidation completes, the old complete image may remain valid; no application bytes have been changed.
2. Erase/read-verify each application page covering the declared span, one operation at a time. Pages outside the image span are not claimed to be rewritten.
3. Accept complete validated command batches only while READY. Copy up to **509 bytes** into session-owned storage and preflight the entire batch against current extension/order/exact-image bounds before any row is submitted.
4. Process one HEX record at a time into at most **three copied rows** (maximum 255-byte record plus a prior pending partial row). Drain each through the nonblocking flash adapter and verify it before continuing. Caller buffers may be reused immediately after accepted submission; no MCU write uses host-owned transient buffers.
5. On EOF, finish all row writes, then read/hash the exact declared image span, at most one 128-byte read per poll. Reject an erased initial word or CRC mismatch.
6. **Write the metadata row last**, after successful image verification. Wait for hardware completion and readback before reporting COMPLETE. No acknowledgment protocol or application jump occurs in this library.

An error poisons the session; it cannot be restarted from FAILED without explicit abort/recovery. Hex record data beyond declared length is rejected even if padded row storage stays in flash. The extended HEX API separates exact data end from aligned final-row end and exposes nonmutating batch preflight.

## Metadata record

A canonical 128-byte row at **0x1D03FC00** stores:

- Magic/version and fixed application origin.
- Declared byte length and CRC32.
- Bitwise complements of length/CRC.
- CRC32 over the first 28 metadata bytes.
- Erased 0xFF tail; no other data shares this row/page.

CRC32 is reflected polynomial 0xEDB88320, init/final XOR FFFFFFFF; **123456789 → CBF43926**. CRC is error detection, not authentication or anti-rollback.

`UPDATE_ImageCheckStored` validates metadata and creates a boot-time hash check. `UPDATE_ImageCheckPoll` hashes the declared image and rejects missing/corrupted/torn metadata, erased entry, invalid lengths or CRC mismatch. It returns validity only—**no jump**. Future startup code must require VALID plus correct entry/vector/state before handing off. Partial metadata tests cover every byte prefix of the 32-byte header and corrupted fields; physical torn-row/power-loss behavior still needs hardware qualification.

## Abort, failure and non-abortable hardware

Abort does not cancel an erase/program operation in hardware. It drops unsubmitted batches/rows, retains the flash adapter's private write buffer until the backend is idle, then erases/read-verifies metadata again. Abort during a metadata commit therefore invalidates any completed marker after the operation settles. Abort of a previously completed session intentionally invalidates that image.

An indefinitely busy backend leaves abort waiting; no unsafe timeout-buffer reuse or immediate software cancellation is claimed. Flash-operation timeout yields FAILED and requires explicit abort/recovery. Persistent NVM error may prevent metadata re-erasure; report failure rather than falsely reporting ABORTED. Power loss after a complete valid commit can leave a valid completed image even if the host never received an acknowledgment; power loss before commit leaves metadata invalid and startup must stay in recovery.

Image verification uses an **inactivity timeout**, renewed when the hash offset advances, so a large valid image can take longer than the individual flash-operation timeout. Backend read must be bounded; this layer cannot recover a hanging read callback.

## Tests and current firmware isolation

`make test` includes `tests/update_session_test.c`, strict -Wall/-Wextra/-Werror. Sanitizers and XC32 v6.00/DFP 1.7.380 -O2 strict cross-compilation pass for image/session/HEX changes.

Cases include:

- Metadata invalidated before touching the old application, bounded erase/write/verification and manifest-length rejection.
- Caller batch copies and the worst three-row queue case.
- Correct CRC and metadata-last commit; corrupted data/header/tail and every torn header prefix rejected.
- No EOF means no valid metadata; CRC mismatch and erased entry fail closed.
- Abort during application row write and metadata commit, busy timeout/quarantine, failed submission/NVM error and read failure.
- A **70,000-byte** image spanning address-extension windows, with progress-based verification timeout.
- Stored boot-time acceptance rehashes the image; no application handoff is executed.

The active diagnostic main/source remains untouched. New UPDATE routines and generated update/NVM erase/write routines are absent from its linked ELF through garbage collection; its size remains **21,753 text / 80 data / 768 BSS**. No hardware reprogramming is needed for this library-only step.

## Next integration work

1. Agree on host manifest transport and verify compatibility; preserve existing command framing or document the necessary extension.
2. Bind a reviewed target NVM backend and command/session orchestration with explicit completion/error responses. Test it with mocked transport/cable abort first.
3. Regenerate/retest and relocate the application, reserving metadata and defining reset/vector/configuration ownership.
4. Integrate stored-image acceptance and physical SW1 recovery policy, not the generated non-erased-word jump helper.
5. Check active updater RAM/flash footprint—the inert diagnostic size does not establish the combined updater fits 32 KB.
6. Only after those checks approve controlled flash/update/power-loss recovery tests with PICkit fallback. Authentication, rollback control, worst-case electrical behavior and hardware power-cut qualification remain outside current verification.
