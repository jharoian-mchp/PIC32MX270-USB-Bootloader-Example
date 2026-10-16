# USB restart ownership audit and inactive coordinator

The generated stack does not expose a proven safe full restart sequence for queued HID IRPs. An inactive, **read-only** `usb_restart.c/.h` coordinator now tests the required order through mock ports. **No target port, new HID command, hardware restart or NVM operation is connected.** The working CAPABILITIES endpoint remains unchanged.

## Source trace

The installed/generated code was inspected directly; the HID/device-layer trace also received a bounded delegated source review.

| Source | Relevant behaviour | Restart constraint |
|---|---|---|
| `usb_device_hid.c` static IRP pool, HID instance and common mutex state | IRPs retain application-buffer pointers; pool entries are reusable only after completion status; common mutex has a one-time initialisation guard | Reinitialising the device layer does not recreate or clear these resources |
| HID descriptor initialisation | Resets endpoint/interface state and direction queue counters | Late completion can affect a newly reset queue counter; drain before new descriptor/generation reuse |
| HID function `deInitialize` | Requests endpoint cancel-all/disable and clears flags | No independent wait for callback/pool/counter drain; callback/context and pool state persist |
| `USB_DEVICE_Deinitialize` | Marks device instance status uninitialised | Does not itself cancel all function drivers, close the controller or retire HID state |
| `USB_DEVICE_Close` | Marks client closed and clears client use | Does not independently stop controller activity or retire queued buffers |
| Device bus-reset/configuration-zero paths | Call function-driver deinitialisers; reconfiguration initialises functions by descriptors | A normal reset path supplies cleanup work, but cannot be repurposed blindly as a software full restart |
| `DRV_USBFS_Close` | Clears `isOpened` and driver callback | No all-IRP/drained guarantee |
| `DRV_USBFS_Deinitialize` | Clears instance/open/callback state, deletes mutex, disables/clears interrupt, disables module power | Resource deletion precedes off/idle confirmation; cannot directly serve as a safe combined power-off/retire port |
| `DRV_USBFS_Initialize` | Claims instance, creates mutex, enables module, programs BDT/mode | Require old instance/IRP/buffer lifetime resolved first; failed init is not a transparent recovery |
| `IRPCancel` / `IRPCancelAll` | Head cancellation can wait for ISR; cancel-all clears endpoint BDT state and flushes queues/callbacks | Completion/cancel ownership must be recorded without recycling buffers; endpoint clearing alone is not USB bus-idle proof |
| `USB_DEVICE_Tasks` initial open | Opens controller exclusively and nonblocking then runs device tasks | Old controller handle and task state must be retired/recreated coherently |

Files are under `bootloader/src/config/default/usb/src/` and `driver/usb/usbfs/src/`. The module power template implements disable by clearing USBPWR; it does not establish a software timeout/USBBUSY wait.

`usb_probe.c` keeps persistent aligned RX/TX buffers and records callback completion before main mutates them. This handles the tested reset/cancel path; it does not establish a full driver/HID restart. `BOARD_USB_DIAG_Tasks` normally continues USB tasks/attach handling, so a future restart owner must control that path too. Reconnect must never call `SYS_Initialize`, which also touches clocks, GPIO, generated NVM initialisation and boot/handoff state.

## Coordinator port contract

```text
READY -> ACK_WAIT -> DRAIN -> OFF_WAIT -> RETIRE -> INIT -> CONFIG_WAIT -> DONE
                       any error/expiry/unexpected event -> FAILED
```

`USB_RestartRequest` returns a nonzero monotonic ticket and causes no hardware effects. Only timely successful **local full-response transfer completion** for that ticket authorises teardown. Wrong/old/duplicate acknowledgement is ignored; failed/late acknowledgement enters FAILED. Request ticket output overlap is rejected before any counter write. Ticket/generation wrapping is forbidden.

Ports must implement these guarantees; **none currently has a real target binding**:

1. `fence_submissions`: atomically fence **every** submission path, including HID, EP0/control, tasks and ISR, for the old generation. Keep the fence until retirement and USB-only initialisation replaces that generation. Do not block the completion processing required to release existing transfers.
2. `cancel_and_detach`: request cancellation/detach under reviewed ownership. This is not proof of drain. It may invoke synchronous callbacks; they must record completion without buffer reuse/reinitialisation.
3. `transfers_drained`: prove all old IRPs/callback lifetime/queue references, including EP0 and HID, are released while the submission fence remains in force. It is not just “our two probe handles completed”.
4. `mask_and_power_off`: mask task/interrupt ownership and disable module without deleting buffers, queue storage or mutexes. The existing `DRV_USBFS_Deinitialize` combines these actions in an unsuitable order and cannot simply be called here.
5. `module_off_idle`: read U1PWRC and confirm USBPWR=0/USBBUSY=0. During OFF_WAIT the coordinator calls only this status port. Other USB register status/writes are invalid while disabled-but-busy; no resource retirement occurs then.
6. `retire_resources`: with old submissions fenced, all transfers drained and module off/idle, safely close/deinitialise/reset the selected driver/device/HID resources. Audit persistent mutex/pool/callback state and exact ordering; no broad memset or blind generated API chain is implemented.
7. `initialize_usb_only`: establish a new USB-only generation after prior retirement, without whole-system/NVM/session reset. Preserve any separate update session/storage. Port failure is quarantined; no automatic second init is attempted.
8. `configured(generation)`: accept only the new generation's configuration evidence. Raw generation matching alone cannot prove old callback lifetime safe; old references were already required to drain before reuse.

The owner remains single-task and must fence unsolicited cable/reset/reconfiguration paths. Port methods must not reenter or mutate the coordinator. These checks are contracts over observations; passing a mock does not produce real bus quiescence or a driver lock.

FAILED retains coordinator and external resource storage; it forbids further coordinator actions/retries. It does **not** mean USB is powered off, every IRP was cancelled, or an allocation may be freed. Failures after power-off leave old storage retained; failures during/after init can leave powered/partially initialised resources. A separately reviewed recovery/reset path is required. Unexpected events after DONE also enter FAILED, preventing a new restart on faulted state.

## Review findings and fixes

Independent bounded coordinator review found:

- The original “stop new reports” contract missed EP0/other submission races between drain and power-off. It now requires the atomic all-path submission fence retained through old-generation retirement, and mocks assert that fence in drain/off/retire.
- Cancel originally ignored DONE despite its documented unexpected-fault behaviour. It now quarantines DONE too; a regression verifies a faulted DONE cannot start another restart.

Follow-up bounded review confirmed those specific source/contract fixes. The delegated stack trace and coordinator review establish no real target port or hardware restart qualification.

## Verification (2026-10-15)

`make test` runs fourteen strict C executables, 16 Python methods and Bun checker self-tests. `make test-sanitize` runs the same mocked C code with ASan/UBSan. Restart tests verify:

- ACK-before-submission-fence/cancel, complete drain before power-off, off/idle before retirement, retirement before new generation/init/configuration.
- All-path fence lifetime, pending-buffer retention, old/wrong ACK and configuration generation rejection, repeated cycles.
- Failed/late ACK, elapsed-time wrap, drain/off/config timeout, each mutating port failure, output alias rejection and counter exhaustion.
- No hidden retirement/init retry after failure, including unexpected fault after DONE.

Strict XC32 v6.00 coordinator compilation, eight-mode NVM unlock checks and diagnostic `make bootloader` pass. Shared MPLAB XML lists 38 C sources; diagnostic remains text26670/data80/bss1568. `USB_Restart` and inactive update execution symbols are absent from its linked image. No hardware programming was performed.

## Next target work

The source audit is complete for the traced paths; target teardown is not implemented. Next define how to fence/drain all HID and EP0 state, split power-off from resource retirement safely, and reset/recreate persistent HID/device resources without affecting update storage. Independently review those target ports, then connect a **read-only restart diagnostic** and run phase B of [the hardware test plan](nvm-hardware-test-plan.md). Only later phases permit separately approved isolated NVM/protection tests. Uploads remain disabled and no reprogramming is requested for this inactive coordinator.
