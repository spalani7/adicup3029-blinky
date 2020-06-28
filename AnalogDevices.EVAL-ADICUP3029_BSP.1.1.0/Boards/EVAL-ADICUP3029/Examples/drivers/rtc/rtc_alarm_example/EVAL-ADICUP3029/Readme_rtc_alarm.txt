         Analog Devices, Inc. ADICUP3029 Application 


Project: rtc_alarm_example

Description:   This example demonstrates how to use and configure the RTC device
               for generating the alarm periodical.


Overview:
=========    
  This example demonstrates how to use and configure the RTC device for generating
  the alarm periodical.

  The example programs a number of RTC alarm events defined by macro ADI_RTC_NUM_ALARMS.
  Each alarm has an interval of ADI_RTC_ALARM_INTERVAL seconds.  As each alarm fires,
  the alarm is reprogrammed.  While awaiting alarms, the example puts the core into
  hibernation mode and the RTC alarms wake it up.  As each alarm is detected, a new
  alarm is programmed and the core is put back into hibernation mode.  This process
  continues until we receive ADI_RTC_NUM_ALARMS alarms.
  
  All the while, a second RTC alarm is configured as a watchdog alarm (not the actual
  WDT peripheral unit) with a value that exceeds the expected sequence of "normal" alarms.
  
  The RTC driver supports only call back mode of operation to notify the user. 

User Configuration Macros:
==========================
    ADI_RTC_NUM_ALARMS, which controls the number of alarms.
    ADI_RTC_ALARM_INTERVAL, which controle the duration of each alarm.

Hardware Setup:
===============
    EVAL-ADICUP3029 configured with default settings.

External connections:
=====================
    None. 

How to build and run:
=====================
    Build the project, load the executable to EVAL-ADICUP3029, open a "Terminal Window" and run it. 
    
    Note: The example should be let to run till completion, until "All done!" is seen. If the execution is stopped midway, then it will lock up the board. 

Expected Result:
=====================
    Upon successful completion the example will print the following:

		Resetting clock to latest build time...
		New time is:
			Raw time: 1493294575
			UTC time: Thu Apr 27 12:02:55 2017

		Programming 3 RTC alarms at 5 second intervals, for total expected test time of 15 seconds.

		ALARM example starting at:
			Raw time: 1493294575
			UTC time: Thu Apr 27 12:02:55 2017

		ALARM example complete at:
			Raw time: 1493294590
			UTC time: Thu Apr 27 12:03:10 2017

		All done!

              
References:
===========
    EVAL-ADICUP3029 Hardware Reference Manual
