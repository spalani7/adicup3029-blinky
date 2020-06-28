            Analog Devices, Inc. ADICUP3029 Application Example

Project Name: wdt_example_reset

Description:  Demonstrate how the Watchdog Timer (WDT) can be used both avoid and trigger a system reset.
 

Overview:
=========
    In this example, the WDT is configured to reset on a timeout. Since WDT configuration can only be done statically,
    a local copy of the configuration file is created since the configuration for the example diverges from the default
    configuration. The example demonstrates how the WDT can be "kicked" (reset) to avoid a timeout using General Purpose 
    (GP) Timer 0. Eventually, the example stops kicking the WDT and waits for a reset to happen. If the reset does not occur 
    in the expected time frame, the example throws an error. 

User Configuration Macros:
==========================
    EXAMPLE_TIME (wdt_example_reset.h) - This macro can be used to control how long the example should run before resetting.

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
