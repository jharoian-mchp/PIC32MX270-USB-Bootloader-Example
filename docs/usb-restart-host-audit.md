# USB restart host verifier and linked-image audit

`host/usb_restart_diag.py` implements a diagnostic STATUS/PAUSE/reconnect verifier, and `tools/audit-restart-image.ts` checks the exact linked opt-in ELF/HEX. **No board image is approved or attached in this stage.** The final independent image/owner review and controlled programming/test procedure are still open; flash updates remain disabled.

## Host rules

The diagnostic transport allows only `67 STATUS` and generation-bound `66 PAUSE`. It is separate from `pic32_hid.py transact`, whose version-1 read-only/update-denial guard remains unchanged. PAUSE deliberately disconnects USB; it does not reset the MCU or erase/program flash.

STATUS discovers ticket/generation/completed cycles/state. A restart test requires exactly one matching board for the whole cycle, even with explicit `--path-hex`. No serial identity is assumed. The host sends PAUSE **once**, using an explicit request ID and the reported generation. Existing firmware rejects stale IDs; the user must not reuse or automatically increment/retry an uncertain pause to force a result.

Successful verification requires:

- Ready or DONE before PAUSE, with no exhausted counters.
- If the PAUSE reply arrives, expected ticket+1/current generation/current cycles/ACK_WAIT.
- Observed removal from HID enumeration, followed by exactly one interface with the identical path.
- Reconnect STATUS reporting ticket+1, generation+1, completed cycles+1 and DONE.
- Any changed/ambiguous path, wrong identity/state, missing removal, open/status failure or deadline overrun stops without retransmission.

A lost PAUSE response is outcome unknown; the verifier may observe removal and query status, but never resend PAUSE. It reports that uncertainty alongside any verified post-state. Same-path counters are controlled-test evidence, not a boot-unique epoch, authentication or reset-safe update resume. A complete MCU reset restarts these counters; do not run this verifier while deliberately resetting or switching boards and claim continuity.

Reconnection time caps the PAUSE transaction and post-open status exchange; blocking OS HID/open/enumeration calls cannot be forcibly cancelled by a Python elapsed-time check. The code rejects deadline overrun when it regains control. No background worker/cancellation guarantee is claimed.

Examples for the **future approved opt-in test firmware**, not the current hardware-tested image:

```powershell
py -3 host\usb_restart_diag.py status
# Do not run until an approved test image and procedure are supplied:
py -3 host\usb_restart_diag.py restart --request-id 100 --confirm-usb-disconnect
```

The explicit confirm flag and request ID are required before sending a pause; status alone is non-destructive. Current CAPABILITIES-only firmware times out on both commands. This stage requests no reprogramming or live PAUSE test.

Host tests simulate success, lost ACK, changed path/reset-like state, multiple boards, no observed removal, closed handles and the single-send allowlist/deadline. Independent bounded review identified a PAUSE/reconnect timeout mismatch, fixed by capping remaining deadlines. Its explicit-path observation was resolved by documenting the intentional single-board policy. Final combined review timed out; no final hardware-delivery approval is claimed.

## Linked ELF/HEX audit

```sh
make restart-image-audit
bun tools/test-restart-image-audit.ts
```

The build target compiles the default diagnostic and opt-in `-Os -flto` image with the original 32KB linker. The audit records SHA-256 hashes of both opt-in artifacts and validates:

- ELF32 little-endian MIPS format and bounded program/section headers.
- Every delivered HEX byte belongs to a loadable ELF Flash segment and matches its byte; every loadable Flash section has delivered bytes. Segment alignment holes may be omitted by XC32 HEX.
- Program Flash load spans remain below physical `1D008000`; config words remain `1FC00BF0..1FC00C00`. RAM loads are within 64KB KSEG1 and initialize through Flash `.dinit`, not burn-address PA0.
- Boot load spans remain within the narrowly allowed reset/exception range. XC32 emits a loadable `._debug_exception` return stub at `BFC00480`, extending to `1FC004AC` past the nominal boot-memory length490. Its tail is allowed only with the expected `__DbgExecReturn` symbol and byte-for-byte match to the default local build. It is not a general allowance for new Boot Flash code.
- Four configuration words are identical to the default local build; PWP/BWP remain OFF. Local ELF/HEX values are **not actual device readback**. Screenshot debug-config choices and production build config can differ; compare final tool readback at the controlled programming stage.
- Pause owner request/ACK symbols are linked, while update/session/lifecycle/flash backend, NVM erase/row/word write and application-jump entrypoints are absent.

Symbol absence alone cannot prove arbitrary instruction safety after LTO; the static source/command whitelist review remains necessary. The audit validates artifact boundaries and build evidence, not MCU peripheral behaviour, power-loss recovery or all inlined instruction effects.

Negative tests mutate HEX data with a valid new checksum, remove a delivered record and corrupt ELF format; each fails the audit. The script also handles XC32 RAM program headers and omitted alignment gaps explicitly rather than broadening delivered Flash ranges to accept them.

## Verification and delivery gate

`make test`/`make test-sanitize` pass nineteen C executables, 19 Python methods and the generated-source/hook/submission/rebuild harnesses. `make usb-retirement-check` and `make bootloader-restart-check` pass. Default firmware keeps its previous footprint and CAPABILITIES behaviour. Linked-image audit and negative tests pass; the resulting audit JSON records `hardware_qualified=false` and `board_delivery_approved=false`.

Final independent host/image review timed out. Next complete the final static owner/image review (including LTO limitations), then supply a hashed test HEX and explicit Windows/PICkit procedure that preserves recovery and documents unchanged protection/flash layout. Run only the controlled read-only restart test first. NVM/protection qualification, application regeneration/relocation and uploads remain later gates. No HEX is delivered and no programming is requested now.
