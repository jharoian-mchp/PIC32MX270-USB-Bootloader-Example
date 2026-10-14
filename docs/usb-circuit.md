# USB mini-B routing and VBUS verification

**Installed U2 identification is resolved as NCN1188.** On 2026-10-14 the user corrected the initial pin count to **12 pins**, including package pins 6 and 12 on the short sides, and supplied its datasheet marking/pinout excerpt (chat attachment 87). The official datasheet defines **AG = device code** and **M = date code** in the marking `AGM`; the observed **AG7** is consistent with that device. Board revision is reported **R7**. J2 VBUS and VBUS_SENSE were both reported **0 V with no USB host attached**.

The supplied carrier schematic routes **mini-B J2** through the EMI/ESD filter **U3** and the **NCN1188 U2 HDN/HDP channel** to the PIC32 USB data pins. Both mux controls high select HDN/HDP on the identified part; preserve the other expander bits when implementing it. The documented cable VBUS sense on **RA1** is separate from the MCU's dedicated VBUS pin, which the carrier pin table connects to its own **5V_SW_DIG** rail.

Reviewed: **2026-10-14**. This is a schematic/source review with the user-reported observations below. Installed connector continuity and enumeration have not been verified. No bootloader code was changed or programmed during this review.

## Installed-board observations

| Item | User-reported result | Interpretation |
|---|---|---|
| Board revision | R7 | Recorded as reported; need the full PCB/assembly identification or photo to match a schematic revision. |
| U2 | 12 pins, marking AG7; user corrected the earlier ten-pin count | NCN1188 identification consistent with UQFN12 pinout and AG device-code/date-code marking. Identification hold cleared. |
| VBUS_SENSE | 0 V | Consistent with the disconnected state. Does not establish the voltage on J2 pin 1. |
| J2 connector VBUS with no host | 0 V, confirmed by user on 2026-10-14 | Initial no-host voltage check passes. Does not prove all reverse-current/fault conditions are safe. |
| J2 connector VBUS with host | 5.16 V, reported 2026-10-14 | Consistent with host USB supply. |
| VBUS_SENSE with host | 2.011 V, reported 2026-10-14 | Lower than the unloaded divider estimate; below guaranteed RA1 digital-high threshold at 3.3 V VDD. |

The earlier ten-pin report prompted a hold and alternative-part search. That report was corrected by the user; no alternative switch is substituted. The NCN1188 truth table now supports the planned mini-B setting **GP5=GP6=1**. This identifies the component; it does not establish measured connector continuity, successful routing, or enumeration.

Host-attached readings are now J2 VBUS **5.16 V** and VBUS_SENSE **2.011 V**. Reconfirm return to near 0 V on unplug and record MCU VDD at the same time. The plain RA1 GPIO-detector plan is on hold because this voltage lacks guaranteed digital-high margin at nominal VDD; assess ADC-based sensing before implementing route/attach logic. Keep the current bootloader skeleton unprogrammed.

## Sources

- [Carrier reference guide](docling/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide.md), Figure A-7, source PDF page 55: USB routing, mux input controls and J2 VBUS sense circuit. Figure A-5/page 53 and pin table: carrier VBUS supply connection.
- [PIM information sheet](docling/PIC32MX270F256D-Bluetooth-Audio-PIM.md), Table 1 and Figures 1–3: MCU/PIM USB pins, RA1 and expander controls.
- [onsemi NCN1188 data sheet](https://www.onsemi.com/pdf/datasheet/ncn1188-d.pdf), July 2024 Rev. 5, page 2 truth table and page 3 pin definitions. The downloaded file's identity was checked before using its table.
- Generated USB driver: `bootloader/src/config/default/driver/usb/usbfs/src/templates/usb_OTG_SessionValid_Default.h` and `drv_usbfs_device.c`, session-valid handling around lines 1880–1915.

High-resolution crops were rendered from the original PDF to verify schematic text and connections, alongside the converted references:

- [Mux and control detail](images/usb-mux-detail.png).
- [Mini-B/filter/VBUS-sense detail](images/usb-minib-vbus-detail.png).

## Data path

| Stage | Negative data | Positive data |
|---|---|---|
| Mini-B connector J2 | Pin 2, J2_N | Pin 3, J2_P |
| EMI/ESD filter U3 | OUT2 pin 2 → INT2 pin 7, U3_N | OUT1 pin 1 → INT1 pin 8, U3_P |
| NCN1188 mux U2 | HDN pin 4 → common D− pin 8 | HDP pin 2 → common D+ pin 10 |
| Carrier/PIM connector | PIM pin 56 | PIM pin 57 |
| PIC32MX270F256D | MCU pin 9, D−/RB11 | MCU pin 8, D+/RB10 |

The filter is passive in the data path; direction names do not mean it is a unidirectional digital buffer. The NCN1188 is a bidirectional analogue switch.

The mux's DN/DP channel leads to the UCS1001 charging/Type-A path; AUDN/AUDP leads to an accessory path. **J2 mini-B uses HDN/HDP**, not the channel merely labelled DN/DP in the switch data sheet. This distinction matters when choosing mux controls.

## Mux controls

The NCN1188 truth table uses **IN1/IN2**:

| IN1 | IN2 | Selected channel |
|---:|---:|---|
| 0 | 0 | High impedance/disconnected |
| 0 | 1 | DN/DP |
| 1 | 0 | AUDN/AUDP |
| 1 | 1 | HDN/HDP — mini-B J2 on this carrier |

PIM **GP5 → connector pin 89 (USB_SEL1)** and **GP6 → pin 90 (USB_SEL0)** control the documented switch. The PIM calls these `USB_SEL/OE#` and `USB_SEL/S`; the carrier schematic's NCN1188 uses two encoded selection inputs. The user-confirmed UQFN12/AG7 identification resolves the earlier interface ambiguity; use the NCN1188 encoded-selection truth table.

For the identified NCN1188 and reviewed J2 route, both encoded controls must be high. The intended implementation is:

- Set **OLAT bits 5 and 6 high** (mask **0x60**) while preserving other latch bits.
- Enable only the required output directions, clearing **IODIR bits 5 and 6** while preserving the other directions.
- When changing an already-driven mux, first establish a reviewed disconnected state (00), then preload/select 11. Do not briefly select a charging/accessory path by careless separate-bit updates.
- Keep GP7/GPIO_32 and unrelated power/charging controls unchanged. Do not enable Type-A host power for the HID-device design.
- Installed NCN1188 identity is confirmed by corrected pin count and marking. Confirm route behavior on hardware before claiming enumeration or safe updates.

R5/R6, each **20 kΩ**, provide pull-downs on the carrier's select controls. With the MCP23008 reset directions as inputs, the documented mux defaults to 00/disconnected. The working LED application intentionally preserves GP5/GP6 and therefore does not establish a USB route.

## VBUS circuit: two different signals

### Cable presence signal

Mini-B **J2 pin 1** goes to:

- **R9, 4.99 kΩ**, then the **VBUS_SENSE** node.
- **R7, 10 kΩ**, from that node to ground.
- **D4, 3.3 V zener**, clamping the node toward ground.
- The filter U3 supply node, as shown in Figure A-7.

The PIM maps **VBUS_SENSE → PIM pin 80 → MCU RA1, pin 20**. Ignoring the clamp and load, 5 V cable VBUS gives `5 × 10 / (4.99 + 10) ≈ 3.34 V` at the divider node; actual voltage depends on the clamp/current/components. The initial approximate 3.3 V expectation was an unloaded-divider estimate, not a guaranteed output of the low-current clamp circuit. The user measured **2.011 V** with **5.16 V** at J2; that takes precedence over the estimate. **Do not connect raw 5 V directly to RA1.**

### Digital-input margin and proposed analogue sensing

The PIC32MX270 data sheet's I/O high-input limit (Table 30-8, DI20) is **VIH(min) = 0.65 × VDD** for ordinary I/O. RA1 is a Schmitt/CMOS input, not a USB receiver: do not use the USB-data VIH=2.0 V limit for this GPIO. At **VDD=3.3 V**, the required high level is **2.145 V**; **2.011 V is not guaranteed high**. The user subsequently confirmed **MCU VDD = 3.3 V**, so the lack of digital-high guarantee applies to this measured setup.

At the reported voltages and schematic resistor values, R9 current is approximately **0.631 mA** and R7 current **0.201 mA**, leaving about **0.430 mA** for the clamp/other loads. Low-current zener behaviour or another load can lower the sense voltage; this calculation does not diagnose which is responsible. No resistor/diode replacement is proposed on this evidence.

RA1 is also **AN1**. **ADC-based cable sensing with hysteresis** is a candidate way to distinguish the observed approximately 0 V disconnected and 2.011 V connected states without relying on GPIO thresholds. On 2026-10-14 the user confirmed the **same measurement point**, **3.3 V VDD**, and **return to near 0 V after unplugging**. That supports ADC-based connection-presence detection for initial bring-up; it does not establish worst-case USB VBUS validity thresholds or ADC sample accuracy.

Proposed bootloader MCC ADC setup:

| Setting | Proposed value |
|---|---|
| Input | RA1/AN1, analog input; negative input AVSS |
| Voltage reference | AVDD/AVSS; do not select external VREF− on shared RA1 |
| Result | Unsigned integer, 10-bit |
| Channels | Single AN1; no channel scan or alternate input mux |
| ADC clock | PBCLK source, ADCS=23 at 48 MHz → TAD=1 µs |
| Acquisition | SAMC=16 → 16 µs, with internal auto-convert trigger (SSRC=111) |
| Sampling start | Software-started, not continuous auto-sampling; one result per request |
| Completion | Poll conversion completion in main context; interrupt not required for this first setup |

The first ADC screenshot (2026-10-14, attachment 88) shows AN1, AVDD/AVSS, Sample A only, scan disabled and Integer 16-bit format. Integer 16-bit is the unsigned container for this device's 10-bit result, not a 16-bit converter. The displayed TAD is **125 ns**, and the conversion trigger is **clearing SAMP**; these differ from the proposed 1 µs/internal-counter setup. Set the divider until the displayed TAD is **1,000 ns** (raw ADCS=23 at 48 MHz PBCLK), select the internal-counter/auto-convert trigger (SSRC=111), and set the then-visible auto-sample time to **16 TAD**. Keep auto-sampling and ADC interrupts disabled for software-started/polled requests. Verify AN1/RA1 is an analog input in the pin settings. The updated ADC screenshot (attachment 89) confirms **TAD 1,000 ns**, internal-counter auto-convert trigger, AN1, AVDD/AVSS, and auto-sampling/interrupts unchecked. The pin screenshot (attachment 90) shows **MCU pin 20 / RA1 → AN1, input, analog, no pulls**. The trigger section is collapsed, so **SAMC=16** still needs confirmation before generation. Generated ADC code and hardware readings are not yet available.

The device data sheet Table 30-35 AD17 recommends source impedance at most **5 kΩ**. The schematic divider's nominal Thevenin resistance is **4.99 kΩ || 10 kΩ ≈ 3.33 kΩ**, before clamp/loading effects. Table 30-36 specifies TAD at least 65 ns and sampling at least 132 ns; the proposed 1 µs/16 µs timing provides headroom and must be checked in generated code and on hardware. Software-triggered sampling with SSRC=111 uses SAMC; a fully software-cleared SAMP trigger would ignore SAMC.

Initial candidate hysteresis is **connect above 1.5 V** and **disconnect below 0.8 V**, retaining state between thresholds. At AVDD=3.3 V this is approximately 465 and 248 counts on a 0–1023 display scale, versus approximately 624 for the measured 2.011 V. These are proposed bring-up thresholds, not worst-case-qualified limits. Read and record actual ADC values before accepting them; sample cadence/debounce and safe attach/detach integration still need implementation and tests. No ADC code or generated configuration has been added yet. A comparator/conditioned hardware input remains another option if needed.

### Dedicated MCU VBUS

The PIM maps **MCU VBUS pin 42 → PIM pin 54**. The carrier table and CPU schematic identify connector pin 54 as **5V_SW_DIG**, from the board's externally supplied rail. This differs from J2's cable-presence node.

The generated USBFS driver presently checks **U1OTGSTAT.SESVD** via `PLIB_USB_OTG_SessionValid()` and generates session-valid/invalid events. The datastream then attaches/detaches on POWER_DETECTED/POWER_REMOVED. With dedicated VBUS supplied by the board, that hardware status cannot be assumed to reflect PC cable presence. The generic GPIO VBUS option is hidden/default-false for PIC32MX in the installed USB v3.16.0 generator; simply checking Self Powered does not redirect sensing to RA1.

Before enumeration, choose and test a board-aware use of RA1 that is compatible with driver/device-layer attach behaviour. Do not blindly replace a generated driver or manually route J2 VBUS into the PIM VBUS pin. Do not treat the board's own 5 V rail as proof that a host is connected.

## Power and back-feed assessment

The reviewed schematic shows J2 VBUS feeding the resistor/clamp sense and filter supply, with **no depicted direct connection to the board 5V_SW_DIG rail or Type-A VBUS outputs**. That is consistent with an externally powered USB device, but it does not prove that the installed board has no assembly/revision changes or reverse-current path.

USB **Self Powered remains checked**. This descriptor bit does not electrically isolate supplies or enable/disable VBUS sourcing. Keep the Type-A port unused during first HID bring-up.

The UCS1001-4 controller is on the separate charging/Type-A path. Its full modes/reverse-current behaviour have not been verified from a component data sheet in this review; no charging-control changes are approved. An attempted legacy Microchip document-number lookup returned an unrelated MCP3903 data sheet, which was discarded rather than used as controller evidence.

## Physical checks before connecting a PC

Use the unmodified tagged LED firmware if firmware is needed while measuring. **Do not program the current bootloader skeleton yet.** Keep PICkit target powering disabled if the external supply powers the board; avoid two power sources fighting.

1. **Identify the board revision and fitted U2.** Send a clear photo of the USB area/silkscreen if labels are hard to read. Keep J1 at address 0x22, J5 at 3.3 V, and reset selector PIM_MCLR.
2. **Externally power the board with no USB host attached.** Using a suitable USB breakout/cable with safely accessible contacts, measure J2 VBUS (pin 1) to board ground. An alternative schematic node is the **J2-facing end of R9**, before the resistor; the other end is VBUS_SENSE and is not this measurement. The supplied photos do not reliably establish which physical R9 pad is which. Identify it only with a matching layout or an unpowered continuity check to J2 pin 1. Use the same verified board-ground point used for VBUS_SENSE; do not assume the connector shell is ground without checking. Attach insulated clips with power off, then power up for DC-voltage measurement. The schematic suggests no intentional board-source voltage here. If it is appreciably powered, stop and investigate before attaching the PC. Do not probe inside the socket or tiny adjacent contacts where an accidental short is likely.
3. **Check VBUS_SENSE/RA1 with no host attached.** Measure at an accessible node such as the R9/R7 junction or PIM connector 80; expect near 0 V. Avoid probing the fine-pitch MCU pin directly.
4. **Check dedicated VBUS separately**, if safely accessible: PIM connector 54 should follow the board's 5 V rail, illustrating why it differs from cable sense. A measurement is required before treating this as the installed circuit.
5. After the disconnected check passes, use an appropriate protected USB connection/breakout to a host. Verify J2 VBUS is about 5 V and VBUS_SENSE is around 3.3 V, then falls near 0 V on unplug. If sense exceeds the device's permitted voltage or does not track cable VBUS, stop.
6. Do resistance/continuity checks only with **all board power, USB and PICkit connections removed**. Switched IC paths cannot always be interpreted by a simple continuity beep; do not use a meter's resistance mode on a powered USB/MCU circuit.

Disconnected J2 VBUS and VBUS_SENSE are both user-reported 0 V, and installed U2 identification is resolved. Host-attached voltages are now reported (J2 5.16 V, sense 2.011 V). The user confirmed return-to-zero after unplug, VDD=3.3 V and the same measurement point. ADC readings/detector margin and actual USB routing still need confirmation. If safe access is unavailable, send the board photo and describe available measurement tools; do not improvise risky probing.

## Next implementation milestone

After the actual switch identification, revision-specific routing and physical power checks are established:

- Implement user-owned MCP23008 route setup from the confirmed NCN1188 truth table: GP5=GP6=1 for mini-B, preserving other pins. The identification hold is cleared; safe initialization, errors and routing behavior still require implementation/tests.
- Assess RA1/AN1 ADC sensing because the measured 2.011 V is below guaranteed GPIO VIH at 3.3 V. Verify signal/reference/acquisition margins, then implement attach/detach coordination and SW1 force-entry without disrupting the preserved application.
- Build/test the code and review nontrivial compiler diagnostics; only then approve a controlled enumeration test, with host erase/program commands unused.
- Record actual VID/PID enumeration and cable disconnect behaviour before testing the update protocol or relocated application.
