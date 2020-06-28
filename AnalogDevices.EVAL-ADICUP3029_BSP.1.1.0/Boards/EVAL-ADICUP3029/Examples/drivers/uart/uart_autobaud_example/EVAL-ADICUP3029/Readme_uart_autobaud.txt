              Analog Devices, Inc. ADICUP3029 Application Example
            
Project Name: uart_autobaud_example

Description:  Demonstrates how to use the UART driver for autobaud detection 

Overview:
=========
    This example shows how to use UART device driver for baudrate detection. 
    The example opens a UART device, configures the device for baudrate detection. 
    After receiving the key character (carrige return), it configures the UART device to the detected baudrate. 
    This example has been tested for baudrates 300 to 921600. In order to detect baudrates lower than 300, 
    decrease the clock speed. 


User Configuration Macros:
==========================
    - UART_DEVICE_NUM: Choose which UART to use for the example. If the the UART to USB connector is used (P6) 
      to communicate with the host PC, then this macro must be set to 0.
    - UART_AUTOBAUD_TIMEOUT: Number of cycles to wait for a character to be sent prior to reporting an error. 


Hardware Setup:
===============
	Move EVAL-ADUCUP3029 switch S2 ("UART Switch Matrix to USB/Arduino/WiFi" on schematic)
	to position 1, routing the processor UART0 TX/RX lines to the MBED/USB shared debugger
	and serial interface connector (P10).

 
External connections:
=====================
	Attach a USB cable from the hosting/debugging PC to the P10 USB connector on the  EVAL-ADUCUP3029 board.


How to build and run:
=====================

	With the MBED USB cable connected between the EVAL-ADUCUP3029 and the PC, download and run the
	Windows "mbed Serial Port" driver from:	https://developer.mbed.org/handbook/Windows-serial-configuration
	
	With the USB cable still connected, browse the Windows Device Manager to the "Ports (COM & PLT)" section
	and note the virtual serial port COM# assigned to named port "mbed Serial Port".
    
    Build the project, load the executable to EVAL-ADUCUP3029, and run it.
    
    Run a host-based serial emulator utility, such as teraterm or PuTTY.
    
    Open a session on the virtual serial port reported by Windows Device Manager.

    Hit the "Enter" key (carrige return) to detect the baud rate. 

    Type any character and it will be echoed on host terminal. Enter "Q" to end the example. 


Expected Result:
=====================
    The following string should be printed to the terminal if you press 'ENTER' in Terminal: 

    "If you can read this then the baudrate was successfully detected!
    The baudrate is: xxxxxxxx.
    Note: This is using integer precision so it could vary slightly from the actual baudrate.
    Please enter any character to echo back on terminal

    Enter 'Q' to end the example:"
    
    Note: Any errors will be printed to terminal IO.
 
 
References:
===========
    ADuCM302x Hardware Reference Manual
    EVAL-ADUCUP3029 evaluation board schematic
