# First canonical 32 KB USB-only cycle — 2026-10-22

The single authorised canonical32KB PAUSE100 passed removal/reconnect verification and returned a separately confirmed live ticket1/generation2/completed_cycles1/state7. The expected HID interface and normal-origin capabilities remained available afterwards. This is one successful controlled cycle on this exact image, not repeated-cycle or production qualification.

## Reported result — 05:27

The user ran the matching `usb-canonical32-09da506` package's restart command once, following the separate05:22 authorisation:

| Field | Reported result |
|---|---|
| Before | ticket0 / generation1 / completed_cycles0 / state0 |
| After | ticket1 / generation2 / completed_cycles1 / state7 (DONE) |
| Removal observed | true |
| PAUSE response uncertainty | null |
| HID path | Same path selected before and after |
| Post-list | One expected VID04D8/PID003C/interface0/vendor usageFF00:1 interface |
| Independent STATUS | ticket1 / generation2 / completed_cycles1 / state7 |
| Plain capabilities probe | protocol1/max510/origin1D008000/end1D03FC00/row128/erase1024 |

This satisfies the declared single-cycle criteria: removal observed, same-path reconnect verified and exact live ticket/generation/cycle progression, followed by independent STATUS and probe. The authorised procedure kept PICkit5 disconnected and external power/settings unchanged; no MCU reset or recovery was requested during or after the command. Counters and HID path do not authenticate a boot or cryptographically exclude an intervening reset.

## Image and verification provenance

- Firmware/host/build source: `09da506e0ce228015c358f99415e08b68f6a19a0`.
- Diagnostic HEX SHA-256: `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7`.
- Package: `usb-canonical32-09da506.zip`, externally confirmed SHA-256 `54f065eded469b7a287f4254a18d6b1f7047366af9c730ced1a8b76fecb08dd3`.
- Packaged procedure `70c3343`; archive verification `94746f7`.
- XC32 v6.00, DFP1.7.380, `-Os -flto -fno-data-sections`; reservation32768 bytes, application origin1D008000.
- Linked footprint text32284/data88/bss2052, program-region use31876/32768; schema4 retained journal128 bytes.

The user previously supplied matching extracted-file/HEX verification, IPE Programming/Verify complete for the expected target/program range, and the five initial read-only responses. The downloaded archive's external hash was reported after programming; no pre-extraction verification timing is inferred. No code, HEX or protection change was made to record this result.

## Established scope and remaining limits

The canonical coalesced-data image completed this controlled USB-only restart without a requested MCU-reset recovery, and its initial layout and post-cycle capabilities use the original1D008000 application boundary. Together with the earlier48KB [first cycle](usb-harmony-powered-first-cycle.md), two distinct images have each passed one separately authorised cycle. This is not evidence of two successive cycles within one runtime, sustained reliability or successful fault recovery.

The PIC32MX270 A1 powered-BUSY concern remains open. Successful reopening does not supply the actual BUSY/USBEN observations at the successful check or a device-specific manufacturer clarification. PREVIOUS and power-previous are boot-captured historical snapshots; reading them now would not expose this successful current-runtime journal. No reset to capture that journal is requested.

The source policy still treats BUSY as evidence during powered reconstruction, requires USBPWR1 and preserves other readiness predicates. Retirement/preparation and USB-off NVM gates remain USBPWR0/BUSY0 with their independent ownership/exclusion conditions. A successful USB cycle does not authorise flash access, protection changes, application handoff or uploads.

Repeated cycles, electrical safety, stale-reference provenance, retention across arbitrary resets/power loss, NVM/protection timing and write-gated transport remain separately unqualified. Pre-delivery artifact audit flags remain false in the archive; this narrower observed board result is recorded separately rather than rewriting historical audit metadata.

## Stop boundary

The one-trial permission is spent. Keep the now-reconnected canonical image running/read-only with external power/settings unchanged and PICkit5 disconnected. No second PAUSE, higher ID, reset, power-cycle, reprogramming, NVM/protection change, upload or fault test is authorised.

Later work can plan broader restart or isolated NVM/protection qualification, but any hardware test or firmware replacement needs its own reviewed scope and explicit approval. Windows MCC application regeneration/relocation and connected write integration remain later gates.
