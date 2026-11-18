# Customer upload review — 18 November 2026

The current stock H3 USB HID upload path has concrete blockers. Do not install it for an application-update test yet. Independent read-only review and parent source checks agree on the findings below; no new firmware build or board operation occurred.

## Findings in active customer sources

Paths are relative to `customer-demo/bootloader/src/config/default/bootloader/`.

- `bootloader_usb_device_hid.c:220–229`: DLE consumes the next input byte immediately. A valid escaped frame whose DLE is the last byte of a64-byte HID receive reads stale inputBuff[64] and loses escape state across reports. This can affect valid uploads, beyond malformed-input robustness.
- `datastream/datastream_usb_device_hid.c:112–120,181–192`: reset/deconfiguration clears only deviceConfigured. An outstanding readRequest can survive cancellation without a matching DataReceived event and prevent receive resubmission after configuration. Exact real-stack cancellation/event behaviour needs a regression before any fix is accepted.
- `bootloader_nvm_interface.c:88–106`: NVM_PageErase and NVM_RowWrite return values are discarded; helpers wait for busy to clear but do not publish failure. The command handler can send success after a failed erase/write. Actual hardware failure behaviour is untested.
- `bootloader_nvm_interface.c:150–165,185–187`: subtracts record length before validating remaining bytes and treats exclusive FLASH_END_ADDRESS as inclusive. Out-of-range/truncated input can escape application bounds. Frame cmdBuff accumulation also has no bound in the parser. Existing [generated-path audit](update-safety.md) records corresponding host reproductions; its passing test means defects were reproduced.

The fixed erase command's normal caller uses APP_START_ADDRESS through exclusive FLASH_END_ADDRESS and helper iteration is `< endAddr`; no normal fixed-command erase overrun was found. Ordered in-range HEX EOF flushes the final partial row; no ordinary partial-row loss was confirmed in this review. These narrower findings do not excuse the error/bounds defects.

## Startup and handoff observations

The linked application reset/startup is9D008000/9D008008 and its exception page remains9D03F000. Its first PT_LOAD begins at9D000000 and includes a padding gap before the reset section. Actual allocated flash sections and HEX contain no application bytes below1D008000, as the structural audit verified. This distinction requires loader-aware review; HEX bounds do not prove every possible ELF loader respects sparse section intent.

Stock JMP_TO_APP waits for its response then triggers reset. Startup samples SW1 and uses run_Application if entry is non-erased. The first-word check is not whole-image validity. SYS_DeInitialize is empty; startup handoff happens before USB/I2C/ADC initialisation, which narrows initial handoff effects, but reset-state/executive/runtime and repeated-handoff behaviour need target evidence. No arbitrary live jump after update is authorised.

The board attach gate has one owner in the datastream and is dependent on mux readback/ADC qualification plus power detection. Host mocks pass; real callback timing, USB reset ownership and cable sensing remain untested.

## Transport decision

John previously authorised a move to stock H3 UART if HID causes blockers, with the32KB application offset fixed. The valid-frame escape and reset-ownership bugs satisfy that trigger for proposing the fallback. Next inspect the actual H3 UART implementation and compatible host before changing the project. UART does not automatically eliminate NVM error reporting, HEX/range validation or handoff concerns; test these in its selected implementation.

Do not import the paused customv2/updater/restart architecture or begin extensive HID repairs without a separate scope decision. Preserve the HID work for reference and use separate UART project paths if selected. UART pin routing/voltage/host checks and hardware approval remain required. No programming, flash command or board change is included in this review.
