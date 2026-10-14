# Bounded HEX rows and flash-operation adapter

User-owned **update_hex.c/.h** and **update_flash.c/.h** now validate HEX sessions and flash-operation bounds/errors. Strict host/XC32 builds, mocked tests and sanitizers pass. **Neither layer is bound to target NVM or called by the working diagnostic firmware. Updates remain disabled.**

Reviewed: **2026-10-14**. This follows [framing/command validation](update-protocol.md) and addresses more cases from the [generated update audit](update-safety.md). The generated sources are unchanged; their separate audit still reproduces original defects.

## HEX session contract

`UPDATE_HexBegin` creates a new session with the current application start **0x1D008000**, a caller-selected exclusive end no higher than **0x1D040000**, and a row sink. End must be row-aligned; it can be reduced for future protected metadata. The caller must establish that writable flash has been erased before beginning—this layer does not erase or attest that condition.

Each `UPDATE_HexFeed` batch is at most **509 binary HEX bytes**, consistent with a 510-byte payload including the command. It is completely preflighted for structure, checksum, addresses and ordering **before any row is emitted from that batch**. A valid batch may still encounter a sink failure after earlier rows have been emitted; there is no rollback.

Accepted record policy:

- Type 00 data: nonempty, complete checksum-valid record, full physical/cached/uncached-alias span in permitted application flash. A record cannot wrap its 16-bit offset window.
- Types 02/04: exactly two bytes and zero record offset; extension state persists between batches. Data ranges are still validated after applying the extension. Extension changes cannot bypass canonical physical ordering.
- Type 05: optional, exactly four bytes and zero offset, entry must map to **application start**; it is allowed once after data and only before EOF. This checks format/entry address, not executable validity.
- Type 01 EOF: zero length/offset, at least one data record, no trailing records in that batch. It flushes any pending row and finishes the row stream—not a committed application.
- Other types, malformed/truncated suffixes, checksum failures, overlap/backward order, boot/config/RAM/SFR addresses and end-crossing spans are rejected.

Addresses are normalized before enforcing **monotonic, nonoverlapping data**. Sparse gaps are allowed and stay `0xFF`; already-flushed rows cannot be revisited. This deliberately restrictive policy avoids multiple program passes on the same row. The selected host HEX ordering must be checked before integration; unusual or reordered HEX files are not silently accepted.

Rows are **128 bytes**, `0xFF` padded, emitted at complete-row boundaries, a move to a later row, or EOF. A sink failure poisons the session and discards pending bytes. Further feeds cannot flush stale data. Abort clears extensions/pending bytes; restart requires a deliberate new session and erase/recovery policy. EOF cannot acknowledge another update when the session is already finished.

## Row sink and asynchronous integration boundary

The row sink must **copy/stage or synchronously verify** the entire row before returning success and must not retain the generator's row pointer. It receives normalized physical addresses and may be called more than once per input batch.

The flash adapter below is asynchronous and uses one private buffer. The later [session controller](update-image.md) now connects the row sink through up to three private row copies per record, draining/verifying them one operation at a time. That controller is mocked and not called by active firmware. A future orchestrator must pause/stage rows, poll and verify each hardware operation, and acknowledge the host only after the required operation completes. Do not pass a one-row asynchronous backend into the synchronous sink and claim that queued rows are safely programmed. The private session row queue now exists; command dispatcher, host acknowledgment and target PLIB binding are still absent.

## Flash adapter contract

`UPDATE_FlashInit` accepts explicit backend callbacks and a page-aligned physical writable interval wholly within application flash. Initialize only a new/quiescent instance; never overwrite an active context or its retained source buffer.

Operation guards:

- Erase: **1,024-byte aligned page**, complete page inside the permitted interval.
- Write: **128-byte aligned row**, complete row inside the interval, nonnull data copied to a private **16-byte-aligned** buffer before backend submission.
- Only IDLE can start an operation, and backend busy/error must be clear.
- Rejected submission, post-operation error, read failure and readback mismatch produce terminal failure status, not success.
- Row write is read back once hardware is no longer busy. Erase verifies all 1,024 bytes, one 128-byte read per poll, so it does not monopolize the main loop with an erase-verification loop.
- Timeout uses wrap-safe elapsed milliseconds. **Timeout does not abort hardware**. It retains the write buffer and refuses reuse/reset while the backend remains busy. Once the hardware settles, the session still needs explicit reset/recovery; timeout is never retroactively reported as success.

The backend must provide nonblocking busy/error/start callbacks and a bounded synchronous read. These callbacks are mocked in current tests. No target register, NVM operation, cache-management path or physical readback has been exercised. Integrating target NVM needs device-specific error/startup/cache/source-address checks and a safe policy for an operation that never clears busy.

## Verification

`make test` includes `tests/update_storage_test.c` with **-Wall -Wextra -Werror**. AddressSanitizer/UndefinedBehaviorSanitizer execution passes. Both sources cross-compile with XC32 v6.00/DFP 1.7.380 at **-O2 -Wall -Wextra -Werror**, no warning suppressions.

Tests cover:

- Sparse/multiple records in a row, row crossing, last valid flash row, physical/KSEG0/KSEG1 aliases and `0xFF` padding.
- Boot/config/RAM/invalid address rejection, offset wrap/end crossing, caller-restricted metadata end, overlap/backward order and checksum/record-length failures.
- Preflight rejection of a batch with a valid first row followed by invalid address or truncated data, with no sink call.
- Abort/restart extension reset, EOF policy, optional start-linear entry validation, sink failure and poisoned-session behavior.
- **100 deterministic sequential/sparse streams**, up to 250 data records each, compared with an independently assembled byte map.
- Flash page/row alignment/range rejection, private buffer copy/lifetime, start rejection, pre/post hardware error, write/erase readback mismatch, read failure, busy refusal, timeout quarantine/reset and timer wrap.

Existing diagnostic/application builds and tests pass; diagnostic ELF remains **21,753 text / 80 data / 768 BSS**. The new uncalled UPDATE functions are removed by section garbage collection, and generated update/parser/NVM erase/write functions remain absent. No hardware retest or reprogramming is needed for these library-only changes.

## Remaining requirements before updates

1. Define final image layout/length/CRC/commit ownership and protected metadata interval. EOF/row-stream completion is not image validity or permission to jump.
2. Add nonblocking orchestration between validated commands, HEX rows and the flash adapter, with bounded buffering and explicit acknowledgment/failure state.
3. Bind a reviewed target NVM backend, including readiness/errors/cache/readback and non-abortable busy/reset handling. Test with mocks first, then a controlled hardware procedure.
4. Handle cable removal, packet timeout and interrupted/power-lost updates without reusing pending operation buffers or accepting partially written images.
5. Regenerate/retest the application after H3 updates, then relocate reset/startup/vector/configuration layout before host transfer.
6. Verify selected host utility protocol/HEX ordering and memory fit with the active updater. Keep SW1 diagnostic mode and PICkit recovery available until that work passes.

These layers improve the software contracts; they are not a firmware-update safety qualification or activation of the generated updater.
