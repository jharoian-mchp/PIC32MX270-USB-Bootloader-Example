# MPLAB X/PICkit 5 mailbox investigation — offline, 2026-10-22

MPLAB X debugging is a plausible route for a halted SRAM token write and later settled-result inspection. Its effective reset behaviour has not been established for this board. IPE's reported Hold In Reset setting supplies no evidence of MPLAB X's current setting. No live PIC32MX270 mailbox access, debug-image profile or board procedure is qualified by this investigation.

The user authorised source/documentation investigation only and cautioned that MPLAB X may behave differently from IPE. No programmer connection, target command, settings change, page choice, firmware build/link/programming, reset, PAUSE or NVM action was performed. Source remains `33c6275`; the [inactive target port](nvm-isolated-target-design.md) and current board image are unchanged.

## IDE debug, IDE run and IPE are separate modes

The PICkit 5 guide supports debugging through MPLAB X or MDB and programming through MPLAB X/IPE or IPECMD (§1). Its PIC32M connection section identifies EJTAG as the debug/programming mechanism; SAM/PIC32C SWD and AVR examples cannot be applied to PIC32MX270.

The documented debug flow starts from Reset, runs to an internal hardware breakpoint and enters the target's debug executive. The executive answers CPU/register queries and uses stack locations (§4.7.2.3, printed p44). It requires a working target clock. A CPU debug halt is separate from holding MCLR/reset asserted.

The same guide's §9.2.1 (p74) states that Debug mode always programs Configuration Memory and Boot Flash. It also warns that program-memory ranges do not apply to Erase, and Erase All Before Program erases all device memory. Preserve options read and reprogram the preserved contents; they are additional target operations. Device-specific available options must be checked. Debug installation therefore needs explicit configuration/Boot Flash/erase-scope approval even if the compiler HEX contains only a small program.

The same guide documents Hold In Reset in **MPLAB X Run Main Project**: execution normally starts immediately after programming; holding reset prevents that execution (§7.2, p61). Thus MPLAB X also has a relevant control. Whether it is selected or whether the IDE releases reset successfully is a separate fact for the actual session. No common IPE/IDE failure or automatic setting inheritance is established.

Both saved projects select `PK5Tool`, XC32v6.00 and DFP1.7.380. Their PICkit settings include:

| Saved key | Value | Limit |
|---|---|---|
| `debugoptions.debug-startup` | `Use system settings` | Actual IDE startup choice is absent from the project |
| `debugoptions.reset-behaviour` | `Use system settings` | Actual IDE reset choice is absent from the project |
| `debugoptions.useswbreakpoints` | `false` | Does not prove the installed session has no software breakpoint |
| `hwtoolclock.frcindebug` | `false` | Does not measure the running clock or establish all halt settings |
| `poweroptions.powerenable` | `false` | Saved programmer-power setting; retain the current external supply |

Sources are `application/app.X/nbproject/configurations.xml` and `bootloader/USB_HID_bootloader.X/nbproject/configurations.xml`. They are existing application/diagnostic configurations, not a future isolated project. Existing user-confirmed MPLAB X PICkit 5 blink debugging shows the IDE/tool worked for that baseline; it does not establish this new mailbox method.

Saved programming settings also include broad `memories.programmemory.ranges=1d000000-1d03ffff` and erase-before-program options. They cannot be reused as approval for an isolated image or page preservation. Actual programmer erase scope must be reviewed independently of HEX load spans and smart-program/preserve options.

## SRAM inspection and token editing

PICkit 5 documentation identifies `Window > Target Memory Views` for device-memory inspection (§4.7.1.2, p39). The accessible older MPLAB X IDE guide, DS50002027D (2015), §§4.20–4.22 describes watch values after Debug Run/Pause, memory-window editing by entering values, and the requirement to use **Debug Run**, not ordinary Run, for editing. These older UI instructions need confirmation against MPLAB Xv6.35 and this device/tool combination before use.

That older guide explicitly labels New Runtime Watch/Run Time Update Interval as **MPLAB REAL ICE only** in §12.16. No retrieved PICkit 5 source establishes non-intrusive live SRAM reads/writes on PIC32MX270F256D. Generic runtime-watch, trace, AVR, PIC32C or Data Visualizer descriptions do not fill that gap. A watch display is not an ordered mailbox snapshot.

The lowest-assumption candidate uses halted memory access before arming and after independently established settlement. It does not require live polling. RAM writes may be implemented through the executive and target instruction execution; an aligned debugger edit is not yet proven to be one atomic word store, ordered or read back correctly. A later non-writing rehearsal must check the effective address alias, width, byte order, write completion and resulting unchanged neighbouring words. No Fill Memory or peripheral-register watch is appropriate for an operation token.

While the producer is running, an accepted snapshot needs sequence-before, all payload words, sequence-after with identical valid even values. At an approved halt, the producer may stop with an odd sequence, so stopping at an arbitrary instruction cannot guarantee a usable snapshot. Read startup and owner reports separately: they have different layouts and state encodings. Zero, odd or `0xFFFFFFFE` is unavailable; an old GUI cache must not be presented as current evidence. The debugger workflow must retain raw sequence/word evidence and actual image/symbol provenance.

## DEBUG profile and executive storage

The current configuration has DEBUG OFF, JTAGEN OFF and ICS_PGx4. DFP1.7.380 `xc32/32MX270F256D/configuration.data` encodes DEBUG OFF as low bits3 and ON as0; ICESEL/ICS_PGx4 is bits4:3 equal0. The target preflight requires the exact words:

- DEVCFG0 `7FFFFFE3`;
- DEVCFG1 `FD74CE5B`;
- DEVCFG2 `FFF97AFA`;
- DEVCFG3 `FFFFFFFF`.

A candidate DEBUG-ON word preserving every other DEVCFG0 bit would be `7FFFFFE0`. This is arithmetic only, not a selected configuration or programming request. The current preflight will reject it. Neither setting `__DEBUG`/`-g` nor picking PICkit 5 in a project establishes the actual programmed configuration or complete debugger image. Existing eight-mode NVM code-generation checks use `-g -D__DEBUG` for object probes; they supply no linked debug executive or debugger qualification.

A debug profile requires a separately reviewed explicit configuration/validation policy. Do not relax the existing DEBUG-OFF profile, accept arbitrary words, enable JTAG, change ICS_PGx4, switch to FRC or change protection/power settings as part of this investigation. Whether MPLAB X patches DEBUG and installs an executive must be established from its actual build/program logs and delivered/debugger-programmed bytes, including differences beyond the compiler-generated HEX.

The existing bootloader linker and installed device linker define `_DBG_EXCPT_ADDR=0xBFC00480`, `_DBG_CODE_ADDR=0x9FC00490`, `_DBG_CODE_SIZE=0x760`. `.dbg_excpt`/`.dbg_code` are conditional `_DEBUGGER` NOLOAD reservations. These describe reserved regions; they do not certify the executive contents subsequently installed by the tool. `_DEBUGGER` is distinct from the source macro `__DEBUG`.

The DFP's `debug-exception-return.S` also contains a production debug-return stub that can redirect to `__pic32_software_reset`. Reusing that reset-capable path blindly conflicts with an isolated non-resuming failure experiment. The final debug exception/runtime path, all executive/stack/RAM reservations, vectors and delivered configuration require inspection. Do not infer extra RAM sizes from generic debugger examples. The final map must keep owner/context/mailboxes/128-byte row/stack/heap and executive storage disjoint and preserve the canonical32KB program reservation/application origin `0x1D008000`.

## Count and peripheral activity during a halt

The original CPU reference DS61113E, §2.12.17/Register2-17, defines CP0 Debug register23/select0 and `COUNTDM` bit25:1 lets Count run in Debug mode;0 stops it. The table marks the bit R/W-1. Crucially, outside Debug mode only `DM` and `VER` are valid; other fields are unpredictable and writes to this register outside Debug mode cause undefined processor operation. The current target code tests only `DM`; it must not start treating a running read of COUNTDM as valid proof.

Count is nominally SYSCLK/2. Cause.DC clear and preflight progress do not prove24MHz, stability or what the executive will do to the timer. A halted snapshot alone cannot establish resumed timebase behaviour. The guide's Freeze on Halt feature (§4.7.1.5, §9.2.5) is device/peripheral dependent; it does not guarantee USB/DMA/NVM inactivity, settled Flash or known Count timing. No freeze setting is changed or proposed as a way to stop an active operation safely.

If Count keeps running while halted, debugger time contributes to modular elapsed budgets after the owner consumes the token. If it stops, physical halt time can disappear from those budgets. Multiple unseen wraps and target/Flash stalls are already outside the owner's timing inference. The debug executive can also run during queries. Therefore no breakpoint, step, manual Pause, automatic watch update requiring a halt, Run-to-Cursor or debug reset may be used between token consumption and established settlement. No outer timeout can preempt `UPDATE_NvmPortWait()` when Count stops.

## Candidate workflow for later review — not board instructions

This is a proposed control-flow contract. There is no isolated main, debugger project, approved page, image or breakpoint address to execute now.

1. **Non-writing method rehearsal first.** Under separate permission, use a dedicated no-launch rehearsal image/profile with USB/DMA absent and no erase/program backend or token-triggered Flash action. Establish the exact IDE startup/reset settings and logs, debugger/fuse/executive profile, allowed SRAM aliases, aligned token edit/readback and resumed Count/IRQ/bus evidence. The existing canonical USB image is not a substitute for this rehearsal. Installing even a no-launch rehearsal changes Flash and needs its own reviewed image/erase scope and recovery approval.
2. **Pre-arm rendezvous.** A future isolated main must publish valid startup/owner READY with token0 and observe stable release before an explicit safe hardware-breakpoint location. Its compiled breakpoint semantics must stop before any token consumption or adapter initializer. A debug halt there can inspect page/profile/raw status/sequence and edit only the owner token. No startup-held button, old gesture or debug executive state arms a transaction.
3. **Single token write, then free run.** After an approved token edit/readback, disable the pre-arm breakpoint and all other unsolicited halt/step/reset mechanisms before continuing. The owner anchors the arm window when it consumes the token during resumed Tasks, so a pre-consumption halt is outside that window. It still validates the current release and requires a fresh debounced SW1 gesture. Approved running Count/IRQ/bus evidence must survive executive return; IRQ masked plus USBPWR0/BUSY0/DMA0 remains mandatory.
4. **No debugger intervention during the transaction.** Let the target run through initialization, one erase/row/full-page verification and any settlement observation. Host wall time is not a settlement certificate. If the expected result does not appear, preserve the state and report uncertainty; do not manually pause/reset/read peripherals or retry under the original transaction permission.
5. **Settled-only readout rendezvous.** A future immutable-result breakpoint can be armed in advance only if reviewed target control flow reaches it after no further launch is possible and proves raw WR0, adapter pending/settlement released, retained buffer ownership and a complete even report. No breakpoint on generic failure, INIT_FAILED, FAIL_UNSETTLED or an arbitrary timeout path: startup could itself detect an already-active WR, and failure names alone do not certify hardware settlement. Exception/breakpoint behaviour must not reset or relaunch. A halted readout then captures both reports and relevant retained evidence without peripheral side effects.

These rendezvous points and debug-profile acceptance need design, mocks, compile-only review and final-image audit before a rehearsal or NVM image. No `sdbbp`/software breakpoint, main, startup path or fuse change is implemented here.

## Settlement contract and bounded review

The bounded reviewer flagged reliance on a generic FAIL_SETTLED label. The current owner `SettleFailure()` relies on the installed backend's `busy()` callback. Inspection of actual `update_nvm.c` confirms it returns true while WR is set; once WR clears, `Settle()` waits the required interval, disarms and clears adapter `pending` before `busy()` returns false. Existing owner/adapter/target mock tests exercise this transition and source-buffer lifetime. A stopped Count can prevent that callback returning.

This contract supplies current source evidence for the owner's settled state; it does not implement or qualify the future debugger rendezvous. That path must additionally check the actual raw WR/pending state, publication stability and no further launch eligibility before any breakpoint. It must capture backend error values as evidence, not require them to be zero on a settled failure. Any unexpected WR, pending settlement, unresolved initialization state or exception must bypass the readout halt path. No owner/backend code changed in this investigation.

The review also confirmed the documented halt/stopped-Count limitation. Neither debugger wall time nor Count-budget expiry certifies completion or permits reset/retry. The candidate is documentation only until the no-launch rehearsal and final startup/image/operation paths pass their separate gates.

## Evidence needed before a method is accepted

- Actual MPLAB Xv6.35/PICkit 5 startup/reset/power/freeze settings and session logs, distinguishing Reset, Debug Pause and Hold In Reset. Capture settings offline first where possible; do not attach or reset to obtain them under this permission.
- Exact debug build flags, configuration words and debugger-installed executive/data, firmware/tool versions, erase scope and unchanged protection/ICS/power profile. The current projects' system-setting placeholders do not supply those values.
- A reviewed no-launch rehearsal demonstrates aligned SRAM edit/readback, fresh ordered snapshots, debug return and resumed Count/IRQ/USB/DMA state. It must not enable the flash experiment to test the GUI.
- Source/mock/final-link evidence for pre-consumption and settled-only rendezvous, no hidden halt/reset/automatic debug routine during NVM ownership, disjoint executive/runtime storage, unchanged32KB reservation and correct exceptions/unlock/settlement.
- Separately approved exact page and content loss, full isolated image, physical clock/VDD/configuration evidence, one transaction and recovery envelope. No protection or connected uploader qualification follows from debugger rehearsal.

Verification passes: reference-section and relative-link checks, DFP DEBUG/ICESEL encoding checks, saved project-setting checks and `git diff --check`. `make isolated-nvm-check` repassed all three isolated mock suites and strict XC32 compile-only checks for the unchanged owner/target sources. Canonical/recovery HEX hashes match the recorded values; firmware sources, Makefile, project settings and configuration are unchanged. No linked image was produced.

The offline investigation is complete; the method and exact disposable page remain unapproved and unverified on the host/board. The subsequent [RAM-only rehearsal design](nvm-debugger-rehearsal-design.md) specifies a separate challenge mailbox and explicit debug-profile proposal without NVM owner/backend, page parameter, main or image. The later [inactive library implementation](nvm-debugger-rehearsal-implementation.md) passes mocks and XC32 object checks; no main/configuration/image or debugger execution is supplied. Host/image/profile/erase/recovery approvals remain separate.

## Source references and retrieval limits

| Source | Used evidence |
|---|---|
| [PICkit 5 User's Guide DS50003525C](https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/UserGuides/MPLAB-PICkit-5-In-Circuit-Debugger-User-Guide-DS50003525.pdf), ©2025 | §1 tool modes; PIC32M/EJTAG; §4.7.1.2 target memory views; §4.7.2.3 debug executive; §7.2 IDE Run/Hold In Reset; §9.2.1 programming/erase limits; §9.2.5 freeze controls |
| [MPLAB X IDE User's Guide DS50002027D](https://ww1.microchip.com/downloads/en/DeviceDoc/50002027D.pdf), ©2011–2015 | §§4.20–4.22 watches/memory edits; §12.16 REAL ICE-only runtime watches. Older than installed IDEv6.35 |
| [CPU FRM DS61113E](pdf/FRM/FRM-02-CPU.pdf), §2.12.17/Register2-17 | Original-PDF Count/DM/COUNTDM access rules; [converted text](docling/FRM/FRM-02-CPU.md) |
| Local DFP1.7.380 `configuration.data`, device linker and `xc32/startup/debug-exception-return.S`; bootloader `btl.ld` | Exact config encodings, conditional executive reservations and production debug return |
| Existing application/bootloader MPLAB XML and user-confirmed blink baseline | Saved tool/settings evidence and prior IDE debugging only |

PDF SHA-256 values: PICkit 5 `0623b1ebb6331147a0cc01611f63ff6165a1c8d990fc4a1dd6b29d0cbd051ee2`; old IDE guide `c85ff4d2bea38f59cc35272bdc78467b16943c538a40d1ece5f40cd41dbdcfcf`; existing CPU original `0ba9c2d27a6385157b0361c636606e69861723ce6a4f04020c59cb80799c3741`.

The search helper failed because `turndown` is missing; the search fallback did not return useful results. Direct official PDF retrieval succeeded for the PICkit guide and older IDE guide. Current IDE-guide URLs returned403, so old UI text is supporting evidence only. Retrieved PDFs/text remain ignored research files under `build/debugger-workflow/`; no dependency or project setting was changed to retrieve them.
