![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000000_43fc7d81ce369561194547f02fc29dea44158d510a76f3e7fe9e48f320c938d7.png)

## PIC32 Bluetooth Audio Development Kit Reference Guide

## Note the following details of the code protection feature on Microchip devices:

- Microchip products meet the specification contained in their particular Microchip Data Sheet.
- Microchip believes that its family of products is one of the most secure families of its kind on the market today, when used in the intended manner and under normal conditions.
- There are dishonest and possibly illegal methods used to breach the code protection feature. All of these methods, to our knowledge, require using the Microchip products in a manner outside the operating specifications contained in Microchip's Data Sheets. Most likely, the person doing so is engaged in theft of intellectual property.
- Microchip is willing to work with the customer who is concerned about the integrity of their code.
- Neither Microchip nor any other semiconductor manufacturer can guarantee the security of their code. Code protection does not mean that we are guaranteeing the product as 'unbreakable.'

Code protection is constantly evolving. We at Microchip are committed to continuously improving the code protection features of our products. Attempts to break Microchip's code protection feature may be a violation of the Digital Millennium Copyright Act. If such acts allow unauthorized access to your software or other copyrighted work, you may have a right to sue for relief under that Act.

Information  contained  in  this  publication  regarding  device applications and the like is provided only for your convenience and may be superseded by updates. It is your responsibility to ensure  that  your  application  meets  with  your  specifications. MICROCHIP MAKES NO REPRESENTATIONS OR WARRANTIES  OF  ANY  KIND  WHETHER  EXPRESS  OR IMPLIED, WRITTEN OR ORAL, STATUTORY OR OTHERWISE, RELATED TO THE INFORMATION, INCLUDING  BUT  NOT  LIMITED  TO  ITS  CONDITION, QUALITY, PERFORMANCE, MERCHANTABILITY OR FITNESS  FOR  PURPOSE . Microchip  disclaims  all  liability arising  from  this  information  and  its  use.  Use  of  Microchip devices in life support and/or safety applications is entirely at the buyer's risk, and the buyer agrees to defend, indemnify and hold  harmless  Microchip  from  any  and  all  damages,  claims, suits,  or  expenses  resulting  from  such  use.  No  licenses  are conveyed, implicitly or otherwise, under any Microchip intellectual property rights.

## QUALITY MANAGEMENT  SYSTEM CERTIFIED BY DNV

== ISO/TS 16949 ==

## Trademarks

The Microchip name and logo, the Microchip logo, dsPIC, FlashFlex, KEELOQ, KEELOQ logo, MPLAB, PIC, PICmicro, PICSTART, PIC 32  logo, rfPIC, SST, SST Logo, SuperFlash and UNI/O are registered trademarks of Microchip Technology Incorporated in the U.S.A. and other countries.

FilterLab, Hampshire, HI-TECH C, Linear Active Thermistor, MTP, SEEVAL and The Embedded Control Solutions Company are registered trademarks of Microchip Technology Incorporated in the U.S.A.

Silicon Storage Technology is a registered trademark of Microchip Technology Inc. in other countries.

Analog-for-the-Digital Age, Application Maestro, BodyCom, chipKIT, chipKIT logo, CodeGuard, dsPICDEM, dsPICDEM.net, dsPICworks, dsSPEAK, ECAN, ECONOMONITOR, FanSense, HI-TIDE, In-Circuit Serial Programming, ICSP, Mindi, MiWi, MPASM, MPF, MPLAB Certified logo, MPLIB, MPLINK, mTouch, Omniscient Code Generation, PICC, PICC-18, PICDEM, PICDEM.net, PICkit, PICtail, REAL ICE, rfLAB, Select Mode, SQI, Serial Quad I/O, Total Endurance, TSHARC, UniWinDriver, WiperLock, ZENA and Z-Scale are trademarks of Microchip Technology Incorporated in the U.S.A. and other countries.

SQTP is a service mark of Microchip Technology Incorporated in the U.S.A.

GestIC and ULPP are registered trademarks of Microchip Technology Germany II GmbH &amp; Co. KG, a subsidiary of Microchip Technology Inc., in other countries.

All other trademarks mentioned herein are property of their respective companies.

© 2013-2014, Microchip Technology Incorporated, Printed in the U.S.A., All Rights Reserved.

Printed on recycled paper.

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000001_0f914d32a74c9598bad900d83d0283abac4e9f3c14bca5dc9dfe4589df215462.png)

ISBN: 978-1-63276-244-3

Microchip received ISO/TS-16949:2009 certification for its worldwide headquarters, design and wafer fabrication facilities in Chandler and Tempe, Arizona; Gresham, Oregon and design centers in California and India. The Company's quality system processes and procedures are for its PIC ®  MCUs and dsPIC ®  DSCs, KEELOQ ®  code hopping devices, Serial EEPROMs, microperipherals, nonvolatile memory and analog products. In addition, Microchip's quality system for the design and manufacture of development systems is ISO 9001:2000 certified.

## Object of Declaration: DV320032, PIC32 Bluetooth ®  Audio Development Kit

## EU Declaration of Conformity

This declaration of conformity is issued by the manufacturer.

This development/evaluation tool complies with EU RoHS2 Directive 2011/65/EU.

The development/evaluation tool is designed to be used for research and development in a laboratory environment.  This development/evaluation tool is not a Finished Appliance, nor is it intended for incorporation into Finished Appliances that are made commercially available as single functional units to end users under EU EMC Directive 2004/108/EC and as supported by the European Commission's Guide for the EMC Directive 2004/108/EC February 2010). (8th

For information regarding the exclusive, limited warranties applicable to Microchip products, please see Microchip's standard terms and conditions of sale, which are printed on our sales documentation and available at wwwmicrochip com

Signed for and on behalf of Microchip Technology Inc. at Chandler, Arizona, USA

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000002_cc25aaa80608f0f27a8e0d991fa857523cc7cf6f9e43b3087100f00519f470f5.png)

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000003_169a01235c08511ac9eb9b0697d5c6189f084f2d54291dff3540cfe5acad0b98.png)

Derek Carlson VP Development Tools

Date

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

NOTES:

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000004_ba9ea31cefb70a00a46f7100709454aa42df494f6e85a002b8f7a056a1675999.png)

Microchip

## Table of Contents

| Chapter 1. Introduction                                                                                          | Chapter 1. Introduction                                                                                                                                          |
|------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|                                                                                                                  | 1.1 Feature Overview ......................................................................................... 15                                                |
|                                                                                                                  | 1.2 Qualification .................................................................................................. 16                                          |
|                                                                                                                  | 1.3 Kit Contents .................................................................................................. 17                                           |
|                                                                                                                  | 1.4 Features ....................................................................................................... 18                                          |
|                                                                                                                  | 1.5 PIC32 Bluetooth Audio Development Kit Functionality and Features .......... 20                                                                               |
| Board                                                                                                            | 1.6 Bluetooth Module Daughter Functionality and Features .................... 22                                                                                 |
| 1.7 Board Functionality                                                                                          | Audio DAC Daughter and Features .............................. 23                                                                                                |
| Chapter 2. Hardware and Software Performance                                                                     | Chapter 2. Hardware and Software Performance                                                                                                                     |
|                                                                                                                  | 2.1 Hardware and Software SBC Audio Quality Performance ........................... 25                                                                           |
| Chapter 3. Interoperability Testing Results                                                                      | Chapter 3. Interoperability Testing Results                                                                                                                      |
|                                                                                                                  | 3.1 Test Conditions Summary ............................................................................ 29                                                      |
|                                                                                                                  | 3.2 Microchip Bluetooth Compatibility Test Matrix ............................................. 29                                                               |
|                                                                                                                  | 3.3 Bluetooth AVRCP Functions ........................................................................ 32                                                        |
| Chapter 4. Bluetooth Audio Demonstrations                                                                        | Chapter 4. Bluetooth Audio Demonstrations                                                                                                                        |
| and                                                                                                              | 4.1 Demonstrations, Hex Files, Voice Prompt Programmer (Free-of-Charge) ..................................................................................... 33 |
|                                                                                                                  | 4.2 Demonstration Suites (For a Fee) ................................................................ 33                                                         |
|                                                                                                                  | 4.3 Demonstration Suites and Resource Requirements .................................... 34                                                                       |
|                                                                                                                  | 4.4 Demonstration Overviews ............................................................................ 38                                                      |
| Chapter 5. Bluetooth Stack Overview                                                                              | Chapter 5. Bluetooth Stack Overview                                                                                                                              |
|                                                                                                                  | 5.1 Bluetooth Stack Block Diagram .................................................................... 43                                                        |
|                                                                                                                  | 5.2 Bluetooth Stack Component Summary ........................................................ 44                                                                |
| Appendix A. Hardware, Board Layout, and Schematics                                                               | Appendix A. Hardware, Board Layout, and Schematics                                                                                                               |
|                                                                                                                  | A.1 Wire List ....................................................................................................... 45                                         |
| A.2                                                                                                              | Daughter Board Pin Mapping ....................................................................... 48                                                            |
| A.3 Block Diagram .............................................................................................. | 49                                                                                                                                                               |
|                                                                                                                  | A.4 Board Layout ................................................................................................ 50                                             |
| A.5 Schematics                                                                                                   | .................................................................................................. 53                                                            |
| Appendix B. Bill of Materials (BOM)                                                                              | Appendix B. Bill of Materials (BOM)                                                                                                                              |
|                                                                                                                  | B.1 PIC32 Bluetooth Audio Development Kit Bill of Materials ............................ 59                                                                      |
| Appendix C. Frequently Asked Questions (FAQ)                                                                     | .....................................................65                                                                                                          |

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

NOTES:

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000005_22e6691812de863e7de1f7d1d828351bb7b9bb98aa2e7e62c189feac623ffd98.png)

Microchip

## Preface

## NOTICE TO CUSTOMERS

All documentation becomes dated, and this manual is no exception. Microchip tools and documentation are constantly evolving to meet customer needs, so some actual dialogs and/or tool descriptions may differ from those in this document. Please refer to our web site (www.microchip.com) to obtain the latest documentation available.

Documents are identified with a 'DS' number. This number is located on the bottom of each page, in front of the page number. The numbering convention for the DS number is 'DSXXXXXXXXA', where 'XXXXXXXX' is the document number and 'A' is the revision level of the document.

For the most up-to-date information on development tools, see the MPLAB ®  IDE online help. Select the Help menu, and then Topics to open a list of available online help files.

## INTRODUCTION

This chapter contains general information that will be useful to know before using the PIC32 Bluetooth Audio Development Kit. Items discussed in this chapter include:

- Document Layout
- Conventions Used in this Guide
- Recommended Reading
- The Microchip Web Site
- Development Systems Customer Change Notification Service
- Customer Support
- Document Revision History

## DOCUMENT LAYOUT

This document describes how to use the PIC32 Bluetooth Audio Development Kit as a development tool to emulate and debug firmware on a target board. This document includes the following chapters:

- Chapter 1. 'Introduction' provides a brief overview of the development kit, highlighting its features and uses.
- Chapter 2. 'Hardware and Software Performance' provides the hardware descriptions of the development kit.
- Chapter 3. 'Interoperability Testing Results' provides a manufacturer test result matrix for the development kit.
- Chapter 4. 'Bluetooth Audio Demonstrations' describes the available demonstrations for the development kit.
- Chapter 5. 'Bluetooth Stack Overview' provides a brief overview of the Microchip Bluetooth Stack for PIC32, which is used by the development kit.

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

- Appendix A. 'Hardware, Board Layout, and Schematics' provides a block diagram, board layout, and detailed schematics of the development kit.
- Appendix B. 'Bill of Materials (BOM)' provides the bill of material descriptions and the reference, manufacturer, and part numbers for the components used in the development kit hardware.
- Appendix C. 'Frequently Asked Questions (FAQ)' provides questions and answers for common issues that may be encountered while using the development kit.

## CONVENTIONS USED IN THIS GUIDE

This manual uses the following documentation conventions:

## DOCUMENTATION CONVENTIONS

| Description                                      | Represents                                                                                                                                                                                                                                                                                           | Examples                                                           |
|--------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------|
| Italic characters                                | Referenced books                                                                                                                                                                                                                                                                                     | MPLAB IDE User's Guide                                             |
| Italic characters                                | Emphasized text                                                                                                                                                                                                                                                                                      | ...is the only compiler...                                         |
| Initial caps                                     | A window                                                                                                                                                                                                                                                                                             | the Output window                                                  |
| Initial caps                                     | A dialog                                                                                                                                                                                                                                                                                             | the Settings dialog                                                |
| Initial caps                                     | A menu selection                                                                                                                                                                                                                                                                                     | select Enable Programmer                                           |
| Quotes                                           | A field name in a window or dialog                                                                                                                                                                                                                                                                   | 'Save project before build'                                        |
| Underlined, italic text with right angle bracket | A menu path                                                                                                                                                                                                                                                                                          | File > Save                                                        |
| Bold characters                                  | A dialog button                                                                                                                                                                                                                                                                                      | Click OK                                                           |
| Bold characters                                  | A tab                                                                                                                                                                                                                                                                                                | Click the Power tab                                                |
| Text in angle brackets < >                       | A key on the keyboard                                                                                                                                                                                                                                                                                | Press <Enter>, <F1>                                                |
| Plain Courier New                                | Sample source code                                                                                                                                                                                                                                                                                   | #define START                                                      |
| Plain Courier New                                | Filenames                                                                                                                                                                                                                                                                                            | autoexec.bat                                                       |
| Plain Courier New                                | File paths                                                                                                                                                                                                                                                                                           | c:\mcc18\h                                                         |
| Plain Courier New                                | Keywords                                                                                                                                                                                                                                                                                             | _asm, _endasm, static                                              |
| Plain Courier New                                | Command-line options                                                                                                                                                                                                                                                                                 | -Opa+, -Opa-                                                       |
| Plain Courier New                                | Bit values                                                                                                                                                                                                                                                                                           | 0, 1                                                               |
| Plain Courier New                                | Constants                                                                                                                                                                                                                                                                                            | 0xFF, 'A'                                                          |
| Italic Courier New                               | A variable argument                                                                                                                                                                                                                                                                                  | file .o , where file can be any valid filename                     |
| Square brackets [ ]                              | Optional arguments                                                                                                                                                                                                                                                                                   | mcc18 [options] file [options]                                     |
| Curly brackets and pipe character: { &#124; }    | Choice of mutually exclusive arguments; an OR selection                                                                                                                                                                                                                                              | errorlevel {0&#124;1}                                              |
| Ellipses...                                      | Replaces repeated text                                                                                                                                                                                                                                                                               | var_name [, var_name...]                                           |
| Ellipses...                                      | Represents code supplied by user                                                                                                                                                                                                                                                                     | void main (void) { ... }                                           |
| Notes                                            | A Note presents information that we want to re-emphasize, either to help you avoid a common pitfall or to make you aware of operating differences between some device family members. A Note can be in a box, or when used in a table or figure, it is located at the bottom of the table or figure. | Note: This is a standard note box. CAUTION This is a caution note. |
| Notes                                            | A Note presents information that we want to re-emphasize, either to help you avoid a common pitfall or to make you aware of operating differences between some device family members. A Note can be in a box, or when used in a table or figure, it is located at the bottom of the table or figure. | Note 1: This is a note used in a                                   |
| Notes                                            | A Note presents information that we want to re-emphasize, either to help you avoid a common pitfall or to make you aware of operating differences between some device family members. A Note can be in a box, or when used in a table or figure, it is located at the bottom of the table or figure. | table.                                                             |
| Notes                                            | A Note presents information that we want to re-emphasize, either to help you avoid a common pitfall or to make you aware of operating differences between some device family members. A Note can be in a box, or when used in a table or figure, it is located at the bottom of the table or figure. |                                                                    |

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## RECOMMENDED READING

This document describes how to use the starter kit. The following Microchip documents are available and recommended as supplemental reference resources.

## Bluetooth Audio Development Kit Readme File

For the latest information on using the development kit and its related demonstrations, please consult the Readme file provided in the installation directory. The Readme file contains information on revision updates and known issues that may not be included in this reference guide.

## PIC32MX330/350/370/430/450/470 Family Data Sheet (DS60001185)

Consult this document for detailed information on PIC32 devices. Reference information found in this data sheet includes:

- Device memory maps
- Device pinout and packaging details
- Device electrical specifications
- List of peripherals included on the devices

## Section 27. 'USB On-The-Go' (DS61126)

This section of the 'PIC32 Family Reference Manual' provides a detailed description and overview of the functionality of the USB OTG module.

## Microchip Bluetooth ®  Stack for PIC32

This application note provides information on the Application Programming Interfaces for various profiles, protocols, and decoders available in the Bluetooth Stack for PIC32 devices.

Note: Please contact Microchip Marketing for information regarding this document.

## MPLAB ®  XC32 C/C++ Compiler User's Guide (DS50001686)

This document details the use of Microchip's MPLAB XC32 C/C++ Compiler to develop an application.

## MPLAB ®  X IDE User's Guide (DS50002027)

Refer to this document for more information pertaining to the installation and implementation of the MPLAB X IDE software, as well as the MPLAB SIM Simulator software that is included with it.

## PICkit™ 3 In-Circuit Debugger/Programmer User's Guide (DS50002116)

This document describes how to use the PICkit 3 as a development tool to emulate and debug firmware on a target board, as well as how to program devices.

## PICkit 3 In-Circuit Debugger/Programmer Online Help File

A comprehensive help file for the debugger is included with MPLAB X IDE. Usage, troubleshooting and hardware specifications are covered. This may be more up-to-date than the printed documentation. Also, limitations are listed for various devices.

## THE MICROCHIP WEB SITE

Microchip provides online support via our web site at: http://www.microchip.com. This web site makes files and information easily available to customers. Accessible by most Internet browsers, the web site contains the following information:

- Product Support - Data sheets and errata, application notes and sample programs, design resources, user's guides and hardware support documents, latest software releases and archived software
- General Technical Support - Frequently Asked Questions (FAQs), technical support requests, online discussion groups, Microchip consultant program member listings
- Business of Microchip - Product selector and ordering guides, latest Microchip press releases, listings of seminars and events; and listings of Microchip sales offices, distributors and factory representatives

## DEVELOPMENT SYSTEMS CUSTOMER CHANGE NOTIFICATION SERVICE

Microchip's customer notification service helps keep customers current on Microchip products. Subscribers will receive e-mail notification whenever there are changes, updates, revisions or errata related to a specified product family or development tool of interest.

To register, access the Microchip web site at www.microchip.com, click on Customer Change Notification and follow the registration instructions.

The Development Systems product group categories are:

- Compilers - The latest information on Microchip C compilers and other language tools
- Emulators - The latest information on the Microchip in-circuit emulator, MPLAB REAL ICE™
- In-Circuit Debuggers - The latest information on the Microchip in-circuit debugger, MPLAB ICD 3
- MPLAB X IDE - The latest information on Microchip MPLAB X IDE, the Windows ®  Integrated Development Environment for development systems tools
- Programmers - The latest information on Microchip programmers including the PICkit™ 3 development programmer

## CUSTOMER SUPPORT

Users of Microchip products can receive assistance through several channels:

- Distributor or Representative
- Local Sales Office
- Field Application Engineer (FAE)
- Technical Support

Customers should contact their distributor, representative or field application engineer (FAE) for support. Local sales offices are also available to help customers. A listing of sales offices and locations is included in the back of this document.

Technical support is available through the web site at: http://support.microchip.com

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## DOCUMENT REVISION HISTORY

## Revision A (September 2013)

This is the initial release of the PIC32 Bluetooth ®  Audio Development Kit Reference Guide.

## Revision B (February 2014)

This revision includes the following updates:

- Changes in support of Release v2.0 of the demonstration software:
- -1.1 'Feature Overview' was updated to include the PIC32MX470F512L)
- -Updated the Bluetooth Qualified Design Information (see Table 1-1)
- -Removed Figure 1-1
- -Updated the Note in 1.3 'Kit Contents'
- -Updated item 2 and the note for item 16 in 1.5 'PIC32 Bluetooth Audio Development Kit Functionality and Features'
- -Updated Demonstrations and Resource Requirements (see Table 4-1)
- -Updated the Demonstration Features Map (see Table 4-3)
- -Updated 4.1.1 'Basic Functionality'
- -Updated the Bluetooth/USB Audio Demonstration 2.5 Controls (see Table 4-4)
- -Updated 4.1.3.1 'Bluetooth USB Demonstration 2.5 Switch Descriptions'
- -Updated the Bluetooth/USB Audio Demonstration 2.5A Controls (see Table 4-5)
- -Updated 4.2.3.1 'Bluetooth USB Demonstration 2.5A Switch Descriptions'
- -Updated the Bluetooth/USB Audio Demonstration 3 Controls (see Table 4-6)
- -Updated 4.3.3.1 'Bluetooth Demonstration 3 Switch Descriptions'
- -Updated the Bluetooth/USB Audio Demonstration 5 Controls (see Table 4-8)
- -Updated 4.5.2.1 'Bluetooth Demonstration 5 Switch Descriptions'
- -Updated the Bluetooth Stack Profile Versions (see Table 5-1)
- -Updated the Demonstration Feature Enhancement Schedule (see Table C-1)
- Added 4.6 'Bluetooth/USB Audio Demonstration 6 with AAC'
- Added 4.7 'Bluetooth Audio Demonstration 7 with SBC and EQ'
- Appendix A. 'Hardware, Board Layout, and Schematics' updates:
- -Added Development Kit Wire List (see Table A-1)
- -Added Daughter Board Pin Mapping (see Table A-2)
- Appendix D. 'Frequently Asked Questions (FAQ)' updates:
- -Added Answer 3 to Question 8
- -Added Answer 2 to Question 12
- -Added Question 15

## Revision C (May 2014)

This revision includes the following updates.

- Demonstration details were removed and relocated to individual 'ReadMe' files, which are included when downloading the demonstrations
- 4.2 'Bluetooth/USB Audio Demonstration 2.5A With SBC And Apple Support' was removed
- Added 4.4.7 'Bluetooth/USB Audio Demonstration 8 with SBC'
- Added 4.4.8 'Bluetooth Audio Demonstration 9 With SBC and Break-In/Party mode'
- Added 4.4.9 'Bluetooth/USB Audio Demonstration 10 With SBC and Break-In/Party Mode'
- Added 4.4.10 'Bluetooth Audio Demonstration 11 With SBC and SPP'
- Appendix C 'Planned Enhancements' was removed

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

NOTES:

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000006_22e6691812de863e7de1f7d1d828351bb7b9bb98aa2e7e62c189feac623ffd98.png)

Microchip

## Chapter 1.  Introduction

Thank you for purchasing the Microchip Technology PIC32 Bluetooth Audio Development Kit (P/N: DV320032). This development kit provides a complete turn-key solution to develop Bluetooth ®  A2DP audio streaming solutions and applications.

The development board included in the kit is coupled with two daughter boards: the Bluetooth HCI Radio Daughter Board that demonstrates a low cost Bluetooth implementation, and the Audio DAC Daughter Board that demonstrates a high-quality 16-bit/24-bit, 32-192 kHz audio conversion/amplification for line-out or headphones. The kit ships with demonstration code that enables wireless streaming digital audio from any Bluetooth enabled smartphone or portable music player or over USB.

## 1.1 FEATURE OVERVIEW

Key features include:

- PIC32MX450F256L, 100 MHz, 256 KB Flash, 64 KB RAM (v1.0 and v2.0 development boards)
- PIC32MX470F512L, 100 MHz, 512 KB Flash, 128 KB RAM (v3.0 development board only)
- HCI Bluetooth Module Daughter Board (QDID certified module)
- 16-bit/24-bit, 32-192 kHz DAC/Amp Daughter Board
- USB Host/Device audio support
- USB charging
- Two-inch color LCD
- Headphone/Line-out
- Audio control function
- Bluetooth/USB audio software support for:
- -Apple ®
- -Samsung ®  audio
- Google™
- -Android™ Open Accessory (AOA)
- -Bluetooth audio with SBC and AAC decode
- -Bluetooth Stack QDID certified

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## 1.1.1 Bluetooth Demonstrations

Currently, many Bluetooth demonstrations are available and many are planned in the near future. We are continually adding many features and functionality. Some Bluetooth software demonstrations also have functionality for USB audio streaming, and these demonstrations include support for AOA and Samsung data formats.

If USB audio support for Apple is required, support for the Bluetooth Stack (A2DP + AVRCP + SPP + SBC decoder) with Android Open Accessory + Apple MFi stack + Apple iAP Type-A USB connection, plus Samsung audio mini-B USB connection is available. The device requires a iAP/MFi software component and special hardware acquired through the Apple MFi program. For more information about the required hardware, contact applesupport@microchip.com. To obtain software for this and other Microchip hardware, which is enabled for Apple products, contact your Microchip sales office. Refer to Figure 1-1 for a diagram that shows several Apple controlled hardware solutions that are compatible with the development kit.

The PIC32 Bluetooth Audio Development Kit supports the PICkit™ 3 In-Circuit Debugger/Programmer for full emulation, programming and debugging capabilities.

See Chapter 4. 'Bluetooth Audio Demonstrations' for full details on which demonstrations are offered free-of-charge, and which are available for purchase. In addition, feature tables and brief overviews of each demonstration are provided. Information on configuring and running each demonstration is provided is a separate 'ReadMe' file (available from www.microchip.com/pic32btsuites).

## 1.2 QUALIFICATION

As of February 1, 2014, the Bluetooth Special Interest Group (Bluetooth SIG) has changed the qualification process for all new products. Microchip will continue to provide Qualified Design ID (QDID) for selected components within the design (see Table 1-1). The new Declaration ID will be required by each manufacturer. Previous Qualified Design Listing (QDL) and End Product Listing (EPL) processes are no longer necessary. For more information, please contact your local Microchip sales office, or refer to the training materials provided by the Bluetooth SIG at:

https://www.bluetooth.org/en-us/test-qualification/qualification-overview/listing-process-updates

TABLE 1-1: BLUETOOTH QUALIFIED DESIGN INFORMATION

| Component                                                                                                                                                    | QDID (See Note 1 )   |
|--------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------|
| Bluetooth Radio Module FLC-BTM805B https://www.bluetooth.org/tpg/listings.cfm (Search: B017701) https://www.bluetooth.org/tpg/EPL_Detail.cfm?ProductID=25584 | B017701              |
| Bluetooth Stack v1.0.3 (RTOS Version) https://www.bluetooth.org/tpg/QLI_viewQDL.cfm?qid=21350                                                                | B021350              |
| Bluetooth Stack v.2.0 (non-RTOS Version)                                                                                                                     | See Note 2           |

Note  1: The Bluetooth software stack and Bluetooth radio module hardware on the PIC32 Bluetooth Audio Development Kit are available from Microchip. Changes to the architecture may require a unique QDID for an end product and additional testing by the customer. All new products require a Declaration ID available only from the Bluetooth SIG.

- 2: New rules by the Bluetooth SIG are scheduled to be active in February 2014. The QDID for the v2.0 Bluetooth Stack will comply with these revised standards.

## 1.3 KIT CONTENTS

The PIC32 Bluetooth Audio Development Kit (P/N: DV320032) contains the following items:

- PIC32 Bluetooth Audio Development Board (P/N: DM320032)
- Bluetooth HCI Radio Module Daughter Board (P/N: AC320032-1)
- 24-bit stereo DAC Line-out/Headphone Amplifier Daughter Board (P/N: AC320032-2)
- On-board preprogrammed PIC32MX450F256L or PIC32MX470F512L (see the following Note )

Note: The PIC32 Bluetooth Audio Development Kit has been designed to function with a PIC32MX470F512L device at location U1. However, initial units will be shipped with U1 populated with a PIC32MX450F256L until the PIC32MX470F512L devices are available.

For those customers who have a development kit populated with the PIC32MX450F256L, you can update to the new device using a PIC32MX470F512L Plug-in Module (PIM) mounted on the U1A headers. When using the PIM, ensure that switch S2 is in the PIM\_MCLR position. When not using the PIM, ensure that switch S2 is in the default PIC32\_MCLR position.

All demonstrations with the exception of Bluetooth/USB Audio Demonstration 6 will operate on PIC32MX450F256L. It is important to note that the PIC32MX470F512L is required for Bluetooth/USB Audio Demonstration 6 with AAC , which incorporates the AAC decoder in addition to Bluetooth SBC and USB audio functions.

## 1.4 FEATURES

Figure 1-1 illustrates the features of the PIC32 Bluetooth Audio Development Kit.

FIGURE 1-1: PIC32 BLUETOOTH AUDIO DEVELOPMENT KIT LAYOUT AND FEATURES

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000007_3c9d70aa3a61320506b0a100550ddc466fbc0872cc1fad8d25d38e911f81db0f.png)

## 1.4.1 Development Kit Block Diagram

Figure 1-2 shows a block diagram of the PIC32 Bluetooth Audio Development Kit.

FIGURE 1-2: PIC32 BLUETOOTH AUDIO DEVELOPMENT KIT BLOCK DIAGRAM

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000008_ac1c4bfaa92a43bcabed79794aaebf25ba2975f35e547617c407d6d3c74750c7.png)

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## 1.5 PIC32 BLUETOOTH AUDIO DEVELOPMENT KIT FUNCTIONALITY AND FEATURES

The PIC32 Bluetooth Audio Development Kit has the following key features, as indicated by the corresponding number in Figure 1-3:

1. PIC32MX450F256L or PIC32MX470F512L 32-bit microcontroller (U1).
2. Hardware selection of PIM or board mounted microcontroller (S1). At all times, S1 should be in the 'down' position towards the display, except when a PIM is installed
3. 6.9V to 14V DC power input (provides +3.3V and +5V (regulated) to kit hardware.
4. Overvoltage protection circuit (14.4V trip) and fused overcurrent protection.
5. Power indicator LED (D13).
6. USB Type-A connector to support USB audio and MP3.
7. USB 3:1 multiplexer (for connecting the PIC32 device to a Type-A, mini-B, or off-board USB source).
8. USB charge management IC (auto-negotiating USB quick charge current up to 2.5 amps and support for simultaneous audio and charging).
9. 2" 176 x 220 RGB graphics TFT display.
10. Device Reset push button.
11. Six user-definable push buttons (SW1 - SW6).
12. Potentiometer for master volume control (i.e., not synchronized with audio device volume control).
13. Five user application indicator LEDs (D5 - D9).
14. I 2 C pull-up headers (selectable to either 3.3V or 5V).
15. SPI Serial EEPROM (2 Mb for v2.0 development boards, 8 Mb for v3.0 development boards).
16. PICtail™ Plus connector with support for an external USB Apple interface PICtail Plus adapter (see Note ).

Note: For USB Apple connected audio devices, a special Apple controlled hardware device is required, which is only available to Apple MFi licensees. Please contact a Microchip sales office for information.

The PICtail Plus connector is not compliant with all Apple revisions. The Apple controlled hardware device to the PICtail Plus connector is suggested for Rev. 2.0 or earlier boards. Rev. 3.0 or later boards are supported by the Apple controlled hardware adapter connector (J13), which is located at the lower left of the main board of the development kit.

17. PICkit 3 In-Circuit Debugger/Programmer connector.
18. 16-bit/24-bit Stereo DAC and 70 mW line-out/headphone connector.
19. Certified HCI Class 1 Bluetooth radio module.
20. PIC32 Bluetooth Audio Development Board (DM320032).
21. 24-bit Audio DAC/AMP Daughter Board (AC320032-2).
22. Bluetooth HCI Radio Module Daughter Board (AC320032-1).

Note: Items 20, 21, and 22 are included with the PIC32 Bluetooth Development Audio Kit (DV320032).

Also, to utilize MPLAB REAL ICE™ or MPLAB ICD 3 with the development kit, an RJ-11 to ICSP adapter is required, which is available from microchipDIRECT (P/N: AC164110) (www.microchipdirect.com).

FIGURE 1-3: PIC32 BLUETOOTH AUDIO DEVELOPMENT KIT LAYOUT (TOP VIEW)

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000009_496a321c7a03189655945991527233c50750385439ce5ed8b32997ceb963a7c7.png)

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## 1.6 BLUETOOTH MODULE DAUGHTER BOARD FUNCTIONALITY AND FEATURES

The Bluetooth HCI Radio Module Daughter Board (BTM805B) (P/N: AC320032-1) has the following key features, as indicated by the corresponding number in Figure 1-4:

1. Female 20-pin dual-row header (J1).
2. Bluetooth Module - FLC-BTM805CL2B (U1).

Note: Microchip may support other Bluetooth wireless solutions in this modular design. Please contact your local Microchip sales office for additional information.

3. Female 12-pin dual-row header (J2).

## FIGURE 1-4: BLUETOOTH DAUGHTER BOARD LAYOUT (TOP VIEW)

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000010_e93e3877ab449fbb46680210fc968023a601b13f12a095a0a816546ad0cb66db.png)

## 1.7 AUDIO DAC DAUGHTER BOARD FUNCTIONALITY AND FEATURES

The Audio DAC/AMP Daughter Board (P/N: AC320032-2) has the following key features, as indicated by the corresponding number in Figure 1-5:

1. Female 20-pin dual-row header (J1).
2. Reverse flow diode (D1).
3. DAC/AMP 5V LDO regulator (U1).
4. 16/24-bit DAC - AK4384VT (U2).
5. Audio headphone/line out connector (J3).
6. Headphone/Line out amplifier - AK4201 (U3).
7. Female 12-pin dual-row header (J7).

FIGURE 1-5: AUDIO DAC DAUGHTER BOARD LAYOUT (TOP VIEW)

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000011_47ac4f58fea6abf86f1a320793907ce0a861678d7cfe61afc2b0c27f21bbe980.png)

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

NOTES:

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000012_295c5f16a76ed3f38e224127b9fd20e7bb3574c7c930962e046e0f21c3661134.png)

## PIC32 BLUETOOTH ® AUDIO DEVELOPMENT KIT REFERENCE GUIDE

Microchip

## Chapter 2.  Hardware and Software Performance

This chapter describes the hardware and software performance of the PIC32 Bluetooth Audio Development Kit.

## 2.1 HARDWARE AND SOFTWARE SBC AUDIO QUALITY PERFORMANCE

## FIGURE 2-1: PIC32 I 2 S THD MEASUREMENT SETUP

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000013_e4c927379a20990d09f14b710f469eab9138f767c9ede311618caaaecb18be15.png)

## 2.1.1 THD+N (16-bit I 2 S Data Uncompressed)

Table 2-1 provides the THD+N as measured on the PIC32 I 2 S port. Data was measured using an uncompressed ideal 1 kHz tone file stored in PIC32 Flash and sent directly out onto the I 2 S port and measured with an Audio Precision Model 585 Multi-channel Audio Analyzer.

## TABLE 2-1: THD+N RAW UNCOMPRESSED PIC32 I 2 S

Note  1: Data in shaded cells is pending.

| THD+N (dB) for 16-bit I 2 S Data at 0 dBFS (Uncompressed Data)   | THD+N (dB) for 16-bit I 2 S Data at 0 dBFS (Uncompressed Data)   | THD+N (dB) for 16-bit I 2 S Data at 0 dBFS (Uncompressed Data)   | THD+N (dB) for 16-bit I 2 S Data at 0 dBFS (Uncompressed Data)   | THD+N (dB) for 16-bit I 2 S Data at 0 dBFS (Uncompressed Data)   | THD+N (dB) for 16-bit I 2 S Data at 0 dBFS (Uncompressed Data)   | THD+N (dB) for 16-bit I 2 S Data at 0 dBFS (Uncompressed Data)   | THD+N (dB) for 16-bit I 2 S Data at 0 dBFS (Uncompressed Data)   |
|------------------------------------------------------------------|------------------------------------------------------------------|------------------------------------------------------------------|------------------------------------------------------------------|------------------------------------------------------------------|------------------------------------------------------------------|------------------------------------------------------------------|------------------------------------------------------------------|
| fs                                                               | 128X                                                             | 192X                                                             | 256X                                                             | 384X                                                             | 512X                                                             | 768X                                                             | Measurement BW                                                   |
| 8.00                                                             | -97.11                                                           | -97.18                                                           | -97.11                                                           | -97.14                                                           | -97.12                                                           | -97.14                                                           | 20 Hz - 4 kHz                                                    |
| 16.00                                                            | -94.46                                                           | -94.53                                                           | -94.78                                                           | -94.46                                                           | -94.47                                                           | -94.55                                                           | 20 Hz - 8 kHz                                                    |
| 32.00                                                            | -95.75                                                           | -95.79                                                           | -95.75                                                           | -95.81                                                           | -95.69                                                           | -96.13                                                           | 20 Hz - 16 kHz                                                   |
| 44.10                                                            | -96.32                                                           | -96.94                                                           | -96.67                                                           | -97.47                                                           | -97.45                                                           | -97.14                                                           | 20 Hz - 20 kHz                                                   |
| 48.00                                                            | -94.73                                                           | -94.46                                                           | -94.42                                                           | -94.44                                                           | -94.43                                                           | -94.29                                                           | 20 Hz - 20 kHz                                                   |
| 88.20                                                            | -98.85                                                           | -98.12                                                           | -98.79                                                           | -98.26                                                           | -98.83                                                           | -98.47                                                           | 20 Hz - 20 kHz                                                   |
| 96.00                                                            | -94.76                                                           | -94.83                                                           | -94.64                                                           | -94.58                                                           | -94.76                                                           | -94.13                                                           | 20 Hz - 20 kHz                                                   |
| 176.40                                                           | -                                                                | -                                                                | -                                                                | -                                                                | -                                                                | -                                                                | N/A                                                              |
| 192.00                                                           | -                                                                | -                                                                | -                                                                | -                                                                | -                                                                | -                                                                | N/A                                                              |

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## 2.1.2 THD+N (16-bit Uncompressed Audio Data-to-PIC32-to-Amp Out)

Table 2-2 provides the THD+N as measured at the output of the analog amplifier and measured with an Audio Precision Model 585 Multi-channel Audio Analyzer.

FIGURE 2-2: ANALOG THD+N MEASUREMENT SETUP

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000014_2d2fa30f21b089230a7cf8ff8c547455377e05446df6fb5c0ecc70031062bd65.png)

TABLE 2-2: THD+N RAW UNCOMPRESSED PIC32 TO ANALOG AMPLIFIER

| THD+N (dB) for Analog: 1.6 Vrms to 5 k Ω (Uncompressed Data) (see Note 2)   |               |        |                      |                             |        |        |                |
|-----------------------------------------------------------------------------|---------------|--------|----------------------|-----------------------------|--------|--------|----------------|
| fs                                                                          | 128X          | 192X   | 256X                 | 384X                        | 512X   | 768X   | Measurement BW |
| 8.00                                                                        |               |        | -90.85               | -90.73                      | -91.59 | -90.15 | 20 Hz - 4 kHz  |
| 16.00                                                                       |               | -87.45 |                      | -86.69                      | -86.84 | -86.74 | 20 Hz - 8 kHz  |
| 32.00                                                                       | Not Supported | by DAC | -85.23               | -81.21                      | -83.89 | -83.95 | 20 Hz - 16 kHz |
| 44.10                                                                       |               | -90.85 |                      | -87.14                      | -87.94 | -87.12 | 20 Hz - 20 kHz |
| 48.00                                                                       |               | -82.40 |                      | -83.12                      | -84.15 | -84.93 | 20 Hz - 20 kHz |
| 88.20                                                                       | -92.59        | -92.72 | -92.32               | -92.16                      |        |        | 20 Hz - 20 kHz |
| 96.00                                                                       | -89.56        | -89.94 | -89.79               | Not Supported by DAC -89.92 |        |        | 20 Hz - 20 kHz |
| 176.40                                                                      | -             | -      |                      |                             |        |        | N/A            |
| 192.00                                                                      | -             | -      | Not Supported by DAC |                             |        |        | N/A            |

Note  1: Data in shaded cells is pending.

- 2: THD+N = SUM (PIC32 + Audio DAC + Audio Amplifier).

## Hardware and Software Performance

## 2.1.3 THD+N (16-bit Compressed Audio Data from Smartphone to Analog Amplifier Output)

Table 2-3 provides the THD+N as measured at the output of the analog amplifier and measured with an Audio Precision Model 585 Multi-channel Audio Analyzer.

FIGURE 2-3: TOTAL SYSTEM SBC + ANALOG THD+N MEASUREMENT SETUP

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000015_d49fbfd082f280fab261ff1be8f5702a6d25e65ca71bd881486fba801b4b765a.png)

TABLE 2-3: TOTAL SYSTEM THD+N COMPRESSED SMARTPHONE TO PIC32 SBC DECODER TO ANALOG AMPLIFIER

| THD+N (dB) for Smartphone to Analog: 1.6 Vrms to 5 k Ω (SBC Compressed Data)   | THD+N (dB) for Smartphone to Analog: 1.6 Vrms to 5 k Ω (SBC Compressed Data)   | THD+N (dB) for Smartphone to Analog: 1.6 Vrms to 5 k Ω (SBC Compressed Data)   | THD+N (dB) for Smartphone to Analog: 1.6 Vrms to 5 k Ω (SBC Compressed Data)   | THD+N (dB) for Smartphone to Analog: 1.6 Vrms to 5 k Ω (SBC Compressed Data)   | THD+N (dB) for Smartphone to Analog: 1.6 Vrms to 5 k Ω (SBC Compressed Data)   | THD+N (dB) for Smartphone to Analog: 1.6 Vrms to 5 k Ω (SBC Compressed Data)   | THD+N (dB) for Smartphone to Analog: 1.6 Vrms to 5 k Ω (SBC Compressed Data)   |
|--------------------------------------------------------------------------------|--------------------------------------------------------------------------------|--------------------------------------------------------------------------------|--------------------------------------------------------------------------------|--------------------------------------------------------------------------------|--------------------------------------------------------------------------------|--------------------------------------------------------------------------------|--------------------------------------------------------------------------------|
| fs                                                                             | 128X                                                                           | 192X                                                                           | 256X                                                                           | 384X                                                                           | 512X                                                                           | 768X                                                                           | Measurement BW                                                                 |
| 44.10                                                                          | Not supported by DAC                                                           | Not supported by DAC                                                           | -70.37                                                                         | -70.55                                                                         | -70.65                                                                         | -70.92                                                                         | 20 Hz - 20 kHz                                                                 |

Note: THD+N = SUM (Smartphone Encoder + Bluetooth Radio + PIC32 SBC Decoder + Audio DAC + Audio Amplifier).

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

NOTES:

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000016_22e6691812de863e7de1f7d1d828351bb7b9bb98aa2e7e62c189feac623ffd98.png)

## PIC32 BLUETOOTH ® AUDIO DEVELOPMENT KIT REFERENCE GUIDE

Microchip

## Chapter 3.  Interoperability Testing Results

This chapter describes the interoperability testing results for the PIC32 Bluetooth Audio Development Kit.

## 3.1 TEST CONDITIONS SUMMARY

The following conditions were used during interoperability testing:

- HCI Bluetooth Class 1 Radio (FLC-BTM805Cl2B)
- 10 meter test range (standard office cubicle walls/environment)
- 15 meter line of sight minimum
- 100 devices, models, and OS version combinations (as of June 2013)
- 18 individual manufacturers

Note: Typical range of 20-30 meters, as tested with the iPhone 4S.

## 3.2 MICROCHIP BLUETOOTH COMPATIBILITY TEST MATRIX

TABLE 3-1: BLUETOOTH INTEROPERABILITY TEST RESULTS

| Manufacturer   | Phone Type       | OS Version        | Connect   | Stream Audio   |
|----------------|------------------|-------------------|-----------|----------------|
| Asus           | Nexus 7 Wi-Fi    | Android v4.2.2    | Yes       | Yes            |
| Apple          | iPhone 4s 4G     | v5.1.1            | Yes       | Yes            |
| Apple          | iPad ®           | n/a               | Yes       | Yes            |
| Apple          | iPhone ® 4       | v6.1.3            | Yes       | Yes            |
| Apple          | iPhone 4         | iOS 6.1.3         | Yes       | Yes            |
| Apple          | iPhone 4         | v6.1 (10B14-AT&T) | Yes       | Yes            |
| Apple          | iPhone 4         | v4.3.3            | Yes       | Yes            |
| Apple          | iPhone 4         | iOS v6.1.3        | Yes       | Yes            |
| Apple          | iPhone 4s        | v5.1.1            | Yes       | Yes            |
| Apple          | iPhone 4S        | v6.1.3            | Yes       | Yes            |
| Apple          | iPhone 3GS       | v6.1.3            | Yes       | Yes            |
| Apple          | iPhone 5         | v6.1.2            | Yes       | Yes            |
| Apple          | iPhone 5         | v6.0.2            | Yes       | Yes            |
| Apple          | iPhone 5         | v6.1.3            | Yes       | Yes            |
| Apple          | iPhone 5         | v6.0.1            | Yes       | Yes            |
| Apple          | iPhone 5         | iOS v6.1.4        | Yes       | Yes            |
| Apple          | iPod Touch ® 5G  | iOS v6.1.3        | Yes       | Yes            |
| Apple          | iPad 2           | iOS v6.1.3        | Yes       | Yes            |
| Apple          | iPad (MC979LL/A) | iOS v6.1.3        | Yes       | Yes            |
| Apple          | iPod touch 4     | v6.1.2            | Yes       | Yes            |
| Apple          | iPad Mini™       | iOS 6.1.2         | Yes       | Yes            |

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

TABLE 3-1: BLUETOOTH INTEROPERABILITY TEST RESULTS (CONTINUED)

| Manufacturer     | Phone Type                      | OS Version           | Connect   | Stream Audio   |
|------------------|---------------------------------|----------------------|-----------|----------------|
| BlackBerry ®     | BlackBerry Z10 (Model STL100-3) | v10.0.10.85          | Yes       | Yes            |
| BlackBerry ®     | BlackBerry Bold 9780            | v6.0.0.723           | Yes       | Yes            |
| BlackBerry ®     | BlackBerry Bold 9900            | v7.1.0.523           | Yes       | Yes            |
| Dell™            | Latitude E6220                  | Windows 7            | Yes       | Yes            |
| Fujitsu          | F12C                            | Android 2.3.4        | Yes       | Yes            |
| Fujitsu          | F-10D                           | Android v4.0.3       | Yes       | Yes            |
| Fujitsu          | F-02E (Docomo)                  | Android v4.1.4       | Yes       | Yes            |
| Fujitsu          | F-05D (Docomo Arrows X LTE)     | Android v2.3.5       | Yes       | Yes            |
| Google ® /Nexus™ | Google Android™                 | v4.x                 | Yes       | Yes            |
| Google ® /Nexus™ | ASUS Nexus 7 tablet             | 4.2.2                | Yes       | Yes            |
| Google ® /Nexus™ | LG Nexus 4                      | 4.2.2                | Yes       | Yes            |
| Google ® /Nexus™ | Nexus 4™                        | Android v4.2.2       | Yes       | Yes            |
| Google ® /Nexus™ | Nexus S™ Google Phone           | 4.1.2                | Yes       | Yes            |
| Google ® /Nexus™ | SAMSUNG Nexus S                 | 4.0.4                | Yes       | Yes            |
| HTC              | HTC Sense v2.1                  | Android v2.3.4       | Yes       | Yes            |
| HTC              | HTC G43                         | Android v2.1         | Yes       | Yes            |
| HTC              | HTC EVO3d                       | Android v4.0.3       | Yes       | Yes            |
| HTC              | HTC PC36100                     | Android v2.3.5       | Yes       | Yes            |
| HTC              | HTC Resound                     | v4.0.3               | Yes       | Yes            |
| HTC              | HTC Google nexus                | v2.3.4               | Yes       | Yes            |
| HTC              | HTC EVO3D                       | v2.3.5               | Yes       | Yes            |
| HTC              | HTC Sense v2.1                  | v2.3.4               | Yes       | Yes            |
| HTC              | HTC 1SV                         | v4.0                 | Yes       | Yes            |
| HTC              | HTC Flyer tablet                | 2.3.4                | Yes       | Yes            |
| HTC              | Incredible S S710e              | 2.3.3                | Yes       | Yes            |
| HTC              | G11                             | 4.0.4                | Yes       | Yes            |
| Huawei           | Huawei C8.6.50                  | Android v2.3.3       | Yes       | Yes            |
| Huawei           | Ascend G300                     | 4.0.4                | Yes       | Yes            |
| IBM              | ThinkPad T420                   | Windows 7 Pro 32-bit | Yes       | Yes            |
| Kyocera          | ISW11K (au)                     | Android v2.3.5       | Yes       | Yes            |
| LG               | L55C                            | Android v2.3.4       | Yes       | Yes            |
| LG               | LG-P769                         | Android v4.0.4       | Yes       | Yes            |
| LG               | L38C                            | Android v2.3.6       | Yes       | Yes            |
| Motorola         | Droid RAZR                      | Android v4.1.2       | Yes       | Yes            |
| Motorola         | Droid3                          | Android v2.3.4       | Yes       | Yes            |
| Motorola         | Razr M                          | Android v4.1.1       | Yes       | Yes            |
| Motorola         | Motorola Verizon                | Android v2.3.4       | Yes       | Yes            |
| Motorola         | Droid X                         | Android v2.3.4       | Yes       | Yes            |
| Motorola         | Motorola Triumph                | Android v2.2.2       | Yes       | Yes            |
| Motorola         | Droid X                         | 2.3.4                | Yes       | Yes            |
| Motorola         | MT788                           | 4.0.4                | Yes       | Yes            |

## Interoperability Testing Results

TABLE 3-1: BLUETOOTH INTEROPERABILITY TEST RESULTS (CONTINUED)

| Manufacturer   | Phone Type                 | OS Version            | Connect   | Stream Audio        |
|----------------|----------------------------|-----------------------|-----------|---------------------|
| Nokia          | Nokia - 810                | Windows 8.0           | Yes       | Yes                 |
| Nokia          | Lumia920                   | Windows 8.0.10211.204 | Yes       | Yes                 |
| Nokia          | Nokia Lumia 900            | Windows 7.8           | Yes       | Yes                 |
| Nokia          | Nokia 5530                 | -                     | Yes       | Yes                 |
| Nokia          | Lumina 800                 | WP 7.8                | Yes       | Yes                 |
| Panasonic      | P-07D (Docomo)             | Android v4.0.4        | Yes       | Yes                 |
| Samsung        | Samsung Galaxy Notepad 2   | V10.1                 | Yes       | Yes                 |
| Samsung        | Samsung                    | v4.1                  | Yes       | Yes                 |
| Samsung        | Samsung Galaxy S3          | Android 4.0.4         | Yes       | Yes                 |
| Samsung        | Nexus Samsung Tablet       | -                     | Yes       | Yes                 |
| Samsung        | Samsung Galaxy S1          | v2.2                  | Yes       | Yes                 |
| Samsung        | Samsung Galaxy S3 SCHI535  | v4.1.2                | Yes       | Yes                 |
| Samsung        | Samsung SCHM828C           | v2.2.2                | Yes       | Connects, but drops |
| Samsung        | Galaxy S2                  | v4.0.4                | Yes       | Yes                 |
| Samsung        | Samsung Galaxy S3 SGH-T999 | v4.1.1                | Yes       | Yes                 |
| Samsung        | Samsung Note 2             | v4.1.1                | Yes       | Yes                 |
| Samsung        | Galaxy TAB-2 Wi-Fi         | Android v4.1.1        | Yes       | Yes                 |
| Samsung        | Galaxy S2                  | Android v4.2.1        | Yes       | Yes                 |
| Samsung        | Galaxy Nexus               | Android v4.2.2        | Yes       | Yes                 |
| Samsung        | GALAXY Nexus               | Android 4.2.1         | Yes       | Yes                 |
| Samsung        | Galaxy SGH-1777            | Android v4.0.4        | Yes       | Yes                 |
| Samsung        | SCH1535                    | v4.1.2                | Yes       | Yes                 |
| Samsung        | Galaxy Vibrant             | v4.2.2                | Yes       | Yes                 |
| Samsung        | Samsung focus              | Windows v7.8          | Yes       | Yes                 |
| Samsung        | Galaxy S3                  | v4.1.1                | Yes       | Yes                 |
| Samsung        | Galaxy S2                  | 4.1.1                 | Yes       | Yes                 |
|                | Galaxy S3                  | 4.0.4                 | Yes       | Yes                 |
|                | Galaxy Note                | 4.1.2                 | Yes       | Yes                 |
|                | Galaxy Note 2              | 4.1.1                 | Yes       | Yes                 |
|                | GT-N7100                   | 4.1.2                 | Yes       | Yes                 |
|                | SCH-i579                   | 2.3.4                 | Yes       | Yes                 |
| Sharp          | SH-02E                     | Android 4.0.4         | Yes       | Yes                 |
|                | SH-13C (Docomo)            | Android v2.3.4        | Yes       | Yes                 |
| Sony           | Sony Experia (Dual SIM)    | Android v4.0.4        | Yes       | Yes                 |
|                | PSVita                     | 2.11                  | Yes       | Yes                 |
|                | SO-03D (Docomo XPERIA)     | Android v2.3.7        | Yes       | Yes                 |
|                | SO-05D (Docomo)            | Android v4.0.4        | Yes       | Yes                 |
|                | LT26ii                     | 4.0.4                 | Yes       | Yes                 |

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## 3.3 BLUETOOTH AVRCP FUNCTIONS

Bluetooth devices, which include PCs, audio players, tablets, and cellular phones, continue to evolve. In addition to the Bluetooth driver functions and hardware, differing third-party applications tend to handle the API differently. Even though it appears to be the same operating system and version number (e.g., Android v4.3), different manufacturers have applied support for some functions uniquely.

The problem with this is that identical Bluetooth profile functions provided to different systems may return very different results. These functions may also differ between how the handset device uses the function, and how the Bluetooth equivalent command is interpreted by that same device. This may cause some confusion for users.

For example, the Fast Forward function on an Apple iPhone generally will start slowly, and then non-linearly accelerate through the existing track while playing some audio at selected intervals. The Fast Forward function will then stop and play as the track is changed, and ignore the Fast Forward button until removed and then pressed again. For those not familiar, this could make for some frustration as each track will essentially require the user to start fast forwarding (and accelerate) through the track again to search in this method.

However, on an Android v4.3-based cellular phone from Samsung, the Fast Forward function will appear to work differently. It will start to accelerate through the current track faster than the Apple iPhone, and it will do so linearly. As it reaches the end of the track, it will likely do so at maximum fast forward speed (generally 16x), and continue fast forwarding through the next track. It will ignore the track change other than providing AVRCP track change information. This functionality can make it difficult to stop at the entry point to the next track.

Still different is an Android v4.3 based cellular phone from LG, where on the handset the Fast Forward function is like other Android devices. However, while other AVRCP functions (i.e., Play / Pause , Next Track / Previous Track , etc.) work as expected, the system will ignore the Fast Forward and Rewind functions entirely. In this case, the handset will continue to play as if no command is sent. This could create some additional confusion for users as from their perspective, the handset works and the demonstration software appears to be non-functional.

Other differences between smartphone operating systems can be seen in the track information, progress bar, progress time, and connection operations. Where possible, the differences have been mitigated in software; however, users will still see functional differences.

To resolve some of these issues, a number of compile-time selections are now offered to help refine the button functions on the PIC32 Bluetooth Audio Development Kit. Note that these selections will not change the functionality as observed directly on the handset, so the user may see differences. These options generally apply to the Fast Forward and Rewind functions, and include both the Apple and Samsung usage models listed previously, as well as some other customer requests. The details of these options are included in the demonstration button descriptions.

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000017_22e6691812de863e7de1f7d1d828351bb7b9bb98aa2e7e62c189feac623ffd98.png)

## PIC32 BLUETOOTH ® AUDIO DEVELOPMENT KIT REFERENCE GUIDE

Microchip

## Chapter 4.  Bluetooth Audio Demonstrations

This chapter provides an overview of the demonstrations that are available for the PIC32 Bluetooth Audio Development Kit. For information about our software, including free versions, documentation, and system architecture details, please visit: www.microchip.com/pic32btsuites

## 4.1 DEMONSTRATIONS, HEX FILES, AND VOICE PROMPT PROGRAMMER (FREE-OF-CHARGE)

## BT Data Demo 4

This demonstration is offered free-of-charge. This is a SPP data demonstration only, which provides no USB audio support and has a special Bluetooth stack for duplex data transmission.

Detailed information on configuring and running this demonstration is available in the related 'ReadMe' file that accompanies the demonstration.

## Hexadecimal Files

Hex files for all demonstrations are available as binary codes only.

## Voice Prompt Programmer

This free utility can be used to install voice prompts and other data in the external Flash memory.

## 4.2 DEMONSTRATION SUITES (FOR A FEE)

Three demonstration software suites are offered for purchase from microchipDIRECT. Detailed information on configuring and running each demonstration is available in the related 'ReadMe' files that accompany each demonstration.

## PIC32 Bluetooth Audio Software Suite 1 (SW320014-1)

http://www.microchipdirect.com/ProductSearch.aspx?Keywords=SW320014-1

Suite 1 offers SBC Bluetooth audio and additional features and includes:

- BT USB Audio Demo 2.5
- BT Audio Demo 5
- BT Audio Demo 7
- BT USB Audio Demo 8
- BT Audio Demo 11

## PIC32 Bluetooth Audio Software Suite 2 (SW320014-2)

http://www.microchipdirect.com/ProductSearch.aspx?Keywords=SW320014-2

Suite 2 offers AAC Bluetooth audio and additional features. The following demonstrations are offered in addition to all demonstrations offered in Suite 1:

- BT Audio Demo 3
- BT USB Audio Demo 6

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## PIC32 Bluetooth Audio Software Suite 3 (SW320014-3)

Note: Contact Microchip marketing or your local sales office for purchase and license information.

This software suite offers Bluetooth multi-connection 'Break-In' demonstrations. The following demonstrations are offered in addition to all demonstrations offered in Suite 1:

- BT Audio Demo 9
- BT USB Audio Demo 10

## 4.3 DEMONSTRATION SUITES AND RESOURCE REQUIREMENTS

Brief descriptions and resource requirements are provided for the v1.0.3, v2.0, and v3.0 demonstrations in the following three tables. In addition, a features map is provided in Table 4-4,

Table 4-1 lists the available v1.0.3 Bluetooth Audio Demonstrations (RTOS version) and their resource requirements.

TABLE 4-1: V1.0.3 DEMONSTRATIONS AND RESOURCE REQUIREMENTS (RTOS VERSION)

|                       |                                                                                                                                                                               | Resource Requirement   | Resource Requirement   | Peak MIPS (See Note   |                                             |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------|------------------------|-----------------------|---------------------------------------------|
| Demonstration Name    | Description                                                                                                                                                                   | Flash (KB)             | RAM (KB)               | 3)                    | Comment                                     |
| BT USB Audio Demo 2.5 | Bluetooth Stack (A2DP+AVRCP+SPP+SBC) + Android Open Accessory audio Type-A USB connection support and Samsung ® audio with mini-B USB connection support. See Notes 1 and 2 . | 227                    | 49                     | ~30                   | No Apple USB audio support.                 |
| BT Audio Demo 3       | Bluetooth Stack (A2DP+AVRCP+SPP+AAC decoder) + Graphics. This demonstration uses the higher quality AAC audio decoder in place of the SBC decoder. See Note 1 .               | 248                    | 51                     | ~65                   | Bluetooth audio only; no USB audio support. |
| BT Audio Demo 4       | Bluetooth Data Stack (SPP only). This data- only, non-audio demonstration provides no USB audio support. See Note 1 .                                                         | 140                    | 14                     | ~8                    | Bluetooth data transport only.              |
| BT Audio Demo 5       | Bluetooth Stack (A2DP+AVRCP+SPP+SBC decoder) + Graphics. See Note 1 .                                                                                                         | 182                    | 41                     | ~30                   | Bluetooth audio only; no USB support.       |

## Note

1: 33 KB of stated Flash resources includes graphics support.

- 2: Total system latency: Bluetooth HCI UART → Bluetooth Stack → SBC Decoder → I 2 S Audio Out Latency = 1.8 ms.
- 3: This information is being provided for guidance purposes only.

## Bluetooth Audio Demonstrations

Table 4-2 lists the available v2.0 Bluetooth Audio Demonstrations (non-RTOS version) and their resource requirements.

TABLE 4-2: V2.0 DEMONSTRATIONS AND RESOURCE REQUIREMENTS (NON-RTOS VERSION)

|                       | Description                                                                                                                                                                                                                                                             | Resource Requirement   | Resource Requirement   | Peak MIPS (See Note   | Comment                                     |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------|------------------------|-----------------------|---------------------------------------------|
| Demonstration Name    |                                                                                                                                                                                                                                                                         | Flash (KB)             | RAM (KB)               | 3)                    |                                             |
| BT USB Audio Demo 2.5 | Bluetooth Stack (A2DP+AVRCP+SPP+SBC) + Android Open Accessory audio Type-A USB connection support and Samsung audio with mini-B USB connection support. See Notes 1 and 2.                                                                                              | 208                    | 40.5                   | ~30                   | No Apple USB audio support.                 |
| BT Audio Demo 3       | Bluetooth Stack (A2DP+AVRCP+SPP+AAC decoder) + Graphics. This demonstration uses the higher quality AAC audio decoder in place of the SBC decoder. See Note 1 .                                                                                                         | 247                    | 33.1                   | ~65                   | Bluetooth audio only; no USB audio support. |
| BT Audio Demo 4       | Bluetooth Data Stack (SPP only). This data-only, non-audio demonstration provides no USB audio support. See Note 1 .                                                                                                                                                    | 94                     | 4.8                    | ~8                    | Bluetooth data transport only.              |
| BT Audio Demo 5       | Bluetooth Stack (A2DP+AVRCP+SPP+SBC decoder) + Graphics. See Note 1 .                                                                                                                                                                                                   | 185                    | 32.8                   | ~30                   | Bluetooth audio only; no USB support.       |
| BT USB Audio Demo 6   | Bluetooth Stack (A2DP+AVRCP+SPP+AAC) + Android Open Accessory audio Type-A USB connection support and Samsung audio with mini-B USB connection support. This demon- stration uses the higher quality AAC audio decoder in place of the SBC decoder. See Notes 1 and 2 . | 269                    | 40.9                   | ~65                   | No Apple USB audio support.                 |
| BT Audio Demo 7       | Bluetooth Stack (A2DP+AVRCP+SPP+SBC decoder) + Graphic Equalizer + User GUI. This demonstration adds additional user GUI func- tionality with menus to adjust gain and observe real-time frequency-band feedback. See Note 1 .                                          | 247                    | 50.2                   | ~90                   | Bluetooth audio only; no USB support.       |

Note

- 1: 33 KB of stated Flash resources includes graphics support.

2: Total system latency: Bluetooth HCI UART → Bluetooth Stack → SBC Decoder → I 2 S Audio Out Latency = 1.8 ms.

3: This information is being provided for guidance purposes only.

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

Table 4-3 lists the available v3.0 Bluetooth Audio Demonstrations (non-RTOS version) and their resource requirements.

TABLE 4-3: V3.0 DEMONSTRATIONS AND RESOURCE REQUIREMENTS (NON-RTOS VERSION)

| Demonstration Name    |                                                                                                                                                                                                                                                                         | Resource Requirement   | Resource Requirement   | Peak MIPS (See Note 3)   | Comment                                                 |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------|------------------------|--------------------------|---------------------------------------------------------|
| Demonstration Name    | Description                                                                                                                                                                                                                                                             | Flash (KB)             | RAM (KB)               | Peak MIPS (See Note 3)   | Comment                                                 |
| BT USB Audio Demo 2.5 | Bluetooth Stack (A2DP+AVRCP+SPP+SBC) + Android Open Accessory audio Type-A USB connection support and Samsung audio with mini-B USB connection support. See Notes 1 and 2 .                                                                                             | 271.7                  | 38.8                   | ~30                      | Bluetooth /USB audio support.                           |
| BT Audio Demo 3       | Bluetooth Stack (A2DP+AVRCP+SPP+AAC decoder) + Graphics. This demonstration uses the higher quality AAC audio decoder in place of the SBC decoder. See Note 1.                                                                                                          | 251.5                  | 34.9                   | ~65                      | Bluetooth audio only; no USB audio support.             |
| BT Audio Demo 4       | Bluetooth Data Stack (SPP only). This data-only, non-audio demonstration provides no USB audio support. See Note 1 .                                                                                                                                                    | 139.6                  | 7.12                   | ~8                       | Bluetooth data transport only.                          |
| BT Audio Demo 5       | Bluetooth Stack (A2DP+AVRCP+SPP+SBC decoder) + Graphics. See Note 1 .                                                                                                                                                                                                   | 190.5                  | 34.6                   | ~30                      | Bluetooth audio only; no USB support.                   |
| BT USB Audio Demo 6   | Bluetooth Stack (A2DP+AVRCP+SPP+AAC) + Android Open Accessory audio Type-A USB connection support and Samsung audio with mini-B USB connection support. This demon- stration uses the higher quality AAC audio decoder in place of the SBC decoder. See Notes 1 and 2 . | 332.7                  | 39.7                   | ~65                      | Bluetooth /USB audio support.                           |
| BT Audio Demo 7       | Bluetooth Stack (A2DP+AVRCP+SPP+SBC decoder) + Graphic Equalizer + User GUI. This demonstration adds additional user GUI func- tionality with menus to adjust gain and observe real-time frequency-band feedback. See Note 1 .                                          | 247                    | 50.2                   | ~90                      | Bluetooth audio only; no USB support.                   |
| BT USB Audio Demo 8   | Bluetooth Stack (A2DP+AVRCP+SPP+SBC) +Graphic Equalizer +User GUI Android Open Accessory audio Type-A USB connection sup- port and Samsung audio with mini-B USB con- nection support. See Notes 1 and 2 .                                                              | 255.1                  | 39.42                  | ~90                      | Bluetooth/USB audio support. Graphic equalizer support. |
| BT Audio Demo 9       | Bluetooth Stack (A2DP+AVRCP+SPP+SBC decoder) + Graphic. This demonstration adds additional user Break-in/Party mode.                                                                                                                                                    | 203                    | 46.34                  | ~30                      | Bluetooth audio only; no USB support.                   |
| BT USB Audio Demo 10  | Bluetooth Stack (A2DP+AVRCP+SPP+SBC) + Android Open Accessory audio Type-A USB connection support and Samsung audio with mini-B USB connection support. See Notes 1 and 2.                                                                                              | 203                    | 46.34                  | ~30                      | Bluetooth/USB audio support                             |
| BT Audio Demo 11      | Bluetooth Stack (A2DP+AVRCP+SPP+SBC decoder) + Graphics. This demonstration adds additional duplex transmission of data using SPP during audio playback.                                                                                                                | 215.2                  | 35.7                   | ~38                      | Bluetooth audio and data transmission support.          |

## Note

- 1: 33 KB of stated Flash resources includes graphics support.
- 2: Total system latency: Bluetooth HCI UART → Bluetooth Stack → SBC Decoder → I 2 S Audio Out Latency = 1.8 ms.
- 3: This information is being provided for guidance purposes only.

Table 4-4 lists the features that are included in each of the demonstrations.

TABLE 4-4: DEMONSTRATION FEATURES MAP

| Demonstration Name     | Bluetooth Audio   | Bluetooth Data   | AAC   | USB Apple Audio   | USB Android/ Samsung Audio   | Graphic Equalizer   | Volume Sync   | Break-in/ Party Mode   |
|------------------------|-------------------|------------------|-------|-------------------|------------------------------|---------------------|---------------|------------------------|
| BT USB Audio Demo 2.5  | X                 | -                | -     | -                 | X                            | -                   | X             | -                      |
| BT USB Audio Demo 2.5A | X                 | -                | -     | X                 | X                            | -                   | -             | -                      |
| BT Audio Demo 3        | X                 | -                | X     | -                 | -                            | -                   | X             | -                      |
| BT Audio Demo 4        | -                 | X                | -     | -                 | -                            | -                   | -             | -                      |
| BT Audio Demo 5        | X                 | -                | -     | -                 | -                            | -                   | X             | -                      |
| BT USB Audio Demo 6    | X                 | -                | X     | -                 | X                            | -                   | X             | -                      |
| BT USB Audio Demo 6A   | X                 | -                | X     | X                 | X                            | -                   | X             | -                      |
| BT Audio Demo 7        | X                 | -                | -     | -                 | -                            | X                   | X             | -                      |
| BT USB Audio Demo 8    | X                 | -                | -     | -                 | X                            | X                   | X             | -                      |
| BT USB Audio Demo 8A   | X                 | -                | -     | X                 | X                            | X                   | X             | -                      |
| BT Audio Demo 9        | X                 | -                | -     | -                 | -                            | -                   | X             | X                      |
| BT USB Audio Demo 10   | X                 | -                | -     | -                 | X                            | -                   | X             | X                      |
| BT Audio Demo 11       | X                 | X                | -     | -                 | -                            | -                   | X             | -                      |

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## 4.4 DEMONSTRATION OVERVIEWS

## 4.4.1 Bluetooth/USB Audio Demonstration 2.5 with SBC

Bluetooth Stack (A2DP + AVRCP + SBC decoder) with Android Open Accessory audio Type-A USB connection support plus Samsung audio with mini-B USB connection support.

Refer to Table 3-1 in Chapter 3. 'Interoperability Testing Results' for the list of tested Bluetooth-enabled devices.

The Bluetooth/USB Audio Demonstration 2.5 with SBC supports three types of streaming audio:

- Streaming wireless Bluetooth audio from any smartphone (i.e., Apple, Samsung, Google, etc.), PC, or Bluetooth-enabled device
- Streaming USB audio over a USB Type-A connector on the development kit from any smartphone or music device that supports the Android Open Accessory protocol where the audio source is a USB Device. For Apple devices, see the following Note .
- Streaming USB audio over a mini-B connector on the development kit from any Samsung smartphone or audio device where the source is a USB Host

Note: The USB audio support feature for Apple is a superset of Bluetooth/USB Audio Demonstration 2.5. It supports all of the same features as Demonstration 2.5 with the addition of Apple USB audio with the inclusion of the iAP/MFi software components, as denoted with the 'A' suffix in the demonstration name as 2.5A. See 1.1.1 'Bluetooth Demonstrations' in Chapter 1. 'Introduction' for more information.

## 4.4.2 Bluetooth Audio Demonstration 3 with AAC

Bluetooth Audio support only with A2DP + AVRCP + SBC/AAC decoder.

Refer to Table 3-1 in Chapter 3. 'Interoperability Testing Results' for the list of tested Bluetooth-enabled devices.

This demonstration uses the same Bluetooth audio stack as used in Demonstration 2.5, but includes the addition of a higher quality AAC software audio codec in place of the default SBC audio decoder. AAC stands for Advanced Audio Coding and generally achieves better sound quality than MP3 at similar bit rates. This demonstration does not provide any USB audio support.

This demonstration supports only one type of streaming audio: Streaming wireless Bluetooth audio from any smartphone (Apple, Samsung, Google, etc.), PC, or Bluetooth-enabled device. The demonstration uses the advanced AAC audio software codec in place of the default SBC decoder for handsets that support AAC via Bluetooth. For devices that do not support the AAC codec, the SBC codec is used.

## 4.4.3 Bluetooth Data Demonstration 4 with SPP

Note: This demonstration does not support data transfer from an Apple iPhone application.

Bluetooth data support only. Bluetooth Stack with SPP profile.

Refer to Table 3-1 in Chapter 3. 'Interoperability Testing Results' for the list of tested Bluetooth-enabled devices.

In this demonstration, the basic Bluetooth audio profiles and decoders have been removed, (i.e., A2DP, AVRCP, SBC and/or AAC). This demonstration provides basic data transport of non-audio full-duplex data transfers over the Bluetooth link. This demonstration does not provide any USB audio support. The demonstration allows the user to perform terminal emulation and echo characters from an Android smartphone or PC over a Bluetooth connection to the graphic display of the development board, and then back to the PC or smartphone emulation application menu.

## 4.4.4 Bluetooth Audio Demonstration 5 with SBC

Bluetooth audio only support (A2DP + AVRCP + SBC decoder).

Refer to Table 3-1 in Chapter 3. 'Interoperability Testing Results' for the list of tested Bluetooth-enabled devices.

This non-USB audio demonstration supports streaming wireless Bluetooth audio from any smartphone (i.e., Apple, Samsung, Google, etc.), PC, or Bluetooth-enabled device.

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## 4.4.5 Bluetooth/USB Audio Demonstration 6 with AAC

Bluetooth Stack (A2DP + AVRCP + AAC/SBC decoder) with Android Open Accessory audio Type-A USB connection support plus Samsung audio with mini-B USB connection support (see Note 1 ).

Refer to Table 3-1 in Chapter 3. 'Interoperability Testing Results' for the list of tested Bluetooth-enabled devices.

This Bluetooth/USB Audio Demonstration 6 with AAC supports three types of streaming audio:

- Streaming wireless Bluetooth audio from any smartphone (i.e., Apple, Samsung, Google, etc.), PC, or Bluetooth-enabled device. This includes the addition of a higher quality AAC software audio codec in place of the default SBC audio decoder. AAC stands for Advanced Audio Coding and generally achieves better sound quality than MP3 at similar bit rates. For devices that do not support the AAC codec, the SBC codec is used.
- Streaming USB audio over a USB Type-A connector on the development kit from any smartphone or music device that supports the Android Open Accessory protocol where the audio source is a USB Device. For Apple devices, refer to Note 2 .
- Streaming USB audio over a mini-B connector on the development kit from any Samsung smartphone or audio device where the source is a USB Host
- Note 1: The Bluetooth/USB Audio Demonstration 6 will operate on the PIC32MX470F512L device. This device is required for Demonstration 6, which incorporates the AAC decoder in addition to Bluetooth SBC and USB audio functions.
- 2: The USB audio support for Apple is a superset of Bluetooth/USB Audio Demonstration 6. It supports all of the same features as Demonstration 6 with the addition of Apple USB audio with the inclusion of the iAP/MFi software components, as denoted with the 'A' suffix in the demonstration name as 6A. See 1.1.1 'Bluetooth Demonstrations' in Chapter 1. 'Introduction' for more information.

## 4.4.6 Bluetooth Audio Demonstration 7 with SBC and EQ

Bluetooth audio only (A2DP + AVRCP + SBC decoder + integrated Graphic Equalizer).

Refer to Table 3-1 in Chapter 3. 'Interoperability Testing Results' for the list of tested Bluetooth-enabled devices.

This non-USB audio demonstration supports streaming wireless Bluetooth audio from any smartphone (i.e., Apple, Samsung, Google, etc.), PC, or Bluetooth-enabled device.

This demonstration enables the real-time output of a 2-channel 6-band graphic equalizer function. The function is supported with new user input modes to set the gain stages, integrated digital filtering, and display of the quantized per-frequency-band magnitude.

## 4.4.7 Bluetooth/USB Audio Demonstration 8 with SBC

Bluetooth Stack (A2DP + AVRCP + SBC decoder) with Android Open Accessory audio Type-A USB connection support plus Samsung audio with mini-B USB connection support (see the following Note ).

Refer to Table 3-1 in Chapter 3. 'Interoperability Testing Results' for the list of tested Bluetooth-enabled devices.

This Bluetooth/USB Audio Demonstration 8 supports three types of streaming audio:

- Streaming wireless Bluetooth audio from any smartphone (i.e., Apple, Samsung, Google, etc.), PC, or Bluetooth-enabled device. For Apple devices, refer to the following Note .
- Streaming USB audio over a USB Type-A connector on the development kit from any smartphone or music device that supports the Android Open Accessory protocol where the audio source is a USB Device
- Streaming USB audio over a mini-B connector on the development kit from any Samsung smartphone or audio device where the source is a USB Host

Note:

The USB audio support for Apple is a superset of Bluetooth/USB Audio Demonstration 8. It supports all of the same features as demonstration 8 with the addition of Apple USB audio with the inclusion of the iAP/MFi software components, as denoted with the 'A' suffix in the demonstration name as 8A. See 1.1.1 'Bluetooth Demonstrations' in Chapter 1. 'Introduction' for more information.

## 4.4.8 Bluetooth Audio Demonstration 9 With SBC and Break-In/Party mode

Bluetooth audio only support (A2DP + AVRCP + SBC decoder) + Break-in mode.

Refer to Table 3-1 in Chapter 3. 'Interoperability Testing Results' for the list of tested Bluetooth-enabled devices.

This non-USB audio demonstration supports streaming wireless Bluetooth audio from any smartphone (i.e., Apple, Samsung, Google, etc.), PC, or Bluetooth-enabled device.

This demonstration also supports multiple mobile connections to a single bluetooth device at the same instance. This demonstration allows users to stream audio in Break-in/Party mode.

## 4.4.9 Bluetooth/USB Audio Demonstration 10 With SBC and Break-In/Party Mode

Bluetooth Stack (A2DP + AVRCP + SBC decoder) with Android Open Accessory audio Type-A USB connection support plus Samsung audio with mini-B USB connection support.

Refer to Table 3-1 in Chapter 3. 'Interoperability Testing Results' for the list of tested Bluetooth-enabled devices.

This Bluetooth/USB Audio Demonstration 10 supports three types of streaming audio:

- Streaming wireless Bluetooth audio from any smartphone (i.e., Apple, Samsung, Google, etc.), PC, or Bluetooth-enabled device
- Streaming USB audio over a USB Type-A connector on the development kit from any smartphone or music device that supports the Android Open Accessory protocol where the audio source is a USB Device. This demonstration also supports multiple mobile connections to a single Bluetooth device at the same time and allows users to stream audio in Break-in/Party mode.
- Streaming USB audio over a mini-B connector on the development kit from any Samsung smartphone or audio device where the source is a USB Host

## 4.4.10 Bluetooth Audio Demonstration 11 With SBC and SPP

Bluetooth audio only support (A2DP + AVRCP + SPP + SBC decoder).

Refer to Table 3-1 in Chapter 3. 'Interoperability Testing Results' for the list of tested Bluetooth-enabled devices.

This non-USB audio demonstration supports streaming wireless Bluetooth audio from any smartphone (i.e., Apple, Samsung, Google, etc.), PC, or Bluetooth-enabled device.

SPP communication is enabled between Bluetooth device and mobile handset for data communication during audio playback. A means of bidirectional data transfer is provided in this demonstration as an example of communication. This communication can be modified to provide any data transfer means needed by an application. In the case of the demonstration application, a means of observation is provided by exchanging some text strings both to and from the handset.

SPP and audio communications are simultaneous but on different channels. A single handset (smart phone) can establish both channels to enable control while streaming music.

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000018_22e6691812de863e7de1f7d1d828351bb7b9bb98aa2e7e62c189feac623ffd98.png)

## PIC32 BLUETOOTH ® AUDIO DEVELOPMENT KIT REFERENCE GUIDE

Microchip

## Chapter 5.  Bluetooth Stack Overview

This chapter provides a brief overview of the Microchip Bluetooth Stack for PIC32, which is used by the PIC32 Bluetooth Audio Development Kit.

The following sections are included:

- 5.1 'Bluetooth Stack Block Diagram'
- 5.2 'Bluetooth Stack Component Summary'

## 5.1 BLUETOOTH STACK BLOCK DIAGRAM

A block diagram for the stack is provided in Figure 5-1.

Note: For a complete description of the stack, refer to the application note, 'Microchip Bluetooth ®  Stack for PIC32' ( DS number pending ), which is available from the Microchip web site (www.microchip.com).

## FIGURE 5-1: MICROCHIP BLUETOOTH ®  STACK FOR PIC32 BLOCK DIAGRAM

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000019_98dca3f0b689749183fd9dad262a43fb2eb4e0f36d308ba6365596b2a5a717d5.png)

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## 5.2 BLUETOOTH STACK COMPONENT SUMMARY

Note: For a complete description of the profiles, protocols, and audio decoders for the stack, refer to the application note, 'Microchip Bluetooth ®  Stack for PIC32' ( DS number pending ), which is available from the Microchip web site (www.microchip.com).

TABLE 5-1: BLUETOOTH STACK PROFILE VERSIONS

| Profile   |   Current Version Level (as of v2.0) |
|-----------|--------------------------------------|
| AD2DP     |                                  1.3 |
| AVRCP     |                                  1.5 |
| SPP       |                                  1.2 |
| SDP       |                                  1.1 |

TABLE 5-2: BLUETOOTH STACK PROTOCOLS

| Protocol   |
|------------|
| HCI        |
| L2CAP      |
| LMP        |
| RFCOMM     |
| AVCTP      |

TABLE 5-3: BLUETOOTH STACK AUDIO DECODERS

| Audio Decoder   |
|-----------------|
| SBC             |
| AAC             |

Note: System latency from Bluetooth UART audio data to audio DAC/AMP output is 1.8 ms.

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000020_0d250624a15aa4a1a725a67230e564c6cb2c66a6dea27a06b88f8e3800ba2a00.png)

Microchip

## Appendix A. Hardware, Board Layout, and Schematics

## A.1 WIRE LIST

## TABLE A-1:

DEVELOPMENT KIT WIRE LIST

| 100-pin PIC32MX450/470 Pin Number   | 44-pin PIC32MX270F256 PIM Pin Number   | Pin Name                  | Connectors       | Connectors        | Connectors           | Connectors                                          | Connectors                                          | Name              |
|-------------------------------------|----------------------------------------|---------------------------|------------------|-------------------|----------------------|-----------------------------------------------------|-----------------------------------------------------|-------------------|
| 100-pin PIC32MX450/470 Pin Number   | 44-pin PIC32MX270F256 PIM Pin Number   | PIC32MX450/470            | Audio DAC        | Audio DAC         | HCI Bluetooth Module | HCI Bluetooth Module                                | Gesture/Apple I 2 S Port I2C2 Connector J12 (7-pin) | Name              |
| 100-pin PIC32MX450/470 Pin Number   | 44-pin PIC32MX270F256 PIM Pin Number   | PIC32MX450/470            | CONN_J9 (20-pin) | CONN_J10 (12-Pin) | CONN_J8 (20-pin)     | PICtail Plus Connector J4 Pin Out CONN_J11 (12-pin) | Gesture/Apple I 2 S Port I2C2 Connector J12 (7-pin) | Name              |
| 1                                   | 25                                     | RG15                      | 8                | -                 | 8                    | - -                                                 | -                                                   | GPIO_12/STBY/RST  |
| 2                                   | -                                      | VDD                       | 18               | -                 | 18                   | - 21, 22, 53, 54, 107, 108                          | 7                                                   | 3.3v_DIG          |
| 3                                   | -                                      | PMD5/RE5                  | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_DB5          |
| 4                                   | -                                      | PMD6/RE6                  | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_DB6          |
| 5                                   | -                                      | PMD7/RE7                  | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_DB7          |
| 6                                   | -                                      | RPC1/RC1                  | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_RESET        |
| 7                                   | -                                      | RPC2/RC2                  | -                | 1                 | 10                   | - -                                                 | -                                                   | GPIO_24/I2S2_LRCL |
| 8                                   | -                                      | RPC3/RC3                  | -                | -                 | -                    | - 13, 14                                            | -                                                   | ACC_ID_SEL        |
| 9                                   | -                                      | RPC4/RC4                  | -                | -                 | -                    | - -                                                 | 4                                                   | GPIO_17           |
| 10                                  | 15                                     | AN16/RPG6/SCK2/RG6        | -                | 3                 | 12                   | - -                                                 | -                                                   | GPIO_22/I2S2_BCLK |
| 11                                  | 11                                     | AN17/RPG7/RG7             | -                | 5                 | 11                   | - -                                                 | -                                                   | GPIO_27/SPI2_SDI  |
| 12                                  | 34                                     | AN18/RPG5/RG8             | -                | 7                 | 13                   | - -                                                 | -                                                   | GPIO_25/I2S2_SDO  |
| 13                                  | 18                                     | MCLR                      | -                | -                 | -                    | - -                                                 | 5                                                   | PIC32_MCLR#       |
| 14                                  | 5                                      | AN19/RPG9/RG9             | -                | -                 | 6                    | - -                                                 | -                                                   | GPIO_21/UART2_RTS |
| 15                                  | 6                                      | VSS                       | 1, 2, 15, 16     | 11, 12            | 1, 2, 15, 16         | 11, 12 -                                            | -                                                   | DGND              |
| 16                                  | 10                                     | VDD                       | 18               | -                 | 18                   | - 21, 22, 53, 54, 107, 108                          | 7                                                   | 3.3v_DIG          |
| 17                                  | 23                                     | RA0                       | -                | -                 | -                    | - -                                                 | -                                                   | SW1#              |
| 18                                  | -                                      | RPE8/RE8                  | -                | 6                 | -                    | - -                                                 | -                                                   | GPIO_15/SS2#      |
| 19                                  | -                                      | RPE9/RE9                  | -                | -                 | -                    | - 17                                                | -                                                   | SELECT            |
| 20                                  | -                                      | AN5/RPB5/RB5              | -                | -                 | -                    | 4 -                                                 | -                                                   | GPIO_29/AN(e)     |
| 21                                  | -                                      | AN4/RB4                   | -                | -                 | -                    | - 18                                                | -                                                   | ACC_PWR           |
| 22                                  | -                                      | AN3/RPB3/RB3              | -                | 8                 | -                    | - -                                                 | -                                                   | GPIO_16/AN( c )   |
| 23                                  | 36                                     | AN2/RPB2/RB2              | -                | -                 | 4                    | 8 -                                                 | -                                                   | GPIO_20/UART2_CTS |
| 24                                  | -                                      | AN1/RPB1/RB1              | -                | -                 | -                    | - 11                                                | -                                                   | OVERCURRENT       |
| 25                                  | -                                      | AN0/RPB0/RB0              | -                | -                 | -                    | 6 -                                                 | -                                                   | GPIO_30/SS1#      |
| 26                                  | 13                                     | PGEC2/RB6                 | -                | -                 | -                    | - -                                                 | -                                                   | PGEC2             |
| 27                                  | 12                                     | PGED2/RB7                 | -                | -                 | -                    | - -                                                 | -                                                   | PGED2             |
| 28                                  | i2ce -16                               | RA9                       | -                | -                 | -                    | - -                                                 | -                                                   | LED5#             |
| 29                                  | 38                                     | RA10                      | -                | -                 | -                    | - -                                                 | -                                                   | SW3#              |
| 30                                  | 17                                     | AVDD                      | 18               | -                 | 18 -                 | - -                                                 | 7                                                   | 3.3v_DIG          |
| 31                                  | 16                                     | AVSS                      | 1, 2, 15, 16     | 11, 12            | 1, 2, 15, 16         | 11, 12 -                                            | 1                                                   | DGND              |
| 32                                  | -                                      | AN8/RPB8/RB8              | -                | 2                 |                      | - -                                                 | -                                                   | GPIO_13/AN(a)     |
| 33                                  | -                                      | AN9/RPB9/RB9              | -                | 4                 | -                    | - -                                                 | -                                                   | GPIO_14/AN(b)     |
| 34                                  |                                        |                           | -                |                   |                      | -                                                   | -                                                   | GPIO_31/AN(f)     |
| 35                                  | 19                                     | AN10/RPB10/RB10 AN11/RB11 | -                | - -               | - -                  | 8 - -                                               | -                                                   | VOLUME            |

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## TABLE A-1: DEVELOPMENT KIT WIRE LIST

| 100-pin PIC32MX450/470 Pin Number   | 44-pin PIC32MX270F256 PIM Pin Number   | PIC32MX450/470 Pin Name   | Connectors       | Connectors        | Connectors           | Connectors                                          | Connectors                                          | Signal Name                 |
|-------------------------------------|----------------------------------------|---------------------------|------------------|-------------------|----------------------|-----------------------------------------------------|-----------------------------------------------------|-----------------------------|
| 100-pin PIC32MX450/470 Pin Number   | 44-pin PIC32MX270F256 PIM Pin Number   | PIC32MX450/470 Pin Name   | Audio DAC        | Audio DAC         | HCI Bluetooth Module | HCI Bluetooth Module                                | Gesture/Apple I 2 S Port I2C2 Connector J12 (7-pin) |                             |
| 100-pin PIC32MX450/470 Pin Number   | 44-pin PIC32MX270F256 PIM Pin Number   | PIC32MX450/470 Pin Name   | CONN_J9 (20-pin) | CONN_J10 (12-Pin) | CONN_J8 (20-pin)     | PICtail Plus Connector J4 Pin Out CONN_J11 (12-pin) | Gesture/Apple I 2 S Port I2C2 Connector J12 (7-pin) | Schematic                   |
| 36                                  | 29                                     | VSS                       | 1, 2, 15, 16     | 11, 12            | 1, 2, 15, 16         | 11, 12 -                                            | 1                                                   | DGND                        |
| 37                                  | 28                                     | VDD                       | 18               | -                 | 18                   | - 21, 22, 53, 54, 107, 108                          | 7                                                   | 3.3v_DIG                    |
| 38                                  | 24                                     | RA1                       | -                | -                 | -                    | - -                                                 | -                                                   | SW2#                        |
| 39                                  | -                                      | RPF13/RF13                | -                | -                 | -                    | - 2                                                 | -                                                   | U4RX                        |
| 40                                  | -                                      | RPF12/RF12                | -                | -                 | -                    | - 4                                                 | -                                                   | U4TX                        |
| 41                                  | 43                                     | AN12/RB12                 | -                | -                 | -                    | - -                                                 | -                                                   | SW4#                        |
| 42                                  | 41                                     | AN13RB13                  | -                | -                 | -                    | - -                                                 | -                                                   | SW5#                        |
| 43                                  |                                        | AN14/RB14                 | -                | -                 | -                    | - -                                                 | -                                                   | USB_SEL#                    |
| 44                                  |                                        | PMPA0/RB15                | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_D/C#                   |
| 45                                  | 39                                     | VSS                       | 1, 2, 15, 16     | 11, 12            | 1, 2, 15, 16         | 11, 12 -                                            | 1                                                   | DGND                        |
| 46                                  | 40                                     | VDD                       | 18               | -                 | 18                   | - 21, 22, 53, 54, 107, 108                          | 7                                                   | 3.3v_DIG                    |
| 47                                  | -                                      | RPD14/RD14                | -                | -                 | -                    | - 19                                                | -                                                   | C_RESET                     |
| 48                                  | -                                      | RPD15/RD15                | -                | -                 | -                    | - 20                                                | -                                                   | IPOD_DETECT                 |
| 49                                  | -                                      | RPF4/RF4                  | -                | -                 | 3                    | - -                                                 | -                                                   | GPIO_18/UART2_RX            |
| 50                                  | 37                                     | RPF5/RF5                  | -                | -                 | 5                    | - -                                                 | -                                                   | GPIO_19/UART2_TX            |
| 51                                  | -                                      | USBID/RF3                 | -                | -                 | -                    | 2 -                                                 | -                                                   | GPIO_28/AN(d)               |
| 52                                  | 21                                     | RPF2/RF2                  | -                | -                 | -                    | - -                                                 | -                                                   | FLASH_CS#                   |
| 53                                  | 26                                     | RPF8/RF8                  | 14               | -                 | 14                   | - -                                                 | -                                                   | GPIO_5/I2S1_MCLK            |
| 54                                  | -                                      | VBUS                      | -                | -                 | -                    | - -                                                 | 6                                                   | 5V_SW_DIG                   |
| 55                                  | -                                      | VUSB3V3                   | -                | -                 | -                    | - -                                                 | 7                                                   | 3.3V_DIG                    |
| 56                                  | 9                                      | D-/RG3                    | -                | -                 | -                    | - -                                                 | -                                                   | PIC32_D-                    |
| 57                                  | 8                                      | D+/RG2                    | -                | -                 | -                    | - -                                                 | -                                                   | PIC32_D+                    |
| 58                                  | -                                      | SCL2/RA2                  | -                | -                 | 7                    | - 6                                                 | 2                                                   | GPIO_23/I2C2_SCL            |
| 59                                  |                                        | SDA2/RA3                  | -                | -                 | 9                    | - 8                                                 | 3                                                   | GPIO_26/I2C2_SDA            |
| 60                                  | - i2ce -12                             | RA4                       | -                | -                 | -                    | - -                                                 | -                                                   | LED1#                       |
| 61                                  | i2ce -13                               | RA5                       | -                | -                 | -                    | - -                                                 | -                                                   | LED2#                       |
| 62                                  | -                                      | VDD                       | 18               | -                 | 18                   | - 21, 22, 53, 54, 107,108                           | 7                                                   | 3.3V_DIG                    |
| 63                                  | 30                                     | OSC1                      | -                | -                 | -                    | - -                                                 | -                                                   | XTAL (8 MHz)                |
| 64                                  | 31                                     | OSC2                      | -                | -                 | -                    | - -                                                 | -                                                   | XTAL (8 MHz)                |
| 65                                  | -                                      | VSS                       | 1, 2, 15, 16     | 11, 12            | 1, 2, 15, 16         | 11, 12 -                                            | 1                                                   | DGND                        |
| 66                                  | 44                                     | SCL1/RPA14/RA14           | 7                | -                 | -                    | - -                                                 | -                                                   | GPIO_7/I2C1_SCL             |
| 67                                  | 1                                      | SDA1/RPA15/RA15           | 9                | -                 | -                    | - -                                                 | -                                                   | GPIO_10/I2C1_SDA            |
| 68                                  | -                                      | RPD8/RD8                  |                  | -                 | -                    | - -                                                 | -                                                   | ALERT#                      |
| 69                                  | 33                                     | RPD9/RD9                  | 10               | -                 | -                    | 1 -                                                 | -                                                   | GPIO_8/I2S1_LRCL            |
| 70                                  | 14                                     | RPD10/SCK1/RD10           | 12               | -                 | -                    | -                                                   | -                                                   | GPIO_6/I2S1_BCLK            |
| 71                                  | -                                      | PMCS1/RD11                |                  |                   |                      | 3                                                   |                                                     |                             |
|                                     | 35                                     | RPD0/INT0/RD0             |                  | -                 | -                    | - - 7                                               | - -                                                 | LCD1_CS#                    |
| 72                                  |                                        |                           | 13               | -                 | -                    | -                                                   |                                                     | GPIO_9/I2S1_SDO             |
| 73                                  | -                                      | RPC13/RC13                | 11               | -                 | -                    | 5 -                                                 | -                                                   | GPIO_11/SPI1_SDI            |
| 74                                  | -                                      | RPC14/RC14                | -                | -                 | -                    | - 65                                                | -                                                   | IPOD_VBUS                   |
| 75                                  | -                                      | VSS                       | 1, 2, 15, 16     | 11, 12            | 1, 2, 15, 16         | 11, 12 -                                            | 1                                                   | DGND                        |
| 76                                  | 27                                     | AN24/RPD1/RD1             | 6                | -                 | -                    | - -                                                 | -                                                   | GPIO_4/UART1_RTS            |
| 77                                  | 2                                      | AN25/RPD2/RD2             | 3                | -                 | -                    | - -                                                 | -                                                   | GPIO_1/UART1_Rx             |
| 78                                  | 3                                      | AN26/RPD3/RD3             | 5                | -                 | -                    | - -                                                 | -                                                   | GPIO_2/UART1_Tx             |
| 79                                  |                                        |                           |                  |                   |                      |                                                     |                                                     |                             |
| 80                                  | 22 20                                  | RPD12/RD12 RD13           | 4 -              | - -               | - -                  | - - - -                                             | - -                                                 | GPIO_3/UART1_CTS VBUS_SENSE |

## TABLE A-1: DEVELOPMENT KIT WIRE LIST

| 100-pin PIC32MX450/470 Pin Number   | 44-pin PIC32MX270F256 PIM Pin Number   | PIC32MX450/470 Pin Name   | Connectors       | Connectors        | Connectors           | Connectors                                          | Connectors                                          |                        |
|-------------------------------------|----------------------------------------|---------------------------|------------------|-------------------|----------------------|-----------------------------------------------------|-----------------------------------------------------|------------------------|
| 100-pin PIC32MX450/470 Pin Number   | 44-pin PIC32MX270F256 PIM Pin Number   | PIC32MX450/470 Pin Name   | Audio DAC        | Audio DAC         | HCI Bluetooth Module | HCI Bluetooth Module                                | Gesture/Apple I 2 S Port I2C2 Connector J12 (7-pin) | Schematic Signal Name  |
| 100-pin PIC32MX450/470 Pin Number   | 44-pin PIC32MX270F256 PIM Pin Number   | PIC32MX450/470 Pin Name   | CONN_J9 (20-pin) | CONN_J10 (12-Pin) | CONN_J8 (20-pin)     | PICtail Plus Connector J4 Pin Out CONN_J11 (12-pin) | Gesture/Apple I 2 S Port I2C2 Connector J12 (7-pin) | Schematic Signal Name  |
| 81                                  | -                                      | PMWR/RD4                  | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_WR#               |
| 82                                  | -                                      | PMRD/RD5                  | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_RD#               |
| 83                                  | 32                                     | RPD6/RD6                  | -                | -                 | -                    | - -                                                 | -                                                   | A_DET#                 |
| 84                                  | i2ce -19                               | RPD7/RD7                  | -                | -                 | -                    | - -                                                 | -                                                   | GPIO_32                |
| 85                                  | -                                      | VCAP                      | -                | -                 | -                    | - -                                                 | -                                                   | 10 µF/0.1 µF Caps Only |
| 86                                  | -                                      | VDD                       | 18               | -                 | 18                   | - 21, 22, 53, 54, 107, 108                          | 7                                                   | 3.3v_DIG               |
| 87                                  | -                                      | RPF0/RF0                  | -                | 9                 | -                    | - -                                                 | -                                                   | GPIO_33/PD1#           |
| 88                                  | -                                      | RPF1/RF1                  | -                | -                 | -                    | - -                                                 | -                                                   | GPIO_34/PD2#           |
| 89                                  | i2ce -17                               | RPG1/RG1                  | -                | -                 | - -                  | -                                                   | -                                                   | USB_SEL1               |
| 90                                  | i2ce -18                               | RPG0/RG0                  | -                | -                 | -                    | - -                                                 | -                                                   | USB_SEL0               |
| 91                                  | i2ce -14                               | RA6                       | -                | -                 | -                    | - -                                                 | -                                                   | LED3#                  |
| 92                                  | i2ce -15                               | RA7                       | -                | -                 | -                    | - -                                                 | -                                                   | LED4#                  |
| 93                                  | -                                      | PMD0/RE0                  | -                | -                 | - -                  | -                                                   | -                                                   | LCD1_DB0               |
| 94                                  | -                                      | PMD1/RE1                  | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_DB1               |
| 95                                  | -                                      | RG14                      | -                | -                 | -                    | - -                                                 | -                                                   | USB_ACC#               |
| 96                                  | -                                      | RG12                      | -                | -                 | -                    | - -                                                 | -                                                   | USB_MINI_B#            |
| 97                                  | -                                      | RG13                      | -                | -                 | -                    | - -                                                 | -                                                   | USB_TYPE_A#            |
| 98                                  | -                                      | PMD2/RE2                  | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_DB2               |
| 99                                  | -                                      | PMD3/RE3                  | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_DB3               |
| 100                                 | -                                      | PMD4/RE4                  | -                | -                 | -                    | - -                                                 | -                                                   | LCD1_DB4               |
| -                                   | -                                      | -                         | -                | -                 | -                    | - 29, 62                                            | -                                                   | ACC_D-                 |
| -                                   | -                                      | -                         | -                | -                 | -                    | - 27, 60                                            | -                                                   | ACC_D+                 |
| -                                   | -                                      | -                         | 20               | -                 | 20                   | - -                                                 | 6                                                   | 5V_SW_DIG              |
| -                                   | -                                      | -                         | 17, 19           | -                 | 17, 19               | - -                                                 | -                                                   | Power                  |
| -                                   | -                                      | -                         | 1, 2, 15, 16     | -                 | 1, 2, 15, 16         | - -                                                 | 1                                                   | DGND                   |
| -                                   | -                                      | -                         |                  | -                 | -                    | - 25, 26, 57, 58                                    | -                                                   | ACC_+9V                |

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## A.2 DAUGHTER BOARD PIN MAPPING

## TABLE A-2: DAUGHTER BOARD PIN MAPPING

| Audio DAC Daughter Board   | Audio DAC Daughter Board   | Audio DAC Daughter Board   | Audio DAC Daughter Board   |
|----------------------------|----------------------------|----------------------------|----------------------------|
| Pin Name                   | Pin Number                 | Pin Number                 | Pin Name                   |
| DGND                       | 1                          | 2                          | DGND                       |
| GPIO_1/UART1_Rx            | 3                          | 4                          | GPIO_3/UART1_CTS           |
| GPIO_2/UART1_Tx            | 5                          | 6                          | GPIO_4/UART1_RTS           |
| GPIO_7/I2C1_SCL            | 7                          | 8                          | GPIO_12/STBY/RST           |
| GPIO_10/I2C1_SDA           | 9                          | 10                         | GPIO_8/I2S1_LRCL           |
| GPIO_11/SPI1_SDI           | 11                         | 12                         | GPIO_6/I2S1_BCLK           |
| GPIO_9/I2S1_SDO            | 13                         | 14                         | GPIO_5/I2S1_MCLK           |
| DGND                       | 15                         | 16                         | DGND                       |
| Power                      | 17                         | 18                         | 3.3V_DIG                   |
| Power                      | 19                         | 20                         | 5V_SW_DIG                  |

| GPIO_24/I2S2_LRCL   |   1 |   2 | GPIO_13/AN(a)   |
|---------------------|-----|-----|-----------------|
| GPIO_22/I2S2_BCLK   |   3 |   4 | GPIO_14/AN(b)   |
| GPIO_27/SPI2_SDI    |   5 |   6 | GPIO_15/SS2#    |
| GPIO_25/I2S2_SDO    |   7 |   8 | GPIO_16/AN(c )  |
| PD1#                |   9 |  10 | NC              |
| AGND                |  11 |  12 | AGND            |

| BTM805 Bluetooth Daughter Board   | BTM805 Bluetooth Daughter Board   | BTM805 Bluetooth Daughter Board   | BTM805 Bluetooth Daughter Board   |
|-----------------------------------|-----------------------------------|-----------------------------------|-----------------------------------|
| Pin Name                          | Pin Number                        | Pin Number                        | Pin Name                          |
| DGND                              | 1                                 | 2                                 | DGND                              |
| GPIO_18/UART2_RX                  | 3                                 | 4                                 | GPIO_20/UART2_CTS                 |
| GPIO_19/UART2_TX                  | 5                                 | 6                                 | GPIO_21/UART2_RTS                 |
| GPIO_23/I2C2_SCL                  | 7                                 | 8                                 | GPIO_12/STBY/RST                  |
| GPIO_26/I2C2_SDA                  | 9                                 | 10                                | GPIO_24/I2S2_LRCL                 |
| GPIO_27/SPI2_SDI                  | 11                                | 12                                | GPIO_22/I2S2_BCLK                 |
| GPIO_25/I2S2_SDO                  | 13                                | 14                                | GPIO_5/I2S1_MCLK                  |
| DGND                              | 15                                | 16                                | DGND                              |
| Power                             | 17                                | 18                                | 3.3V_DIG                          |
| Power                             | 19                                | 20                                | 5V_SW_DIG                         |

| GPIO_8/I2S1_LRCL   |   1 |   2 | GPIO_28/AN(d)   |
|--------------------|-----|-----|-----------------|
| GPIO_6/I2S1_BCLK   |   3 |   4 | GPIO_29/AN(e)   |
| GPIO_11/SPI1_SDI   |   5 |   6 | GPIO_30/SS#1    |
| GPIO_9/I2S1_SDO    |   7 |   8 | GPIO_31/AN(f)   |
| PD2#               |   9 |  10 | NC              |
| AGND               |  11 |  12 | AGND            |

## A.3 BLOCK DIAGRAM

FIGURE A-1: HIGH-LEVEL BLOCK DIAGRAM OF THE PIC32 BLUETOOTH AUDIO DEVELOPMENT KIT

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000021_6ed8a38400cf7c93f219d02f5e95558af858183975105848cbcf8e51ba966581.png)

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## A.4 BOARD LAYOUT

FIGURE A-2: PIC32 BLUETOOTH ®  AUDIO DEVELOPMENT BOARD LAYOUT (TOP ASSEMBLY)

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000022_b528d0f18304c66763c77ea3e80a64f36486d16df0ed4846c9ad6bc2836fbb1b.png)

FIGURE A-3: BLUETOOTH HCI RADIO MODULE DAUGHTER BOARD LAYOUT (TOP ASSEMBLY)

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000023_cd7e55c7635beaaf3f707bb5853688a9160284291dc405ae4004717117dda50e.png)

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

## FIGURE A-4: AUDIO DAC/AMP DAUGHTER BOARD LAYOUT (TOP ASSEMBLY)

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000024_3df20db38b02539eb3da322975b7f9930449761e0694dd0335cb5ed8832d9743.png)

## A.5 SCHEMATICS

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000025_4b6b144c7a2cc6d5402ad599dd139d9afd79e6829a4bb3ac90ddcbc0458b8082.png)

FIGURE A-6:

POWER

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000026_15ec9afa984ee1a513902e1a8fc6eb98a546df28216c284a91587b24c168b265.png)

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000027_295e5603be7a43017ebc6b412444434e5a6046971aafded02e74912217049a34.png)

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

FIGURE A-8: PUSH BUTTONS AND LEDS

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000028_c5a307073b17b22045dfd23b48db8f49cdb6c11f5b30ae5c6342f4b23358df6f.png)

FIGURE A-9: ICSP™

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000029_6c8491f053018aa6286d5e55ae40779444b24a71eae9edb535afb3c810dd413c.png)

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000030_3cd925e4c7f697871c715b5316761146e8b803136ac9a37133c12f9513a3cffc.png)

FIGURE A-11: AUDIO DAC/AMP DAUGHTER BOARD

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000031_564016eda612eaf90e7158228d07bf166ed1357c636dc3850d0b9cb24d866218.png)

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000032_22e6691812de863e7de1f7d1d828351bb7b9bb98aa2e7e62c189feac623ffd98.png)

## PIC32 BLUETOOTH ® AUDIO DEVELOPMENT KIT REFERENCE GUIDE

Microchip

## Appendix B. Bill of Materials (BOM)

## B.1 PIC32 BLUETOOTH AUDIO DEVELOPMENT KIT BILL OF MATERIALS

TABLE B-1: PIC32 BLUETOOTH AUDIO DEVELOPMENT BOARD BOM

| Reference                                                           | Description                       | Manufacturer                     | Part No.            |
|---------------------------------------------------------------------|-----------------------------------|----------------------------------|---------------------|
| C1, C4, C11, C13, C17, C20, C23, C26, C27, C29, C30, C31, C35, C36, | Cap, Ceramic, 0.1 µF, 50V X7R     | TDK Corporation                  | C1608X7R1H104M      |
| C2                                                                  | CAP ALUM 47 µF 25V 20% SMD        | United Chemi-Con                 | EMVA250ADA470MF55G  |
| C3, C5, C6                                                          | CAP CER 10 µF 25V 10% X5R 1206    | Taiyo Yuden                      | TMK316BJ106KL-T     |
| C7                                                                  | CAP CER 0.022 µF 50V 20% X7R 0603 | Murata Electronics North America | GRM188R71H223MA01D  |
| C8, C10                                                             | CAP CER 22 µF 16V 10% X5R 0805    | TDK Corporation                  | C2012X5R1C226K      |
| C9, C38                                                             | Cap, Ceramic, 1 µF, 16V X5R       | TDK Corporation                  | C1608X5R1C105K      |
| C12, C14, C21                                                       | Cap, Ceramic, 10 µF, 16V X5R      | Taiyo Yuden                      | EMK212BJ106MG-T     |
| C15                                                                 | CAP CER 100 µF 10V 20% X5R 1206   | TDK Corporation                  | C3216X5R1A107M      |
| C16                                                                 | CAP CER 2.2 µF 10V 20% X5R 0603   | TDK Corporation                  | C1608X5R1A225M/0.80 |
| C18, C19, C22, C25, C32, C33, C34                                   | Cap, Ceramic, 0.01 µF, 50V X7R    | TDK Corporation                  | C1608X7R1H103M      |
| C24, C28                                                            | CAP CER 18 pF 50V 5% C0G 0603     | Murata Electronics North America | GRM1885C1H180JA01D  |
| D1, D3                                                              | DIODE SCHOTTKY 40V 2A SOD123H     | Comchip Technology               | CDBMT240-HF         |
| D2                                                                  | DIODE ZENER 13.79V 500 mW SOD323F | Diodes Inc.                      | DDZ15ASF-7          |
| D4                                                                  | DIODE ZENER 3.3V 500 mW SOD123    | Diodes Inc.                      | MMSZ5226B-7-F       |
| D5, D6, D7, D8, D9                                                  | LED, SMD, RED, 0603 package       | Kingbright Corp.                 | APT1608EC           |
| D10, D11, D12                                                       | LED, SMD, YEL, 0603 package       | Kingbright Corp.                 | APT1608YC           |
| D13                                                                 | LED, SMD, GRN, 0603 package       | Kingbright Corp.                 | APT1608SGC          |
| F1                                                                  | FUSE 2A 125V 1206 FAST C1Q        | Bel Fuse Inc.                    | C1Q 2               |
| J1                                                                  | CONN POWERJACK MINI R/A PCMT      | Switchcraft Inc.                 | RAPC712X            |

Note  1: Available on v1.0 and v2.0 development boards.

- 2: Available on v3.0 and later development boards.
- 3: Available on all v1.0 and select v2.0 and v3.0 development boards.
- 4: Available on most v2.0 and all v3.0 and later development boards.

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

TABLE B-1: PIC32 BLUETOOTH AUDIO DEVELOPMENT BOARD BOM (CONTINUED)

| Reference                                                               | Description                                     | Manufacturer                    | Part No.          |
|-------------------------------------------------------------------------|-------------------------------------------------|---------------------------------|-------------------|
| J2                                                                      | Receptacle, Mini-USB, UX60-MB-5ST, Type B       | Hirose Electric Co., Ltd.       | UX60-MB-5ST       |
| J3                                                                      | CONN USB TYPE A R/A BLACK                       | On Shore Technology Inc.        | USB-A1HSB6        |
| J4 (DNP)                                                                | CONN_MEC1-160-CE                                |                                 |                   |
| J5, J6                                                                  | CONN HEADER 3 POS .100" SGL GOLD                | Samtec Inc.                     | TSW-103-07-G-S    |
| J5,J6 (+5V)                                                             | SHUNT JUMPER .1" BLACK GOLD                     | 3M                              | 969102-0000-DA    |
| J7                                                                      | CONN HEADER 6 POS .100 R/A 30 AU                | FCI                             | 68016-106HLF      |
| J8, J9                                                                  | CONN HEADER 20 POS .100" DL TIN                 | Samtec Inc.                     | TSW-110-07-T-D    |
| J10, J11                                                                | Terminal Strip, 2 x 6, 0.100 sp, 0.025 SQ. Post | Samtec Inc.                     | TSW-106-07-F-D    |
| J12                                                                     | CONN HEADER 7POS .100" SNGL TH                  | FCI                             | 68016-107H        |
| L1                                                                      | 4.7 µH 20% Isat 3.2A                            | CoilCraft                       | LPS6225-472M      |
| LCD1                                                                    | DISPLAY_FPC _37PIN                              |                                 |                   |
| P1                                                                      | CONN HEADER 25 POS SNGL 1.27 mm T/H             | OUPIIN                          | 2246-1*25GOOSU    |
| P2                                                                      | Potentiometers 10mm Linear 5K PC Mount          | Alpha (Taiwan)                  | RV100F-30-4K1-B53 |
| Q1                                                                      | TRANS BIPO NPN 300MW 45V SOT 23-3               | Diodes Inc.                     | BC847B-7-F        |
| Q2                                                                      | MOSFET N-CH 60V 300 mA SOT 223                  | ON Semiconductor                | MMFT960T1G        |
| Q3                                                                      | MOSFET P-CH 30V 3.8A SOT 23-3                   | Diodes Inc.                     | DMP3098L-7        |
| R1, R25                                                                 | RES 1 k Ω 1/10W 1% 0603 SMD                     | Stackpole Electronics Inc.      | RMCF0603FT1K00    |
| R2, R5, R6, R18, R19, R20, R21, R22, R23, R35, R36, R39, R40, R42, R43, | RES 20.0 k Ω 1/10W 1% 0603 SMD                  | Yageo                           | RC0603FR-0720KL   |
| R3, R4, R10, R11, R12, R13                                              | Res, 3.3K 1/10W 1%                              | Stackpole Electronics Inc.      | RMCF0603FT3K30    |
| R7, R24                                                                 | RES 10 k Ω 1/10W 1% 0603 SMD                    | Stackpole Electronics Inc.      | RMCF0603FT10K0    |
| R8                                                                      | RES 120 k Ω 1/10W 1% 0603 SMD                   | Yageo                           | RC0603FR-07120KL  |
| R9                                                                      | RES 4.99 k Ω 1/10W 1% 0603 SMD                  | Yageo                           | RC0603FR-074K99L  |
| R14, R16, R17                                                           | RESISTOR 6.8 Ω 1/10W 1% 0603                    | Panasonic Electronic Components | ERJ-3RQF6R8V      |
| R15, R37, R38                                                           | RES 0.0 Ω 1/10W 0603 SMD                        | Rohm Semiconductor              | MCR03EZPJ000      |
| R26, R27, R28, R29, R30, R31, R32, R33, R34                             | RES 470 Ω 1/10W 1% 0603 SMD                     | Rohm Semiconductor              | MCR03EZPFX4700    |

- Note  1: Available on v1.0 and v2.0 development boards.
- 2: Available on v3.0 and later development boards.
- 3: Available on all v1.0 and select v2.0 and v3.0 development boards.
- 4: Available on most v2.0 and all v3.0 and later development boards.

TABLE B-1: PIC32 BLUETOOTH AUDIO DEVELOPMENT BOARD BOM (CONTINUED)

| Reference                         | Description                         | Manufacturer              | Part No.              |
|-----------------------------------|-------------------------------------|---------------------------|-----------------------|
| R41                               | RES 150 k Ω 1/10W 5% 0603 SMD       | Yageo                     | RC0603JR-07150KL      |
| S1                                | SW SLIDE DPDT 6 VDC 0.3A PCMNT      | C&K Components            | JS202011CQN           |
| SW1, SW2, SW3, SW4, SW5, SW6, SW7 | SWITCH TACT 160 GF H = 5.0 mm SMT   | E-Switch                  | TL3301AF160QG         |
| TP1 (DNP)                         | TEST POINT PC MINI .040"D WHITE     | Keystone Electronics      | 5002                  |
| TP2, TP3 (DNP)                    | TEST POINT PC MINI .040"D BLACK     | Keystone Electronics      | 5001                  |
| TP4, TP5 (DNP)                    | TEST POINT PC MINI .040"D RED       | Keystone Electronics      | 5000                  |
| U1                                | IC REG BUCK SYNC 5V 3A 16 VQFN      | Microchip Technology Inc. | MCP16323T-500E/NG     |
| U2                                | IC USB SWITCH 3:1 AUD/MHL 12 UQFN   | ON Semiconductor          | NCN1188MUTAG          |
| U3                                | IC FILTER COMMON MODE ESD 8WDFN     | ON Semiconductor          | EMI2121MTTAG          |
| U4                                | USB Port PWR controller with CHRG   | Microchip Technology Inc. | UCS1001-4             |
| U5                                | IC LDO REG 500 mA 3.3V SOT 223-3    | Microchip Technology Inc. | MCP1825S-3302E/DB     |
| U6 (1)                            | IC FLASH SER 2 MB 80 MHz SPI 8 SOIC | Microchip Technology Inc. | SST25VF020B-80-4C-SAE |
| U6 (2)                            | IC FLASH SER 8 MB 80 MHz SPI 8 SOIC | Microchip Technology Inc. | SST25VG08B-80-4l-S2AE |
| U7 (3)                            | IC MCU 32-bit 256 KB FLASH 100 TQFP | Microchip Technology Inc. | PIC32MX470F256L       |
| U7 (4)                            | IC MCU 32-bit 512 KB FLASH 100 TQFP | Microchip Technology Inc. | PIC32MX470F512L       |
| Y1                                | CRYSTAL 12 MHz 18 pF SMD            | TXC CORPORATION           | 7A-12.000MAAJ-T       |
| Rubber Feet                       | BUMPON CYLINDRICAL .312X.215 BLK    | 3M                        | SJ61A6                |

- 2: Available on v3.0 and later development boards.
- 3: Available on all v1.0 and select v2.0 and v3.0 development boards.
- 4: Available on most v2.0 and all v3.0 and later development boards.

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

TABLE B-2: BLUETOOTH HCI RADIO MODULE DAUGHTER BOARD BOM

| Reference              | Description                     | Manufacturer           | Part No.            |
|------------------------|---------------------------------|------------------------|---------------------|
| C1, C2, C3, C4, C6, C7 | CAP CER 2.2UF 10V 20% X5R 0603  | TDK Corporation        | C1608X5R1A225M/0.80 |
| C5                     | Cap, Ceramic, 0.1 µF, 50V X7R   | TDK Corporation        | C1608X7R1H104M      |
| D1                     | LED, SMD, GRN, 0603 package     | Kingbright Corporation | APT1608SGC          |
| J1                     | CONN RCPT .100" 20 POS DUAL TIN | Samtec Inc.            | SLW-110-01-T-D      |
| J2                     | CONN RCPT .100" 12 POS DUAL TIN | Samtec Inc.            | SLW-106-01-T-D      |
| R1                     | RES 470 Ω 1/10W 1% 0603 SMD     | Rohm Semiconductor     | MCR03EZPFX4700      |
| R2                     | RES 150 k Ω 1/10W 5% 0603 SMD   | Yageo                  | RC0603JR-07150KL    |
| U1                     | Dual-mode Bluetooth HCI         | Flaircomm              | BTM805CL2B          |

## TABLE B-3: 24-BIT AUDIO DAC/AMP DAUGHTER BOARD BOM

| Reference           | Description                       | Manufacturer               | Part No.         |
|---------------------|-----------------------------------|----------------------------|------------------|
| C1                  | CAP CER 10 µF 25V 10% X5R 1206    | Taiyo Yuden                | TMK316BJ106KL-T  |
| C2, C3, C5, C8, C10 | Cap, Ceramic, 0.1 µF, 50V X7R     | TDK Corporation            | C1608X7R1H104M   |
| C4, C6, C7, C9, C12 | CAP CER 10 µF 16V 20% X7R 1206    | TDK Corporation            | C3216X7R1C106M   |
| C11, C13            | Cap, Ceramic, 1 µF, 16V X5R       | TDK Corporation            | C1608X5R1C105K   |
| C14, C15            | CAP CER 0.47 µF 10V 10% X5R 0603  | TDK Corporation            | C1608X5R1A474K   |
| C16, C17            | CAP CER 68 pF 50V 5% NP0 0603     | TDK Corporation            | C1608C0G1H680J   |
| D1                  | DIODE SCHOTTKY 1A 40V SMA         | Fairchild Semiconductor    | SS14             |
| D2, D3              | LED, SMD, GRN, 0603 package       | Kingbright Corporation     | APT1608SGC       |
| J1                  | CONN RCPT .100" 20 POS DUAL TIN   | Samtec Inc.                |                  |
| J2                  | CONN RCPT .100" 12 POS DUAL TIN   | Samtec Inc.                | SLW-106-01-T-D   |
| J3                  | CONN JACK STEREO 5 POS 3.5 mm SMD | CUI Inc.                   | SJ1-3515-SMT     |
| Q1, Q2              | TRANSISTOR DARL PNP 30V SOT23-3   | Diodes Inc.                | MMBTA64-7-F      |
| R1, R2              | RES 7.5 k Ω 1/10W 1% 0603 SMD     | Stackpole Electronics Inc. | RMCF0603FT7K50   |
| R3, R6, R7, R8, R11 | RES 20.0 k Ω 1/10W 1% 0603 SMD    | Yageo                      | RC0603FR-0720KL  |
| R4, R5              | RES 470 Ω 1/10W 1% 0603 SMD       | Rohm Semiconductor         | MCR03EZPFX4700   |
| R9, R10             | RES 30K Ω 1/10W 5% 0603 SMD       | Yageo                      | RC0603JR-0730KL  |
| R12, R13            | RES 33 Ω 1/10W 1% 0603 SMD        | Stackpole Electronics Inc. | RMCF0603FT33R0   |
| R14                 | RES 150 Ω 1/10W 1% 0603 SMD       | Stackpole Electronics Inc. | RMCF0603FT150R   |
| R15                 | RES 150K Ω 1/10W 5% 0603 SMD      | Stackpole Electronics Inc. | RC0603JR-07150KL |
| U1                  | IC REG LDO 5V .8A SOT223-4        | Texas Instruments          | TLV1117-50IDCY   |
| U2                  | 106 dB 192 kHz 24-bit 2-ch ∆Σ DAC | AKM                        | AK4384VT         |
| U3                  | Stereo Capless HP-Amp             | AKM                        | AK4201           |

## FIGURE B-1: DAUGHTER BOARD DIMENSIONS

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000033_62dca62fb791415a3365430c476c29d73f1427fdb46ac149920076b8a202183d.png)

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

NOTES:

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000034_0d250624a15aa4a1a725a67230e564c6cb2c66a6dea27a06b88f8e3800ba2a00.png)

## PIC32 BLUETOOTH ® AUDIO DEVELOPMENT KIT REFERENCE GUIDE

Microchip

## Appendix C. Frequently Asked Questions (FAQ)

Question 1: When I  connect  my  smartphone  or  tablet  to  the  USB  connector,  the Bluetooth Audio Development Board shuts down, operates intermittently, or resets continuously.

## Answer:

The Bluetooth Audio Development Board hardware supports smart/fast charging, which requires a power source capable of delivering 9V @ 1.7 amp or 12V @ 1.25 amp. Some smartphones require up to 1.5 amp and tablets up to 2.5 amp of charging current supplied over the Type-A USB connector from the 'Bluetooth Audio Development Board'. If the user is using an underrated power source and the externally connected USB device has entered a fast charging/playback mode then the input voltage from the external power source will drop below the operational levels of the on-board regulators resulting in intermittent power cycling and/or loss. Disconnect all USB devices, if the board powers up normally then replace the power source with an appropriate supply.

Question 2: With no USB connected, when I connect the power source, the Bluetooth Audio Development Board does not power up normally, no power LED indication, and/or it operates intermittently.

Answer 1:

There is an overvoltage protection circuit that will trip and shut down all power to the development board as long as the input power source voltage level exceeds ~14.4V. This is a safety measure as some of the on-board LDO regulators have a maximum 15V input range specification, specifically the DAC/AMP Daughter Board. To confirm, measure the external power supply input level on the power connector and if it's greater than 13.5V, switch to an appropriate 9-12 volt power supply rated for 9V @ 1.7 amp or 12V @ 1.25 amp, respectively.

Answer 2:

Answer 3:

Answer 4:

If you are attempting to power the development board in an automotive application, the typical accessory or auxiliary power outlet in a vehicle is on the non-regulated side of the electrical system with respect to the alternator. When the engine RPM is sufficiently high enough, the voltage output of the alternator can rise above the 14.4V overvoltage protection circuit trip point. This can be confirmed if the development board works when the vehicle engine is at idle versus a higher RPM.

If the power input is less than 14V and there is still no power LED indication, it is possible that the overcurrent protection fuse is damaged. To confirm, with power applied, use a DVM and measure from each side of the fuse to ground. If both readings are not the same, replace the fuse.

Ensure that the Master Clear (MCLR) Reset select slide switch, which is located to the left of the CPU, is in the appropriate position. The default position, if using the soldered down CPU on the board, is PIC32\_MCLR. However, if you are using a PIC32 Plug-In Module (PIM), the slide switch must in the PIM\_MCLR position.

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

Question 3:

Answer:

Question 4:

Answer:

The manual reset button does not reset the board and/or the graphics screen is a solid background color with no text or graphics.

Ensure that the Master Clear (MCLR) Reset select slide switch, which is located to the left of the CPU, is in the appropriate position. The default position, if using the soldered down CPU on the board, is PIC32\_MCLR. However, if you are using a PIC32 Plug-In Module (PIM), the slide switch must in the PIM\_MCLR position.

I cannot establish a Bluetooth connection even though it's discoverable on my smartphone or Bluetooth enabled device.

Open the Bluetooth settings on your smartphone and from the list of devices, select the detail icon immediately to the right of the Bluetooth device name in question. For Apple devices, select Forget, and for Android, select unpair. At this point, you should be able to rediscover and select the device for connect and pairing. This is the expected recovery behavior, if for example, the user presses SW1 and forces the Bluetooth Audio Development Board to unpair.

Question 5: When connected over the USB port, my Apple audio device will not play

music; however, other non-Apple devices seem to work fine.

Answer:

Question 6:

Answer:

Question 7:

Answer:

Only demonstrations with the 'A' suffix, denoting Apple USB compatibility, can be used with USB audio, which contain the iAP/MFi software layers. In addition, special Apple hardware available only to Apple MFi licensees must be utilized. For more information, please contact applesupport@microchip.com. Refer to Figure 1-1 in Chapter 1. 'Introduction' for a diagram that shows several Apple controlled hardware solutions that are compatible with the development kit.

I do not hear any music even though I successfully connected either to the Bluetooth or USB port.

The master volume control, located on the upper left side of the development board below the audio headphone/line-out connector, is independent of the Bluetooth, as well as any USB audio device's volume control. Adjust the control counter-clockwise to increase the volume. Visually, the user can determine the cause of the issue based on either of the following two conditions:

- The music 'progress bar' on the smartphone, as well as the graphics display on the audio development board, is advancing as expected.
- The 'Left' and 'Right' green stereo LEDs are not active on the Audio DAC/HP AMP Daughter Bard.

When I connect and pair with a Bluetooth smartphone it connects and pairs briefly, but then disconnects.

For Android smartphones this is a standard procedure. After the user manually selects the desired Bluetooth device for the first time from their smartphone, it will briefly connect and pair. However, unlike Apple devices, if the Android device successfully connects and pairs, it will then disconnect, and then move the Bluetooth device to the Android smartphone's 'paired' list, at which time the user must again (for a second time), manually select the Bluetooth device from the paired list to reconnect.

Question 8:

Answer 1:

Answer 2:

Answer 3:

Question 9:

Answer:

When I connect my Samsung USB audio device to the mini-B USB connector I cannot play audio from the device.

Ensure that the Bluetooth connection is not active or that a USB Type-A audio device was not the last device connected. Remember that a Bluetooth connection takes priority over a USB connection, and the last USB connected device takes precedence over any other USB connection.

Ensure that you are using a demonstration that includes USB audio support. See Table 4-1 in Chapter 4. 'Bluetooth Audio Demonstrations' for details.

Ensure that you are using the proper connector scheme to enable USB master function via the mini-B connector. This should have the form of a cable that connects to the Samsung Type-B micro connector to a Type-A, and then cabling from the Type-A to the USB mini-B connector on the Bluetooth Audio Development Kit. The user must use the mini-B connector and not use the Type-A connector when interfacing to a Samsung device.

What Bluetooth module is used in this development kit?

The Flaircomm FLC-BTM805CL2B Bluetooth HCI module is used in this development kit. For more information, please visit http://www.flairmicro.com/en/index.aspx.

Question 10: Why do multiple Bluetooth Audio Development Board demonstrations with the same name appear in the list of Bluetooth devices on my smartphone?

Answer:

If a Bluetooth connection is made with a Bluetooth Audio Development Board demonstration, that device ID (or name) is saved in a list in your smartphone. Each time a user reprograms the development board, a new, random ID is generated for that device name. Then, if the development board is paired with a smartphone, the new ID will be saved on the smartphone, but with the same name. This can make it difficult to distinguish between the duplicate named devices. Therefore, it is recommended to first 'forget' the previous device ID (or name) on the smartphone before reprogramming the development board with another demonstration.

Question 11: How many MIPS are used by the Bluetooth stack?

Answer:

The MIPS usage is dependent on the particular demonstration, as follows:

- Bluetooth Stack + SBC = ~30 MIPS
- Bluetooth Stack + AAC = ~65 MIPS

Note: PIC32MX430/450/470 devices are being respecified from 80 MIPS to 100 MIPS operation. Please check back in October 2013 for the current status.

## PIC32 Bluetooth ®  Audio Development Kit Reference Guide

Question 12: Does the Bluetooth stack and decoders need external memory?

Answer 1:

No, all memory (Flash and RAM) is internal to the PIC32MX device.

Answer 2:

While the stack does not require external memory, our demonstrations make use of external memory for two items. The demonstration stores some display information, such as fonts within the external memory. The demonstrations also store the optional voice prompt raw audio data in the external memory.

Question 13: Is the source code for the Bluetooth stack available?

Answer:

The Bluetooth stack and audio decoders are supplied as a library object file, along with a C level API. Users can link this stack into their project and access it through this API.

Question 14: How much does the Bluetooth stack cost?

Answer:

There is a one-time license fee of $299.00 (US) for the Bluetooth stack.

Question 15: demonstrations compile normally?

Why does Demonstration 6 give loading errors, but other

Answer:

Some Rev. 2.0 boards are equipped with a PIC32MX450F256L device. This device is limited to just more than 256K of Flash memory. Demonstration 6 requires slightly more than this, and will generate a 'unable to write to Flash memory' error in MPLAB X IDE, although it will compile and start the load process normally. A PIM board with a PIC32MX470F512L device is available for the board through your Microchip sales channel.

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000035_15b84c670780224606407715c665809871bfc9b53c1842dc72187ff525f01f07.png)

## NOTES:

![Image](images/PIC32%20Bluetooth%20Audio%20Dev%20Kit%20Reference%20Guide/image_000036_46401d722e47efd2dffc66137a0fe3e4add81e1ebb2cebe6da045d17f1fe9f5a.png)

## Worldwide Sales and Service

## ASIA/PACIFIC

Asia Pacific Office Suites 3707-14, 37th Floor Tower 6, The Gateway Harbour City, Kowloon Hong Kong Tel: 852-2943-5100 Fax: 852-2401-3431

Australia - Sydney Tel: 61-2-9868-6733 Fax: 61-2-9868-6755

China - Beijing Tel: 86-10-8569-7000 Fax: 86-10-8528-2104

China - Chengdu Tel: 86-28-8665-5511 Fax: 86-28-8665-7889

China - Chongqing Tel: 86-23-8980-9588 Fax: 86-23-8980-9500

China - Hangzhou Tel: 86-571-8792-8115 Fax: 86-571-8792-8116

China - Hong Kong SAR Tel: 852-2943-5100 Fax: 852-2401-3431

China - Nanjing Tel: 86-25-8473-2460 Fax: 86-25-8473-2470

China - Qingdao Tel: 86-532-8502-7355 Fax: 86-532-8502-7205

China - Shanghai Tel: 86-21-5407-5533 Fax: 86-21-5407-5066

China - Shenyang Tel: 86-24-2334-2829 Fax: 86-24-2334-2393

China - Shenzhen Tel: 86-755-8864-2200 Fax: 86-755-8203-1760

China - Wuhan Tel: 86-27-5980-5300 Fax: 86-27-5980-5118

China - Xian Tel: 86-29-8833-7252 Fax: 86-29-8833-7256

China - Xiamen Tel: 86-592-2388138 Fax: 86-592-2388130

China - Zhuhai Tel: 86-756-3210040 Fax: 86-756-3210049

## ASIA/PACIFIC

India - Bangalore Tel: 91-80-3090-4444 Fax: 91-80-3090-4123

India - New Delhi Tel: 91-11-4160-8631 Fax: 91-11-4160-8632

India - Pune Tel: 91-20-3019-1500

Japan - Osaka Tel: 81-6-6152-7160 Fax: 81-6-6152-9310

Japan - Tokyo Tel: 81-3-6880- 3770 Fax: 81-3-6880-3771

Korea - Daegu Tel: 82-53-744-4301 Fax: 82-53-744-4302

Korea - Seoul Tel: 82-2-554-7200 Fax: 82-2-558-5932 or 82-2-558-5934

Malaysia - Kuala Lumpur Tel: 60-3-6201-9857 Fax: 60-3-6201-9859

Malaysia - Penang Tel: 60-4-227-8870 Fax: 60-4-227-4068

Philippines - Manila Tel: 63-2-634-9065 Fax: 63-2-634-9069

Singapore Tel: 65-6334-8870 Fax: 65-6334-8850

Taiwan - Hsin Chu Tel: 886-3-5778-366 Fax: 886-3-5770-955

Taiwan - Kaohsiung Tel: 886-7-213-7830

Taiwan - Taipei Tel: 886-2-2508-8600 Fax: 886-2-2508-0102

Thailand - Bangkok Tel: 66-2-694-1351 Fax: 66-2-694-1350

## AMERICAS

Corporate Office 2355 West Chandler Blvd. Chandler, AZ 85224-6199 Tel: 480-792-7200 Fax: 480-792-7277 Technical Support:

http://www.microchip.com/ support

Web Address:

www.microchip.com

Atlanta Duluth, GA Tel: 678-957-9614 Fax: 678-957-1455

Austin, TX Tel: 512-257-3370

Boston Westborough, MA Tel: 774-760-0087 Fax: 774-760-0088

Chicago Itasca, IL Tel: 630-285-0071 Fax: 630-285-0075

Cleveland Independence, OH Tel: 216-447-0464 Fax: 216-447-0643

Dallas Addison, TX Tel: 972-818-7423 Fax: 972-818-2924

Detroit Novi, MI Tel: 248-848-4000

Houston, TX Tel: 281-894-5983

Indianapolis Noblesville, IN Tel: 317-773-8323 Fax: 317-773-5453

Los Angeles Mission Viejo, CA Tel: 949-462-9523 Fax: 949-462-9608

New York, NY Tel: 631-435-6000

San Jose, CA Tel: 408-735-9110

Canada - Toronto Tel: 905-673-0699 Fax: 905-673-6509

## EUROPE

Austria - Wels Tel: 43-7242-2244-39 Fax: 43-7242-2244-393

Denmark - Copenhagen Tel: 45-4450-2828 Fax: 45-4485-2829

France - Paris Tel: 33-1-69-53-63-20 Fax: 33-1-69-30-90-79

Germany - Dusseldorf Tel: 49-2129-3766400

Germany - Munich Tel: 49-89-627-144-0 Fax: 49-89-627-144-44

Germany - Pforzheim Tel: 49-7231-424750

Italy - Milan Tel: 39-0331-742611 Fax: 39-0331-466781

Italy - Venice Tel: 39-049-7625286

Netherlands - Drunen Tel: 31-416-690399 Fax: 31-416-690340

Poland - Warsaw Tel: 48-22-3325737

Spain - Madrid Tel: 34-91-708-08-90 Fax: 34-91-708-08-91

Sweden - Stockholm Tel: 46-8-5090-4654

UK - Wokingham Tel: 44-118-921-5800 Fax: 44-118-921-5820

03/25/14