# RAM-only package delivery verification — 2026-10-23

The approved file-delivery package `ram-rehearsal-cb89e7db.zip` is verified and contains the exact existing ELF, matching map, reference source/evidence, two recovery images and a gated installation/pre-edit proposal. No target image was rebuilt or relinked. Package approval covers file delivery only; no programmer connection, installation, debug/reset, target RAM access, recovery or NVM action is authorised.

## Delivered archive identity

- Workspace export: `/workspace/exports/ram-rehearsal-cb89e7db.zip`.
- Size:687,933 bytes.
- SHA-256: `e4f03157dfef7cd5895609746683e07ace27dd0b81d8375454f74e7a70749c3e`.
- Inventory:23 regular file entries;22 files covered by `SHA256SUMS.txt`, which excludes only itself.
- Candidate ELF SHA-256: `cb89e7dbdbed7a084826e44cc7ddc3842ca4b40cdb83b827a93c9c46dcbe5b3a`.
- Matching map SHA-256: `f6dc317709d4299e431f32f03575f1271f64f4523a8754bdacd0032b7eed4eec`.
- Bundled canonical restoration/probe recovery SHA-256: `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7` / `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`.

Compare the downloaded ZIP against the independently posted chat hash before extraction. Internal manifest/checksum consistency alone does not establish archive identity. The package's native structural result leaves application safety unsupported and hardware/delivery approval false; the later operator message authorises file delivery only and does not alter those recorded audit limits.

## Checks completed

- The packaging source hashes the existing ELF/map/recovery images against fixed identities and copies them without invoking a target compiler/linker/HEX converter.
- Every bundled application source/header matches exactly one original attempt input/hash, and its bytes match the declared original source revision `7b5d38b9cc72805fb78606e0239ad38668ad8dbf`. Exact linked linker bytes remain distinct from later whitespace-only repository cleanup.
- The native parser runs read-only on the existing artifact/policy and preserves unsupported startup/control-flow/application safety, unknown installed executive and false delivery/hardware/NVM flags.
- ZIP CRC/local-central structure passed `unzip -t`; fresh extraction yielded exactly23 files. All22 checksum rows are unique and complete; extracted bytes equal staging bytes, candidate identities match and every target-operation scope flag remains false.
- Native44-case and strict46-case parser regressions repass, with the local artifact unchanged. Original canonical/recovery HEX hash checks also pass.
- Proposal and verifier source underwent bounded independent reviews. Two initial gaps were found: recovery authorization was not explicitly checked, and bundled source hash/declared-revision coverage was incomplete. Both were fixed; a completed narrow follow-up confirms their source-level closure.

PowerShell is unavailable locally. `VERIFY-PACKAGE.ps1` received static review; it was **not executed here**. Equivalent extracted-file/inventory/hash/scope checks passed locally in Bun, and the exact Windows verifier execution remains user-side offline evidence. Do not claim those equivalent checks prove PowerShell runtime behaviour.

Staging and fresh-extraction directories are transient build outputs and may be removed after verification. The original local experiment/attempt records and the export ZIP are preserved. No hardware discovery, register read, debugger or programmer command was used for verification.

## Next decision

The [installation/pre-edit proposal](nvm-debugger-package-proposal.md) remains non-operational until explicit approval, with exact project/tool/profile, actual erase/content-loss scope, session-ending behaviour and recovery decided. Current user screenshots show Erase All Before Program enabled, preservation disabled and broad Flash range; code placement does not constrain that erase action. First report whether any current Flash/application/metadata content needs preservation. Do not read/back up/erase it automatically.

After offline archive/file checks and that content-preservation answer, request installation/pre-edit permission separately. There is no RAM challenge approval, no NVM test backend in this application and no exact disposable page selection. The board remains canonical/read-only, PICkit5 disconnected and externally powered with settings unchanged.
