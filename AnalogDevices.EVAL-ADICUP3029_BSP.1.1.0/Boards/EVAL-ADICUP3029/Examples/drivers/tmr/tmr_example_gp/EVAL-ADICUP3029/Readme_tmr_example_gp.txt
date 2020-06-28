            Analog Devices, Inc. ADICUP3029 Application Example

Project Name: tmr_example_gp

Description:  Demonstrate how to use the General Purpose (GP) timers to generate a periodic interrupt and capture events.
 

Overview:
=========
    In this example, the GP0 and GP1 timers are used to generate a periodic timeout. GP1 is configured to run 4 times faster 
    than GP0. GP1 is also configured to capture the GP0 timeout. This means that the GP1 callback function will be called when 
    GP0 times out, and the current count of GP1 will be captured when this event occurs. This example acts as a tutorial on how
    to use the timer driver API and configuration data structures to perform basic timer operations. This example does not cover 
    PWM configuration, please see the RGB timer example for a demonstration of the PWM features.

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
    The string "All done!" should be printed to the terminal.
 
References:
===========
    ADuCM302x Hardware Reference Manual
