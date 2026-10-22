# Power observations: source-tested temporary debug layout

The three-site evidence-only instrumentation is implemented and passes host tests, sanitizers, enabled XC32 compilation and the temporary 48 KB image audit. It adds no BUSY wait or readiness correction. The normal 32 KB firmware and application origin remain unchanged. No replacement package, programming or new PAUSE is authorised by this source stage.

The user permitted exceeding 32 KB temporarily while diagnosing USB. The first instrumented 32 KB link failed; the new diagnostic target uses a separate generated linker file rather than changing the canonical reservation. The board still runs the recovered `a57fef3` image, whose retained first rejection was USBBUSY1024/observations3. Actual USB restart remains unqualified.

## Sampling and owner recording

The implementation follows the [reviewed design](usb-power-observation-design.md):

- PRE_ENABLE: Count then U1PWRC, inside driver initialization immediately before the existing power-enable call. The observer commits before entering that call.
- POST_ENABLE: Count then U1PWRC immediately after power-enable returns, before following initialization. The observer commits the new sample.
- READY_POWER: reuses the original driver's `power=U1PWRC` sample after its existing software/IRQ-source checks, then reads Count. The owner copies the summary without committing until the original detail callback records the check's outcome.

Each site latches once per attempt. Only a preparing, diagnostic-enabled stack samples. PRE/POST add at most two power reads; READY adds no power read. At most three Count reads are added. Normal startup and default-off firmware add no diagnostic reads. Compile-time checks require the supported single USB instance and 48 MHz CPU profile, yielding nominal24MHz Count. Count is never written.

Current evidence resets on an accepted request and entry to rebuild, not ordinary checkpoint changes. It stays with subsequent failure/configuration records. Previous evidence is captured at startup and remains immutable. Sample fields participate in record deduplication, so an unchanged phase cannot suppress PRE/POST changes.

Readiness predicates and their order are unchanged: IRQ-source, BUSY, then USBEN and queues/BDT. No observation authorises admission, writes USB registers, changes the NVM gate, or adds a retry. READY retains the first eligible sample even if later existing checks disagree. A missing sample means it was not committed, not that the hardware passed. A PRE-only record cannot prove enable started or returned.

Deltas use unsigned subtraction from PRE Count modulo2^32. Reads are non-atomic; deltas include pre/post journal work and normal construction. They are raw ticks, not settling time, wall time or deadline proof. Multiple wraps/debug stalls/clock variation are not reconstructed. Instrumentation perturbs timing, so an eventual instrumented success would not qualify an uninstrumented image.

## Journal and wire

Schema4 uses two64-byte slots,128 bytes total. Words0–9 retain status/checkpoint/detail, words10–12 contain PRE Count/POST delta/READY delta, word13 metadata, word14 CRC over words0–13 and word15 commit. Commit invalidation and final commit each have compiler ordering barriers; slot stores are volatile. Older schemas are rejected, with no migration.

Metadata bits0–2 indicate reached sites; power/BUSY pairs occupy bits3–8. Reserved bits, invalid-site data and invalid dependencies are rejected. The journal also rejects reserved wait-evidence bits outside0–4. Nonzero ticket/generation remain required, including valid records with time0 or no samples.

PREVIOUS0x68 keeps its30-byte reply. New read-only opcode0x69/version1/exact6-byte request returns a30-byte payload: envelope8, ticket/generation/time3u32, POST/READY deltas2u32, metadatau16. Python command `power-previous` sends one query and validates length, identity, metadata, deadlines and padding. No record returns zeros; valid identity with no samples is distinct. PRE Count is retained but not transmitted.

PREVIOUS additionally rejects reserved evidence bits and nonzero stale fields when valid=false. This closes a pre-existing parser gap without changing its valid wire format. Pairing PREVIOUS and power-previous can compare boot-captured ticket/generation/time only: sequence is not transmitted, and an intervening reset with colliding fields cannot be excluded. Neither query supplies restart authority or reset-safe authentication.

## Temporary 48 KB build profile

Use `make power-debug-image-audit` for source/build verification. It builds the normal recovery image, then generates `build/power-debug/btl-debug.ld` from the canonical linker, changing only the program reservation to49152 bytes for this target. Compile flags require retirement hooks, restart diagnostics and `USB_RESTART_DEBUG_LAYOUT=1`.

| Item | Normal default | Temporary power debug |
|---|---|---|
| Linker reservation | 32768 bytes | 49152 bytes |
| Reported application origin | `0x1D008000` | `0x1D00C000` |
| ELF text/data/bss | 26670 / 80 / 1568 | 33241 / 88 / 2052 |
| Journal | Inactive |128-byte retained NOBITS at `0xA0000000` |
| Host capabilities probe | Normal `probe` | Explicit `probe --power-debug` |

At the implementation-stage cutoff the inspected debug map used32833 bytes of49152 and the enabled canonical32KB link failed. The subsequent [32KB restoration](usb-restart-32kb-restoration.md) adds an opt-in data-section build flag with runtime sources unchanged; canonical `make restart-image-audit`/`bootloader-restart-check` now pass. The historical debug target remains separate and unchanged.

The debug profile changes both internal APP_START/APP_JUMP constants and reported capabilities consistently. Application handoff/update/NVM-write entrypoints remain excluded. The normal Python offline planner still uses origin1D008000; `--power-debug` affects only the read-only capabilities probe. Normal probing rejects the expanded layout, and debug probing rejects a normal image.

**Programming this debug image may overwrite existing application flash in `0x1D008000..0x1D00C000`.** Any future delivery must make that temporary layout explicit and obtain separate programmer approval, with preservation/backup of existing application data as needed. No application relocation, protection change or board write has happened in this task. The normal application reservation and standalone LED baseline are unchanged.

The audit accepts the larger span only with explicit `--power-debug`, reports48KB/origin1D00C000 and the application overlap warning, and retains `hardware_qualified=false`/`board_delivery_approved=false`. Configuration words match the normal local image. It requires the entire128-byte journal in retained NOBITS SRAM and rejects every linked `.dinit` destination overlapping that span. The normal32KB audit rejects the expanded candidate.

## Verification and review

Passing checks:

- `make test`, `make test-sanitize`: 30 Python methods and strict/sanitized C/mock suites, including explicit normal/debug capabilities tests.
- `make usb-retirement-check`: checked overlays and enabled XC32 compilation.
- `make power-debug-image-audit`: separate expanded link and ELF/HEX/configuration/retained-startup checks.
- `bun tools/test-restart-image-audit.ts --power-debug`: corruption/missing bytes/unsafe headers and undefined-symbol regressions, `.dinit` overlap through the final32 bytes of the journal, and expanded-image rejection under normal audit.
- Disposable five-file overlay base-strip/reapply/check round-trip; the original MCC base hashes are unchanged.
- `git diff --check`; normal recovery HEX SHA256 remains `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`.

Tests verify PRE/POST commits, READY RAM-only copy then detail commit, one-shot behavior across repeated readiness checks, Count wrap, unchanged BUSY/USBEN/IRQ rejection, all65536 metadata values, CRC coverage of all new words, older-schema rejection, invalid metadata/evidence, sequence exhaustion, empty snapshots, fixed-header one-report framing and read-only transport.

The delegated host task timed out after editing its files; its output was inspected and tested locally. Completed implementation reviews found the PREVIOUS/reserved-evidence validation gaps, now fixed; an explicit post-invalidation barrier was added to make the already volatile ordering clear. A follow-up review confirmed those fixes and found no additional must-fix within supplied files. One broader final review timed out; it is not counted as approval. Hardware accesses and Count behavior are mocked in host suites.

The failed32KB build left an older local HEX/audit beside its failed map. Those stale build files were removed to prevent accidental identity confusion; existing delivered archives and user data were preserved. Transient implementation and round-trip scripts were removed.

## Current board boundary

Keep the board recovered/read-only with external power/settings unchanged and PICkit5 disconnected. Its previous PAUSE and recovery permissions are spent. This implementation is verified source, not a delivered or hardware-qualified image. The subsequent [debug candidate review](usb-power-debug-candidate-review.md) passes its bounded offline checks and defines the overlap/preservation gate. No package or board permission follows from it. Explicit overlap-aware programming approval and initial checks are required before any later PAUSE decision. No uploads, NVM/protection changes or fault tests are authorised.
