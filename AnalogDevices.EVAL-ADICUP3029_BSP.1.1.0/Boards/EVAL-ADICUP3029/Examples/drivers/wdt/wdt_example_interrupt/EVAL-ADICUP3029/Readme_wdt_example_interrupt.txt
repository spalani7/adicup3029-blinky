            Analog Devices, Inc. ADICUP3029 Application Example

Project Name: wdt_example_interrupt

Description:  Demonstrate how the Watchdog Timer (WDT) can be used to trigger an interrupt on timeout.
 

Overview:
=========
    In this example, the WDT is configured to interrupt on a timeout. Since WDT configuration can only be done 
    statically, a local copy of the configuration file is created since the configuration for the example diverges 
    from the default configuration. The example shows how a callback function can be set to perform a specific user 
    action (instead of resetting) when the WDT timeout occurs. In the example, General Purpose (GP) Timer 1 is used 
    to ensure that the WDT is configured for the intended timeout period. GP1 is also configured to capture the WDT 
    timeout. The WDT is configured to timeout after 5 seconds. Once GP1 captures the WDT timeout, the number of WDT 
    timeouts and GP1 timeouts are checked to ensure they are the expected values.

User Configuration Macros:
==========================
    None. 

Hardware Setup:
===============
    None.
 
External connections:
=====================
    None.


How to build and run:
=====================
    Build the project and download to a single ADuCM302x target. Open the output terminal and run.

Expected Result:
=====================
    The string "All done!" should be printed to the terminal, after several seconds.
 
References:
===========
    ADuCM302x Hardware Reference Manual
