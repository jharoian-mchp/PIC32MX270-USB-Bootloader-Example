# RAM-only candidate disposition — ready for staged planning

The existing RAM-only candidate has sufficient bounded application evidence to prepare a staged MPLAB X/PICkit5 rehearsal proposal. No concrete must-fix application defect was found in the scoped serialized entry/challenge/terminal path. This closes that planning gate; it does not certify all-fault safety, authenticate missing build inputs, approve delivery or authorise a debugger session.

The user acknowledged closing concrete test gates on2026-10-23. This milestone is read-only review and documentation. No new audit framework, target/parser change, build, relink, image delivery, programming, reset, PICkit connection, settings change or RAM operation was performed. The prior one-shot link permission remains spent. The board stays on canonical `09da506`, read-only, with PICkit5 disconnected and external power/settings unchanged.

## Candidate identity

This disposition concerns only the existing local RAM rehearsal:

| Artifact | SHA-256 |
|---|---|
| ELF | `cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a` |
| HEX | `918d3cab3801a75860a17b2adfd3cd03590299886db5add8e600d1796670ce33` |
| Native map | `f6dc317709d4299e431f32f03575f1271f64f4523a8754bdacd0032b7eed4eec` |

It contains the RAM rehearsal application and runtime, with no NVM owner/adapter/test page. Installing it would nevertheless program Flash/configuration/Boot Flash through the tool; those installation effects require separate approval. Compiler HEX alone does not describe everything a debug tool installs.

## Application findings and limited dispositions

| Concern | Existing evidence | Disposition for RAM-only planning |
|---|---|---|
| One-shot lifecycle | Main masks IRQ once, checks IE, calls Init once and polls Tasks forever; first nonzero edit is spent before validation | No identified blocker in the scoped entry path. Init itself is a trusted once-per-boot API, not protected against arbitrary caller reinitialization |
| Containment | Fixed200-byte context/148-byte mailbox; guarded report, alignment/overlap checks, bounded copy/clear callers and declared KSEG1/debug separation | Adequate to plan the RAM challenge under valid storage/ABI assumptions; no arbitrary-pointer or fault-containment claim |
| Terminal handling | No ordinary re-edit/retry/report publication after terminal; only sequence-marker invalidation; denial is sticky | No identified blocker in the reviewed serialized path; debugger writes must remain restricted to the edit word |
| Breakpoint rendezvous | Kept inert return/nop blocks; pre-edit/readout eligibility and one-attempt guards reviewed | Adequate locations for a proposed method. Actual hardware halt/resume PC and executive effects are host evidence still needed |
| CRT trap | Retained `sdbbp` is called by the post-main return loop; ordinary main does not return, standard application exception paths stop | Conditional normal-path unreachability is sufficient for preparing this RAM-only experiment. It is not all-fault unreachability or an exemption for future NVM code |
| Other startup effects | Exact clear records/table, empty constructor path, enumerated pre-main CP0 setup and bypassed BMX stores reviewed | No newly identified normal-path blocker. Runtime/exception behavior outside the reviewed model stays a stated limitation |
| Application frames | Reviewed scoped maximum240 bytes from incoming `_main_entry` SP; explicit heap/stack symbols and object ranges | Supports planning, not installed-executive or worst-case asynchronous stack capacity |
| Historical build provenance | Eight input/four object recorded hashes match; system-header closure and three compiler temporary objects were not retained | Historical reproduction remains incomplete. Do not recover it by inventing inputs or require another build solely to fill old records; the proposed method must name this residual risk |
| Executable/structural checks | 909 linked words/115 stores/233 controls enumerated; native structural tests pass | Concrete bounded evidence. Larger independent reviews timed out; no independent whole-program proof or automated safety pass exists |

The scoped independent invariant review found no concrete must-fix bug and supports preparing a staged RAM-method plan. Earlier completed reviews cover runtime/trap, publication and caller-buffer/frame slices. Parent review supplies the broader enumeration. Those scopes and timeouts remain recorded in the [coverage report](nvm-debugger-instruction-coverage-review.md).

No additional general audit layer is required merely to make a RAM-only proposal. A new concrete contradictory finding would reopen this disposition. Existing parser `application_safety_status=unsupported` stays unchanged; this document is a limited planning decision, not a hidden safety flag or an unsigned approval certificate.

## Concrete next gates

### 0. Offline host workflow and settings capture

First establish how MPLAB Xv6.35 will use the exact prebuilt ELF, preserve its symbols/addresses/linker placement and install the appropriate PICkit5 executive. The repository has no delivered rehearsal IDE project. Do not assume opening the diagnostic project and pressing Debug will run this candidate: that can rebuild or program a different image.

Needed offline information:

- Available prebuilt/loadable debug-project workflow and any requirement to rebuild locally; a rebuilt artifact would need separate identity/review and link approval.
- Selected device, XC32/DFP/tool-pack versions and the effective Debug startup/reset settings, not just `Use system settings` in the saved XML.
- Hold In Reset, external-power versus tool-power, software-breakpoint and peripheral-freeze choices, and available programming/erase/preserve-memory options.
- Whether the debugger UI can address the candidate's SRAM symbols/uncached address and edit a single32-bit word while halted.

Use installed IDE help or offline dialogs/screenshots with the programmer still disconnected. Do not press Debug/Run/Program/Read/Erase, alter project/settings, attach PICkit or interrogate the board to collect this information under the current scope. Avoid uploading secrets or unrelated project data.

IPE's observed Hold In Reset issue does not establish MPLAB X behavior. The actual IDE workflow must resolve that distinction rather than inherit the earlier diagnosis.

### 1. Separately approved installation and pre-edit inspection

After the offline workflow is established, prepare the exact package/procedure and ask explicitly for its delivery, installation/erase/configuration scope, tool connection, initial debug halt, limited SRAM inspection and recovery envelope. No such package or approval is supplied here. Preserve the tested diagnostic/recovery artifacts and state precisely which programmed contents may be lost.

The proposed first target session stops at the pre-edit rendezvous with edit word0; it performs no challenge write/resume. Collect logs, candidate/programmed identity, expected PC, guards/sequence/WAIT_EDIT and only the specified SRAM evidence. Configuration/executive installation may differ from compiler bytes; capture the tool/version/reservations and available installation evidence without treating vendor reservations as inspected executive contents.

Some executive behavior can only be observed after installation. Do not require nonexistent pre-session measurements to close it, or silently waive the uncertainty: identify the available offline evidence and explicitly disclose remaining executive/setup risks in the separately approved installation-only experiment. Missing-byte evidence remains unknown if the tool cannot provide it. Pre-edit success alone approves no challenge or NVM test.

If the halt, identity, memory layout or profile evidence is wrong, stop/report. Do not patch state/registers, resend, rebuild, reset or recover unless that precise action was separately authorised. Ending a debug session can alter execution/reset state and must be included in the later procedure rather than assumed inert.

### 2. Separately approved one RAM challenge

Only after reviewing the installation/pre-edit results, ask for one permitted RAM1 edit, one free-running continue and one readout inspection under a fixed procedure. It must define the exact edit word/width/address, completed readback, breakpoint removal/selection, guard/sequence checks, expected readout PC/result and stop-on-uncertainty behavior. No GUI commands or executable host instructions are provided now.

This stage measures the exact tool/application halt-return and RAM visibility behavior. It cannot certify every installed-executive instruction, arbitrary transient interference or NVM timing. No live peripheral watches, software traps, target function calls, PC jumps, automatic retries or implicit reset/recovery are permitted by a future RAM challenge approval.

### 3. Actual isolated NVM image and one-page transaction

After successful RAM-method evidence, bind the existing owner/adapter to an independently reviewed isolated NVM entrypoint. Resolve its strict DEBUG-OFF versus debug-profile difference explicitly; do not transplant the RAM acceptance policy as flash authority. The final linked NVM path/row placement/unlock/settlement and no-halt interval need their own review.

Select one exact operator-approved disposable1024-byte page, excluded from bootloader/configuration/metadata/test code and any content requiring preservation. Prepare one erase/verify, one128-byte pattern row/write and full-page verification, with USBPWR0/USBBUSY0, DMA off and IRQ exclusion throughout. Page loss, clock/VDD/configuration evidence, image installation, one transaction and recovery are separate approvals. RAM rehearsal success qualifies none of those by itself.

## Current permission and verification

Only the offline disposition/planning work is complete. No image is offered for installation, no debugger session is approved and the board remains unchanged. Missing historical provenance, installed executive and hardware-method evidence are retained honestly; complete automated application safety and NVM qualification remain unsupported.

Native44-case and original strict46-case regressions repass, including read-only native inspection of the unchanged artifact. Exact ELF/HEX/map and canonical/recovery HEX hashes match earlier records. The scoped invariant review and a completed bounded disposition review find no concrete must-fix defect or stage/permission contradiction within their stated scope. Document links/diff checks pass. This is planning evidence only; no full-image safety or hardware approval is issued. No compiler or linker runs in this milestone.
