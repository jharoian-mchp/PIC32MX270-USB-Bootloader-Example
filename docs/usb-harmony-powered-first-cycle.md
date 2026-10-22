# First successful Harmony-powered USB-only cycle — 2026-10-22

The single authorised PAUSE100 on the experimental Harmony-powered image passed removal/reconnect verification and returned a separately confirmed live ticket1/generation2/completed_cycles1/state7. HID and debug capabilities remained available afterwards. This is one successful controlled cycle, not production or repeated-cycle qualification.

## Reported result — 03:51

The user ran the matching `usb-harmony-powered-f1d9e3c` package's restart command once. Its output reported:

| Field | Result |
|---|---|
| Before | ticket0 / generation1 / completed_cycles0 / state0 |
| After | ticket1 / generation2 / completed_cycles1 / state7 (DONE) |
| Removal observed | true |
| PAUSE response uncertainty | null |
| HID path | Same path used before and after |
| Separate STATUS | ticket1 / generation2 / completed_cycles1 / state7 |
| Post-list | One expected VID04D8/PID003C/interface0/vendor usageFF00:1 HID |
| Debug probe | protocol1/max510/origin1D00C000/end1D03FC00/row128/erase1024 |

The result satisfies this trial's declared success criteria: observed removal, verified same-path reconnect and live generation/cycle progression, followed by independent STATUS and probe. The authorised procedure kept PICkit5 disconnected and external power/settings unchanged, with no MCU reset or recovery requested during the command. Counters and HID path are not boot authentication or cryptographic proof that no reset occurred.

Firmware/host source: `f1d9e3cfdb08d30f6768df8cf2a1930c8f4bd97b`; delivered procedure `30aaf5b`, package verification `a171d7b`. This is the temporary48KB/origin1D00C000 image. No firmware change was made in recording the result.

## What this establishes and leaves open

- The previously failing controlled USB-only cycle completed on this image and board for one attempt. A reset/reprogramming recovery was not needed after it.
- The experimental powered readiness policy, source-masked binding and later attachment path produced a working host link in this trial. This does not identify the actual USBBUSY/USBEN values at the successful readiness sample; no new retained snapshot was captured.
- The prior failure's BUSY samples and timing remain historical evidence. Current PREVIOUS/power-previous queries expose the boot-captured snapshot, not the successful current-runtime journal. No reset is requested to retrieve that journal.
- The PIC32MX270 A1 powered-BUSY concern remains open. A successful cycle is not a manufacturer clarification, universal BUSY interpretation, electrical qualification or proof that every reconstruction operation is safe.
- Repeated cycles, longer-run behavior, fault recovery, stale-reference provenance, power-loss retention, USB-off NVM/protection and connected uploads remain separately unqualified. No production readiness is inferred.

The artifact audit flags remain false in the delivered package: those records describe pre-delivery artifact verification and are not rewritten retroactively. This document records the narrower hardware result separately.

## Stop boundary and next source work

The single-trial permission is spent. Keep the now-reconnected board running/read-only with external power/settings unchanged and PICkit5 disconnected. No second PAUSE, higher ID, reset, reprogramming, uploads, NVM/protection changes or fault test is authorised.

The subsequent [source-only32KB restoration](usb-restart-32kb-restoration.md) passes compile/link and artifact checks through a build-only data-section change. This cycle's success does not qualify that smaller image; any replacement programming or trial still needs its own verified package and explicit approval.
