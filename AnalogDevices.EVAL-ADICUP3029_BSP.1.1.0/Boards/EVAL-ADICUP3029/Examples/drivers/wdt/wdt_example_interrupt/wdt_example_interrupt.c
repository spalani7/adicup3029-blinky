/*! *****************************************************************************
 * @file    wdt_example_interrupt.c
 * @brief   Example of how to use the WDT in Interrupt Mode.
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

#include "wdt_example_interrupt.h"

#ifndef FAILURE
#define FAILURE -1
#endif

/*WDT uses GP0 for ADUCM302x while GP1 for ADUCM4050*/
#if defined (__ADUCM302x__)
#define GP1_LOAD_VALUE_FOR_1SEC_PERIOD		GP0_LOAD_VALUE_FOR_1SEC_PERIOD
#define ADI_TMR_DEVICE_GP1					ADI_TMR_DEVICE_GP0
#define GP1_EVENT_ID_FOR_WDT_TIMEOUT		GP0_EVENT_ID_FOR_WDT_TIMEOUT
#endif/*__ADUCM302x_-*/

volatile static uint32_t gNumGpTimeouts  = 0u;
volatile static uint32_t gNumWdtTimeouts = 0u;
volatile static uint32_t gNumGpCaptures  = 0u;
volatile uint32_t    nTimeout;


void GP1CallbackFunction(void *pCBParam, uint32_t Event, void  * pArg)
{   
    /* IF(Interrupt occurred because of a GP1 timeout) */
    if ((Event & ADI_TMR_EVENT_TIMEOUT) == ADI_TMR_EVENT_TIMEOUT) {
        gNumGpTimeouts++;
    } /* ENDIF */
    
    /* IF(Interrupt occurred because of a WDT timeout) */
    if ((Event & ADI_TMR_EVENT_CAPTURE) == ADI_TMR_EVENT_CAPTURE) {
        gNumGpCaptures++;     
    } /* ENDIF */
}

void WDTCallbackFunction(void *pCBParam, uint32_t Event, void  * pArg)
{    
    gNumWdtTimeouts++;
}

int main(void) 
{   
    ADI_TMR_EVENT_CONFIG evtConfig;
    ADI_TMR_CONFIG       tmrConfig;
    ADI_TMR_RESULT       eTimerResult;
    ADI_WDT_RESULT       eResult;
  
    common_Init();

     /* Initialize the power service */
    if(ADI_PWR_SUCCESS != adi_pwr_Init())
    {
      return FAILURE;
    }
  
    if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_HCLK,1))
    {
      return FAILURE;
    }
  
    if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_PCLK,1))
    {
      return FAILURE;
    }
    /*------------------ GP TIMER 1 CONFIGURATION ------------------------------
    GP1 configured to interrupt every 1 second. This timer will be used to 
    profile the WDT and make sure it is operating at the expected period. The 
    WDT timeout will be captured by GP1 using the event capture feature.
    --------------------------------------------------------------------------*/
    eTimerResult = adi_tmr_Init(ADI_TMR_DEVICE_GP1, GP1CallbackFunction, NULL, true);
    DEBUG_RESULT("Error initializing GP1.", eTimerResult, ADI_TMR_SUCCESS);
    
    /* Please see the GP timer example for timer configuration details */
    tmrConfig.bCountingUp    = false;
    tmrConfig.bPeriodic      = true;
    tmrConfig.ePrescaler     = ADI_TMR_PRESCALER_256;
    tmrConfig.eClockSource   = ADI_TMR_CLOCK_LFOSC;
    tmrConfig.nLoad          = GP1_LOAD_VALUE_FOR_1SEC_PERIOD;
    tmrConfig.nAsyncLoad     = GP1_LOAD_VALUE_FOR_1SEC_PERIOD;
    tmrConfig.bReloading     = false;
    tmrConfig.bSyncBypass    = false;
    eTimerResult = adi_tmr_ConfigTimer(ADI_TMR_DEVICE_GP1, &tmrConfig);
    DEBUG_RESULT("Error configuring GP1.", eTimerResult, ADI_TMR_SUCCESS);
    
    /* Please see the GP timer example for event configuration details */
    evtConfig.bEnable        = true;
    evtConfig.bPrescaleReset = false;
    evtConfig.nEventID       = GP1_EVENT_ID_FOR_WDT_TIMEOUT; 
    eTimerResult = adi_tmr_ConfigEvent(ADI_TMR_DEVICE_GP1, &evtConfig);
    DEBUG_RESULT("Error configuring GP1 event capture.", eTimerResult, ADI_TMR_SUCCESS);    

    /* Start the timer */
    eTimerResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP1, true);
    DEBUG_RESULT("Error starting GP1.", eTimerResult, ADI_TMR_SUCCESS); 

    /*----------------------- WDT CONFIGURATION --------------------------------
    The WDT operates at 32 kHz, setting the prescaler to 256 (2u) reduces
    the clock to 128 Hz. Using a load value of 625 gives a WDT timeout
    of 5 seconds. Using periodic mode allows the load value to cause a 
    timeout. So configuration equation is:

    PERIOD(seconds) = (1 / (32768/PRESCALER))*LOAD

    This configuration is done in the local adi_wdt_config.h.

    Since we are in interrupt mode, a callback function should be provided. 
    --------------------------------------------------------------------------*/
    eResult = adi_wdt_Enable(true, WDTCallbackFunction);
    DEBUG_RESULT("Error enabling the WDT.", eResult, ADI_WDT_SUCCESS);
    
    /* WHILE(The WDT has not timed out) */
    nTimeout = 0u;
    while(gNumGpCaptures == 0u) {
        /* IF(Timeout has occurred) */
        if (nTimeout == TIMEOUT_SPIN_LOOPS) {
            /* Failure will be caught in the next if/else since gNumGpCaptures is still zero */
            break;
        } /* ENDIF */
        nTimeout++;
    } /* ENDWHILE */
    
    /* Stop the timer */
    eTimerResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP1, false);
    DEBUG_RESULT("Error stopping GP1.", eTimerResult, ADI_TMR_SUCCESS);
        
    /* IF(The example did not run as expected) */
    if ((gNumGpCaptures  != 1u)  ||
        (gNumGpTimeouts  != 5u)  ||
        (gNumWdtTimeouts != 1u))  {
            common_Fail("WDT interrupt example did not run as expected.");
            return 1u;
     /* ELSE(Example ran properly) */
     } else {
            common_Pass();
            return 0u;
     } /* ENDIF */
}
