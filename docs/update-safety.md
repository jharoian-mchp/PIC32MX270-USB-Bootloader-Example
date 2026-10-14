# Generated update-path safety audit

**Keep firmware updates disabled.** Host-only tests against the exact generated code reproduce **12 unsafe behaviors**. The hardware-tested SW1 diagnostic source is unchanged; its linked ELF still excludes the update parser, update tasks and NVM erase/write routines.

Reviewed: **2026-10-14**. Baseline is the generated bootloader source present at **8e4d5e0** (generated components bootloader v3.9.0, USB v3.16.0, core v3.17.1, CSP v3.26.1). This is a focused bounds/error audit, not an exhaustive security review or flash-hardware test.

## Reproduce without hardware

```sh
make update-audit
```

Requires a host C compiler with AddressSanitizer/UndefinedBehaviorSanitizer on Linux. `tests/update_nvm_audit.c` and `tests/update_protocol_audit.c` include the **unmodified generated C sources** in host translation units. Mock includes under `tests/update_mocks/` replace peripheral/flash/datastream operations. Mock flash operations record calls and never dereference PIC32 addresses or contact hardware.

The test executable is deliberately separate from the firmware build: tests are not in `bootloader/src/` or the MPLAB project. Generated-source hashes/diffs are not modified. `make update-audit` returns success only if valid baselines and the expected bad behaviors reproduce. **A passing audit means the defects are reproduced; it does not mean updates are safe.**

Detailed sanitizer logs are under ignored `build/update-audit/`. The script disables core dumps, requires explicit sanitizer evidence for crashing cases and fails if a reproduction unexpectedly succeeds or fails for an unrelated reason. Re-evaluate the expected results when generator content changes; future fixes need tests asserting safe rejection instead.

## Results

Valid controls passed:

- Correctly checksummed data record at application start, EOF flush and 0xFF row padding; corrupted checksum rejected.
- CRC-protected READ_BOOT_INFO frame recognised and response constructed.

Unsafe cases reproduced:

| Case | Evidence | Required correction before updates |
|---|---|---|
| Truncated HEX record | Five-byte allocation claiming 255 data bytes causes AddressSanitizer heap-buffer-overflow. | Verify complete `data_length + 5` record fits before subtracting length, reading checksum or accessing data. Reject truncated suffixes. |
| Data at flash end | Row-write mock called at **0x9D040000**, exclusive flash end. | Use exclusive end bounds; verify the whole aligned row stays inside writable application flash. |
| Data crossing flash end | Four-byte record starting at 0x9D03FFFF queues a second row at 0x9D040000. | Validate complete byte span and overflow-safe address arithmetic before buffering or flushing. |
| Failed NVM row write | Mock returns false and reports error; EOF still returns HEX_REC_NORMAL. | Propagate operation rejection and NVM_ErrorGet, verify write/readback, and fail the update session. |
| Erase helper below application | Direct helper call accepts bootloader-region FLASH_START and erases it in the mock. | Put alignment/full-range guards in the flash helper as well as caller; protect boot/configuration regions. The current fixed erase command does not expose a caller-selected address, so this is a helper-contract defect. |
| Configuration-region data | HEX data for configuration storage is silently skipped yet acknowledged normal. | Define explicit reject/allow rules; an acknowledgment must not imply ignored bytes were installed. Application update files should exclude boot/configuration records. |
| Incomplete trailing HEX bytes | A valid record plus one incomplete trailing byte is accepted as normal. | Require full consumption/valid record boundaries before acknowledgment. |
| DLE split across transfers | DLE as final received byte reads a stale next-buffer byte and corrupts a valid split frame. | Persist escape/frame state across HID transfers; never read beyond actual transfer length. |
| Oversized protocol frame | Two chunks overflow the 512-byte processing buffer; sanitizer reports out-of-bounds index/store. | Bound decoded frame accumulation, reject oversized frames and resynchronise at a valid SOH. |
| Short READ_CRC command | Dispatcher reads absent parameters from stale command-buffer slots and dereferences the resulting invalid address; sanitizer fault. | Validate exact command payload length before parameter access. This reproduction seeds stale slots directly to isolate dispatcher behavior. |
| READ_CRC invalid range | Even a correctly sized command causes an invalid-memory read when its address is unchecked; sanitizer fault. | Restrict CRC reads to permitted flash spans with overflow-safe checks; never accept arbitrary SFR/RAM/invalid memory addresses. |
| Failed PROGRAM_FLASH response | Rejected HEX program call is repeated each task invocation, staying in PROCESS_COMMAND without error response/resynchronisation. | Return a defined error outcome, terminate/reset parser/session state appropriately and let the host recover deliberately. |

Primary sources:

- `bootloader/src/config/default/bootloader/bootloader_nvm_interface.c`: record-length subtraction/checksum around lines 150–165, application bounds around line 185, row flush and erase/write helpers around lines 85–130.
- `bootloader/src/config/default/bootloader/bootloader_usb_device_hid.c`: receive framing around lines 175–238, command dispatcher around lines 243–324, CRC address/length reads at 298–300.
- `bootloader/src/config/default/peripheral/nvm/plib_nvm.c`: operation-start returns true and exposes `NVM_ErrorGet`; generated NVM wrapper does not consume that error status.

## First hardening layer added

User-owned **update_protocol.c/.h** now supplies bounded framing/CRC/encoding, command-length/HEX-shape checks and full application-span validation for READ_CRC. Safe-behavior host tests, deterministic randomized coverage, sanitizers and strict XC32 compilation pass; see [the implementation contract](update-protocol.md).

The new layer is **not called by active firmware** and executes no commands. Generated code stays untouched, so this reproduction audit intentionally still shows its failures. User-owned HEX session/row validation and a backend-based flash-operation adapter now also pass [storage-layer tests](update-storage.md), including bounds/error/readback/timeout cases. They are not connected to each other asynchronously or bound to target NVM. Image validity/commit and interruption policy remain open. The active SW1 diagnostic image remains unchanged.

## Additional untested concerns

- Flash busy loops have no timeout; an indefinitely busy peripheral was not run as a hanging host test. A timeout does not itself abort an in-progress erase/program operation safely; establish failure/reset behavior from the device requirements.
- Start/abort/disconnect must reset framing, HEX address extensions and pending row state; the generated parser keeps static state. Session recovery and interrupted updates need tests.
- Sparse, reordered and overlapping records need deliberate policy, especially repeated programming of a row without erase. Current partial-row behavior is not a complete correctness guarantee.
- A non-erased first application word is insufficient evidence of a complete image. Header/length/CRC/commit policy, flash protection and power-loss behavior are not yet designed or tested.
- Out-of-range host-memory CRC tests prove unchecked reads; they do not imply every legitimate PIC32 application-flash read is unsafe. Fix range checks before exercising valid address reads on hardware.
- HID response-size/escape expansion, report padding, endpoint queue/error returns, exact command semantics and host utility compatibility require a subsequent audit. This harness does not qualify the USB stack.
- No authenticated update, anti-rollback or cryptographic image verification exists in the current example. A CRC is error detection, not source authentication.

## Hardening sequence

1. Preserve the working enumeration/SW1 diagnostic firmware. Do not restore generated SYS_Tasks or command reception just to test this audit on hardware.
2. Add user-owned bounded framing and command-validation code with malformed/split/oversized-frame tests. Keep generated code identifiable and document any integration seam; do not bury fixes in files MCC silently overwrites.
3. Add a bounded HEX/row writer and flash-operation adapter that validates entire spans, alignment and errors, with explicit session lifecycle and mocked flash tests. Convert these reproductions into **safe-rejection regression tests**.
4. Define an image-completion/commit contract and startup acceptance checks before any application handoff. Ensure interrupted updates cannot be accepted simply because the reset word was written.
5. Regenerate/retest the standalone application after the recorded H3 changes, then create its bootloader-compatible memory/startup/vector layout. Filter boot/configuration records from the delivered image according to the agreed contract.
6. Integrate the hardened update flow with already-tested ADC/mux/SW1 behavior. Check memory fit and protocol compatibility, then approve a controlled hardware erase/write/update/recovery test. Keep PICkit recovery available.

## Checks completed this turn

- `make update-audit`: two valid baselines, eight non-crashing bad-behavior cases, four expected sanitizer failures pass their reproduction assertions.
- `make test bootloader application`: existing application and SW1 diagnostic tests/builds pass.
- Linked diagnostic symbol check: update tasks/parser and NVM erase/write symbols remain absent.
- Generated bootloader/application sources unchanged. No hardware operation, host update utility or firmware-update task was enabled.
