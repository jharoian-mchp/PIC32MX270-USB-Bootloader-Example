# Restart phase and reset-retained diagnostic evidence

The instrumented board PAUSE100 failed at **INIT / REBUILD**; powered SW1/MCLR recovery restored HID and recovered the retained failure record. Its exact failing sub-step is still unknown. The current source-only stage adds rebuild sub-step and first-rejection diagnostics without changing safety predicates. No replacement package is delivered or authorised. **Leave the board recovered: no repeat PAUSE, higher request ID, reset or reprogramming.** Uploads, NVM writes, protection changes and fault tests remain disabled. The [18384b2 package procedure](usb-instrumented-initial-check.md) records the spent trial.

## Board result — 2026-10-21

Following instrumented PAUSE100 failure and requested powered MCLR/SW1 recovery, PREVIOUS validated ticket1/generation2/stateFAILED/phaseINIT/faultINIT/checkpointREBUILD/time1351925/evidence0. The journal survived this reported recovery and host decode; it is not general power-loss retention. Live STATUS restarted at ticket0/generation1/cycles0/state0.

The USB-only initializer returned failure after REBUILD entry. The exact preparation/construction/binding/readiness sub-step remains unknown. The validated source instrumentation below is not installed on the board and cannot reconstruct that missing detail.

## Failure fields

`USB_RESTART` now records the first failed phase and reason without altering its safety transitions. Later cancellation cannot overwrite the original failure. Reasons: NONE=0, TIMEOUT=1, ACK=2, FENCE=3, CANCEL=4, POWER_OFF=5, OFF_IDLE=6, DRAIN=7, RETIRE=8, INIT=9, EXTERNAL=10. Existing phase values stay READY0/ACK_WAIT1/DRAIN2/OFF_WAIT3/RETIRE4/INIT5/CONFIG_WAIT6/DONE7/FAILED8.

A wait timeout reports the phase it timed out in. Rejected ACK and failed port operations report their exact category. This instrumentation does not reinterpret or relax a failed drain/off-idle predicate.

## Retained SRAM journal

`usb_restart_journal.c/.h` stores two 48-byte slots, each with magic/version, monotonic sequence/complement, ticket/generation/time, state/phase/fault/evidence/checkpoint, rebuild detail, CRC32 and a final commit word. The alternate slot is invalidated before writing and committed last. A torn/corrupted latest slot falls back to the older validated slot; sequence exhaustion refuses another write rather than wrapping.

Journal schema 3 uses the formerly reserved word9 for checksummed rebuild detail, keeping both 48-byte slots and the 96-byte retained allocation. Schema 1 and 2 records are rejected. The delivered 18384b2 firmware uses schema 2; it has no rebuild detail. Checkpoints stay NONE0/FENCE1/CANCEL2/DRAIN3/POWER_OFF4/OFF_IDLE5/RETIRE6/REBUILD7/CONFIGURED8. Each owner port wrapper commits its checkpoint **before** invoking the potentially blocking operation. A failure record keeps the last entered port as well as the coordinator's first failure phase/reason. It does not prove that port returned.

One IRQ-excluded owner serialises **both reads and writes**. Request eligibility, acceptance and journal write now share IRQ exclusion; an ISR reset cannot slip between eligibility and recording. A checksum is not a lock. The owner captures previous validated evidence at initialization, before current operations write the journal. A current-runtime cache avoids CRC work on unchanged task passes. There are no file/flash writes and the journal is never used to authorize or resume a restart.

XC32 `persistent` alone was insufficient under this LTO build: the initial linked symbol landed in ordinary BSS. The source now also specifies `.persist.usb_restart_journal` and `used`. Final ELF audit requires the 96-byte journal inside `_persist_begin.._persist_end` and a retained NOBITS section. It currently links at A0000000..A0000060, outside normal BSS initialization.

The final linked `.dinit` audit now parses the inspected XC32 v6 clear/copy records, requires allocated/file-backed Flash data matching the runtime `_dinit_addr` and delivered HEX, and rejects any initialization destination overlapping the journal. Unknown operations or malformed section-name/`.dinit` metadata fail closed. A negative ELF/HEX pair that would clear the journal is rejected even with matching delivered data.

These startup checks support evidence retention across MCLR only if target SRAM and startup preserve it. Power loss, programming, a changed RAM layout or arbitrary SRAM corruption can invalidate it. This trial provides one successful retained-record recovery result; broader reset/power/programming preservation remains unqualified. Tickets/generation restart in each boot and are not global identity; a previous record is historical diagnostic evidence only. Valid retained evidence is not proof of nonvolatile cancellation or hardware-idle state.

## Wait evidence bits

The drain hook samples all required software predicates (without weakening them):

- Bit0 (1): probe report ownership not released.
- Bit1 (2): HID pool/ledger/counter drain not satisfied.
- Bit2 (4): device EP0 drain not satisfied.
- Bit3 (8): driver queues/BDT/context drain not satisfied.
- Bit4 (16): module off/idle wait not satisfied, from the U1PWRC-only idle check.

The bits describe the last sampled wait, not necessarily an atomic hardware snapshot of the whole stack. A last committed phase is evidence of the last recorded transition; it cannot prove a later call returned if the processor stalled within it. Rebuild detail is separate from this wait bitmap.

## Rebuild sub-step and first rejection

Detail packs `stage:u8` in bits0–7, `rejected_mask:u16` in bits8–23 and `observation_bits:u3` in bits24–26. Bits27–31 are reserved zero. The owner clears detail at a new port entry and commits each entered rebuild step before its potentially blocking call. A rejection records the first failed check; later checks are not evaluated. Successful new checks clear stale rejection masks. Partial failures still quarantine the stack and keep the caller's handle unpublished.

| Stage | Value | Rejection bits (bit0 first) |
|---|---:|---|
| NONE | 0 | No rebuild detail |
| INPUT | 1 | bit0: invalid generation, callback or output pointer |
| PREPARE | 2 | bit0: preparation call rejected; composite guard unchanged |
| DRIVER_INIT | 3 | bit0: driver initializer returned invalid object |
| DEVICE_INIT | 4 | bit0: device initializer returned invalid object |
| DEVICE_OPEN | 5 | Entered device task/open stage; no separate return-value check |
| BIND_DEVICE | 6 | bit0: device binding call rejected |
| BIND_HID | 7 | bit0: HID binding call rejected |
| REOPEN | 8 | bit0: not preparing; bit1: generation mismatch |
| HID_READY | 9 | bit0–5: not preparing, mutex uninitialised, ledger pending, callback missing, TX count nonzero, RX count nonzero |
| DEVICE_READY | 10 | bit0–9: not preparing, not ready, task not running, client not open, callback missing, descriptor missing, interface missing, not detached, EP0 RX pending, EP0 TX pending |
| DRIVER_READY | 11 | bit0–13: not preparing, not in use, not open, not ready, callback missing, attached, in ISR, endpoint table missing, BDT missing, IRQ enabled, USBBUSY set, USBEN set, queue pending, BDT owned |
| COMPLETE | 12 | Rebuild returned success and published the private handle; later attachment/configuration still required |

Driver observations are bit0=`USBPWR`, bit1=`USBBUSY`, bit2=`USBEN`. `U1PWRC` is sampled only after the preceding software/IRQ checks pass. `U1CON` is read only after `USBBUSY` is clear. Zero observation bits before that point mean unsampled registers; if busy is set, USBEN is unsampled. The three bits are observations from ordered reads, not an atomic hardware snapshot or restart permission. For example, stage11/mask1024/observations3 means the driver rejected USBBUSY with USBPWR and USBBUSY observed set; it says nothing about USBEN or later queue/BDT checks.

The observer writes only the IRQ-serialized SRAM journal. Reopen retains its original ordered HID/device/driver short-circuit checks. No detail field changes acceptance, failure, thaw, NVM ownership or resume authority.

## Read-only PREVIOUS query

Only the opt-in diagnostic gains opcode68/version1/nonzero request ID, exact6-byte query. It causes no pause, clear or write. The current source response is 30 payload bytes: existing8-byte response envelope, then ticket:u32/generation:u32/time_ms:u32/state:u8/phase:u8/fault:u8/evidence:u8/valid:u8/checkpoint:u8/detail:u32. All multibyte fields are little-endian. The schema-2 package has a 26-byte response. CRC/framing/report padding are unchanged; the fixed header bounds the expanded framed reply within one 64-byte report. Host `usb_restart_diag.py previous` decodes named phases/reasons and rebuild detail, rejects reserved/unknown stages and invalid first-rejection masks, and retains the SRAM warning.

The query reports only the prior validated snapshot captured at initialization; current operation cannot replace that snapshot. Existing status/tickets are not resumed from it. Use the board's matching 18384b2 package-local scripts for its 26-byte PREVIOUS response. Do not mix the current 30-byte decoder with that firmware. The older 2c91a8e firmware has no68 handler. Any future schema-3 package needs matching firmware/scripts and a separate gated delivery review.

## Verification and limits

`make test`/`test-sanitize` pass twenty-one C executables, 22 Python test methods and existing generated-source/hook/submission/rebuild harnesses. Journal tests simulate reset preservation, corrupt words, every uncommitted slot prefix and fallback; coordinator tests check exact first failure reason/phase and timeout wrap. Probe/host tests validate PREVIOUS as non-authorizing read-only evidence. Owning ISR/drain/USB hardware behaviour remains mocked in those suites.

`make usb-retirement-check` and `make restart-image-audit` pass, including negative linked-image regressions. The current source-only opt-in image fits the unchanged 32KB linker at text32041/data88/bss1952 with little margin; default diagnostic remains text26670/data80/bss1568 and excludes opt-in paths. The delivered schema-2 image was text31341/data88/bss1936. The final artifact audit now checks retained placement in addition to existing spans/configuration/symbol checks. Shared MPLAB XML lists 42 C sources.

Journal source review found no concrete slot-format bug and required read/write serialization, now documented/enforced at capture. Instrumented-owner review found stale pre-port evidence and a request acceptance/journal race; pre-call checkpoints and the request critical section fix both, confirmed in a bounded follow-up. A private test-TU view asserts committed journal checkpoints inside mock FENCE/POWER_OFF/REBUILD calls. Audit review identified malformed section-name and decoy `.dinit` cases; type/index/runtime-address/load/HEX validation and negative regressions now reject them. Broader driver failure investigation and owner integration review attempts timed out; a narrow description review found no obvious resume-authority defect while noting boot-scoped identity/SRAM limits. The later board result above now establishes a validated retained INIT/REBUILD failure record for one recovery; earlier source tests alone did not.

The rebuild-detail review found a REOPEN outer-precondition reporting gap; explicit not-preparing/generation-mismatch bits and regressions fix it without changing eligibility. Tests also verify pre-call detail, journal word9 preservation, schema rejection, stale-mask clearing, driver busy/enable/IRQ rejections and expanded reply framing. A documentation consistency review also found that the host accepted rejection bit0 for NONE/DEVICE_OPEN/COMPLETE; explicit empty masks now reject those combinations, with strict/sanitizer regressions and bounded follow-up confirmation. Source tests and bounded follow-up review pass. Hardware restart qualification still fails, and the board's exact rejection is unknown. No replacement package or further hardware action is authorised.
