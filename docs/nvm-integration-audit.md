# NVM integration audit — uploads disabled

Independent static review completed on 2026-10-15. The compiler-dependent key/WR sequence has been replaced by one ordered assembly block and checked in eight XC32 compile modes. **USB bus exclusion, flash protection and hardware recovery are not qualified.** No firmware caller or host uploader enables writes.

## Review and code generation

The reviewer identified that ordinary volatile C register stores preserve access order but allow compiler-inserted instructions between key/WR stores. The earlier `-O2` object had been inspected; other build modes lacked a repeatable check.

`UPDATE_NvmPortLaunch` now uses one input-only volatile assembly block with `.set noreorder`, four consecutive `sw` instructions and a memory clobber. All address/key/WR operands are prepared before the block. It changes memory, not input registers; no earlyclobber output is needed. This fixes compiler/assembler scheduling inside the sequence. It does not suspend peripheral bus masters.

```sh
make nvm-codegen-check
```

`tools/check-nvm-codegen.ts` checks `-O0/-O1/-O2/-Os`, each with production and `-g -D__DEBUG` flags, using XC32 v6.00 and DFP 1.7.380. It strictly compiles the real adapter and verifies a direct launch probe's object code:

- Registers/address constants precede the four stores.
- Store values are `0`, `AA996655`, `556699AA`, `8000`.
- Destinations are `NVMKEY`, `NVMKEY`, `NVMKEY`, `NVMCONSET`.
- Instruction addresses are consecutive four-byte MIPS instructions, without inserted loads, spills or other instructions.
- Relocation addends are zero and HI/LO base regions are compatible. The DFP register-address map is checked. GCC may share one LUI for both registers, whose signed LO16 high regions match.

A follow-up reviewer found that the first checker discarded relocation addends. It is fixed and has regression fixtures for wrong values/destinations, interleaving, spills, wrong key order, nonzero HI/LO addends and bad HI/LO pairs. A positive shared-LUI fixture covers the actual `-O2` optimisation. `make test` runs checker self-tests; full codegen checks require the target compiler.

These checks examine a compile probe and compile the adapter. They do not claim a complete MPLAB debug-image build, linked-image instruction validation under future LTO/instrumentation, measured hardware timing or an established CPU-clock unlock window. The Flash reference specifies peripheral-bus transaction ordering; bus exclusion still matters even with consecutive instructions.

## USB bus master

The original `docs/pdf/FRM/FRM-27-USB.pdf` section 27.1 lists an integrated Bus Master accessing RAM/Flash, and explicitly states USB operation does not require the PIC32 DMA module. The converted [USB FRM](docling/FRM/FRM-27-USB.md) contains the same features and the BDT/UOWN semaphore description. The device data sheet's USB module and generated driver use that buffer-descriptor mechanism.

Concrete implications:

- `DMACON.ON == 0` excludes the central DMA controller only. Interrupt disable also leaves USB hardware able to access owned buffers. Neither proves the bus is quiet.
- `DRV_USBFS_DEVICE_IRPSubmit` sets UOWN after physical buffer address/size setup. In `drv_usbfs_device.c`, `IRPCancel` can defer head-transfer completion to an ISR; buffer release must wait for its callback.
- `IRPCancelAll` clears the endpoint's BDT status and flushes its software queue. That is useful for report-buffer ownership on reset/disconnect. It supplies no general acknowledgement that the USB bus master is halted for a flash-unlock window; do not equate cancellation callbacks with hardware bus quiescence.
- CPU Program-Flash fetch and interrupt handling can stall during erase/program. Core Count underlies the current diagnostic milliseconds, so elapsed software time can catch up afterwards; that does not make USB callbacks responsive during a stall.

The backend subsequently gained a fail-closed USBPWR/USBBUSY gate, checked before init/start. It does not halt or resume the USB stack; the single owner must keep USB inactive for the whole operation. Before writing with an enumerated host, choose and verify a device-supported USB bus-quiescence or arbitration policy from device/USB register documentation. Define ordering for response completion, remaining EP0/control traffic, OUT requests, endpoint ownership, unlock, resume, cancellation and host timeouts. Review it independently and measure it on target. Directly toggling USB registers around an arbitrary live IRP would require a driver ownership audit; this revision does not do that.

## Flash protection

Generated `bootloader/src/config/default/initialization.c` currently sets `PWP=OFF`, `BWP=OFF`, `CP=OFF` and `DEBUG=OFF`. The diagnostic image therefore has no configured hardware bootloader/program write protection. The inactive adapter rejects destinations below physical `1D008000`, but that check is not hardware write protection for arbitrary callers or corrupted execution. A subsequent [launch/protection policy](flash-launch-policy.md) narrows applicability: the listed 256 KB Flash / 64 KB RAM A1/A2 row is affected by issue 21 and marked unaffected by issues 22–24. Confirm raw board identity before selecting PWP32K/BWP.

The original family errata `docs/pdf/PIC32MX1XX-2XX-28-36-44-pin-Family-Silicon-Errata-DS80000531.pdf`, Flash Memory items 21–23, describe PWP/BWP dependency/overlap and off-by-one-page behaviour. The device data sheet DEVCFG0 section describes nominal 1024-byte PWP granularity. Before changing configuration:

1. Record the target DEVID/silicon revision and applicable errata.
2. Derive the exact protected physical pages and first writable page for the 32 KB reservation, using the applicable PWP encoding/erratum; do not select a nominal 32 KB menu value blindly.
3. Confirm the relocated application first page and metadata page remain writable, while bootloader code/configuration stay protected as intended.
4. Establish PICkit/bulk-erase recovery and test the resulting protection boundaries. Save a recovery HEX and unchanged baseline tag first.

This audit changes no configuration bits. Protection settings can affect future erase/program operations and recovery; they need an explicit reviewed plan and hardware evidence.

## Remaining gates

| Gate | Current evidence | Required next evidence |
|---|---|---|
| Unlock source/codegen | Independent static review; ordered assembly; eight target compile modes | Recheck final integrated production/debug images and compiler/options changes |
| Startup/settle/source | PDF review, 10 µs/1 µs Count waits, KSEG1 row checks, mocked errors/buffer retention | Verify clock/Count, voltage and actual row/page behaviour on target |
| USB/peripheral bus | Independent USB bus master identified; IRP ownership/cancel source inspected | Reviewed bus-exclusion/arbitration policy and measured HID responsiveness/recovery |
| Hardware protection | PWP/BWP OFF identified; errata recorded | Revision-specific encoding, protected/writable boundaries and recovery tests |
| Application | Tagged standalone blink preserved | Windows MCC regeneration with LED preservation, relocated startup/linker review, both-platform builds |
| Upload/recovery | Mocked dispatcher/plan/metadata-last tests | Write-gated transport/uploader, power-cut/reset/cable-loss and valid-image handoff tests |

`make test`, `make test-sanitize`, `make nvm-codegen-check` and the diagnostic `make bootloader` pass. The diagnostic footprint remains text26670/data80/bss1568 and excludes inactive update execution symbols. No board programming or erase/program was performed for this audit. See [backend timing/ownership](update-nvm.md) and [manifest dispatcher](update-dispatch.md).
