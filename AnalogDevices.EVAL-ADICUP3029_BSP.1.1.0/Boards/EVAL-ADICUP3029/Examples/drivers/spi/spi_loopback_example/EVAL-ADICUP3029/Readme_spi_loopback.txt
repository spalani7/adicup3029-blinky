         Analog Devices, Inc. ADICUP3029 Application Example

Project Name: spi_loopback

Description:
=========== 
		This example demonstrates how to use the SPI driver in the following modes of operation:
             
		Blocking Mode
		Non-Blocking Mode 
			Polling API used to detect transaction completion.
			Non-polling API to detect transaction completion.
             
		And for each of the above modes of operation, the following features are demonstrated:
			DMA		both DMA and non-DMA transactions.
			RD_CTL	both RD_CTL and non-RD_CTL transactions .
               
		The example uses SPI0 internal loopback mode and no external connections are required.
		Loopback mode is controlled by configuration macro ENABLE_INTERNAL_SPI_LOOPBACK, which
		is enabled by default.  If this macro is undefined, successful operation may be obtained
		by adding a physical hardware jumper across the (SPI0) MISO and MOSI pins of header
		block P6 (pins 6 and 7).
            
		SPI pins (MISO, MOSI, and SCLK) are configured by PinMix.c file.  SPI-CS is driven manually.

Overview:
=========
		This example shows how to use SPI device for transmitting/receiving data.
		The transmit buffer is filled with known values and submitted to the SPI device 
		for transmitting.  Similarly, an empty buffer is also submitted to SPI  device 
		for storing the received data.  The content of the received buffer is verified
		for all modes of operation.

User Configuration Macros:
==========================
		None.

Hardware Setup:
===============  
		None.
             
External connections:
=====================
		None by default.
             
		If ENABLE_INTERNAL_SPI_LOOPBACK is undefined, then an external jumper is required across
		the (SPI0) MISO and MOSI pins at header block P6 (pins 6 and 7).

How to build and run:
=====================    
		Prepare hardware as explained in the Hardware Setup section.
		Build the project, load the executable to ADuCM3029, and run it.

Expected Result:
=================    
		Should see "All Done" after the execution is complete.

References:
===========
		ADuCM302x Hardware Reference Manual
