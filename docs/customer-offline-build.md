# Customer demo offline builds — 18 November 2026

Both customer projects compile, link and generate HEX with Linux XC32 v6.00 and PIC32MX_DFP1.7.380. They are offline candidates; no installation or update test is approved.

`make customer-demo-build` extracts unique C source entries from the shared MPLAB XML, compiles bootloader26sources atO2/application12sources atO1 with section garbage collection, then links the reviewed scripts and converts toHEX. It records commands/output/source inventory under build/customer-demo/. This is a production-style Linux build using vendor startup; it does not reproduce an IDE Debug operation or qualify Windows generated rules.

The first build caught ADC_ResultGet's missing buffer argument. The platform call now explicitly selects ADC_RESULT_BUFFER_0 and the mock signature matches. A retry passed both builds; other warnings are generated/support unused-parameter categories. No observed target compile errors remain.

| Artifact | Result |
|---|---|
| Bootloader | text27095/data224/bss1960; physicalPFM bytes1D000000..1D006A4F, inside32KB |
| Application | text5776/data0/bss100; entry9D008000/startup9D008008; physicalPFM bytes1D008000..1D03F767 |
| App vector/EBase | EBase9D03F000; vector8 at9D03F300; BEV9D008380 |
| ELF/HEX | Exact file-backed flash section bytes match HEX; no extraHEXbytes; app contains noBootFlash/configuration bytes |

The structural auditor checks flash sections only. Initialised RAM in the bootloader is encoded by runtime initialisation records; full RAM-copy/clear, PT_LOAD coverage and executable control-flow audit are not completed by this check.

## Hardware configuration finding

Both new generated projects currently select ICESEL=ICS_PGx1. The established PIM ICSP connection uses ICS_PGx4. Bootloader DEVCFG0 is7FFFFFFB (DEBUGOFF/JTAGOFF/PGx1/protectionOFF), while desired board PGx4 would be7FFFFFE3. Other words match FFFFFFFF/FFF97AFA/FD74CE5B. The auditor pins the actualPGx1 candidate word without treating it as board approval. Correct both source and savedMCCsetting before new installation; application configinputs remain excluded from itsHEX. Regeneration must preserve/reapply documented user main/datastream/linker edits.

## Hash-bound candidates

- Bootloader ELF:0d05b8f8569ad0b58f2ebaba552f9579a0abbb65f8a22076dbda7540b412540f
- Bootloader HEX:022babe43203ec4638d3e23d29caf651b69411f3b6396f74798d87ba3d20f9a6
- Application ELF:56ab5befc314cba3d1f27810c66336721f0da094c8a0271f87061d8af3916ebd
- Application HEX:6d33354c0171a409ec84e4205409ce3223390747cafab0c195d2159ec7e031d0

Host LED and board mocks/source-layout checks repass after the ADC fix. `make test` also passes. Startup/handoff cleanup, complete USBcallbackownership, stockparser/write boundaries and vendorhostcommand compatibility still need review before connected operations. run_Application only checks a non-erased firstword, calls the generated empty SYS_DeInitialize, masksinterrupts and jumps: no robust imagevalidation or complete hardwarecleanup is qualified.

No board operation, USB command, debugger launch or reset occurred. Do not install these PGx1 candidates.

## Corrected PGx4 candidates — 18 November 15:33 approval

After the saved MCC configuration and both generated ICESEL pragmas were aligned in15d0522, both projects rebuilt successfully. The structural auditor now requires DEVCFG0=7FFFFFE3 (PGx4); configuration words3..0 areFFFFFFFF/FFF97AFA/FD74CE5B/7FFFFFE3. The earlier PGx1 candidates above are superseded for installation planning.

- Bootloader ELF:248989b8ed48f8c00637c67d18ea5d1eebb4c633147aae0cec7d16fa84b43ad2
- Bootloader HEX:85e861e67e49bdc4488e5e1a3b3e96973eb39e602a59f267d7be6bdc96cd8938
- Application ELF:a0103d371c7150af3afda7971ab09e0c3553fcf73a4bb3b09b35bd30855e168a
- Application HEX:6d33354c0171a409ec84e4205409ce3223390747cafab0c195d2159ec7e031d0

Bootloader/application flash ranges and byte counts are unchanged. The application HEX is byte-for-byte unchanged because its configuration inputs are excluded. Customer app/board tests and source-layout checks repass. Full startup/RAM/USBcallback/hostparser/handoff review is still open; these results grant no hardware authority.
