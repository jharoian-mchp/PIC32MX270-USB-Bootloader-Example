# Single-owner restart integration and size check

The opt-in read-only restart build now replaces the old board USB callback/attach/task path at compile time. `make bootloader-restart-check` links with the unchanged **32 KB** bootloader linker at **`-Os`**. **No host pause command or approved board test image exists yet.** Default firmware and protection/application layout stay unchanged.

## Explicit board handover

`USB_RESTART_DIAGNOSTIC` defaults to 0. Enabling it requires `USB_RETIREMENT_HOOKS=1`; selecting it without the hooks fails compilation.

In the opt-in configuration:

- The old board HID/device callbacks, attach/detach protocol calls and probe/device task loop are compiled out.
- SW1 startup qualification, AN1 sampling/hysteresis, mux/I2C readback and physical boot-safe policy remain in `board_usb_diag.c`.
- Once the device opens, the board initialises the restart owner exactly once and passes qualified cable/mux state to it. The owner becomes the sole USB/probe handler; board diagnostics mirror its attached/configured status.
- Failed owner initialization explicitly detaches, records USB_OPEN fault and quarantines without retrying a possibly partial initializer. Its handle/storage are retained until reset. The board still qualifies ADC/mux state but does not service USB through another owner.

Default builds keep the previously hardware-tested USB probe behaviour. The new branch has mocked handover tests but no board evidence.

## Size evidence

The former enabled link retained both USB owners and failed at `-O2/-Os`. Removing the duplicate old path still fails at `-O2`, but the integrated `-Os` branch passes the actual unchanged linker:

| Build | Evidence |
|---|---|
| Default diagnostic, `-O2` | text26670/data80/bss1568; unchanged |
| Integrated restart check, `-Os` | text32206/data84/bss1700; actual 32KB linker passes |
| Integrated restart, `-O2` | program-memory overflow; not a supported delivery configuration |

Text/data/BSS totals include different address regions; the **actual linker pass** establishes fit in the configured program reservation. Do not sum RAM BSS into Flash usage or infer ample space from these totals. Remaining program margin is small and a future host command may require more optimisation.

A temporary ignored inspection ELF used an enlarged linker solely to measure section growth. It was never converted/delivered for programming. The successful check was rerun against the canonical linker with length32768 and physical application origin1D008000 unchanged. No source linker/protection bits were edited.

```sh
make bootloader-restart-check
```

This target selects the single-owner flags/`-Os`, retains request/ACK APIs for link measurement and checks that update/NVM write routines are absent. It is **compile/link evidence only**; the generated check HEX is not approved for programming. No restart packet handler invokes those retained APIs, and no HEX is attached.

## Tests/review and next delivery gates

`make test`/`make test-sanitize` pass seventeen C executables, 16 Python methods and existing source/hook/submission/rebuild harnesses. The new board handover test verifies:

- Only owner USB tasks run after entry; old callbacks/attach/task loop do not run.
- Owner initializes once, with ADC/mux inputs passed by the board.
- Initialization rejection produces detached USB_OPEN quarantine, no silent stranded loop or unsafe retry.

Independent bounded handover review found the silent owner-init failure path; it was fixed and tested. The review verified only the supplied board/owner sources and did not certify retirement/probe internals. A final follow-up owner review timed out; full final owner/host-command approval remains open.

Next add/test the small read-only pause request and its completed-send ticket authority, fit it within the same linker, complete final owner/transport review and deliver explicit Windows/PICkit restart instructions with a verified image hash. Then run repeated restart, cable/reset faults and generation checks on the board. NVM/protection/application/upload work stays gated separately. **No reprogramming is requested for this size/handover stage.**
