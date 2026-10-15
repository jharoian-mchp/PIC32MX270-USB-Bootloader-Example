# Flash launch gate and conditional protection plan

The inactive NVM adapter now rejects initialisation and erase/write launch while **USBPWR or USBBUSY is set**, as well as central DMA activity. It does not stop/restart USB, change protection bits or enable an uploader. This is a conservative software gate; board timing/recovery qualification is still required.

## USB-off launch requirement

PIC32MX270 data sheet DS60001168L register 10-5, page 116, defines:

| U1PWRC field | Meaning |
|---|---|
| USBPWR=1 | USB module turned on |
| USBPWR=0 | Module disabled; outputs inactive, USB pins unused, analog features shut down |
| USBBUSY=1 | Module active, or disabled but not ready to enable |
| USBBUSY=0 | Module inactive and ready to enable |
| USUSPEND=1 | 48 MHz USB clock gated; transceiver low-power |

When USBPWR=0/USBBUSY=1, all **other** USB status is invalid and writes to USB module registers produce undefined results. The backend reads only U1PWRC for its USB gate; it writes no USB register.

The [USB FRM](docling/FRM/FRM-27-USB.md) confirms USB's dedicated Bus Master is independent of the PIC32 DMA module. U1CON.PKTDIS inhibits token/packet processing; its definition supplies no general bus-idle acknowledgement. USBEN, suspension, UOWN clearing and cancellation callbacks also lack a sufficient standalone guarantee for our flash-launch policy. This stage does not use them as substitutes for USBPWR=0/USBBUSY=0.

`UPDATE_NvmPortUsbActive` reads the two U1PWRC bits in one mask test. `UPDATE_NvmInit` rejects either bit; every `Start` repeats the test with interrupts disabled, **before any NVM register writes**. Tests separately cover USB powered and USB disabled-but-busy states, for init/erase/write, with unchanged launch/delay counts and restored interrupts. Independent static review found no concrete new guard defect.

This check is point-in-time. A single main-task owner must keep USB and DMA off throughout the operation, any timeout quarantine, settlement and readback. No other task/ISR may enable USB or submit transfers during that interval. The guard does not own a driver lock, halt an in-flight transfer or implement a persistent bus interlock. It excludes the documented USB active/powered states; mocks and compile checks do not establish hardware qualification.

## Host/driver work before connected updates

The current HID probe stays read-only. Integrating this backend directly into its configured, powered endpoint would fail the gate. Do not weaken the guard just to let that integration run.

A future connected-update design must choose one of these approaches and qualify it separately:

- Establish a supported arbitration/quiescence policy that demonstrably excludes interfering USB bus activity during unlock, without assuming PKTDIS or endpoint cancellation is sufficient. Replacing the conservative gate requires source/device evidence, independent review and measurements.
- Use explicit USB teardown/reinitialisation with host-visible loss of the interface. That needs a reviewed driver lifecycle, bounded USBPWR-off/USBBUSY polling, report ownership, acknowledgement ordering and a reconnect transaction model. A raw register power toggle around live IRPs is not implemented.

For teardown, the integration owner must define when to finish the host acceptance response, stop new reports/control traffic, disable driver task/ISR ownership, retain/cancel owned buffers, turn off the module, wait for inactive status, launch/settle/read back NVM, and initialise/re-enumerate safely. Never access other USB registers during disabled-but-busy status. Failure to establish inactive status must leave flash untouched; timeout buffers remain retained.

There is a protocol conflict to resolve: `UPDATE_DispatchDisconnect` aborts an incomplete session, whereas an intentional update-related USB shutdown would need an explicit, bounded suspension policy to retain the session. Host `transact` and offline `plan` implement neither reconnect mutation delivery nor session resumption. Full-image staging is also unavailable: maximum image length is 228352 bytes, exceeding the target's 64 KB SRAM. Per-row USB shutdown would be unsuitable without a host-tested batching/latency design. This revision does not choose or claim that transport design.

## Correct target/revision first

The original family errata **DS80000531R**, Table 3 page 2, lists PIC32MX270F256D device ID **0660A053**, with silicon revisions A1=1 and A2=2. Device data sheet register 27-6 defines DEVID bits 27:0 as device ID and 31:28 as revision. Record the raw 32-bit register value before any protection change:

```text
part_id = raw_DEVID & 0x0FFFFFFF
revision = raw_DEVID >> 28
```

For the listed D part, raw A1/A2 examples are **1660A053 / 2660A053**. These are expected examples, not raw board-register measurements. The DB variant has a different device ID; unknown IDs/revisions require a fresh applicability check.

### Board evidence received 2026-10-15

The user supplied Device ID **0660A053** and tool-reported “Device Revision Id = 0xA1”. This identifies the expected PIC32MX270F256D **A1** revision. `0xA1` is the tool's revision label; it is not a raw four-bit VER value. A raw combined DEVID register value was not supplied and is not required to repeat the tool's part/revision identification.

[The configuration screenshot](images/device-config-a1-2026-10-15.png) displays:

| Field | Displayed value | Decoded protection |
|---|---|---|
| DEVCFG0 at physical 1FC00BFC | **7FFFFFE0** | PWP field **1FF**, BWP **1**, CP **1** |
| PWP | OFF | Program Flash unprotected |
| BWP | OFF | Boot Flash unprotected |
| CP | OFF | Code protection disabled |

The screenshot also shows DEVCFG3=FFFFFFFF, DEVCFG2=FFF9FAFA and DEVCFG1=FD74CE5B. These are IDE-displayed configuration values; the image alone does not establish whether the table is a live device readback or a loaded image/configuration view. Do not promote it to post-programming readback evidence. The protection choices match the generated configuration; no bits have been changed here.

This evidence completes the part/revision identification input for the conditional A1 plan. Programming configuration bits, bulk erase and protection tests still require an explicit reviewed hardware/recovery plan.

## Errata applicability and 32 KB candidate

The earlier audit listed family protection issues without selecting a silicon row. Inspection of the original PDF tables, including rendered pages 15–16, gives this distinction for the **256 KB Flash / 64 KB RAM A1/A2** row:

| Erratum | A1/A2 applicability | Effect on this conditional plan |
|---|---|---|
| 21 — PWP depends on BWP | Affected | Enable BWP with PWP if protection is selected |
| 22 — BWP also overlaps first program page | Marked unaffected | Do not apply this family's first-page workaround blindly |
| 23 — PWP off by one page | Marked unaffected | Do not subtract one from nominal PWP32K for this row |
| 24 — whole-Flash protection encoding | Marked unaffected | No whole-Flash encoding workaround selected; this plan protects only the bootloader prefix |

The original PDF tables govern; the converted text loses some table alignment. The installed DFP 1.7.380 `xc32/32MX270F256D/configuration.data` defines **PWP32K**, masked DEVCFG0 value **00077C00**, field value **1DF** under mask **0007FC00**. This is the nominal first-32-KB protection encoding.

**Conditional candidate, not applied:** the reported PIC32MX270F256D A1 fits the listed applicability row. After a reviewed recovery plan, select `PWP=PWP32K`, `BWP=ON`, retaining other settings. The intended boundaries are:

- Program Flash pages 0–31: physical `[1D000000, 1D008000)` protected (32 × 1024 bytes).
- Relocated application begins on page 32: `1D008000`, intended writable.
- Application data ends before page 255; metadata page 255: `[1D03FC00, 1D040000)`, intended writable.
- Boot Flash/configuration protection and PICkit/bulk-erase recovery must be reviewed under BWP. Do not test a forbidden erase by invoking an unbounded or generated updater.

Generated bootloader configuration currently has **PWP=OFF/BWP=OFF** and remains unchanged. Software span checks protect only those validated calls; they do not establish hardware protection against other callers or corrupted code.

Before applying the candidate, verify the board ID/revision, linker/HEX/configuration output, exact configuration-bit readback, recovery method, and both protected and writable boundaries on hardware using a controlled test plan. Protection-setting changes can affect debug/programming/recovery. Keep the standalone baseline tag and PICkit recovery artifacts intact.

## Verification and next input

`make test`, `make test-sanitize`, `make nvm-codegen-check` and `make bootloader` pass with the USB guard. Diagnostic footprint stays text26670/data80/bss1568 and inactive update execution symbols are absent. No hardware writes were performed. A delegated USB research attempt timed out; the guard then received a completed bounded static review, which explicitly noted the whole-operation ownership requirement.

Board part/revision and IDE-displayed DEVCFG0 are now recorded. Next define the host/driver lifecycle and protection-boundary/recovery test plan; obtain actual device configuration readback when that test is performed. Windows MCC regeneration and relocated-application linker/startup review still precede a live uploader. See [integration audit](nvm-integration-audit.md), [backend contract](update-nvm.md) and [application regeneration requirement](application.md).
