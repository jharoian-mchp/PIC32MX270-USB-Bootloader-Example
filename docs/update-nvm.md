# Inactive PIC32MX270 NVM backend

`update_nvm.c/.h` supplies a target-specific backend for `UPDATE_FLASH`. `update_nvm_port.h` performs register access on **PIC32MX270F256D only**. The diagnostic firmware does not install or call this backend; **hardware updates remain disabled**.

## Source review

The generated `peripheral/nvm/plib_nvm.c/.h` confirms 128-byte rows, 1024-byte pages, physical destination/source register conversion, WR polling and WRERR/LVDERR masks. Its launcher sets WREN, writes unlock keys and sets WR without an explicit LVD startup wait; it also leaves WREN enabled after completion. The new backend uses its own launcher. Generated code, including startup `NVM_Initialize`, is unchanged; this stage does not validate or replace that startup no-operation sequence.

References were checked against local original PDF text, alongside the converted Markdown:

- [Device data sheet](docling/PIC32MX1XX2XX283644-PIN_Datasheet_DS60001168L.md), section 5: 1 KB pages/128-byte rows; device table: PIC32MX270F256D has 64 KB SRAM. NVMCON section 5.1 defines WREN, WR, WRERR, LVDERR and LVDSTAT. **LVDSTAT signals a low-voltage event**, not detector-startup completion.
- [Flash FRM](docling/FRM/FRM-05-Flash-Programming.md), sections 5.3/5.4: suspend peripheral-bus initiators, enable LVD and allow startup, unlock/start, wait WR clear, clear WREN and inspect errors. Row source is word-aligned physical SRAM. Program-Flash instruction fetch/interrupt execution stalls during erase/program.
- `docs/pdf/PIC32-Flash-Programming-Specification-DS60001145.pdf`, section 13.4 Table 13-4: row-write procedure specifies **6 µs delay** after enabling writes, LVDSTAT check for PIC32MX, physical SRAM source and unlock/WR sequence. Section 13.3 Table 13-3 and section 13.4 notes require **at least 500 ns after WR clears before another NVM-register write**. The four-NOP examples assume an 8 MHz programming executive and are unsuitable as fixed timing at this project's 48 MHz SYSCLK.
- Device data sheet Table 30-11: read/erase/write VDD 2.3–3.6 V; typical word/row/page/chip operations 411/6675/20011/80180 **FRC cycles**, without specified maximums. Row programming requires SYSCLK at least 4 MHz. These typical timings are not an upper bound for host timeouts.
- [Family errata](docling/PIC32MX1XX-2XX-28-36-44-pin-Family-Silicon-Errata-DS80000531.md), Flash-protection items: PWP/BWP interaction and page-boundary behaviour require a separate protection-layout review before enabling updates. The adapter does not configure protection bits.

Original PDFs: `docs/pdf/FRM/FRM-05-Flash-Programming.pdf`, `docs/pdf/PIC32MX1XX2XX283644-PIN_Datasheet_DS60001168L.pdf`, the programming specification above and matching family errata. They define hardware requirements; mocked tests establish only software behaviour.

## Launch and completion

`UPDATE_NvmInit` accepts the genuine SYSCLK frequency in Hz (4–50 MHz). Core Count must run at SYSCLK/2. Conservative startup/settle delays are **10 µs / 1 µs**, rounded up in Count ticks: 240/24 at 48 MHz. Each wait samples Count and uses unsigned subtraction for wraparound. There is no interrupt-tick dependency.

The backend:

1. Validates a physical destination in `[1D008000, 1D040000)`, exact row/page alignment and complete span. The last metadata page is intentionally accessible; bootloader/configuration addresses are rejected.
2. Requires the whole write source row in **uncached KSEG1 SRAM**, `[A0000000, A0010000)`, with word alignment. Cached KSEG0 and physical-only pointers are rejected. The existing bootloader linker places data/BSS in KSEG1; do not move the session's owned row buffer into cached RAM without revising this contract. A `sync` instruction precedes NVMSRCADDR programming.
3. Saves/disables interrupt state; rejects WR-busy, pending settlement, DMA enabled, or existing NVM errors. Destination/source setup happens before unlock keys. NVMCON is set with WREN and operation together.
4. Waits 10 µs, rejects asserted LVDSTAT, writes keys `0`, `AA996655`, `556699AA`, then starts WR. Low-voltage rejection is latched as LVDERR in the backend even without hardware setting that error bit.
5. If Flash fetch stalls until completion, waits 1 µs and clears WREN before restoring interrupts. If WR is still active, restores the previous interrupt state and retains `pending`; later busy polling waits 1 µs after first observing WR clear and disarms WREN.
6. Reads application/metadata bytes through **KSEG1** only, after completion/settlement. `UPDATE_FLASH` performs row/erased-page readback and timeout handling; the backend does not copy, cancel or recycle the source buffer.

Hardware error bits are preserved and reported. The adapter does not clear errors to retry silently. Initialisation is allowed only on a new/quiescent instance and fails if hardware is busy, DMA enabled or error bits are set. A live adapter/flash instance must survive disconnect/abort/timeout until the source is released; never reinitialise its storage during an operation.

## Tests and compilation (2026-10-15)

`make test` runs ten strict C executables and 13 Python methods. `make test-sanitize` runs the same C tests with ASan/UBSan, including Python-plan/dispatcher/flash integration from the earlier stage. NVM port mocks test bounds, source rejection, interrupt-state restoration, startup/settle tick counts, DMA/busy/error/low-voltage rejection, source retention after timeout, and launch returning either active or completed. Mocks do not execute target register writes or validate actual address aliases.

Strict XC32 v6.00 compile of `update_nvm.c` with `-Wall -Wextra -Werror` passes for the target. Object disassembly confirms core-count wait loops, `sync`, consecutive NVMKEY stores followed by WR store, and delayed WREN clear. Independent review subsequently replaced ordinary C key/WR stores with one ordered assembly block. `make nvm-codegen-check` verifies the four consecutive stores at -O0/-O1/-O2/-Os with production/debug compile flags; checker regression tests cover relocation addends and shared high-region instructions. The diagnostic build remains text26670/data80/bss1568 within 32 KB; `UPDATE_Nvm`, dispatcher/session/flash/HEX/image execution symbols are absent. Shared MPLAB XML includes the new files; regenerate stale local makefiles if they omit sources.

The first two delegated NVM review attempts timed out. A later bounded review completed, found compiler-dependent unlock sequencing, and reviewed the assembly fix. A checker relocation-addend finding was also fixed with regression tests. [The integration audit](nvm-integration-audit.md) records those results and the independent USB bus-master/protection constraints. No hardware qualification is claimed.

## Before enabling writes

- Independently review the target launcher, device-revision errata, clock/VDD and write-protection layout. Confirm the supplied SYSCLK and Count behaviour; measure actual row/page timing and verify error recovery on the board.
- Qualify USB/peripheral bus traffic around unlock and CPU stalls. USB has its own bus master independent of DMACON.ON; interrupt disable plus DMA rejection does not exclude it. The active HID transport must coordinate report buffers and flash launch windows; this backend alone does not establish USB responsiveness or cancellation safety.
- Keep callbacks/ISRs away from NVM ownership. The backend assumes a single main-task owner. No general lock or multi-client arbitration is supplied.
- Regenerate the application's MCC configuration while preserving LED code, then create/review a bootloader-compatible relocated linker/startup image. The tagged standalone blink image is unchanged and unsuitable for upload.
- Integrate explicit write permission and the dispatcher into a tested HID transport; implement a host uploader with state/identity checks and controlled retries. Neither exists in the active firmware.
- Run recovery tests for cable loss, reset, power interruption, CRC/readback/NVM errors and metadata commit before accepting an uploaded application. Preserve PICkit recovery and physical SW1 entry.

The [offline plan/dispatcher](update-dispatch.md) and [image-session contract](update-image.md) remain the integration references. No reprogramming is needed for this inactive backend milestone.
