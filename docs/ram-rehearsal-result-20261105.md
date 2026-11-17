# RAM rehearsal — 5 November 2026

The scoped RAM-only rehearsal passed in MPLAB X v6.35 with PICkit 5, XC32 v6.00 and PIC32MX_DFP 1.7.380. The target remains halted at RAM_RehearsalReadout; no session termination or further board operation is authorised.

## Artifact and target

- IDE ELF SHA-256: `09458d693083dc96786684dcc3aac814e4f2bac7b0510ed3c66ea767e2a6d709`.
- All 3,728 file-backed allocated bytes and allocated layout, including NOLOAD reservations, match the previously checked Windows ELF. There are 67 nonempty executable sections / 3,636 bytes.
- PICkit reported PIC32MX270F256D, revision A1, device ID `0x660a053`, external target voltage detected.
- Reset: `0xBFC00000`; pre-edit: `0x9D000278`; readout: `0x9D000288`. MPLAB's physical breakpoint addresses are `0x1D000278` and `0x1D000288`.
- Context: `0xA0000200`, 200 bytes; mailbox: `0xA00002C8`, 148 bytes.
- Configuration DEVCFG0..3: `7FFFFFE0/FD74CE5B/FFF97AFA/FFFFFFFF`. IDE Configuration Bits displayed DEVCFG0 `7FFFFFE3` and omitted the IDE-controlled DEBUG field. DFP mask `3` accounts for the difference; the display was not target readback.

## Evidence

Attachment numbers refer to chat `web:chrisk-agent`.

| Evidence | Finding |
|---|---|
| 336–340 | Successful IDE Debug build/load; expected pack and checked ELF/map |
| 345–346 | Reset halt: IDE current-PC source fallback `_reset`, crt0.S:93 maps to BFC00000 |
| 348–352 | Single pre-edit execution breakpoint, Always Break; hardware Program BP 1 used / 5 free; pre-edit halt |
| 356 | Baseline WAIT_EDIT, fault 0, edit 0, guards intact, sequence 6; all 33 report words match private context |
| 360 | Single accepted 32-bit edit at offset 2CC; only that word changes from 0 to 52414D31; no execution/reset |
| 362–367 | Sole readout hardware breakpoint at 1D000288; approved Continue reached RAM_RehearsalReadout |
| 368–369 | Final report RAM_COMPLETE / fault 0; target-upload export method shown |
| User confirmation 21:57:48 | Final export completed; same readout halt; no reset or further execution |

## Final report checks

All 87 intended words at word-start offsets `0x200..0x358` are present.

- Version/mode/state/fault: `1/1/4/0` (RAM_COMPLETE).
- Edit attempts/observations: `1/1`; challenge observed: `52414D31`; edit cleared to zero by firmware.
- Guards: `13579BDF/2468ACE0`; mailbox and private expected sequence: `12`.
- All 33 published report words equal the private report.
- Before/after sampled clock, interrupt, USB/DMA/NVM exclusion and exact configuration checks pass the source profile.
- Private flags show initialised, spent, terminal, pre-edit called and readout attempted; sequence invalid and readout denied are false, reason zero.
- Post-consumption Count anchor `0x872`; final `0xB58`; unsigned delta `0x2E6` = 742 ticks. Frequency and wall-clock timing were not measured.

These checks used read-only Bun parsing and comparisons of the saved uploads. No new build, link or board operation was used for consolidation.

## Deviations

- After the first configuration-only programming log, the user performed a separate erase and repeated discrete programming despite the no-retry checkpoint. The second log reported program memory `0x1D000000..0x1D001FFF`, configuration and boot configuration, with Programming/Verify complete. The first operation's cause is unknown.
- The first baseline export contained only 44 words. The user performed a second Output To File before further approval.
- Full dumps contain 92 words through `0x36C`, five words beyond the intended range. The final notification shows a selected range through `0x360`; row expansion accounts for the file extending to `0x36C`. Qualification checks use only the intended 87 words.

## Limits and next decisions

This evidence supports the breakpoint, single-word RAM edit, target-upload readback and challenge workflow in this session. It does not qualify debug-executive storage or behaviour, historical build provenance, full executable safety, calibrated Count timing, NVM writes or protection behaviour.

Offline follow-up must define the NVM image/profile policy (the existing isolated target requires DEBUG OFF), final linked write-path audit, disposable-page identity and 1024-byte/128-byte geometry, bounded no-halt transaction timing, and recovery procedure. Firmware changes, another link, installation, page selection, connected writes and session termination each require explicit scope before execution.

Current hardware instruction: preserve the readout halt, external power and PICkit connection. No Continue, Step, Reset, Read, RAM write, programming, disconnect or session termination is included in this evidence package.
