# EM9304 OTP Programming Tool

## Overview

This program is designed to run on the ADuCM3029 in order to switch the EM9304
BLE Radio between the Host Controller Interface (HCI) and Application Controller Interface 
(ACI). To use the BLE software provided in the ADI-BleSoftware package, you need to place the radio
in ACI mode.

## Modes of Operation

### Debug

If the program is run in Debug mode (`ADI_DEBUG` is defined), the program will automatically 
switch the current mode of the radio. For instance, if the part is in HCI mode it will 
switch to ACI mode. If the part is in ACI mode it will switch to HCI mode.

### Release

If the program is run in Release mode (`ADI_DEBUG` is NOT defined), the program will only
attempt to switch the current mode into ACI. If the EM9304 is already in ACI mode, the program
will do nothing and exit sucessfully. This is designed for a "factory floor" setting and 
prevents multiple programs from occurring.

## ChangeMac_XX

If the program is run in one of the ChangeMac modes, the programmer will use ACI commands to 
patch the OTP to a new MAC address. This mode requires the EM9304 is in ACI mode, by using
the Debug and Release modes. The different configurations provide different MAC address:

Configuration | Hex File          | MAC Address (hex)
-----------------------------------------------------
Debug         | None              | 00-05-F7-01-41-44
Release       | programmer.hex    | 00-05-F7-01-41-44
ChangeMAC_45  | change_mac_45.hex | 00-05-F7-01-41-45
ChangeMAC_46  | change_mac_46.hex | 00-05-F7-01-41-46
ChangeMAC_47  | change_mac_47.hex | 00-05-F7-01-41-47
ChangeMAC_48  | change_mac_48.hex | 00-05-F7-01-41-48
ChangeMAC_49  | change_mac_49.hex | 00-05-F7-01-41-49

Note that this mode of operation has no "gating" to prevent multiple reruns.
Every time you run the program in ChangeMac mode, you are writing a patch to 
One-Time Programmable (OTP) memory.

## Output

There are three ways the user can verify that the program ran correctly.

### LEDs

The green LED will blink if the program ran successfully. The red LED will blink if
the program failed. The LEDs are off while the program is running. 

### Serial

More information can be found if the user opens a serial terminal (such as TeraTerm or PuTTY) while
the program is running. Select the proper COM port (should be labeled "mbed Serial Port"), and ensure 
the baud rate is set to 9600. Make sure the board being used has any switches/jumpers in the right setting
to use the UART output.

Four possible strings will be printed to the terminal in Debug or Release.

* "PROGRAMMING COMPLETE" - The program switched the EM9304 interface from HCI -> ACI or ACI -> HCI.
* "PROGRAMMING SKIPPED" - The program detected the interface was already ACI and did not switch it back to HCI.
* "PROGRAMMING PASSED" - The program completed successfully.
* "PROGRAMMING FAILED" - The program failed.

Two possible strings will be printed to the terminal in ChangeMac:

* "MAC CHANGE FAILED." - The MAC address change failed.
* "MAC CHANGE SUCCESS." - The MAC address change succeeded.

### Debugger

If the program is run in Debug mode, there will be verbose printing over the debugger via semihosting.

## Using the Program

### Pre-Built Binary

The `programmer.hex` file contains a Release mode version of the programmer application. Users
are recommended to use this as a means of programming rather than the source code provided.

Hex files have also been provided for the ChangeMac versions of the program.

### DAP Drive

The simplest method is to drag and drop the `.hex` file onto the DAPLINK drive that 
appears when the board is connected to a host PC over USB. The drive will briefly disappear
from the host PC and then reappear. The user can optionally open a serial terminal, and then push
the processor reset button on the board.

### Project Files and Sources

The project files and sources for the programmer tool are delivered in case the user wishes to modify
or change the program. The program is a single C file called `programmer.c`. It uses the ADuCM302x IoT
Device Family Pack (DFP) and the BLE sources located in ADI-BleSoftware package.

## Disclaimer

Using this tool will modify the one-time programmable memory space of the EM9304. Note that writing to this memory space
is irreverisble and the user should take caution when doing so. There is a macro in `programmer.c` called `INSPECT_OTP`
which will remove all write operations and simply dump the contents of OTP to the terminal via semihosting. It is 
recommended that the user set this macro to 1 when prototyping and working with the programmer source code.

