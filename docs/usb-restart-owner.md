# Default-off USB restart owner — board delivery blocked

`usb_restart_owner.c/.h` binds the restart coordinator to target cancellation, drain, power-off, retirement and detached rebuild helpers. **It has no active caller or host restart command.** The initial enabled link retaining both old/new USB owners exceeded the **32 KB** reservation. A subsequent [single-owner integration](usb-owner-integration-size.md) now passes the unchanged linker at `-Os`; `-O2` still overflows. No board test image is delivered; application origin/protection and working diagnostic firmware are unchanged.

## Exclusive owner boundary

Before using this internal owner, the caller must permanently stop the old `BOARD_USB_DIAG_Tasks` USB attach/open/task/event paths and supply qualified ADC cable/mux state separately. The owner does not independently enforce that handover. Two owners operating on the same handles/layers would invalidate the cancellation and restart contract.

Initialisation requires a valid handle and no pending probe report transfers. It installs generation-tagged device/HID routes, begins detached and resets only probe/owner state. It does not initialise clocks, GPIO, ADC, mux, NVM or update sessions. A future host protocol must send a read-only pause response and provide its successful full local transfer completion before `USB_RestartOwnerAck`; no such request/response exists today.

## Implemented target paths

- Fence all admitted USB submission paths using the checked retirement overlay.
- Configure probe inactive, cancel both HID directions and EP0 RX/TX, then detach under interrupt exclusion. Cancellation return failure quarantines the coordinator; no hidden retry or buffer release follows.
- `USB_PROBE_RetireReady` consumes already-recorded completion flags without submitting transfers or writing USB registers. It requires both probe pending handles released before the owning HID/device/driver drain predicates pass.
- Power off only through retirement authorization; OFF_WAIT observes U1PWRC-only off/idle state. Normal probe/device tasks and attach calls do not run during drain/off/retirement.
- Retire all layers, call the staged detached USB-only rebuild with a new callback context generation, publish the new handle and initialise probe buffers only after successful retirement/rebuild.
- Resume owner-controlled attachment and probe/device tasks in configuration wait. ADC/mux qualification remains an input, not a new hardware implementation.

These source paths depend on the existing overlay, ledger and rebuilt-layer predicates. They do not establish actual USB bus/interrupt/hardware old-reference quiescence beyond those checked assumptions.

## Event and callback handling

HID callbacks reject the wrong transport generation before touching probe state. Device callbacks only record flags; main owns restart transitions. Reset invalidates any earlier CONFIGURED flag in the same pending batch, while a later CONFIGURED after reset can establish a fresh enumeration. ERROR dominates configuration and clears probe/configuration state. Cable/mux loss clears diagnostics consistently and cancels any active restart phase.

ACK processing excludes interrupts and rejects pending reset/error before authorising fencing. A fresh enumeration reset in READY/CONFIG_WAIT is expected; reset during ACK wait or after established DONE faults the coordinator. A wrong-phase CONFIGURED event does not revive a failed/retiring generation. Late callbacks still require full old-reference drain before software address reuse; context matching is not hardware authentication.

FAILED executes no normal USB task/attach path and does not permit another restart. Final review added explicit detachment before shutdown, clearing configuration/probe state; after any power-off attempt ordinary USB-register access is forbidden and resources remain quarantined. Failure tests cover pending RX retention and off/busy state. It is quarantine, not proof of off/idle or freed resources. An actual partial-failure recovery policy and fault-injection hardware test remain required.

## Review and tests

The first owner review timed out. A subsequent bounded source review found event-order and stale-state defects: CONFIGURED before RESET could be revived, ERROR left configuration live, and detach diagnostics were stale. Those paths were corrected with event-order/error/reset-ACK regression tests. A follow-up review attempt timed out; complete final owner approval remains open.

`make test` and `make test-sanitize` pass with sixteen strict C executables, 16 Python methods and existing source/hook/submission/rebuild harnesses. Owner mock tests cover:

- Initial detached attachment and both RESET/CONFIGURED orderings.
- Cancellation of HID/EP0 directions under the fence, no normal task during off/busy/retirement, and off-idle before rebuild.
- Two restart cycles with new callback generations; old-generation configuration rejection.
- Reset/error fault quarantine, ACK rejected after pending reset, stale-config request refusal, and recorded completed cycles.
- Probe completion consumption before retirement, separately tested in the endpoint suite.

`make usb-retirement-check` includes target compilation of the owner. Default `make bootloader` stays text26670/data80/bss1568 and excludes owner/retirement/rebuild/update execution symbols. Shared MPLAB XML lists 41 C sources.

## Enabled-link blocker

Compile/link checks retaining `USB_RestartOwnerInit/Request/Ack/Tasks` with `USB_RETIREMENT_HOOKS=1` fail at `-O2`; a separate `-Os` attempt also fails with “Not enough space in program memory”. Both use the unchanged generated bootloader linker and 32 KB reservation. The passing earlier detached-rebuild-only link does not imply this complete owner fits.

Do not increase the reservation, move `1D008000`, enable protection bits or program a partial/size-check output to hide this failure. No new HEX is attached. The accepted evidence is source/host tests and target object compilation; a deliverable enabled image has not passed link.

Duplicated ownership is now compiled out in the opt-in branch and the integrated `-Os` link passes. Next add/review the final read-only owner/transport pause command and repeat its enabled link. Only after a complete bounded image, exclusive board-task handover and host callback protocol pass can the [read-only hardware restart phase](nvm-hardware-test-plan.md) be delivered. NVM/protection and uploads remain disabled and separate.
