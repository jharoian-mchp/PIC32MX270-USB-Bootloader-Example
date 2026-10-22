# Enabled diagnostics restored to the canonical 32 KB link

The enabled Harmony-powered diagnostics now fit the unchanged 32 KB reservation through one build-only change: `bootloader-restart-check` adds `-fno-data-sections`. All runtime sources, diagnostics, wire formats and safety predicates are unchanged. Tests, sanitizers, XC32 compilation and both layout-specific artifact audits pass. The new 32 KB image has not been programmed or hardware-qualified.

## Build change and measured result

The common build uses `-fdata-sections`. The opt-in restart-check flags occur later on the compiler command line, so `-fno-data-sections` overrides it for that target only. Function section splitting, `--gc-sections`, `-Os` and LTO remain enabled. The default recovery and historical 48 KB debug targets keep their original flags.

Coalescing data sections reduces XC32 startup initialization records and placement overhead. It also changes data-section garbage collection and linked SRAM placement; it is not a guarantee of identical target timing. The final ELF must be audited rather than assuming that a source-level no-op preserves every binary property.

| Item | Historical 48 KB diagnostic | Restored 32 KB diagnostic | Default recovery |
|---|---:|---:|---:|
| Program reservation | 49152 | 32768 | 32768 |
| Application origin | `0x1D00C000` | `0x1D008000` | `0x1D008000` |
| ELF text/data/bss | 33237 / 88 / 2052 | 32284 / 88 / 2052 | 26670 / 80 / 1568 |
| Program-region bytes used | 32829 | 31876 | Unchanged |
| `.dinit` bytes | 1104 (`0x450`) | 144 (`0x90`) | Unchanged |
| Startup clear/copy records | 84 | 4 | Unchanged |

The canonical diagnostic map leaves 892 nominal bytes in its32768-byte program region; this is not an allowance for arbitrary future code or alignment changes. The text column includes other Flash sections and must not be substituted for the program-region map usage. The canonical linker `btl.ld`, vectors and application constants were not changed.

Local canonical diagnostic HEX SHA-256: `f886f8b008ef7088baa0fbeb051d42fe89b6a2fad416006220554cf674114ad7`.

Historical 48 KB debug HEX remains `6e0dc7304bc6c1468be328fc97277c3cfb997b55291aff360855c84bc7f88873`, matching the delivered image that passed one controlled board cycle. Default recovery HEX remains `8e5a8451e43cdb4ee96b80f34ce5e3a5e166ba619ade144468c02a1de056d14c`. These local identities do not constitute a new delivered package.

## Retained startup and exclusion checks

The 128-byte schema4 journal remains in NOBITS `.persist` at `0xA0000000..0xA0000080` and is excluded from every linked `.dinit` clear/copy destination. The four canonical startup destinations are:

| Destination | Operation | Bytes |
|---|---|---:|
| `0xA0000080` | copy | 5 |
| `0xA0000088` | clear | 221 |
| `0xA0000168` | copy | 80 |
| `0xA0000200` | clear | 1700 |

Those actual destination spans were checked for mutual overlap and do not overlap. The generic audit checks SRAM bounds and journal exclusion but does not currently reject all pairwise initialization-record overlaps; that non-blocking audit-hardening gap is recorded rather than silently claiming it is enforced.

The canonical audit verifies ELF/HEX load equality, Flash spans inside the32KB reservation, configuration equality, retained placement/startup exclusion and required restart-owner code. Linked update, NVM-write and application-handoff entrypoints remain excluded. Configuration/protection settings are unchanged. Audit flags remain `hardware_qualified=false` and `board_delivery_approved=false`.

Coarser data sections can retain objects that section-level garbage collection would otherwise discard. Symbol exclusion and the actual startup record inspection passed for this build; they do not prove arbitrary LTO instruction safety. The normal recovery image is unaffected by the new opt-in flag.

## Exploration, tests and review

The initial exploration tested runtime function boundaries and alternative journal CRC/metadata/record-copy implementations. Some saved bytes but did not restore a valid canonical link; others increased size. The unchanged committed runtime plus the single data-section flag was sufficient, so all exploratory runtime edits were restored. No algorithm or noinline attribute from that exploration is retained. Transient scan scripts and source-copy snapshots were removed; verification logs preserve the measurements.

Passing verification:

- `make test` and `make test-sanitize`: strict/sanitized C/mock suites and30 Python test methods, preserving the full diagnostics and safety regression coverage.
- `make usb-retirement-check`: checked generated overlays and enabled XC32 compilation.
- `make restart-image-audit` and `bun tools/test-restart-image-audit.ts`: canonical32KB link and retained-startup/HEX/configuration/negative artifact checks.
- `make power-debug-image-audit` and `bun tools/test-restart-image-audit.ts --power-debug`: unchanged historical debug target and its artifact checks; debug HEX identity matched the delivered archive.
- Direct comparison confirming no changes under `bootloader/src`, `host`, `tools` or `tests`, plus `git diff --check`.

A delegated read-only size exploration timed out and supplied no completed recommendation. A completed bounded build review found no blocking defect in the flag ordering or target isolation. It noted the generic pairwise `.dinit` overlap gap; direct inspection of this generated four-record table passed. Neither review qualifies the new image on hardware.

## Board and delivery boundary

The [first controlled Harmony-powered cycle](usb-harmony-powered-first-cycle.md) passed on the older 48 KB build. It does not qualify this newly compiled32KB image, whose instruction/data layout and initialization timing changed. The PIC32MX270 A1 powered-BUSY concern remains open; the Harmony-powered readiness decision and USB-off retirement/NVM gates remain exactly as before.

Keep the existing board running/read-only with PICkit5 disconnected and external power/settings unchanged. The single-trial permission is spent. No replacement package, second PAUSE, higher ID, reset, reprogramming, uploads, NVM/protection changes or fault tests are authorised. The subsequent [canonical-image candidate review](usb-canonical32-candidate-review.md) passes its bounded offline checks and defines initial-only package requirements. It grants no package or board permission; explicit delivery/programming scope is still required before any new hardware evaluation.
