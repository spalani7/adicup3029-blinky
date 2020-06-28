/*****************************************************************************
 * @file:    rtc_alarm_example.c
 * @brief:   Real-Time Clock Alarm example source file
 *-----------------------------------------------------------------------------

Copyright (c) 2012-2017 Analog Devices, Inc.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Modified versions of the software must be conspicuously marked as such.
  - This software is licensed solely and exclusively for use with processors
    manufactured by or for Analog Devices, Inc.
  - This software may not be combined or merged with other code in any manner
    that would cause the software to become subject to terms and conditions
    which differ from those listed here.
  - Neither the name of Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.
  - The use of this software may or may not infringe the patent rights of one
    or more patent holders.  This license does not release you from the
    requirement that you obtain separate licenses from these patent holders
    to use this software.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF CLAIMS OF INTELLECTUAL
PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *****************************************************************************/

#include <time.h>
#include <stddef.h>  /* for 'NULL' */
#include <stdio.h>   /* for scanf */
#include <string.h>  /* for strncmp */
#include <drivers/pwr/adi_pwr.h>
#include <drivers/rtc/adi_rtc.h>
#include "common.h"
#include "rtc_alarm_example.h"

/* device memory */
static uint8_t rtcAlarmMem[ADI_RTC_MEMORY_SIZE];
static uint8_t rtcWakeupMem[ADI_RTC_MEMORY_SIZE];

/* device handles */
ADI_RTC_HANDLE hDevRtcAlarm  = NULL;
ADI_RTC_HANDLE hDevRtcWakeup = NULL;

/* did an actual alarm status bit get set? */
volatile bool bRtcAlarmFlag;

/* interrupt flags */
volatile bool bRtcAlarmInterrupt;
volatile bool bRtcWakeupInterrupt;

/* hibernate exit flag */
volatile uint32_t iHibernateExitFlag;

/* Alarm count*/
volatile uint32_t AlarmCount;

/* prototypes */
static ADI_RTC_RESULT initializeRtcAlarm(void);
static ADI_RTC_RESULT rtc_Calibrate(void);
static ADI_RTC_RESULT rtc_ReprogramAlarm (void);
static ADI_RTC_RESULT rtc_AlarmTest(void);
static void           rtc_ReportTime(void);
static uint32_t       BuildSeconds(void);
static ADI_PWR_RESULT InitClock(void);

/* callbacks */
static void           rtcAlarmCallback  (void *pCBParam, uint32_t Event, void *EventArg);
static void           rtcWakeupCallback (void *pCBParam, uint32_t Event, void *EventArg);


ADI_RTC_RESULT rtc_Calibrate (void)
{
    ADI_RTC_RESULT eResult= ADI_RTC_FAILURE;
#ifdef ADI_RTC_CALIBRATE

    /*

    Compute the LF crystal trim values to compensate the RTC.  This can
    come from a static measure (a frequency counter), a real-time drift measure
    based external reference.

    Commercial crystals typically run between 20-100 ppm.  As an exercise, we
    demonstrate trimming a particular crystal and board configuration in which
    we measure an untrimmed error of about +58.6ppm (0.00586%).  This corresponds
    to a raw clock about 35.5 seconds/week fast (30 minutes/year).

    Available Trim Corrections:
    	X axis: trim interval (seconds)
    	Y axis: trim value (seconds)
    	Table content: trim correction (ppm)
      Value     16384    32768    65536   131072 (Interval)
        0        0.00     0.00     0.00     0.00
        1       61.04    30.52    15.26     7.63
        2      122.07    61.04    30.52    15.26
        3      183.11    91.55    45.78    22.89
        4      244.14   122.07    61.04    30.52
        5      305.18   152.59    76.29    38.15
        6      366.21   183.11    91.55    45.78
        7      427.25   213.62   106.81    53.41

    Referencing the trim table, we see the closest matching ppm correction for
    our example is 61.04.  In case there are different combinations yielding
    the same desired correction, we prefer the shortest trim interval (and
    smallest trim value) so as to minimize instantaneous drift.

    So we choose a trim interval of 2^14 seconds with a negative trim value of 1
    second, subtracting 1 second every 4.5 hours to "slow" the fast crystal down
    to a more reasonable rate.  This particular trim leaves a residual error of
    negative 2.44ppm (0.000244%), making the trimmed clock a tad slow (less than
    1.5 seconds/week or about 1.3 minutes/year), but much better than the
    untrimmed accuracy of 30 minutes/year.

    */

    ADI_PWR_RESULT ePowResult;
    if( ADI_PWR_SUCCESS != (ePowResult = adi_pwr_SetLFClockMux(ADI_CLOCK_MUX_LFCLK_LFXTAL)))   /*  select LFXTL clock */
    {
        DEBUG_RESULT("\n Failed to set the LF Clock Mux", ePowResult, ADI_PWR_SUCCESS);
        return(ADI_RTC_FAILURE);
    }


    if(ADI_RTC_SUCCESS != (eResult = adi_rtc_SetTrim(hDevRtcAlarm,ADI_RTC_TRIM_INTERVAL_14,ADI_RTC_TRIM_1,ADI_RTC_TRIM_SUB)))
    {
        DEBUG_RESULT("\n Failed to set the device %04d", eResult, ADI_RTC_SUCCESS);
        return(eResult);
    }
    if(ADI_RTC_SUCCESS != (eResult = adi_rtc_EnableTrim(hDevRtcAlarm, true)))
    {
        DEBUG_RESULT("\n Failed to enable the trim %04d", eResult, ADI_RTC_SUCCESS);
        return(eResult);
    }

#else
    printf("Use \"ADI_RTC_CALIBRATE\" preprocessor macro for RTC calibration.");
#endif
    return(eResult);
}


ADI_RTC_RESULT rtc_ReprogramAlarm (void) {
    ADI_RTC_RESULT eResult;
    uint32_t count;

    if(ADI_RTC_SUCCESS != (eResult = adi_rtc_GetCount(hDevRtcAlarm,&count)))
    {
        DEBUG_RESULT("\n Failed to get RTC Count %04d",eResult,ADI_RTC_SUCCESS);
        return(eResult);
    }
    if(ADI_RTC_SUCCESS != (eResult = adi_rtc_SetAlarm(hDevRtcAlarm, count + ADI_RTC_ALARM_INTERVAL)))
    {
        DEBUG_RESULT("\n Failed to set RTC Alarm %04d",eResult,ADI_RTC_SUCCESS);
        return(eResult);
    }
    return(eResult);
}


int main (void)
{
    ADI_RTC_RESULT eResult;
    /* test system initialization */
    common_Init();

    /* Initialization of clock */
    ADI_PWR_RESULT ePwrRes = InitClock();
    DEBUG_RESULT("\n Failed to initialize the clock %04d", ePwrRes, ADI_PWR_SUCCESS);


    /* initialize the RTC normal alarm */
    if(ADI_RTC_SUCCESS != initializeRtcAlarm())
    {
      common_Fail("Failed to initialize RTC device ");
      return 1;
    }

    /* calibrate */
    eResult = rtc_Calibrate();
    if( eResult != ADI_RTC_SUCCESS )
    {
      common_Fail("RTC calibaration failed");
    }
    else
    {

      /* test alarm */
      eResult =  rtc_AlarmTest();

      if(eResult == ADI_RTC_SUCCESS)
      {
        common_Pass();
      }
      else
      {
        common_Fail("Alarm Test Failed");
      }
    }
}


static ADI_RTC_RESULT initializeRtcAlarm (void) {

    uint32_t buildTime = BuildSeconds();
    ADI_RTC_RESULT eResult;

    /* Use both static configuration and dynamic configuration for illustrative purpsoes */
    do
    {
        eResult = adi_rtc_Open(RTC_ALARM_DEV_NUM, rtcAlarmMem, ADI_RTC_MEMORY_SIZE, &hDevRtcAlarm);
        DEBUG_RESULT("\n Failed to open the device %04d",eResult,ADI_RTC_SUCCESS);

        eResult = adi_rtc_RegisterCallback(hDevRtcAlarm, rtcAlarmCallback, hDevRtcAlarm);
        DEBUG_RESULT("\n Failed to register callback",eResult,ADI_RTC_SUCCESS);

        eResult = adi_rtc_SetCount(hDevRtcAlarm, buildTime);
        DEBUG_RESULT("Failed to set the count", eResult, ADI_RTC_SUCCESS);

        eResult = adi_rtc_SetTrim(hDevRtcAlarm, ADI_RTC_TRIM_INTERVAL, ADI_RTC_TRIM_VALUE, ADI_RTC_TRIM_DIRECTION);
        DEBUG_RESULT("Failed to set the trim value",eResult,ADI_RTC_SUCCESS);


    /* force a reset to the latest build timestamp */
        DEBUG_MESSAGE("Resetting clock to latest build time...");
        eResult = adi_rtc_SetCount(hDevRtcAlarm, buildTime);
        DEBUG_RESULT("Failed to set count",eResult,ADI_RTC_SUCCESS);

        DEBUG_MESSAGE("New time is:");
        rtc_ReportTime();

        eResult = adi_rtc_Enable(hDevRtcAlarm, true);
        DEBUG_RESULT("Failed to enable the device",eResult,ADI_RTC_SUCCESS);

    } while(0);

    return(eResult);
}


static ADI_RTC_RESULT rtc_AlarmTest (void) {

    char buffer[128];
    ADI_RTC_RESULT eResult;
    uint32_t count;
    AlarmCount = 0;

    /* initialize flags */
    bRtcAlarmFlag = bRtcAlarmInterrupt = bRtcWakeupInterrupt = false;

    /* enable RTC alarm */
    eResult = adi_rtc_EnableAlarm(hDevRtcAlarm, true);
    DEBUG_RESULT("adi_RTC_EnableAlarm failed",eResult,ADI_RTC_SUCCESS);

     /* enable alarm interrupting */
    eResult = adi_rtc_EnableInterrupts(hDevRtcAlarm, ADI_RTC_ALARM_INT, true);
    DEBUG_RESULT("adi_RTC_EnableInterrupts failed",eResult,ADI_RTC_SUCCESS);

	/* open/configure RTC Wakeup Timer */
    eResult = adi_rtc_Open(RTC_WAKEUP_DEV_NUM, rtcWakeupMem, ADI_RTC_MEMORY_SIZE, &hDevRtcWakeup);
    DEBUG_RESULT("Failed to open the device",eResult,ADI_RTC_SUCCESS);

    eResult = adi_rtc_RegisterCallback(hDevRtcWakeup, rtcWakeupCallback, hDevRtcWakeup);
    DEBUG_RESULT("Failed to register callback %04d",eResult,ADI_RTC_SUCCESS);

    eResult = adi_rtc_SetTrim(hDevRtcWakeup,ADI_RTC_TRIM_INTERVAL_6, ADI_RTC_TRIM_4, ADI_RTC_TRIM_ADD);
    DEBUG_RESULT("Failed to set Trim value  ",eResult,ADI_RTC_SUCCESS);

    eResult = adi_rtc_EnableTrim(hDevRtcWakeup,true);
    DEBUG_RESULT("\n Failed to enable Trim ",eResult,ADI_RTC_SUCCESS);

    eResult = adi_rtc_GetCount(hDevRtcAlarm, &count);
    DEBUG_RESULT("\n Failed to get count",eResult,ADI_RTC_SUCCESS);

	/* set the wakeup alarm well after normal alarm */
    eResult = adi_rtc_SetAlarm(hDevRtcWakeup,(2*ADI_RTC_ALARM_INTERVAL*ADI_RTC_NUM_ALARMS*10) + count);
    DEBUG_RESULT("\n Failed to set alarm",eResult,ADI_RTC_SUCCESS);

    eResult = adi_rtc_EnableAlarm(hDevRtcWakeup,true);
    DEBUG_RESULT("Failed to enable alarm",eResult,ADI_RTC_SUCCESS);

    eResult = adi_rtc_EnableInterrupts(hDevRtcWakeup, ADI_RTC_ALARM_INT, true);
    DEBUG_RESULT("Failed to enable interrupts",eResult,ADI_RTC_SUCCESS);

    /* program the normal alarm */
    rtc_ReprogramAlarm();

    iHibernateExitFlag = 0;

    eResult = adi_rtc_Enable(hDevRtcWakeup, true);
    DEBUG_RESULT("Failed to enable the device",eResult,ADI_RTC_SUCCESS);

    /* test banner */
    sprintf (buffer, "Programming %d RTC alarms at %d second intervals, for total expected test time of %d seconds.\n", ADI_RTC_NUM_ALARMS, ADI_RTC_ALARM_INTERVAL, ADI_RTC_NUM_ALARMS*ADI_RTC_ALARM_INTERVAL);
    DEBUG_MESSAGE(buffer);

    /* go to sleep and await RTC ALARM interrupt */
    DEBUG_MESSAGE("ALARM example starting at:");

    rtc_ReportTime();

    /* enter full hibernate mode with wakeup flag and no masking */
    if (adi_pwr_EnterLowPowerMode(ADI_PWR_MODE_HIBERNATE, &iHibernateExitFlag, 0))
    {
        DEBUG_MESSAGE("System Entering to Low Power Mode failed");
    }

    DEBUG_MESSAGE("ALARM example complete at:");

    rtc_ReportTime();

    /* disable both alarms */
    eResult = adi_rtc_EnableAlarm(hDevRtcAlarm, false);
    DEBUG_RESULT("\n Failed to disable the device",eResult,ADI_RTC_SUCCESS);
    eResult = adi_rtc_EnableAlarm(hDevRtcWakeup, false);
    DEBUG_RESULT("\n Failed to disable the device",eResult,ADI_RTC_SUCCESS);

    /* close both driver instances */
    eResult = adi_rtc_Close(hDevRtcAlarm);
    DEBUG_RESULT("\n Failed to close the device",eResult,ADI_RTC_SUCCESS);
    eResult = adi_rtc_Close(hDevRtcWakeup);
    DEBUG_RESULT("\n Failed to close the device",eResult,ADI_RTC_SUCCESS);

    /* verify expected results */
    if (bRtcWakeupInterrupt)
    {
        DEBUG_RESULT("rtc_AlarmTest got unexpected WUT interrupt", bRtcWakeupInterrupt, false);
        return(ADI_RTC_FAILURE);
    }
    if (!bRtcAlarmInterrupt)
    {
        DEBUG_MESSAGE("rtc_AlarmTest failed to get expected RTC interrupt");
        return(ADI_RTC_FAILURE);
    }
    if (!bRtcAlarmFlag)
    {
        DEBUG_MESSAGE("rtc_AlarmTest failed to get expected RTC ALARM interrupts");
        return(ADI_RTC_FAILURE);
    }

    return(eResult);
}


/*  standard ctime (time.h) constructs */
static void rtc_ReportTime(void) {

    char buffer[128];


    time_t rawtime;

    /* get the RTC count through the "time" CRTL function */
    time(&rawtime);

    /* print raw count */
    sprintf (buffer, "    Raw time: %d", (int)rawtime);
    DEBUG_MESSAGE(buffer);

    /* convert to UTC string and print that too */
    sprintf (buffer, "    UTC time: %s", ctime(&rawtime));
    DEBUG_MESSAGE(buffer);

}


static uint32_t BuildSeconds(void)
{
    /* count up seconds from the epoc (1/1/70) to the most recient build time */

    char timestamp[] = __DATE__ " " __TIME__;
    int month_days [] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint32_t days, month = 1u, date, year, hours, minutes, seconds;
    char Month[4];

    /* parse the build timestamp */
    sscanf(timestamp, "%s %d %d %d:%d:%d", Month, (int *)&date,(int *)&year, (int *)&hours, (int *)&minutes, (int *)&seconds);

    /* parse ASCII month to a value */
    if     ( !strncmp(Month, "Jan", 3 )) month = 1;
    else if( !strncmp(Month, "Feb", 3 )) month = 2;
    else if( !strncmp(Month, "Mar", 3 )) month = 3;
    else if( !strncmp(Month, "Apr", 3 )) month = 4;
    else if( !strncmp(Month, "May", 3 )) month = 5;
    else if( !strncmp(Month, "Jun", 3 )) month = 6;
    else if( !strncmp(Month, "Jul", 3 )) month = 7;
    else if( !strncmp(Month, "Aug", 3 )) month = 8;
    else if( !strncmp(Month, "Sep", 3 )) month = 9;
    else if( !strncmp(Month, "Oct", 3 )) month = 10;
    else if( !strncmp(Month, "Nov", 3 )) month = 11;
    else if( !strncmp(Month, "Dec", 3 )) month = 12;

    /* count days from prior years */
    days=0;
    for (int y=1970; y<year; y++) {
        days += 365;
        if (LEAP_YEAR(y))
            days += 1;
    }

    /* add days for current year */
    for (int m=1; m<month; m++)
        days += month_days[m-1];

    /* adjust if current year is a leap year */
    if ( (LEAP_YEAR(year) && ( (month > 2) || ((month == 2) && (date == 29)) ) ) )
        days += 1;

    /* add days this month (not including current day) */
    days += date-1;

    return (days*24*60*60 + hours*60*60 + minutes*60 + seconds);
}


/* Alarm Callback handler */
static void rtcAlarmCallback (void *pCBParam, uint32_t Event, void *EventArg) {

    bRtcAlarmInterrupt = true;

    /* process RTC interrupts (cleared by driver) */
    if( 0 != (ADI_RTC_WRITE_PEND_INT &  Event ))
    {
        DEBUG_MESSAGE("Got RTC interrupt callback with ADI_RTC_INT_SOURCE_WRITE_PEND status");
    }

    if( 0 != (ADI_RTC_WRITE_SYNC_INT & Event))
    {
        DEBUG_MESSAGE("Got RTC interrupt callback with ADI_RTC_INT_SOURCE_WRITE_SYNC status");
    }

    if( 0 != (ADI_RTC_WRITE_PENDERR_INT &  Event))
    {
        DEBUG_MESSAGE("Got RTC interrupt callback with ADI_RTC_WRITE_PENDERR_INT status");
    }

    if( 0 != (ADI_RTC_ISO_DONE_INT & Event))
    {
        DEBUG_MESSAGE("Got RTC interrupt callback with ADI_RTC_INT_SOURCE_ISO_DONE status");
    }

    if( 0 != (ADI_RTC_MOD60ALM_INT & Event))
    {
        DEBUG_MESSAGE("Got RTC interrupt callbackwithon ADI_RTC_INT_SOURCE_MOD60_ALARM status");
    }

    if( 0 != (ADI_RTC_ALARM_INT & Event))
    {
        /* Update alarm count */
        AlarmCount++;

        /* IF (Enough alarms registered) */
        if (AlarmCount >= ADI_RTC_NUM_ALARMS)
        {
            bRtcAlarmFlag = true;       /* note alarm flag */
            iHibernateExitFlag = 1   ;  /* exit hibernation on return from interrupt */
        }
        /* ELSE (more alarms needed) */
        else
        {
            /* Update RTC alarm */
            rtc_ReprogramAlarm();
        }
    }
}


/* Wakeup Callback handler (should never get here...) */
static void rtcWakeupCallback(void* hWut, uint32_t Event, void* pArg)
{
    iHibernateExitFlag = 1;
    bRtcWakeupInterrupt = true;
}


static ADI_PWR_RESULT InitClock()
{
  ADI_PWR_RESULT eResult;
  if(ADI_PWR_SUCCESS !=(eResult =adi_pwr_Init()))
  {
    return(eResult);
  }

  if(ADI_PWR_SUCCESS !=(eResult =adi_pwr_SetLFClockMux(ADI_CLOCK_MUX_LFCLK_LFXTAL)))
  {
    return(eResult);
  }

  if(ADI_PWR_SUCCESS !=(eResult =adi_pwr_EnableClockSource(ADI_CLOCK_SOURCE_LFXTAL,true)))
  {
    return(eResult);
  }

  if(ADI_PWR_SUCCESS !=(eResult =adi_pwr_SetClockDivider(ADI_CLOCK_HCLK,1)))
  {
    return(eResult);
  }
  if(ADI_PWR_SUCCESS !=(eResult =adi_pwr_SetClockDivider(ADI_CLOCK_PCLK,1)))
  {
    return(eResult);
  }

  return(eResult);
}
