# Restart phase and reset-retained diagnostic evidence

The failed board PAUSE100 removed USB and did not reconnect; SW1/MCLR restored startup. Its stopped phase is still **unknown**. This source stage adds first-failure phase/reason and a two-slot retained-SRAM journal before another explicitly authorised board evaluation. A separate [initial read-only instrumented package](usb-instrumented-initial-check.md) is now delivered for programming and list/probe/STATUS/PREVIOUS checks only. **No repeat PAUSE is authorised.** NVM writes, protection changes and update commands remain disabled.

## Failure fields

`USB_RESTART` now records the first failed phase and reason without altering its safety transitions. Later cancellation cannot overwrite the original failure. Reasons: NONE=0, TIMEOUT=1, ACK=2, FENCE=3, CANCEL=4, POWER_OFF=5, OFF_IDLE=6, DRAIN=7, RETIRE=8, INIT=9, EXTERNAL=10. Existing phase values stay READY0/ACK_WAIT1/DRAIN2/OFF_WAIT3/RETIRE4/INIT5/CONFIG_WAIT6/DONE7/FAILED8.

A wait timeout reports the phase it timed out in. Rejected ACK and failed port operations report their exact category. This instrumentation does not reinterpret or relax a failed drain/off-idle predicate.

## Retained SRAM journal

`usb_restart_journal.c/.h` stores two 48-byte slots, each with magic/version, monotonic sequence/complement, ticket/generation/time, state/phase/fault/evidence, reserved zeros, CRC32 and a final commit word. The alternate slot is invalidated before writing and committed last. A torn/corrupted latest slot falls back to the older validated slot; sequence exhaustion refuses another write rather than wrapping.

Journal schema version2 adds a port-entry checkpoint in the checksummed record; old schema1 slots are rejected. Checkpoints are NONE0/FENCE1/CANCEL2/DRAIN3/POWER_OFF4/OFF_IDLE5/RETIRE6/REBUILD7/CONFIGURED8. Each owner port wrapper commits its checkpoint **before** invoking the potentially blocking operation. A failure record keeps the last entered port as well as the coordinator's first failure phase/reason. It does not prove that port returned.

One IRQ-excluded owner serialises **both reads and writes**. Request eligibility, acceptance and journal write now share IRQ exclusion; an ISR reset cannot slip between eligibility and recording. A checksum is not a lock. The owner captures previous validated evidence at initialization, before current operations write the journal. A current-runtime cache avoids CRC work on unchanged task passes. There are no file/flash writes and the journal is never used to authorize or resume a restart.

XC32 `persistent` alone was insufficient under this LTO build: the initial linked symbol landed in ordinary BSS. The source now also specifies `.persist.usb_restart_journal` and `used`. Final ELF audit requires the 96-byte journal inside `_persist_begin.._persist_end` and a retained NOBITS section. It currently links at A0000000..A0000060, outside normal BSS initialization.

The final linked `.dinit` audit now parses the inspected XC32 v6 clear/copy records, requires allocated/file-backed Flash data matching the runtime `_dinit_addr` and delivered HEX, and rejects any initialization destination overlapping the journal. Unknown operations or malformed section-name/`.dinit` metadata fail closed. A negative ELF/HEX pair that would clear the journal is rejected even with matching delivered data.

These startup checks support evidence retention across MCLR only if target SRAM and startup preserve it. Power loss, programming, a changed RAM layout or arbitrary SRAM corruption can invalidate it. There is no hardware retention result yet. Tickets/generation restart in each boot and are not global identity; a previous record is historical diagnostic evidence only. Valid retained evidence is not proof of nonvolatile cancellation or hardware-idle state.

## Wait evidence bits

The drain hook samples all required software predicates (without weakening them):

- Bit0 (1): probe report ownership not released.
- Bit1 (2): HID pool/ledger/counter drain not satisfied.
- Bit2 (4): device EP0 drain not satisfied.
- Bit3 (8): driver queues/BDT/context drain not satisfied.
- Bit4 (16): module off/idle wait not satisfied, from the U1PWRC-only idle check.

The bits describe the last sampled wait, not necessarily an atomic hardware snapshot of the whole stack. A last committed phase is evidence of the last recorded transition; it cannot prove a later call returned if the processor stalled within it. Specific rebuild sub-step instrumentation is still a possible next diagnostic if phase INIT is observed.

## Read-only PREVIOUS query

Only the opt-in diagnostic gains opcode68/version1/nonzero request ID, exact6-byte query. It causes no pause, clear or write. The response is now 26 payload bytes: existing8-byte response envelope, then ticket:u32/generation:u32/time_ms:u32/state:u8/phase:u8/fault:u8/evidence:u8/valid:u8/checkpoint:u8. CRC/framing/report padding are unchanged. Host `usb_restart_diag.py previous` accepts this new read-only opcode and prints named phases/reasons plus a retained-SRAM warning.

The query reports only the prior validated snapshot captured at initialization; current operation cannot replace that snapshot. Existing status/tickets are not resumed from it. The currently delivered `2c91a8e` firmware has no68 handler, so **do not run this command against the board yet**; use only the new instrumented package and its [limited initial procedure](usb-instrumented-initial-check.md). Do not use the old package's host scripts for the new26-byte response.

## Verification and limits

`make test`/`test-sanitize` pass twenty-one C executables, 21 Python test methods and existing generated-source/hook/submission/rebuild harnesses. Journal tests simulate reset preservation, corrupt words, every uncommitted slot prefix and fallback; coordinator tests check exact first failure reason/phase and timeout wrap. Probe/host tests validate PREVIOUS as non-authorizing read-only evidence. Owning ISR/drain/USB hardware behaviour remains mocked in those suites.

`make usb-retirement-check` and the linked LTO image audit pass. The instrumented opt-in image fits the unchanged 32KB linker at text31341/data88/bss1936; default diagnostic remains text26670/data80/bss1568 and excludes opt-in paths. The final artifact audit now checks retained placement in addition to existing spans/configuration/symbol checks. Shared MPLAB XML lists 42 C sources.

Journal source review found no concrete slot-format bug and required read/write serialization, now documented/enforced at capture. Instrumented-owner review found stale pre-port evidence and a request acceptance/journal race; pre-call checkpoints and the request critical section fix both, confirmed in a bounded follow-up. A private test-TU view asserts committed journal checkpoints inside mock FENCE/POWER_OFF/REBUILD calls. Audit review identified malformed section-name and decoy `.dinit` cases; type/index/runtime-address/load/HEX validation and negative regressions now reject them. Broader driver failure investigation and owner integration review attempts timed out; a narrow description review found no obvious resume-authority defect while noting boot-scoped identity/SRAM limits. No actual board stopped-phase or journal-retention evidence is claimed.

The identified owner/journal review fixes, retained-startup audit and bounded initial read-only delivery review now pass. The [initial package](usb-instrumented-initial-check.md) may be programmed for the four read-only checks; send results before any new restart request. A controlled single PAUSE and MCLR/previous read could collect evidence, but it is **not authorised now**. Keep the current board in its recovered read-only state; no repeat PAUSE or higher request ID on the old image.
