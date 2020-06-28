/*! *****************************************************************************
 * @file    wdt_example_interrupt.h
 * @brief   Header file for wdt_example_interrupt.c
 -----------------------------------------------------------------------------
Copyright (c) 2016 Analog Devices, Inc.

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

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-
INFRINGEMENT, TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF
CLAIMS OF INTELLECTUAL PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/


#ifndef ADI_WDT_EXAMPLE_INTERRUPT
#define ADI_WDT_EXAMPLE_INTERRUPT

#include <stdint.h>
#include <stdbool.h>

#include <adi_processor.h>
#include <common.h>
#include <drivers/pwr/adi_pwr.h>
#include <drivers/tmr/adi_tmr.h>
#include <drivers/wdt/adi_wdt.h>

#if defined (__ADUCM4x50__)
/* Private, non-modifiable macros */ 
#define GP1_LOAD_VALUE_FOR_1SEC_PERIOD (120u)
#define GP1_EVENT_ID_FOR_WDT_TIMEOUT   (17u)
#define TIMEOUT_SPIN_LOOPS             (40000000u)

#elif defined (__ADUCM302x__)
#define GP0_LOAD_VALUE_FOR_1SEC_PERIOD (120u)
#define GP0_EVENT_ID_FOR_WDT_TIMEOUT   (5u)
#define TIMEOUT_SPIN_LOOPS             (10000000u)

#else
#error WDT driver is not ported for this processor
#endif

#endif /* ADI_WDT_EXAMPLE_INTERRUPT */
