         Analog Devices, Inc. ADICUP3029 Application Example

Project Name: xint_example

Description:  Demonstrates the use of XINT driver to Toggle LED when the
              BOOT pushbutton is pressed on the EVAL-ADICUP3029 board.


Overview:
=========
    This example does two things:
    	1) It runs a simple green LED (DS3) toggle.
    	2) Optionally, catch a GPIO GroupA pin interrupt from the BOOT
    	   pushbutton (S3) over a loopback jumper and forward to one of
    	   the dedicated external pin interrupts that is then caught by
    	   the XINT device driver interrupt handler, which in turn toggles
    	   the red LED (DS4).
    
    Control flow for the optional XINT mode of operation is:
    
	Pusbutton event is caught by ->
		GPIO GroupA Interrupt handler, which ->
			Drives GPIO Output that goes out over ->
				Loopback, which stimulates the dedicated XINT pin, which is caught by ->
					XINT Interrupt handler, which ->
						Toggles the red LED.


User Configuration Macros:
==========================
    DEBOUNCE_MS
    	Configures the number of milliseconds to wait for pushbutton bounce to settle.


Hardware Setup:
===============
    EVAL-ADICUP3029 board should be configured with default settings jumper 
    settings. Please refer EVAL-ADICUP3029 board Manual for default
    hardware settings.


External connections:
=====================
    Optional XINT mode operation requires a jumper across P7 Arduino Connector
    pins: "IO13" and "IO15".  IO13 (GPIO13, aka, P0.13) serves as an output pin
    driven by the Boot pushbutton press event which is detected by the GroupA
    generic GPIO pin interrupt handler.  IO15 (GPIO15, aka, P0.15) serves as an
    input pin that is configured to capture the dedicated external interrupt pin,
    XINT0.
    

How to build and run:
=====================    
    Prepare hardware as explained in the Hardware Setup section.
    Build the project, load the executable to EVAL-ADICUP3029 board, and run it.


Expected Result:
=================    
	Watch green LED (DS3) blink continuously.
    Optionally (IFF loopback is installed)...
    	Press S3 (Boot button) and observe red LED (DS4) toggle.


References:
===========
    ADuCM302x Hardware Reference Manual
