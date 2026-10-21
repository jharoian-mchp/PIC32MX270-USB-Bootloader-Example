# Power-enable observations — evidence-only design

This design records three fixed observations around USB power enable and the existing driver-ready check. It adds no wait, retry or change to readiness outcomes. It is not implemented, linked or approved for delivery. The recovered board stays read-only; no PAUSE, reset, reprogramming or fault test is authorised.

The latest callback-mask trial rejected USBBUSY with USBPWR set. [Reference findings](usb-powered-busy-semantics.md) supply no general powered-clear guarantee or time bound. Three observations can locate changes in the inspected sequence; they cannot establish a clearing deadline or transient-versus-permanent behavior.

## Three sample sites

Capture only when diagnostic instrumentation is enabled and the trusted owner is already in `USB_RetirementPreparing()`. Ordinary startup and hooks-off builds perform no new reads. The driver owns these observations; they are not taken by host queries.

| Site / valid bit | Exact placement and reads | Meaning |
|---|---|---|
| PRE_ENABLE / bit0 | In `DRV_USBFS_Initialize()`, after assigning the USB ID and before its existing `PLIB_USB_Enable()` call. Read CP0 Count, then U1PWRC; update evidence and commit it before entering the enable call | A pre-call observation; software off-idle was already required by preparation. The extra read grants no authority and does not re-evaluate that guard |
| POST_ENABLE / bit1 | Immediately after `PLIB_USB_Enable()` returns, before its following driver setup. Read CP0 Count, then U1PWRC; update and commit | The enable call returned before the read pair. This is not proof initialization or hardware settling completed |
| READY_POWER / bit2 | In the existing driver-ready predicate, only after its earlier software and IRQ-source checks pass. Reuse that predicate's original local `power=U1PWRC` value, then read Count | The exact U1PWRC sample used by the guard. No second power read, no early U1CON read and no changed short-circuit order |

For each site record selected USBPWR and USBBUSY bits from that site's U1PWRC value. Do not record USBEN: the existing guard reads U1CON only if BUSY is clear, and this design must not introduce an early access. The existing detail word continues to report first rejection and its original observations.

Each valid bit is set only after both reads for its site execute. Each site is latched once per rebuild attempt: PRE/POST at the first enable call in the supported single-instance profile, READY_POWER at the first readiness invocation that reaches its original power read. Once valid, a site cannot overwrite its stored sample, read Count again or trigger another power-evidence commit. Later readiness invocations still perform their unchanged original predicate reads/checks and detail recording; they may disagree with the first latched READY sample. Do not label the latch as the last/current readiness state. Additional driver instances or enable calls require a separate design; they cannot reset or overwrite these samples. Absent sites remain invalid, with zero fields; zero bits in an invalid site mean unsampled. A valid power0/busy0 sample is distinguishable from an absent sample. READY_POWER is valid even if its original sample rejects BUSY; it remains absent if an earlier readiness check rejects IRQ or another software prerequisite.

PRE_ENABLE is a diagnostic snapshot and does not authorize module enable if the state is unexpected. Existing control flow and guards remain responsible for authorization. No new USB register write, mode change, detach, source enable, admission, resource retirement or NVM action follows from these fields.

## Timing representation and perturbation

Use the existing `_CP0_GET_COUNT()` primitive; never reset Count or alter Compare. `definitions.h` sets `CPU_CLOCK_FREQUENCY=48000000U`, and existing board `TimeGet()` uses Count at CPU_CLOCK_FREQUENCY/2. This profile therefore has a nominal 24 MHz counter. Implementation must compile-check the expected clock profile and require matching host/package identification. A changed profile needs a separately defined encoding/decoder; never silently interpret ticks at the old rate.

Retain PRE_ENABLE Count as `pre_count`. Store `post_delta = (post_count - pre_count) mod 2^32` and `ready_delta = (ready_count - pre_count) mod 2^32`. These are raw ticks; hosts may show a nominal conversion explicitly labelled as such. They do not prove wall-clock elapsed time, a deadline or an upper bound. Count can wrap; multiple wraps, processor stalls/debug halt and actual clock behavior are not reconstructed from this record. Invalid sites have no interpretable delta.

Read ordering differs deliberately: PRE/POST take Count before power; READY uses the existing power read before Count. Each pair spans instructions and is not atomic. PRE-to-POST includes the pre-enable journal commit as well as the enable call. Later deltas also include intervening journal commits and normal construction. Do not label them “power settling time” or subtract presumed logging costs.

No polling, extra task pass, deliberate delay or speculative resampling is added. At most two new U1PWRC reads (PRE/POST), three new Count reads and three latched observations occur per attempt; READY reuses an original read rather than limiting how often the original predicate itself may run. These reads and journal CRC work are still perturbations: they can change the timing at which existing predicates run. A later passing guard on an instrumented image would not prove that an uninstrumented image succeeds. Separate qualification would be required.

## Owner and partial-record behavior

Reset current power evidence at a new accepted restart and at entry to its rebuild, without changing the boot-captured previous record. Use one IRQ-excluded, non-reentrant SRAM observer, analogous to the existing detail observer. It only copies evidence and journals; it never invokes USB APIs or coordinator transitions.

PRE_ENABLE is committed before the enable call. POST_ENABLE is committed after the call returns. READY_POWER is copied into current owner RAM and included in the existing readiness/detail journal commit, preserving the original rejection. Record deduplication must compare the new fields as well as detail/state; unchanged phase alone must not suppress samples. Keep the three-site summary through subsequent failure/configuration records rather than clearing it at CONFIGURED or another unrelated checkpoint.

A reset or stall before PRE leaves no valid sample. After PRE but before POST commit, the latest validated record can show PRE only; it does not establish that enable returned or even that the call started. An interrupted slot write falls back to the older validated slot. A reset between the original READY power read and its journal commit can leave only earlier samples. Missing later validity must not be promoted to complete sampling. A valid later record is historical evidence; none of these states permits recovery or restart automatically.

Use the existing boot capture to preserve a single immutable previous snapshot. Current queries must not rewrite it, clear the journal or fabricate missing sites. No automatic reset/recovery is part of the instrumentation.

## Proposed retained schema 4

Three-site evidence does not fit the schema-3 reserved bits without losing sample association or timing. Keep existing status/detail fields intact and extend the journal to two 64-byte slots (128 bytes total):

| Word | Field |
|---:|---|
| 0–9 | Existing magic, version, sequence/complement, ticket, generation, record time, packed status, checkpoint and detail; version becomes4 |
| 10 | PRE_ENABLE raw Count |
| 11 | POST_ENABLE delta ticks |
| 12 | READY_POWER delta ticks |
| 13 | Metadata in low16 bits; upper16 reserved zero |
| 14 | CRC32 over words0–13 |
| 15 | Final commit marker |

Metadata bits0–2 are sample-valid bits. Bits3–4 contain PRE USBPWR/USBBUSY, bits5–6 POST USBPWR/USBBUSY, and bits7–8 READY USBPWR/USBBUSY. Bits9–15 are reserved zero. Invalid site bits/associated Count or delta must be zero. POST or READY requires PRE; READY requires POST. Set validity by reached site only, not by a register being zero. Detail remains the original stage/rejection/observation encoding.

Reject schemas1–3; do not attempt migration or combine slots of different layouts. Reading a schema-3 record under the new layout will not preserve that old evidence reliably. The observed old failure is already documented separately. Preserve the two-slot sequence/complement, no-wrap policy and commit-last/torn-write fallback.

Changing the allocation from96 to128 bytes requires explicit retained-placement and startup-audit updates: exact expected size, NOBITS, `_persist_begin.._persist_end`, SRAM/load overlap checks and every `.dinit` clear/copy destination must be checked for the whole new span. RAM addresses can shift. A size-only source assertion does not replace the linked ELF/HEX/startup audit.

## Proposed read-only power query

Keep PREVIOUS opcode0x68/version1 and its existing30-byte response unchanged. Add diagnostic-only opcode0x69/version1 with the same exact6-byte request envelope and nonzero request ID. Tentative host command: `power-previous`. This query reports only the power summary from the same boot-captured previous record; it causes no pause, write, clear or USB reinitialization.

Proposed payload is exactly30 bytes:

| Offset | Field |
|---:|---|
| 0–7 | Existing response envelope: response opcode0xE9, version, result/reserved and request ID |
| 8–11 | ticket:u32 |
| 12–15 | generation:u32 |
| 16–19 | record time_ms:u32 |
| 20–23 | post_delta:u32 |
| 24–27 | ready_delta:u32 |
| 28–29 | metadata:u16 |

All integers are little-endian. PRE raw Count is retained for source diagnostics but is not transmitted; deltas are sufficient for the limited observation comparison. No validated record gives all-zero identity/time/deltas/metadata. A valid previous record can have no reached sample; retain its nonzero ticket/generation with metadata0. The journal reader/writer requires nonzero ticket and generation for every validated record, including records with no samples; preserve this invariant in schema4. The query emits those identities only if boot capture validated the record. The host derives record validity only when both ticket and generation are nonzero, rejects mixed zero/nonzero identities, and requires all-zero time/deltas/metadata when both are zero. A valid record may have time_ms0 and no samples. Sample validity comes solely from metadata; do not equate it with record validity.

Existing PREVIOUS does not expose journal sequence. Consequently, hosts can compare ticket/generation/record time and use both immutable snapshots from the same uninterrupted boot; they cannot perform a sequence join. Neither comparison nor matching fields prove reset-safe identity or authentication. If either query fails/mismatches, report separate evidence rather than automatically retrying or asserting correlation. A hidden intervening reset with colliding boot-scoped fields cannot be excluded by this protocol; a stronger future identity would require a separately reviewed wire change.

The fixed response envelope provides the same one-report bound as current PREVIOUS. Mechanically verify worst-case stuffing including CRC and padding; do not rely on generic30-byte payload sizing alone. Preserve the existing single-write transport, request matching, deadline checks and exact body-length validation. New opcode must remain outside active mutation/upload dispatch; older firmware does not support it and needs matching package scripts.

## Size and verification gates before implementation is accepted

The current link reports text32117/data88/bss1952. The inspected map uses31709 bytes of the32768-byte `kseg0_program_mem` region, with1059 nominal bytes unused there; this is not a predicted allowance for the new code or all load/alignment effects. No new-schema image has been built. Keep the fixed reservation and application origin0x1D008000. If instrumentation cannot fit, stop and report; do not widen Flash, drop safety checks or silently omit required evidence.

Required tests:

- Exact sample sites/default-off behavior, at most two added power reads and three added Count reads per attempt, and their ordering; no early U1CON and no Count writes, polling or interrupt-dependent clock. Repeated readiness rejection followed by readiness success cannot overwrite the first eligible READY sample or add diagnostic Count reads/commits.
- Driver/check failures before every site, interrupted enable/call paths, evidence observer ordering and journal commits from actual owning-TU function bodies.
- Valid power/busy bit combinations versus unsampled sites; delta zero/wrap vectors and invalid metadata/dependency/reserved-bit cases.
- Predicate/source comparison confirming unchanged boolean outcomes and first rejection; preserve fencing, publication, callback/attach masks and partial-failure quarantine.
- All new journal words covered by CRC; torn-slot prefixes, corruption/fallback, invalid schemas, sequence exhaustion and immutable prior capture.
- New read-only query request/result/body/metadata bounds, exact one-report stuffing, trailing padding, single send/deadlines, missing record versus missing samples (including valid nonzero identity with time0), mixed-zero identity rejection, and failed/cross-reset correlation warnings.
- Strict/sanitizer/default-off parity, generated overlay strip/reapply, XC32 compile/link and128-byte retained/dinit/ELF/HEX audit, including negative initialization-overlap regressions.

A bounded design review requested explicit sample-site validity, timing anchors/read ordering and partial/torn-record behavior; these are specified above. Earlier sketch wording incorrectly described PREVIOUS as carrying sequence; the proposed reply uses the existing exposed identity/time fields and explicitly accepts the correlation limitation. A follow-up review also required one-shot latching and an explicit nonzero-record identity invariant; both were added and confirmed closed in a bounded confirmation. Mechanical checks verified the proposed30-byte field layout, fixed-header frame examples, modulo tick vector,128-byte slot allocation and current map arithmetic. These design checks do not constitute firmware tests or a new image audit. Implementation and any package/procedure still need their own review.

Keep the board recovered/read-only with external power/settings unchanged and PICkit5 disconnected. The current image's PAUSE and recovery are spent. This design grants no code deployment, replacement package or new trial.
