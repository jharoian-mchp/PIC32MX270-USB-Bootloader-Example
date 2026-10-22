# Inactive isolated NVM owner — mock-tested implementation

`isolated_nvm.c/.h` implements the one-shot isolated test owner and one-page backend wrapper. It is not called by firmware startup, USB, application or updater code. No standalone test image, sacrificial page, programming procedure or hardware operation is approved or supplied. The current canonical diagnostic remains running/read-only; its PAUSE permission is spent.

## Library boundary

The trusted C caller supplies an explicit page, nominal24MHz Count callbacks, exclusion/preflight callbacks and an adapter initializer. The owner has no default page and the mailbox cannot select an address or operation. Initialization validates page alignment and subtraction-based upper bounds before calculating the page end. Mock addresses are test vectors only; the future target must use a compiled, separately approved page and prove code/data/image non-overlap in its final map.

`ISOLATED_NvmInit()` initializes new/quiescent owner/mailbox storage and publishes READY without invoking the adapter initializer or accessing Flash. The target lifecycle must call it once per boot; it is not a recovery/reset API. Public C structs are inspectable trusted-owner storage, not authenticated debugger inputs. Do not alias mailbox, owner, configuration, port, row or result storage; all must outlive the operation and quarantine.

`ISOLATED_NvmTasks()` is a single serialized main-owner API. Callers must already hold interrupt/bus ownership; no general thread lock is installed. `unsafe()` must report nonzero for powered/busy USB, enabled DMA, unmasked interrupts or invalid clock preflight without mutating those states. This library does not power down USB/DMA, connect a report transport or call whole-system/generated NVM startup.

The nominal timing profile is24MHz Count. Configured debounce, arm and operation budgets must be positive and fit the stated modular interval; target delivery will need exact chosen values. Software time checks cannot preempt a Flash-fetch stall or identify multiple unseen wraps, a stopped Count or debugger-induced time distortion.

## Arming and containment

Zero token is inert. The first nonzero mailbox token consumes the boot attempt before validity/preflight checks; later tokens are ignored. Valid arming requires the fixed token and a currently debounced stable release, previously observed since startup. Every sampled button transition clears debounce eligibility before token handling. Gesture history resets at reservation; then a fresh debounced press/release must complete before the arm window expires. Expiry is checked before and after exclusion checks so a stale Count sample cannot accept a late gesture.

The initializer runs only after that valid gesture, returning a complete private `UPDATE_FLASH_BACKEND`. Erase/write return acceptance, not completion: backend `busy()` must remain true through WR and settlement/disarm; false means settled. The existing `UPDATE_NVM` adapter provides this contract and is tested through a mocked hardware port.

The wrapper allows one exact-page erase and one exact-page128-byte row write after complete erased-page verification. It requires the row pointer to be `UPDATE_FLASH`'s owned aligned row copy and a separate `erase_verified` latch. Attempt counters are reserved before forwarding and never refunded. Read spans stay within the one page and verification phases. It verifies1024 erased bytes, writes `i ^ 0xA5` for128 bytes and verifies that row plus896 remaining FF bytes. No metadata, forbidden-region test, second pattern or restore operation is included.

## Timeout and settlement

The owner anchors the phase budget before a launch-capable Flash call and checks before/after the real backend callbacks and outer Flash API. Erase verification shares its erase anchor; row and final-page verification share the row anchor. A late-returned acceptance/OK cannot advance the next phase or PASS. The generic `UPDATE_FLASH` behavior is unchanged.

Failure is latched. No remaining ordinary readback, new launch, reinitialization or flash reset follows failure. If an operation attempt may have reached hardware, the owner keeps row/state allocated and runs only backend busy/settlement observation. After actual WR-clear/settle/disarm it may publish FAIL_SETTLED, never PASS or READY. An attempted backend call may be rejected without a physical WR launch; reporting distinguishes attempts from physical completion, and tests do not claim every pre-launch rejection performs no NVM register writes.

An unsettled failure can remain quarantined indefinitely. Timeout is not hardware cancellation or recovery authority. Exclusion interference invalidates the ownership claim; retained buffers and fail-closed behavior cannot undo activity already launched. No USB/DMA re-enable, error clear, reset or automatic recovery exists.

## Mailbox publication

Target payload fields are aligned volatile words; the operator owns only the token while READY. Publications bracket every payload update with odd/even sequence values and explicit `sync` barriers on XC32. A private expected sequence detects altered mailbox sequence and prevents reuse. Sequence exhaustion blocks further publications/launches without wrapping; the last stable report can remain stale, so its state is not proof the target still permits arming. Readers require matching even sequence before/after copying.

Settled terminal payloads are immutable. FAIL_UNSETTLED may transition only to FAIL_SETTLED through the same publication protocol. Snapshot consistency is not authentication. The eventual target must place mailbox/owner/row in uncached KSEG1 SRAM, verify aligned word visibility and use ordered debugger reads; object compilation does not establish final placement or debug-probe behavior.

## Verification

Passing checks:

- `make test` and `make test-sanitize`: two isolated-owner suites alongside existing strict/sanitized tests and30 Python methods.
- `make isolated-nvm-check`: strict XC32 compile-only of the uninstrumented owner, plus mocks. Object disassembly contains the intended `sync` barriers.
- `make restart-image-audit` and `bun tools/test-restart-image-audit.ts`: current canonical diagnostic still passes startup/HEX/configuration checks. Its symbol exclusion explicitly rejects isolated-owner entrypoints.

The suites exercise wrong/duplicate tokens, startup-held SW1, release bounce, stale debounce, exact deadline and a slow exclusion check crossing the deadline; page/span/row-pointer rejection; one erase/row/full-page data; late erase/write/read/settlement returns; asynchronous timeout source retention; low-voltage adapter rejection; Count wrap; sequence exhaustion/corruption and snapshots interrupted after every field. Test hooks are inserted only into a copy of the actual source for deterministic single-thread interleavings; they do not create production hook APIs or make a C-thread-safety claim.

The adapter suite links the actual `update_nvm.c` and `update_flash.c` behind mocked hardware operations. It verifies pending WR ownership and conservative settlement/disarm after timeout, without subsequent readback or relaunch. No NVM register or HID operation is performed on the board by any test.

Canonical/recovery HEX hashes remain `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7` and `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`. No isolated sources are added to active firmware source lists or generated project configuration.

## Review status and remaining gates

A narrow implementation review found the current-release debounce gap; it was fixed and an actual-code follow-up confirmed that transition invalidation and stable-release reservation close it. A bounded timeout invariant review accepted post-call expiry and required a clear backend completion contract; the busy-through-settlement contract and actual-adapter tests address that point. Broader implementation and timeout/snapshot review attempts timed out and are not counted as completed approval. This stage is tested inactive library work, not exhaustive implementation review or hardware qualification.

Next complete a bounded full implementation review, including trusted-input/alias contracts, snapshot exhaustion, attempt reporting and exclusion at every callback boundary. Then separately design a minimal target port/startup and require a final linked-image/row/mailbox/whitelist/unlock audit before any delivery. No default address, standalone linker/image or on-board arm command is provided now.

Exact page loss, image/hash, measured clock/VDD/configuration, debugger running/arming method, PICkit Hold In Reset behavior and recovery scope require separate operator approval. The A1 powered-BUSY concern remains open; USB-off NVM ownership/protection and connected writes are unqualified. Keep the board running/read-only with PICkit5 disconnected and external power/settings unchanged. No PAUSE, reset, programmer reconnection, flash write, protection change, upload or fault test is authorised.
