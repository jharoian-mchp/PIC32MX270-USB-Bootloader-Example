# Stock H3 UART target demonstration — 19 November 2026

The stock UART bootloader programmed the relocated LED application, verified its CRC and reset the target. John confirmed LED1 blinking after manually continuing from the debugger's reset halt. The debug session remained running.

## Tested configuration

- PIC32MX270F256D, tool-reported revision A1/deviceID0x660a053, existing Bluetooth Audio PIM/carrier.
- H3 bootloader package3.9.0; generated wire-version constants3.7 (reply50 03 07).
- MPLABX6.35/XC326.00/DFP1.7.380; installed Windows DebugELF SHA256ed70857a7db59ab01bc54324ba3b91607af4d91a5aa6b33f082af154c4e963de. Production candidate was not restored for this test.
- UART2/115200/8N1, FTDI TTL-232R-3V3 COM41; blackJ8pin2DGND/orangeTXpin3RC8/yellowRXpin5RC4, VCC/flow-control leads disconnected; no Bluetooth daughterboard. Both signal paths continuity checked to MCU pins4/37.
- Application entry9D008000, fixed32KB reservation; sparse binary227328bytes/222pages covers physical1D008000..1D03F7FF withFFpadding and high exception page.
- Application SHA256c13edd9338e2a720abd84baf574efd65b7e78be66b1192b9acdd7d43acffdacd.
- Stock Pythonhost SHA256964dde1b8c9be0a006ebd284d422c275520b8703fc5f36f0b762e738a3b9af84, uvvenvCPython3.14.3/pyserial3.5.

## Observed sequence

1. Earlier empty-reply captures were taken with both FTDI and Saleae onJ9; John corrected them toJ8 after continuity testing. Those captures do not establish a UART2 firmware fault.
2. A receive-path hardware breakpoint demonstrated data available and overrun (U2STA9513) while stopped before FIFO consumption. This interrupted frame did not qualify parser failure.
3. With reset-vector halt verified and no enabled breakpoints, one running A6 request returned50 03 07/version3.7; existing capture526 retained in workspace.
4. On19November19:20 John approved one stock-host application transfer/CRC/reset. The command usedCOM41/PIC32MX/cached9D008000/sector1024/application.bin; no stock UART parser/NVM hardening was applied.
5. At19:24 the host reported ReadingBootloaderVersion/v3.7, Unlocking, Programming100%, Verification success, RebootDone.
6. At19:26 John reported a debugger reset halt, an additional manualContinue, then LED1blink and debugsessionrunning.

The additional Continue occurred before the requested halt review; it is recorded as a checkpoint deviation. The observed application execution followed that Continue. Autonomous production reset-to-application execution has not been demonstrated by this session.

## Scope and remaining work

This passes the normal stock UART transfer, host CRC verification and relocated application execution in the tested Debug session. The UART parser/NVM source remains unchanged from generated6c44d8e (sourceSHA b9cef7a74626e65fb371b4753ab9209dc2d873e5eb1a89a3d5a1d1144ff0c0da). Known malformed-input/error-reporting limitations remain in [UART review](customer-uart-review.md); [HID findings](customer-upload-review.md) are separate.

Production installation/handoff without debugger, power-cycle recovery, repeated uploads, bootloader re-entry, interruption/fault/protection qualification and the larger customer variant have not been tested. Stock triggerfalse automatically starts a non-erased application; noSW1re-entry hook was added to the UART baseline.

Last reported board state: application running/LED1blinking, active debugger/PICkit, external supply and FTDI/Saleae retained. No new halt/reset/program/upload/disconnect/session-end action is authorised by this result.
