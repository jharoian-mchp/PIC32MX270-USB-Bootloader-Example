# RAM-only rehearsal package — delivery approved; board actions not approved

This package delivers the exact prebuilt PIC32MX270F256D RAM rehearsal ELF and a proposed installation/pre-edit scope. The user approved package preparation/delivery at2026-10-23T22:29:07.016Z. **That approval does not authorise programming, connecting PICkit5, starting Debug/Run, resetting, reading/writing target RAM, executing the RAM challenge or NVM operations.** Keep the current canonical diagnostic running/read-only, PICkit5 disconnected and external power/settings unchanged until a separately approved procedure is issued.

## What is delivered

- `ram-rehearsal.elf`: exact local artifact with symbols, no recompilation/relink.
- `ram-rehearsal.map`: matching native map for reference; do not edit it to obtain a parser pass.
- `usb-canonical32-restore.hex`: exact previously installed diagnostic; for a separately approved restoration only.
- `usb-probe-recovery.hex`: known default capabilities-only recovery; use only under separate recovery approval.
- `PACKAGE-MANIFEST.json`, `SHA256SUMS.txt`, `VERIFY-PACKAGE.ps1`: offline identities/inventory/checks.
- `evidence/`: original attempt record, exact linked linker bytes, native structural result/policy and bounded executable/store/control ledgers. These are evidence, not safety or operator certificates.
- `source/`: the four application inputs and three project headers matched to the original record. They are supplied for inspection, not a new build instruction.

The archive contains no candidate HEX, programmer commands, IDE project or automatic target script. Select the **ELF** for a future prebuilt/loadable-image project. Do not point the existing diagnostic project at it or rebuild these sources. Configuration/debug-executive installation is a tool action separate from ELF delivery.

## Verify downloaded files offline

1. Compare the downloaded ZIP SHA-256 to the value independently posted in the delivery chat **before extraction**. If it differs or the external hash is unavailable, stop/report. Do not trust a hash found only inside the archive.
2. Extract into a new directory named for this package, separate from previous USB packages or source builds. Do not execute any hardware tool.
3. Inspect `VERIFY-PACKAGE.ps1`, then run it from the package directory to check the exact inventory and hashes. It reads local files only; no network, compiler, device/tool discovery, debugger or programmer operation is implemented. A PowerShell execution-policy prompt is not a reason to change machine policy automatically; report it or perform the equivalent file checks.
4. Check the manifest's `ram-rehearsal.elf` identity against the table below. All checks must pass before using a file in a later approved procedure. Keep the downloaded archive and verification output.

The checksum list covers every regular package file except itself. The verifier requires exactly the manifest-declared unique names plus `SHA256SUMS.txt`, no symlinks/reparse points, no extra files and no missing files. Do not save verification output inside the package directory until this exact-inventory check has completed. Internal consistency is anchored only by the independently posted ZIP hash; it is not build or safety authentication.

| Exact delivered image | SHA-256 |
|---|---|
| RAM rehearsal ELF | `cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a` |
| Native map | `f6dc317709d4299e431f32f03575f1271f64f4523a8754bdacd0032b7eed4eec` |
| Canonical diagnostic restore HEX | `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7` |
| Default probe recovery HEX | `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c` |

## Candidate evidence and limitations

The local experiment used XC32v6.00/DFP1.7.380, `-Os -g -mno-mips16 -mdebugger`, no LTO,32KB program prefix, minimum stack1024/heap512. The package copies existing bytes only. The exact linked linker SHA is `632cb125dca3cbd0b6a29d4e8902dfbbb75b485c980f63a0a83710fba1771354`; the later repository copy differs by trailing whitespace and must not be presented as the original link input. The build record's original local-only scope is historical; this later delivery approval does not retroactively authorise board use.

The ELF stores DEVCFG3/2/1/0 `FFFFFFFF/FFF97AFA/FD74CE5B/7FFFFFE0`. DEBUG ON differs from the canonical diagnostic; JTAG OFF, ICS_PGx4, clock choices and protection OFF are preserved in the artifact. No NVM owner, adapter, test page, Flash command or updater is in the RAM application. Installing any image nevertheless programs Flash and may erase existing content.

Native structural checks, host mocks and bounded source/linked reviews pass. The serialized entry and RAM path had no concrete must-fix defect in scoped review. The retained CRT `sdbbp` return path is conditionally unreachable from normal non-returning main; it remains present. Three compiler-generated input objects, complete system-header closure and build-time tool/runtime hashes were not retained. Larger reviews timed out. Stack capacity, arbitrary fault containment and the installed debugger executive are not certified by these records.

The native result reports startup/control-flow/application safety unsupported, installed executive unknown and delivery/hardware/NVM approval false. Those parser flags remain unchanged. This package is approved for **file delivery only** and supplies no new safety flag or hardware permission. The staged RAM test is proposed to investigate the actual debugger method, with remaining executive/setup risks disclosed.

## Installation and pre-edit inspection — proposal only

The following describes the approval scope to request, **not steps to execute now**. An operational procedure must be issued after explicit approval and unresolved setup choices are reviewed.

### Required setup decisions before any target connection

- Exact prebuilt-project identity: MPLAB Xv6.35, device **PIC32MX270F256D**, build DFP1.7.380 and PICkit5. Use the verified `ram-rehearsal.elf`, not a PIC32CK default, diagnostic image or locally rebuilt candidate.
- Explicit per-project Debug startup **Halt at Reset vector** and Debug reset **ResetVector**. User screenshots show these choices in the existing project; verify they are saved separately in the new prebuilt project. A CPU debug halt differs from MCLR Hold In Reset. Do not infer a common IPE/IDE reset issue.
- User reports no software-breakpoint option for this device. Do not enable a substitute trap or assume the linked CRT trap is absent. Any future rendezvous breakpoint must be hardware and its actual halted PC inspected.
- External supply unchanged and PICkit target-power OFF. The displayed3.25 field is a configured tool value, not a current VDD measurement. Do not change supplies, jumpers, ICS/debug fuses, freeze selections or tool packs without explicitly reviewed differences.
- Program/erase/preserve scope must be approved separately. Current screenshots show **Erase All Before Program ON**, preserve-program-memory OFF and a broad1D000000..1D03FFFF range. The32KB code region does **not** constrain the tool's erase action. DEBUG mode also programs configuration and Boot Flash/executive. Do not rely on automatic memory selection or smart programming to imply preservation.

Before approval, state whether any application/metadata/other Flash content must be preserved. If yes, stop and devise a separately approved backup/preservation approach; do not perform a read/backup automatically. If no, the operator must explicitly approve the actual erase/content-loss envelope. Earlier48KB overlap permission and diagnostic programming permissions are spent and do not cover this RAM installation. Including recovery files does not authorise a bulk erase or recovery programming.

### Proposed single session envelope

The proposed scope would authorise only one candidate installation/verification and one debug session with a reset-vector stop, then one controlled continue to the pre-edit rendezvous and the specified SRAM inspection. It would permit no challenge write and no continue after the pre-edit capture. It must name any startup reset/connection effects and the planned session-ending behaviour; these are not assumed inert.

For the exact ELF, the reference reset entry is `BFC00000`; the pre-edit inert block is `9D000278..9D00027F`. The instruction/return delay slot and possible hardware breakpoint skid require actual-PC inspection. A pre-edit breakpoint should be removed once consumed; if the tool cannot place or report it reliably, stop/report rather than step/skip instructions or switch to software breakpoints. These addresses are reference data, not an instruction to set a breakpoint now.

Candidate SRAM reference layout:

| Address/span | Meaning |
|---|---|
| `A0000200..A00002C7` |200-byte private context; read-only evidence, never operator-written |
| `A00002C8..A000035B` |148-byte mailbox |
| `A00002C8` | Low guard `13579BDF` |
| `A00002CC` | Edit word, must remain0 during this proposed first session |
| `A00002D0` | High guard `2468ACE0` |
| `A00002D4` | Sequence |
| `A00002D8..A000035B` |33-word report; version1/mode1, state2 WAIT_EDIT, fault0, edit_attempts0 |

At the reset-vector halt, application BSS/report may not yet be initialized; do not interpret stale RAM as READY or edit it. The proposed pre-edit capture would contain the actual PC, fresh tool/programming/debug logs, guards/edit0 and ordered sequence/report/sequence reads with identical valid even sequence (not0/odd/FFFFFFFE). Capture raw words and candidate filename/hash, not only GUI watch text. Read only approved SRAM ranges; no auto-refresh peripheral windows, function calls, register writes, PC changes or cache-alias substitutions.

The private `preedit_called` flag is set by the application before the inert call. This first capture observes state only; no operator token may be written. Failure, missing halt, ambiguous PC, bad profile/report or incomplete executive/setup evidence means stop and preserve/report the result. No reinstall, repeat reset, manual Pause/step, state repair, cable/power change or alternative recovery occurs without a new precise approval.

### Session end and recovery are unresolved choices

Ending Debug, disconnecting the tool or holding/releasing reset may alter execution/state. A later operational procedure must select and authorise the exact end state after pre-edit capture, including what happens if the breakpoint is missed. The current proposal does not silently authorise leaving a board halted indefinitely, resuming it, asserting reset or restoring an image. If required, a bounded recovery action must be approved with exact image/program/erase scope and power/tool state; otherwise report before recovery.

## Later, separately approved stages

After reviewing installation/pre-edit evidence, request one RAM1 edit/readback, one free-running continue and one readout inspection. That challenge is not approved by this package. No automatic second token, retry, reset, arbitrary debug halt or USB/NVM command is permitted.

Only after the RAM method works can a real isolated NVM image be prepared and reviewed with explicit debug-profile handling, exact approved disposable1024-byte page, one erase/verify and one128-byte row/full-page verification, whole-operation USB/DMA/IRQ exclusion, clock/VDD evidence and recovery approval. This package contains no such NVM experiment and establishes no protection/write qualification.

## What to report next

After **offline archive verification only**, send the ZIP/hash/verifier result and state whether any current Flash contents require preservation. Do not connect PICkit or launch a target operation. We can then settle the remaining erase/session-end/recovery scope and request explicit installation/pre-edit approval.
