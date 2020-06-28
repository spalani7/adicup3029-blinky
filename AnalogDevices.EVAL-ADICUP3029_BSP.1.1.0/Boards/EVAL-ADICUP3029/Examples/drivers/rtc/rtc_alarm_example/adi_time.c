/*!
 *****************************************************************************
 * @file:    adi_time.c
 * @brief:   rtc device data file
 * @version: $Revision: 34933 $
 * @date:    $Date: 2016-06-28 07:11:25 -0400 (Tue, 28 Jun 2016) $
 *-----------------------------------------------------------------------------
 *
Copyright (c) 2010-2016 Analog Devices, Inc.

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
 *
 *****************************************************************************/

#include <time.h>
#include <stddef.h>  /* for 'NULL' */
#include <drivers/rtc/adi_rtc.h>
#include "rtc_alarm_example.h"

#ifdef CLOCKS_PER_SEC
#undef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1   /* number of RTC clock ticks in a second */
#endif

/*
 * Implementation of standard time-rleated C APIs which in turn leverage the RTC */

/*
    clock_t: a integer 32-bit type supporting arithmetic operations.
    clock():
        query the RTC interface for current time.
        return seconds since last time setting.
        return -1 if "reliable" time is not available or RTC is uninitialized.
*/


clock_t clock(void)
{
    ADI_RTC_HANDLE pDev = hDevRtcAlarm;  /* OK for singular RTC... */
    uint32_t t;
    if (adi_rtc_GetCount(pDev, &t) != ADI_RTC_SUCCESS) {
        t = (uint32_t)-1;
    }
    return (clock_t)t;
}



/*
      time_t: an integral 32-bit type supporting arithmetic operations.
    __time32(__time_t *t):
        return seconds since last time set.
        return -1 if "reliable" time is not available.
        sets time to address of t, if provided.
*/

time_t __time32(time_t *t)
{
    clock_t now = clock();
    if (t) {
        *t = (time_t) now;
    }
    return (time_t) now;
}


/*
      time_t: an integral 32-bit type supporting arithmetic operations.
      time(time_t *t):
        return seconds since last time set.
        return -1 if "reliable" time is not available.
        sets time to address of t, if provided.
*/
#if !defined (__ICCARM__)
time_t time(time_t *t)
{
    clock_t now = clock();
    if (t) {
        *t = (time_t) now;
    }
    return (time_t) now;
}
#endif
