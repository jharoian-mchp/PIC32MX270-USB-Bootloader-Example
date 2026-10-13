![Image](images/PIC32MX1XX-2XX-28-36-44-pin-Family-Silicon-Errata-DS80000531/image_000000_69a83a11ab1e217aba5baf6a81ebe5d3363f79be6c1794266a394646b919330f.png)

## PIC32MX1XX/2XX 28/36/44-PIN

Microchip

## PIC32MX1XX/2XX 28/36/44-pin Family Silicon Errata and Data Sheet Clarification

The PIC32MX1XX/2XX 28/36/44-pin family of devices that you have received conform functionally to the cur -rent Device Data Sheet (DS60001168 L ), except for the anomalies described in this document.

The errata described in this document will be addressed in  future  revisions of the PIC32MX1XX/2XX 28/36/44pin silicon.

## Note:

The silicon issues discussed in the following  pages  are  for  silicon  revisions with the Device and Revision IDs listed in Table 1 through Table 4. The last column of each table represents the latest silicon revision for the devices listed. The silicon issues are summarized in Table 5.

Data Sheet clarifications and corrections start on page 14, following the discussion of silicon issues.

The silicon  revision  level  can  be  identified  using  the current  version  of  MPLAB ® X  IDE  and  Microchip's programmers, debuggers and emulation tools, which are available at the Microchip corporate  website (www.microchip.com).

For  example,  to  identify  the  silicon  revision  level using MPLAB X IDE in conjunction with a hardware debugger:

1. Using the appropriate interface, connect the device to the hardware debugger.
2. Open an MPLAB X IDE project.
3. Configure the MPLAB X IDE project for the appropriate device and hardware debugger.
4. Select Window &gt; Dashboard , and then click the Refresh  Debug  Tool  Status icon ( ). G2
5. The  part  number  and  the  Device  and Revision  ID  values  appear  in  the Output window.

Note:

If  you  are  unable  to  extract  the  silicon revision  level,  please  contact  your  local Microchip sales office for assistance.

The  Device  and  Revision  ID  values  for  the  various silicon revisions are provided in Table 1 through Table 4.

TABLE 1: SILICON DEVREV VALUES FOR DEVICES WITH 16/32 KB FLASH

| Part Number     | Device ID (1)   | Revision ID for Silicon Revision (1)   | Revision ID for Silicon Revision (1)   |
|-----------------|-----------------|----------------------------------------|----------------------------------------|
|                 |                 | A0                                     | A1                                     |
| PIC32MX110F016B | 0x4A07053       | 0x0                                    | 0x1                                    |
| PIC32MX110F016C | 0x4A09053       | 0x0                                    | 0x1                                    |
| PIC32MX110F016D | 0x4A0B053       | 0x0                                    | 0x1                                    |
| PIC32MX210F016B | 0x4A01053       | 0x0                                    | 0x1                                    |
| PIC32MX210F016C | 0x4A03053       | 0x0                                    | 0x1                                    |
| PIC32MX210F016D | 0x4A05053       | 0x0                                    | 0x1                                    |
| PIC32MX120F032B | 0x4A06053       | 0x0                                    | 0x1                                    |
| PIC32MX120F032C | 0x4A08053       | 0x0                                    | 0x1                                    |
| PIC32MX120F032D | 0x4A0A053       | 0x0                                    | 0x1                                    |
| PIC32MX220F032B | 0x4A00053       | 0x0                                    | 0x1                                    |
| PIC32MX220F032C | 0x4A02053       | 0x0                                    | 0x1                                    |
| PIC32MX220F032D | 0x4A04053       | 0x0                                    | 0x1                                    |

Note 1: Refer to the 'Memory Organization' and 'Special Features' chapters in the current Device Data Sheet (DS60001168 J ) for detailed information on Device and Revision IDs for your specific device.

## PIC32MX1XX/2XX 28/36/44-PIN

TABLE 2: SILICON DEVREV VALUES FOR DEVICES WITH 64/128 KB FLASH

|                 | Device ID (1)   | Revision ID for Silicon Revision (1)   | Revision ID for Silicon Revision (1)   | Revision ID for Silicon Revision (1)   | Revision ID for Silicon Revision (1)   | Revision ID for Silicon Revision (1)   |
|-----------------|-----------------|----------------------------------------|----------------------------------------|----------------------------------------|----------------------------------------|----------------------------------------|
| Part Number     |                 | A0                                     | A1                                     | A3                                     | B0                                     | B1                                     |
| PIC32MX130F064B | 0x4D07053       | 0x0                                    | 0x1                                    |                                        | 0x6                                    | 0x7                                    |
| PIC32MX130F064C | 0x4D09053       | 0x0                                    | 0x1                                    |                                        | 0x6                                    | 0x7                                    |
| PIC32MX130F064D | 0x4D0B053       | 0x0                                    | 0x1                                    | 0x3                                    | 0x6                                    | 0x7                                    |
| PIC32MX230F064B | 0x4D01053       | 0x0                                    | 0x1                                    |                                        | 0x6                                    | 0x7                                    |
| PIC32MX230F064C | 0x4D03053       | 0x0                                    | 0x1                                    |                                        | 0x6                                    | 0x7                                    |
| PIC32MX230F064D | 0x4D05053       | 0x0                                    | 0x1                                    |                                        | 0x6                                    | 0x7                                    |
| PIC32MX150F128B | 0x4D06053       | 0x0                                    | 0x1                                    | -                                      | 0x6                                    | -                                      |
| PIC32MX150F128C | 0x4D08053       | 0x0                                    | 0x1                                    | -                                      | 0x6                                    | -                                      |
| PIC32MX150F128D | 0x4D0A053       | 0x0                                    | 0x1                                    | -                                      | 0x6                                    | -                                      |
| PIC32MX250F128B | 0x4D00053       | 0x0                                    | 0x1                                    | -                                      | 0x6                                    | -                                      |
| PIC32MX250F128C | 0x4D02053       | 0x0                                    | 0x1                                    | -                                      | 0x6                                    | -                                      |
| PIC32MX250F128D | 0x4D04053       | 0x0                                    | 0x1                                    | -                                      | 0x6                                    | -                                      |

TABLE 3: SILICON DEVREV VALUES FOR DEVICES WITH 256 KB FLASH AND 64 KB RAM

| Part Number      | Device ID (1)   | Revision ID for Silicon Revision (1)   | Revision ID for Silicon Revision (1)   |
|------------------|-----------------|----------------------------------------|----------------------------------------|
|                  |                 | A1                                     | A2                                     |
| PIC32MX170F256B  | 0x6610053       | 0x1                                    | 0x2                                    |
| PIC32MX170F256D  | 0x661A053       | 0x1                                    | 0x2                                    |
| PIC32MX270F256B  | 0x6600053       | 0x1                                    | 0x2                                    |
| PIC32MX270F256D  | 0x660A053       | 0x1                                    | 0x2                                    |
| PIC32MX270F256DB | 0x660C053       | 0x1                                    | 0x2                                    |

TABLE 4: SILICON DEVREV VALUES FOR DEVICES WITH 256 KB FLASH AND 16 KB RAM

| Part Number     | Device ID (1)   | Revision ID for Silicon Revision (1) A0   |
|-----------------|-----------------|-------------------------------------------|
| PIC32MX130F256B | 0x6703053       | 0x0                                       |
| PIC32MX130F256D | 0x6705053       | 0x0                                       |
| PIC32MX230F256B | 0x6700053       | 0x0                                       |
| PIC32MX230F256D | 0x6702053       | 0x0                                       |

## PIC32MX1XX/2XX 28/36/44-PIN

s TABLE 5: SILICON ISSUE SUMMARY

| Module                       | Affected Device   | Affected Device   | Affected Device   | Affected Device   | Affected Device   | Affected Device   | Affected Device   | Affected Device   |
|------------------------------|-------------------|-------------------|-------------------|-------------------|-------------------|-------------------|-------------------|-------------------|
|                              | Flash Memory (KB) | Data Memory (KB)  | Silicon Revision  | Silicon Revision  | Silicon Revision  | Silicon Revision  | Silicon Revision  | Silicon Revision  |
|                              | Flash Memory (KB) | Data Memory (KB)  | A0                | A1                | A2                | A3                | B0                | B1                |
| 1. Module: Voltage Regulator | 16/32             | 4/8               | X                 |                   | -                 | -                 | -                 | -                 |
| 1. Module: Voltage Regulator | 64                | 16                | X                 |                   | -                 |                   |                   |                   |
| 1. Module: Voltage Regulator | 128               | 32                | X                 |                   | -                 | -                 |                   | -                 |
| 1. Module: Voltage Regulator | 256               | 16                |                   | -                 | -                 | -                 | -                 | -                 |
| 1. Module: Voltage Regulator | 256               | 64                | -                 |                   |                   | -                 | -                 | -                 |
| 2. Module: Oscillator        | 16/32             | 4/8               | X                 | X                 | -                 | -                 | -                 | -                 |
| 2. Module: Oscillator        | 64                | 16                | X                 | X                 | -                 | X                 | X                 | X                 |
| 2. Module: Oscillator        | 128               | 32                | X                 | X                 | -                 | -                 | X                 | -                 |
| 2. Module: Oscillator        | 256               | 16                | X                 | -                 | -                 | -                 | -                 | -                 |
| 2. Module: Oscillator        | 256               | 64                | -                 | X                 | X                 | -                 | -                 | -                 |
| 3. Module: I2C               | 16/32             | 4/8               | X                 | X                 | -                 | -                 | -                 | -                 |
| 3. Module: I2C               | 64                | 16                | X                 | X                 | -                 | X                 | X                 | X                 |
| 3. Module: I2C               | 128               | 32                | X                 | X                 | -                 | -                 | X                 | -                 |
| 3. Module: I2C               | 256               | 16                | X                 | -                 | -                 | -                 | -                 | -                 |
| 3. Module: I2C               | 256               | 64                | -                 | X                 | X                 | -                 | -                 | -                 |
|                              | 16/32             | 4/8               | X                 | X                 | -                 | -                 | -                 | -                 |
|                              | 64                | 16                | X                 | X                 | -                 | X                 | X                 | X                 |
|                              | 128               | 32                | X                 | X                 | -                 | -                 | X                 | -                 |
|                              | 256               | 16                | X                 | -                 | -                 | -                 | -                 | -                 |
|                              | 256               | 64                | -                 | X                 | X                 | -                 | -                 | -                 |
| 5. Module: ADC               | 16/32             | 4/8               | X                 | X                 | -                 | -                 | -                 | -                 |
| 5. Module: ADC               | 64                | 16                | X                 | X                 | -                 | X                 | X                 | X                 |
| 5. Module: ADC               | 128               | 32                | X                 | X                 | -                 | -                 | X                 | -                 |
| 5. Module: ADC               | 256               | 16                | X                 | -                 | -                 | -                 | -                 | -                 |
| 6. Module: ADC               | 16/32             | 4/8               | X                 | X                 | -                 | -                 | -                 | -                 |
| 6. Module: ADC               | 64                | 16                |                   |                   | -                 |                   |                   |                   |
| 6. Module: ADC               | 128               | 32                |                   |                   | -                 | -                 |                   | -                 |
| 6. Module: ADC               | 256               | 16                |                   | -                 | -                 | -                 | -                 | -                 |
| 6. Module: ADC               | 256               | 64                | -                 |                   |                   | -                 | -                 | -                 |

Legend:

An 'X' indicates the issue is present in this revision of silicon;

shaded cells with an Em dash ('-') indicate that this silicon revision does not exist for this issue;

blank cells indicate an issue has been corrected in this revision of silicon.

## PIC32MX1XX/2XX 28/36/44-PIN

TABLE 5: SILICON ISSUE SUMMARY  (CONTINUED)

| Module                                  | Affected          | Affected         | Affected         | Affected         | Affected         | Affected         | Affected         | Affected         |
|-----------------------------------------|-------------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|
|                                         | Flash Memory (KB) | Data Memory (KB) | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision |
|                                         | Flash Memory (KB) | Data Memory (KB) | A0               | A1               | A2               | A3               | B0               | B1               |
| 7. Module: ADC Conversion Trigger from  | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 7. Module: ADC Conversion Trigger from  | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 7. Module: ADC Conversion Trigger from  | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 7. Module: ADC Conversion Trigger from  | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 7. Module: ADC Conversion Trigger from  | 256               | 64               | -                | X                | X                | -                | -                | -                |
| 8. Module: PMP Address                  | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 8. Module: PMP Address                  | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 8. Module: PMP Address                  | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 8. Module: PMP Address                  | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 8. Module: PMP Address                  | 256               | 64               | -                | X                | X                | -                | -                | -                |
| 9. Module: I/ O Ports RA0 RA1           | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 9. Module: I/ O Ports RA0 RA1           | 64                | 16               | X                | X                | -                |                  | X                |                  |
| 9. Module: I/ O Ports RA0 RA1           | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 9. Module: I/ O Ports RA0 RA1           | 256               | 16               |                  | -                | -                | -                | -                | -                |
| 9. Module: I/ O Ports RA0 RA1           | 256               | 64               | -                |                  |                  | -                | -                | -                |
|                                         | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
|                                         | 64                | 16               | X                | X                | -                |                  | X                |                  |
|                                         | 128               | 32               | X                | X                | -                | -                | X                | -                |
|                                         | 256               | 16               |                  | -                | -                | -                | -                | -                |
|                                         | 256               | 64               | -                |                  |                  | -                | -                | -                |
| 11. Module: Oscillator Reference: Clock | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 11. Module: Oscillator Reference: Clock | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 11. Module: Oscillator Reference: Clock | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 11. Module: Oscillator Reference: Clock | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 12. Module: Input Capture Idle and Mode | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 12. Module: Input Capture Idle and Mode | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 12. Module: Input Capture Idle and Mode | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 12. Module: Input Capture Idle and Mode | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 12. Module: Input Capture Idle and Mode | 256               | 64               | -                | X                | X                | -                | -                | -                |

Legend:

An 'X' indicates the issue is present in this revision of silicon; shaded cells with an Em dash ('-') indicate that this silicon revision does not exist for this issue; blank cells indicate an issue has been corrected in this revision of silicon.

## PIC32MX1XX/2XX 28/36/44-PIN

TABLE 5: SILICON ISSUE SUMMARY  (CONTINUED)

| Module                           | Affected          | Affected         | Affected         | Affected         | Affected         | Affected         | Affected         | Affected         |
|----------------------------------|-------------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|
|                                  | Flash Memory (KB) | Data Memory (KB) | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision |
|                                  | Flash Memory (KB) | Data Memory (KB) | A0               | A1               | A2               | A3               | B0               | B1               |
| 13. Module: Watchdog Timer (WDT) | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 13. Module: Watchdog Timer (WDT) | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 13. Module: Watchdog Timer (WDT) | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 13. Module: Watchdog Timer (WDT) | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 13. Module: Watchdog Timer (WDT) | 256               | 64               | -                | X                | X                | -                | -                | -                |
| 14. Module: Non-5V Tolerant Pins | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 14. Module: Non-5V Tolerant Pins | 64                | 16               | X                | X                | -                |                  | X                |                  |
| 14. Module: Non-5V Tolerant Pins | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 14. Module: Non-5V Tolerant Pins | 256               | 16               |                  | -                | -                | -                | -                | -                |
| 14. Module: Non-5V Tolerant Pins | 256               | 64               | -                |                  |                  | -                | -                | -                |
| 15. Module: 5V Tolerant Pins     | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 15. Module: 5V Tolerant Pins     | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 15. Module: 5V Tolerant Pins     | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 15. Module: 5V Tolerant Pins     | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 15. Module: 5V Tolerant Pins     | 256               | 64               | -                | X                | X                | -                | -                | -                |
| 16. Module:                      | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 16. Module:                      | 64                | 16               | X                | X                | -                |                  | X                |                  |
| 16. Module:                      | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 16. Module:                      | 256               | 16               |                  | -                | -                | -                | -                | -                |
| 16. Module:                      | 256               | 64               | -                |                  |                  | -                | -                | -                |
| 17. Module: I/O Ports            | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 17. Module: I/O Ports            | 64                | 16               | X                | X                | -                |                  | X                |                  |
| 17. Module: I/O Ports            | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 17. Module: I/O Ports            | 256               | 16               |                  | -                | -                | -                | -                | -                |
| 18. Module: I/O Ports            | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 18. Module: I/O Ports            | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 18. Module: I/O Ports            | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 18. Module: I/O Ports            | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 18. Module: I/O Ports            | 256               | 64               | -                | X                | X                | -                | -                | -                |

Legend:

An 'X' indicates the issue is present in this revision of silicon; shaded cells with an Em dash ('-') indicate that this silicon revision does not exist for this issue;

blank cells indicate an issue has been corrected in this revision of silicon.

## PIC32MX1XX/2XX 28/36/44-PIN

TABLE 5: SILICON ISSUE SUMMARY  (CONTINUED)

| Module                                                                                                      | Affected          | Affected         | Affected         | Affected         | Affected         | Affected         | Affected         | Affected         |
|-------------------------------------------------------------------------------------------------------------|-------------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|
|                                                                                                             | Flash Memory (KB) | Data Memory (KB) | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision |
|                                                                                                             | Flash Memory (KB) | Data Memory (KB) | A0               | A1               | A2               | A3               | B0               | B1               |
| 19. Module: UART Synchroniz                                                                                 | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 19. Module: UART Synchroniz                                                                                 | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 19. Module: UART Synchroniz                                                                                 | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 19. Module: UART Synchroniz                                                                                 | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 19. Module: UART Synchroniz                                                                                 | 256               | 64               | -                | X                | X                | -                | -                | -                |
| 20. Module: Timer1 Interrupts                                                                               | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| Timer1 will not generate interrupts with an external asynchronous clock input and prescaler other than 1:1. | 64                | 16               | X                | X                | -                | X                | X                | X                |
| Timer1 will not generate interrupts with an external asynchronous clock input and prescaler other than 1:1. | 128               | 32               | X                | X                | -                | -                | X                | -                |
| Timer1 will not generate interrupts with an external asynchronous clock input and prescaler other than 1:1. | 256               | 16               | X                | -                | -                | -                | -                | -                |
| Timer1 will not generate interrupts with an external asynchronous clock input and prescaler other than 1:1. | 256               | 64               | -                | X                | X                | -                | -                | -                |
| 21. Module: Flash Memory Protection                                                                         | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 21. Module: Flash Memory Protection                                                                         | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 21. Module: Flash Memory Protection                                                                         | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 21. Module: Flash Memory Protection                                                                         | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 21. Module: Flash Memory Protection                                                                         | 256               | 64               | -                | X                | X                | -                | -                | -                |
|                                                                                                             | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 22. Module: Write                                                                                           | 64                | 16               | X                | X                | -                |                  | X                |                  |
| 22. Module: Write                                                                                           | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 22. Module: Write                                                                                           | 256               | 16               |                  | -                | -                | -                | -                | -                |
| 22. Module: Write                                                                                           | 256               | 64               | -                |                  |                  | -                | -                | -                |
| 23. Module:                                                                                                 | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| Flash Memory Protection                                                                                     | 64                | 16               | X                | X                | -                |                  | X                |                  |
| Flash Memory Protection                                                                                     | 128               | 32               | X                | X                | -                | -                | X                | -                |
| Flash Memory Protection                                                                                     | 256               | 16               |                  | -                | -                | -                | -                | -                |
| Flash Memory Protection                                                                                     | 256               | 64               | -                |                  |                  | -                | -                | -                |
| 24. Module: Flash Memory Write Protection                                                                   | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 24. Module: Flash Memory Write Protection                                                                   | 64                | 16               | X                | X                | -                |                  | X                |                  |
| 24. Module: Flash Memory Write Protection                                                                   | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 24. Module: Flash Memory Write Protection                                                                   | 256               | 16               |                  | -                | -                | -                | -                | -                |
| 24. Module: Flash Memory Write Protection                                                                   | 256               | 64               | -                |                  |                  | -                | -                | -                |

Legend:

An 'X' indicates the issue is present in this revision of silicon; shaded cells with an Em dash ('-') indicate that this silicon revision does not exist for this issue; blank cells indicate an issue has been corrected in this revision of silicon.

## PIC32MX1XX/2XX 28/36/44-PIN

TABLE 5: SILICON ISSUE SUMMARY  (CONTINUED)

| Module                                                                                                                 | Affected          | Affected         | Affected         | Affected         | Affected         | Affected         | Affected         | Affected         |
|------------------------------------------------------------------------------------------------------------------------|-------------------|------------------|------------------|------------------|------------------|------------------|------------------|------------------|
|                                                                                                                        | Flash Memory (KB) | Data Memory (KB) | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision | Silicon Revision |
|                                                                                                                        | Flash Memory (KB) | Data Memory (KB) | A0               | A1               | A2               | A3               | B0               | B1               |
| 25. Module: Power- Saving                                                                                              | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 25. Module: Power- Saving                                                                                              | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 25. Module: Power- Saving                                                                                              | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 25. Module: Power- Saving                                                                                              | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 25. Module: Power- Saving                                                                                              | 256               | 64               | -                | X                | X                | -                | -                | -                |
| 26. Module:                                                                                                            | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| The CTMU internal temperature sensing diode does not function for VDD/AVDD operating voltages that are less than 2.5V. | 64                | 16               | X                | X                | -                |                  | X                |                  |
| The CTMU internal temperature sensing diode does not function for VDD/AVDD operating voltages that are less than 2.5V. | 128               | 32               | X                | X                | -                | -                | X                | -                |
| The CTMU internal temperature sensing diode does not function for VDD/AVDD operating voltages that are less than 2.5V. | 256               | 16               |                  | -                | -                | -                | -                | -                |
| The CTMU internal temperature sensing diode does not function for VDD/AVDD operating voltages that are less than 2.5V. | 256               | 64               | -                | X                | X                | -                | -                | -                |
| 27. Module: I2C                                                                                                        | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 27. Module: I2C                                                                                                        | 64                | 16               | X                | X                | -                |                  | X                |                  |
| 27. Module: I2C                                                                                                        | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 27. Module: I2C                                                                                                        | 256               | 16               |                  | -                | -                | -                | -                | -                |
| 27. Module: I2C                                                                                                        | 256               | 64               | -                | X                | X                | -                | -                | -                |
| 28. Module: Bus Matrix                                                                                                 | 16/32             | 4/8              |                  |                  |                  |                  |                  |                  |
| 28. Module: Bus Matrix                                                                                                 | 64                | 16               |                  |                  |                  |                  |                  |                  |
| 28. Module: Bus Matrix                                                                                                 | 128               | 32               |                  | Date             | code             |                  | pre-1750         |                  |
| 28. Module: Bus Matrix                                                                                                 | 256               | 16               |                  |                  |                  |                  |                  |                  |
| 28. Module: Bus Matrix                                                                                                 | 256               | 64               |                  |                  |                  |                  |                  |                  |
| 29. Module: ADC                                                                                                        | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 29. Module: ADC                                                                                                        | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 29. Module: ADC                                                                                                        | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 29. Module: ADC                                                                                                        | 256               | 16               | X                | -                | -                | -                | -                | -                |
| 29. Module: ADC                                                                                                        | 256               | 32               | -                | X                | X                | -                | -                | -                |
| 30. Module: USB Low-                                                                                                   | 16/32             | 4/8              | X                | X                | -                | -                | -                | -                |
| 30. Module: USB Low-                                                                                                   | 64                | 16               | X                | X                | -                | X                | X                | X                |
| 30. Module: USB Low-                                                                                                   | 128               | 32               | X                | X                | -                | -                | X                | -                |
| 30. Module: USB Low-                                                                                                   | 256               | 16               | X                | X                | -                | -                | -                | -                |
| 30. Module: USB Low-                                                                                                   | 256               | 32               | -                | X                | X                | -                | -                | -                |

Legend:

An 'X' indicates the issue is present in this revision of silicon; shaded cells with an Em dash ('-') indicate that this silicon revision does not exist for this issue;

blank cells indicate an issue has been corrected in this revision of silicon.

## PIC32MX1XX/2XX 28/36/44-PIN

## Silicon Errata Issues

- Note 1: This document summarizes all silicon errata issues from all revisions of silicon, previous as well as current. The table provided in each issue indicates which issues exist for a particular revision of silicon based on the Flash memory size.
- 2: The following applies to the Affected Silicon Revision tables in each silicon issue:
- An 'X' indicates the issue is present in this revision of silicon.
- Shaded cells with an Em dash ('-') indicate that this silicon revision does not exist for this issue.
- Blank cells indicate an issue has been corrected or does not exist in this revision of silicon.

## 1. Module: Voltage Regulator

Device may not exit the Brown-out Reset (BOR) state if a BOR event occurs.

## Work arounds

## Work around 1:

VDD must remain within the published specification (see parameter DC10 of the device data sheet).

## Work around 2:

Reset the device by providing the Power-on Reset (POR) condition.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         |                           | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         |                           | -                         |                           |                           |                           |
| 128                        | 32                 | X                         |                           | -                         | -                         |                           | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         |                           |                           | -                         | -                         | -                         |

## 2. Module: Oscillator

If the Primary Oscillator (POSC) mode is implemented and a Fail-Safe Clock Monitor (FSCM)  event  occurs  (failure  of the  external primary clock), the internal clock source will switch to the FRC oscillator. Subsequent firmware clock switch  requests from  the FRC oscillator to  other clock sources will fail and the device will continue to execute on the FRC oscillator. On repair of the external clock  source  and  a  power-on  state,  the device  will  resume  operation  with  the  primary oscillator clock source.

## Work around

None.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 3. Module: I 2 C

The  client  address,  0x78,  is  one  of  a  group  of reserved addresses. It is used as the upper byte of a 10-bit address when 10-bit addressing is enabled. The  I 2 C module  control  register  allows the programmer to enable both 10-bit addressing and  strict  enforcement  of  reserved  addressing, with  the  A10M  and  STRICT  bits,  respectively. When  both  bits  are  cleared,  the  device  should respond to the reserved address 0x78, but it does not.

## Work around

None.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 4. Module: USB

If  the bus has been idle for more than 3 ms, the UIDLE interrupt flag is set. If software clears the interrupt flag and the bus remains idle, the UIDLE interrupt flag will not be set again.

## Work around

Software can leave the UIDLE bit set until it has received some indication of bus resumption (i.e., Resume, Reset, SOF, or Error).

| Note:   | Resume and Reset are the only inter - rupts that should be following UIDLE assertion. If the UIDLE bit is set, it should be okay to suspend the USB module (as long as this code is pro - tected by the GUARD and/or ACT - PEND logic). This will require software to clear the UIDLE interrupt enable bit to exit the USB ISR (if using interrupt driven code).   |
|---------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 5. Module: ADC

If  the ADC module is configured to operate at a maximum conversion  rate  of  1.1  Msps,  missing codes are possible every 2 5  codes and the DNL parameter will not be within the published specification.

## Work around

Configure  the  ADC  module  to  operate  for  a maximum conversion rate of 500 ksps.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 6. Module:  ADC

If the ADC module is used in conjunction with the CTMU module in Absolute Capacitive/Time Measurement  mode,  Channel  0  positive  input must remain open (CH0SA&lt;3:0&gt; = 1111 or CH0SB&lt;3:0&gt; = 1111 ) during the calibration step. However,  open  selection  for  Channel  0  positive input  is  not  functional  and connects this input to AVSS.

## Work around

Connect the ADC module to any unused pin and perform the CTMU calibration step. This connection will add a small amount of additional capacitance,  but  will  have  minimal  impact  on overall measurements.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 |                           |                           | -                         |                           |                           |                           |
| 128                        | 32                 |                           |                           | -                         | -                         |                           | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         |                           |                           | -                         | -                         | -                         |

## PIC32MX1XX/2XX 28/36/44-PIN

## 7. Module: ADC

When  the  ADC  module  is  configured  to  start conversion on an external interrupt (SSRC&lt;2:0&gt; = 001 ), the start of conversion always  occurs  on  a  rising  edge  detected  at  the INT0  pin,  even  when  the  INT0  pin  has  been configured  to  generate  an  interrupt  on  a  falling edge (INT0EP = 0 ).

## Work around

Generate ADC  conversion  triggers  on  the  rising edge of the INT0 signal.

Alternately,  use  external  circuitry  to  invert  the signal appearing at the INT0 pin, so that a falling edge  of  the  input  signal  is  detected  as  a  rising edge by the INT0 pin.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 8. Module: PMP

If the PMP module is enabled, any pin with a PMP addressing capability (PMAx) cannot be used as a general purpose output pin,  even  when  the corresponding  PTEN&lt;10:0&gt;  bit  in  the  PMAEN register is cleared. All other functionality on these pins,  including  GPIO  input  functionality  is  not affected.

## Work around

To use a GPIO pin as an output when this pin is shared  with  PMP  addressing  functionality  and PMP is enabled, do the following:

1. Enable  PMP  addressing  by  setting  the corresponding PTEN&lt;10:0&gt; bit in the PMAEN register.
2. Instead of using corresponding LATx registers  to  output  GPIO  data,  use  the PMADDR register.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 9. Module: I/O Ports

When  I2C1  is  enabled,  all  digital  output-only functions and all analog functions on the RA0 and RA1 pins do not function correctly.

Digital output VOH/IOH does not meet the specification in the data sheet and analog signal input loading increases with an increase in applied voltage on any enabled analog function on RA0/ RA1.  If  I2C1  is  enabled,  any  analog  or  digital output-only function enabled on RA0/RA1 will also cause a corresponding 40 mA/pin increase in IDD.

## Work around

Disable slew rate control  of the  I2C1  module  by setting the DISSLW bit (I2C1CON&lt;9&gt;) = 1 .

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         |                           | X                         |                           |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         |                           |                           | -                         | -                         | -                         |

## PIC32MX1XX/2XX 28/36/44-PIN

## 10. Module: CPU

During normal operation, if a CPU write operation is interrupted by an incoming interrupt, it should be aborted  (not  completed)  and  resumed  after  the interrupt is serviced. However, some of these write operations  may  not  be  aborted,  resulting  in  a double write to  peripherals by the CPU (the first write during the interrupt and the second write after the interrupt is serviced).

## Work around

Most peripherals are not affected by this issue, as a  double  write  will  not  have  a  negative  impact. However, the following communication peripherals will  double-send  data  if  their  respective  transmit buffers  are  written  twice:  SPI,  I 2 C,  UART,  and PMP. To avoid double transmission of data, utilize DMA  to  transfer  data  to  these  peripherals  or disable interrupts while writing to these peripherals.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         |                           | X                         |                           |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         |                           |                           | -                         | -                         | -                         |

## PIC32MX1XX/2XX 28/36/44-PIN

## 11. Module: Oscillator

A  clock  signal  is  present  on  the  CLKO  pin, regardless of the clock source and setting of the CLKO Enable Configuration bit,  OSCIOFNC (DEVCFG1&lt;10&gt;), during a Power-on Reset (POR) condition.

## Work around

Do  not  connect  the  CLKO  pin  to  a  device  that would be adversely affected by rapid pin toggling or  a  frequency  other  than  that  defined  by  the oscillator configuration. Do not use the CLKO pin as an input if the device connected to the CLKO pin would be adversely affected by the pin driving a signal out.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 12. Module: Input Capture

All input capture modes selectable by ICM&lt;2:0&gt;, with the exception of Interrupt-only mode, will not work when the CPU enters Idle or Sleep mode.

## Work around

Configure the Input Capture module for Interruptonly mode (ICM&lt;2:0&gt; = 111 ) when the CPU is in Sleep or Idle mode.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 13. Module: Watchdog Timer (WDT)

When  the  Watchdog  Timer  module  is  used  in Windowed mode, the module may issue a reset even if the user tries to clear the module within the allowed window.

## Work around

None.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## PIC32MX1XX/2XX 28/36/44-PIN

## 14. Module: Non-5V Tolerant Pins

When  internal  pull-ups  are  enabled  on  non-5V tolerant pins, the level as measured on the pin and available to external device inputs may not exceed the minimum value of VIH, and therefore qualify as a logic 'high'. However, with respect to the PIC32 device, as long as VDD  3V and the load does not exceed -50 µA, the internal pull-ups are guaranteed  to  be  recognized  as  a  logic  'high' internally to the device.

## Work around

It is recommend to only use external pull-ups:

- To guarantee a logic 'high' for external logic input circuits outside of the PIC32 device
- For PIC32 device inputs, if the external load exceeds -50 µA or VDD &lt; 3V

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         |                           | X                         |                           |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         |                           |                           | -                         | -                         | -                         |

## 15. Module: 5V Tolerant Pins

When internal pull-ups are enabled on 5V tolerant pins,  the  level  as  measured  on  the  pin  and available to external device inputs may not exceed the minimum value of VIH, and therefore qualify as a logic 'high'. However, with respect to the PIC32 device, as long as VDD  3V and the load does not exceed -50 µA, the internal pull-ups are guaranteed  to  be  recognized  as  a  logic  'high' internally to the device.

## Work around

It is recommend to only use external pull-ups:

- To guarantee a logic 'high' for external logic input circuits outside of the PIC32 device
- For PIC32 device inputs, if the external load exceeds -50 µA or VDD &lt; 3V

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 16. Module: I/O Ports

The Open Drain selection (ODCx) on I/O port pins is  not  available  when  the  pin  is  configured  for anything  other  than  a  standard  port  output.  In addition, the Open Drain feature is not available for dedicated  or  remappable  Peripheral  Pin  Select (PPS) output features.

## Work around

None.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         |                           | X                         |                           |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         |                           |                           | -                         | -                         | -                         |

## PIC32MX1XX/2XX 28/36/44-PIN

## 17. Module: I/O Ports

When  the  I2C2  module  is  enabled,  all  digital output-only functions and all analog functions on the RB5 and RB6 pins do not function correctly.

Digital output (VOH/IOH) does not meet the specifications in the data sheet, and analog signal input loading increases with an increase in applied voltage  on  any  enabled  analog  function  on  the RB5 and RB6 pins.  If  the  I2C2  is  enabled,  any analog or digital output-only function enabled on the RB5 and RB6 pins will also cause  a corresponding ~40 mA/pin increase in IDD.

## Work around

Disable the I2C2 module slew rate by setting the DISSLW bit in the I2C2CON register = 1 .

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         |                           | X                         |                           |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         |                           |                           | -                         | -                         | -                         |

## 18. Module: I/O Ports

Certain  functions  are  not  available  when  using PGED3/PGEC3 or PGED4/PGEC4 while in Debug mode.

When using the  PGED3/PGEC3 pins while debugging, these functions are not available:

- VREF+/CVREF+/AN0/C3INC
- VREF-/CVREF-/AN1.

On  44-pin  devices,  when  using  the  PGED4/ PGEC4 pins while debugging, these functions are not available:

- AN6
- AN7

## Work around

Use  either  the  PGED1/PGEC1  pin  pair  or  the PGED2/PGEC2 pin pair for debugging.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 19. Module: UART

During  a  RX  FIFO  overflow  condition,  the  shift register  stops  receiving  data.  This  causes  the UART to lose synchronization with the serial data stream. The only way to recover from this is to turn the  UART  off  and  on  until  it  synchronizes.  This could require several off or on sequences.

## Work arounds

## Work around 1:

Avoid the RX overrun condition by ensuring that the  UARTx module  has  a high enough  interrupt priority such that other peripheral interrupt processing  latencies  do  not  exceed  the  time  to overrun the UART  RX  buffer  based  on the application baud rate. Alternately or in addition to, set  the  URXISEL  bits  in  the  UxSTA  register  to generate  an  earlier  RX  interrupt  based  on  RX FIFO  fill  status  to  buy  more  time  for  interrupt latency processing requirements.

## Work around 2:

If  avoiding  RX  FIFO  overruns  is  not  possible, implement a  ACK/NAK software handshake protocol to repeat  lost packet  transfers after restoring UART synchronization.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## PIC32MX1XX/2XX 28/36/44-PIN

## 20. Module: Timer1

Timer1 will not generate interrupts with an external asynchronous clock input and prescaler other than 1:1.

## Work around

With external clock asynchronous mode, use 1:1 prescaler  mode  with  a  software  timer  overflow variable to keep track of desired equivalent &gt; 1:1 prescaler setting. Alternately, use external synchronous clock mode if this is an option for the application.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 21. Module: Flash Memory

The Program Write Protection (PWP) bits (DEVCFG0&lt;18:10&gt;) are not  enabled  unless  the Boot Write Protect (BWP) bit (DEVCFG0&lt;24&gt; is also enabled (that is = 0 ).

## Work around

## None.

Refer to silicon issues 22, 23, and 24 for related information.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## PIC32MX1XX/2XX 28/36/44-PIN

## 22. Module: Flash Memory

When enabled, the Boot Write Protect (BWP) bit inadvertently also  protects  and  overlaps  the  first page of PWP user program space below 0x0400, (i.e., PWP&lt;8:0&gt; = 0x1FE), in addition to the boot segment, regardless of the state of the Program Write Protection (PWP) bits (DEVCFG0&lt;18:10&gt;). If Boot Write Protect is enabled by setting the BWP bit (DEVCFG0&lt;24&gt;) = 0 , users will not be able to Page Erase or program the first page of the PWP user  program  space.  Only  user  run-time  Page Erase/Program  operations  are  affected,  which does not include a Bulk erase of the entire Flash.

## Work around

None.

Refer to silicon issues 21., 23., and 24. for related information

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         |                           | X                         |                           |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         |                           |                           | -                         | -                         | -                         |

## 23. Module: Flash Memory

The Program Write Protection (PWP) bit field is off by one page relative to the data sheet definition. In silicon,  PWP&lt;8:0&gt;  =  (n  +  1),  where  'n'  is  the DEVCFG0&lt;18:10&gt; value as defined  in  the  data sheet.

## TABLE 6: PWP BITS (DEVCFG0&lt;18:10&gt;)

| Value     | Expected                                 | Actual                                   |
|-----------|------------------------------------------|------------------------------------------|
| 111111111 | Disabled                                 | Disabled                                 |
| 111111110 | Memory below 0x400 is write- protected   | Disabled                                 |
| 111111101 | Memory below 0x800 is write- protected   | Memory below 0x400 is write- protected   |
| • • •     | • • •                                    | • • •                                    |
| 011111111 | Memory below 0x40000 is write- protected | Memory below 0x3FC00 is write- protected |

## Work around

Set  the  PWP&lt;8:0&gt;  bits  (DEVCFG0&lt;18:10&gt;)  = {DEVCFG0&lt;PWP&gt; - 1} to correct for the first page protection offset. Refer to silicon issues 21., 22., and 24. for related information.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         |                           | X                         |                           |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         |                           |                           | -                         | -                         | -                         |

## PIC32MX1XX/2XX 28/36/44-PIN

## 24. Module: Flash Memory

Attempts to protect the entire Flash memory using the following values, will result in no pages being protected.

Program Write Protection bits (DEVCFG0&lt;PWP&gt;):

111101111 = Memory below 0x4000 (16K) address is write-protected.

111011111 = Memory below 0x8000 (32K)

address is write-protected.

110111111 = Memory below 0x10000 (64K) address is write-protected.

101111111 = Memory below 0x20000 (128K) address is write-protected.

## Work around

To protect the entire Flash including the last page, use the following values:

## DEVCFG0&lt;PWP&gt;:

111110000 = Memory below 0x4000 (16K) address is write-protected.

111100000 = Memory below 0x8000 (32K) address is write-protected.

111000000 = Memory below 0x10000 (64K) address is write-protected.

10000000 = Memory below 0x20000 (128K) address is write-protected.

Please refer to silicon issues 21., 22., and 23. for related information.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         |                           | X                         |                           |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         |                           |                           | -                         | -                         | -                         |

## 25. Module: Power-Saving Modes

On exit  from  Sleep  mode,  both  the  SLEEP  and IDLE status bits in the RCON register are set.

## Work around

Add the following code to the user application at the point it wakes from Sleep mode:

```
rcon_var1 = RCON; // ... enter Sleep mode if (rcon_var1 & 0x4) Nop(); // If IDLE bit already set previously // before sleep do nothing else RCONbits.IDLE = 0x0; // If IDLE bit is not set previously // and is after Sleep mode then clear
```

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 26. Module: CTMU

The  CTMU  internal  temperature  sensing  diode does not function for VDD/AVDD operating voltages that are less than 2.5V.

## Work around

None.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         |                           | X                         |                           |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## PIC32MX1XX/2XX 28/36/44-PIN

## 27. Module: I 2 C

When  the  I 2 C  module  is  operating  as  a  client, some reserved bus addresses may be Acknowledged (ACKed) when they should not be Acknowledged (NAKed).

As  a  result,  there  will  be  multiple  data  NAK interrupts until the Stop condition is asserted.

## Work around

When  the  address  interrupt  arrives,  check  the address to determine if it is a reserved address. If the address is a reserved address, set a flag and use the flag to ignore subsequent data interrupts. When the Stop condition occurs, clear the flag.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         |                           | X                         |                           |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 16                 |                           | -                         | -                         | -                         | -                         | -                         |
| 256                        | 64                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 28. Module: Bus Matrix

The  Flash  Memory  Size  register  (BMXPFMSZ) was not programmed with the correct value.

## Work around

Use a fixed number based on the size of the part being used.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                |                           |                           |                           |                           |                           |                           |
| 64                         | 16                 |                           |                           |                           |                           |                           |                           |
| 128                        | 32                 | Date code                 | pre-1750                  | pre-1750                  | pre-1750                  | pre-1750                  | pre-1750                  |
| 256                        | 64                 |                           |                           |                           |                           |                           |                           |
| 256                        | 16                 |                           |                           |                           |                           |                           |                           |

## 29. Module: ADC

Using the ADC to convert the Internal IV REF  Band Gap  1.2V  source  can  result  in  indeterminate behavior. This feature is not supported.

## Work around

None.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 64                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 16                 | -                         | X                         | X                         | -                         | -                         | -                         |

## 30. Module: USB Low-Speed Mode

USB  Low-Speed  mode  is  not  functional  in  both Device  and  Host  modes  due  to  signal  integrity compliance issues.

## Work around

None.

## Affected Silicon Revisions

| Device Flash Memory (KB)   | Data Memory (KB)   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   | Device Silicon Revision   |
|----------------------------|--------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|---------------------------|
| Device Flash Memory (KB)   | Data Memory (KB)   | A0                        | A1                        | A2                        | A3                        | B0                        | B1                        |
| 16/32                      | 4/8                | X                         | X                         | -                         | -                         | -                         | -                         |
| 64                         | 16                 | X                         | X                         | -                         | X                         | X                         | X                         |
| 128                        | 32                 | X                         | X                         | -                         | -                         | X                         | -                         |
| 256                        | 64                 | X                         | -                         | -                         | -                         | -                         | -                         |
| 256                        | 16                 | -                         | X                         | X                         | -                         | -                         | -                         |

## PIC32MX1XX/2XX 28/36/44-PIN

## Data Sheet Clarifications

The following typographic corrections and clarifications are to be noted for the latest version of the device data sheet (DS60001168 K ):

| Note:   | Corrections are shown in bold . Where possible, the original bold text formatting has been removed for clarity.   |
|---------|-------------------------------------------------------------------------------------------------------------------|

The  Data  Sheet  Electrical values for  parameters DC40n, DC40m, and MDC40n are incorrect. The cor -rected values are listed in the following tables:

|        |                    |                                                                                     | Affected Device   | Affected Device   | Affected Device   | Affected Device   | Affected Device   | Affected Device   | Affected Device   | Affected Device   |
|--------|--------------------|-------------------------------------------------------------------------------------|-------------------|-------------------|-------------------|-------------------|-------------------|-------------------|-------------------|-------------------|
| Module | Feature            | Issue Summary                                                                       | Flash             | Data Memory       | Silicon Revision  | Silicon Revision  | Silicon Revision  | Silicon Revision  | Silicon Revision  | Silicon Revision  |
|        |                    |                                                                                     | Memory            |                   | A0                | A1                | A2                | A3                | B0                | B1                |
| Device | Power-Down Current | Data Sheet Electrical values for parameters DC40n, DC40m, and MDC40n are incorrect. | 256               | 16                | X                 | X                 |                   |                   |                   |                   |
| Device | Power-Down Current | Data Sheet Electrical values for parameters DC40n, DC40m, and MDC40n are incorrect. | 256               | 64                |                   | X                 | X                 |                   |                   |                   |

| Param No.   |   Typical |   Max. | Units   | Temp   |
|-------------|-----------|--------|---------|--------|
| DC40n       |       168 |    420 | uA      | +85C   |
| DC40m       |       335 |    840 | uA      | +105C  |
| MDC40n      |       150 |    840 | uA      | +85C   |

## PIC32MX1XX/2XX 28/36/44-PIN

## APPENDIX A: REVISION HISTORY

Rev A Document (10/2011)

Initial  release  of  this  document;  issued  for  revision A0 silicon.

Includes silicon issues 1 (1. Module: Voltage Regulator), 2 (2. Module: Oscillator), 3 (3. Module: I2C), 4 (4. Mod -ule: USB), 5 (5. Module: ADC), 6 (6. Module: ADC), 7 (7. Module: ADC), 8 (8. Module: PMP), and 9 (I/O Ports).

Rev B Document (2/2012)

Added silicon revision A1 for 16/32 KB Flash devices.

Added 64/128 KB Flash devices.

Added silicon issues 10 (10. Module: CPU) and 11 (11. Module: Oscillator Reference:).

Rev C Document (4/2012)

Updated silicon issue 10 (10. Module: CPU).

Added silicon issue 12 (12. Module: Input Capture).

Rev D Document (10/2012)

Updated silicon issue 6 (6. Module: ADC).

Added silicon  issue  13  (13.  Module:  Watchdog  Timer (WDT)).

Updated the note in the Silicon DEVREV Values tables (see Table 1 and Table 2).

Rev E Document (4/2013)

Updated the Device ID for the PIC32MX150F128B in Table 2.

Updated silicon issue 9 (9. Module: I/O Ports).

Added silicon issues 14 (14. Module: Non-5V Tolerant Pins) and 15 (15. Module: 5V Tolerant Pins).

Added data sheet clarification 1 ( The following typo -graphic corrections and clarifications are to be noted for the latest version of the device data sheet (DS60001168K):).

Rev F Document (6/2014)

Updated  Device  ID  values  in  Table 1,  Table 2,  and Table 3.

Added Silicon DEVREV  Values for Devices with 256 KB Flash (see Table 3).

Removed Data Sheet Clarification 1.

Updated silicon issue 9 (9. Module: I/O Ports).

Added  silicon  issues  16  (16.  Module:  I/O  Ports),  17 (17. Module: I/O Ports), 18 (18. Module: I/O Ports), 19 (19. Module: UART), 20 (20. Module: Timer1), 21 (21. Module: Flash Memory), 22 (22. Module: Flash Mem -ory),  23 (23.  Module:  Flash  Memory),  and  24  (24. Module: Flash Memory).

Rev G Document (4/2015)

Updated  the  Rev  A3  Silicon  DEVREV  Values  for Devices with 64/128 KB Flash in Table 2.

Updated the title of Table 3 to:  Silicon  DEVREV Values  for  Devices  with  256  KB  Flash  and  64  KB RAM.

Added  Silicon  DEVREV  Values  for  Devices  with  256 KB Flash and 16 KB RAM (Table 4).

Updated  the  Affected  Revisions  for  256  KB  Flash Memory  devices  in Table 5 and  in silicon issues 22 (22.  Module:  Flash  Memory)  and  23 (23.  Module: Flash Memory).

Updated issue 9 (9. Module: I/O Ports).

Added  Silicon  Issues  25  (25.  Module:  Power-Saving Modes) and 26 (26. Module: CTMU).

Added Data Sheet Clarification 1 (Power-Down Current (IPD)).

Rev H Document (4/2016)

Removed Data Sheet Clarification 1.

Added the PIC32MX270FDB device to Silicon DEVREV Values for  Devices  with  256  KB  Flash  and 64 KB RAM.

Added silicon issue 27 (27. Module: I2C).

Rev J Document (5/2018)

Added silicon issue 28 (28. Module: Bus Matrix).

Rev. K Document (08/2018)

Added silicon issue 29 (ADC).

Rev. L Document (08/2019)

Corrected Typographical Errors.

Updated silicon issue 29 (29. Module: ADC)

## Rev. M Document (11/2019)

Added  Silicon  Issue  30.  Module:  USB  Low-Speed Mode

## Rev. N Document (04/2020)

Updated  the  verbiage  for  silicon  issue  30.  Module: USB Low-Speed Mode

## Rev. P Document (02/2021)

Added silicon revision B0 for 64 KB Flash devices.

Updated silicon issues 3. Module: I2C and 27. Module: I2C to replace the term "slave" with "client.'

Replaced  the  term  "Parallel  Master  Port"  with "PMP"

## Rev. Q Document (08/2021)

Added Silicon revision B1 to  TABLE 2: Silicon DEVREV Values  for  Devices  with  64/128  KB  Flash, and TABLE 5: Silicon Issue Summary.

## Rev. R Document (04/2026)

Added  Data Sheet Clarifications for Data  Sheet Electrical Values.

## PIC32MX1XX/2XX 28/36/44-PIN

NOTES:

## Microchip Information

## Trademarks

The 'Microchip' name and logo, the 'M' logo, and other names, logos, and brands are registered and unregistered trademarks of Microchip Technology Incorporated or its affiliates and/or subsidiaries in the United States and/or other countries ('Microchip Trademarks'). Information regarding Microchip Trademarks can be found at https://www.microchip.com/en-us/about/legalinformation/microchiptrademarks.

ISBN: 979-8-3371-3223-5

## Legal Notice

This publication and the information herein may be used only with Microchip products, including to design, test, and integrate Microchip products with your application. Use of this information in any other manner violates these terms. Information regarding device applications is provided only for your convenience and may be superseded by updates. It is your responsibility to ensure that your application meets with your specifications. Contact your local Microchip sales office for additional support or, obtain additional support at www.microchip.com/en-us/support/design-help/client-support-services.

THIS INFORMATION IS PROVIDED BY MICROCHIP "AS IS". MICROCHIP MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND WHETHER EXPRESS OR IMPLIED, WRITTEN OR ORAL, STATUTORY OR OTHERWISE, RELATED TO THE INFORMATION INCLUDING BUT NOT LIMITED TO ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE, OR WARRANTIES RELATED TO ITS CONDITION, QUALITY, OR PERFORMANCE.

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL, OR CONSEQUENTIAL LOSS, DAMAGE, COST, OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE INFORMATION OR ITS USE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THE INFORMATION OR ITS USE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THE INFORMATION.

Use of Microchip devices in life support and/or safety applications is entirely at the buyer's risk, and the buyer agrees to defend, indemnify and hold harmless Microchip from any and all damages, claims, suits, or expenses resulting from such use. No licenses are conveyed, implicitly or otherwise, under any Microchip intellectual property rights unless otherwise stated.

## Microchip Devices Code Protection Feature

Note the following details of the code protection feature on Microchip products:

- Microchip products meet the specifications contained in their particular Microchip Data Sheet.
- Microchip believes that its family of products is secure when used in the intended manner, within operating specifications, and under normal conditions.
- Microchip values and aggressively protects its intellectual property rights. Attempts to breach the code protection features of Microchip product is strictly prohibited and may violate the Digital Millennium Copyright Act.
- Neither Microchip nor any other semiconductor manufacturer can guarantee the security of its code. Code protection does not mean that we are guaranteeing the product is 'unbreakable'. Code protection is constantly evolving. Microchip is committed to continuously improving the code protection features of our products.