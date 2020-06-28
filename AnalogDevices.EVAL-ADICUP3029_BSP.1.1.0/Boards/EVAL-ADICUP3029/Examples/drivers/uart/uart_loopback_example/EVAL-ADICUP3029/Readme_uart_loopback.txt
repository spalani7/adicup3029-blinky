            Analog Devices, Inc. ADICUP3029 Application Example

Project Name: uart_loopback_example

Description:  Demonstrates how to use the UART driver. 
              The example transfers the content of one buffer to another by using the
              loopback feature of the device. The example also demonstrates how to enable DMA
              operation as well as PIO operation. 
 

Overview:
=========
	This example shows how to use UART device for transmitting/receiving data. 
	It opens a UART device and configures the device to perform loopback. 
	A data buffer is filled with known values and submitted to the UART device for 
	transmitting. Similarly, an empty buffer is submitted to UART device 
	for storing the received data. At the end of the example, content of received 
	buffers are verified against the content of transmit buffers. This example runs
	in nonblocking mode only and showcases the use of the DMA in addition to PIO mode.

	The UART driver supports both nonblocking and blocking calls, which are 
	demonstrated in this example. A nonblocking call is turned into a blocking
	call with "adi_uart_GetTxBuffer" and "adi_uart_GetRxBuffer". Please take note that
	this example is only using nonblocking mode, even though some of the calls may 
	block.  Refer to the ADuCM302x Device Driver User's Guide for more information on the
	various modes of operation. 

	Please note that the external hardware connection for this loopback example 
	is not the same as testing the internal loopback hardware on the device.


User Configuration Macros:
==========================

	UART_DEVICE_NUM
		Selects physical UART device instance.  Configured in uart_loop_back.h.
		Default setting is "0", designating UART device instance 0.
		Change this as-needed, but this example relies on pin I/O for UART0.

	UART_INTERNAL_LOOPBACK
		Selects internal loopback UART mode.  Configured in uart_loop_back.h.
		Default setting is defined, designating use of internal loopback UART
		mode, avoiding need for physical external loopback jumper and switch settings.
		Undefine this macro to force use of physical external loopback mode (see
		further description in Hardware Setup section).
		


Hardware Setup:
===============
	Two physical scenarios are possible: internal or external loopback mode.
	
	INTERNAL LOOPBACK MODE
		Define the UART_INTERNAL_LOOPBACK macro (defined above) and build/run the application.
		No hardware setup required.
		Build, load, run example.
	
	EXTERNAL LOOPBACK MODE
		Move EVAL-ADUCUP3029 switch S2 ("UART Switch Matrix to USB/Arduino/WiFi" on schematic)
		to position 2, routing the processor UART0 TX/RX lines to the "Arduino" header strip.

		Attach a physical loopback jumper connecting Arduino header strip (P7) pins TX and RX.
		Build, load, run example.


External connections:
=====================
	External loopback jumper (described above) if using external loopback mode. 


How to build and run:
=====================
	Set the baud rate using the fractional divide macros listed in 'adi_uart_config.h' file. 
	Please use the utility "UartDivCalculator.exe" provided along with the BSP package 
	to generate the optimum values used to configure the device for different baud rates. 

	Prepare hardware as explained in the Hardware Setup section.

	Build the project, load the executable to ADuCM302x, and run it. 

	Look for the debug information displayed on terminal IO. 


Expected Result:
=====================
	Upon successful completion the program should output:

		All done!

		UART loop back example completed successfully.

		Successfully tested:

			* Nonblocking mode

			* Interrupt transfers

			* DMA transfers


References:
===========
    ADuCM302x Hardware Reference Manual
    EVAL-ADUCUP3029 evaluation board schematic
