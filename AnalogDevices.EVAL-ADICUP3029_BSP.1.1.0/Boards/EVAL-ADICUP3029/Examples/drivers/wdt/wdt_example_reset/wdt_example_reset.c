/*! *****************************************************************************
 * @file    wdt_example_reset.c
 * @brief   Example of how to use the WDT in Reset Mode.
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

#include "wdt_example_reset.h"

#ifndef FAILURE
#define FAILURE -1
#endif

volatile static uint32_t gNumTimeouts     = 0u;
volatile static uint32_t gNumTimeoutsKick = 0u;

void GP0CallbackFunction(void *pCBParam, uint32_t Event, void  * pArg)
{      
    /* IF(More kicks remain) */
    if (gNumTimeoutsKick < NUM_KICKS_BEFORE_RESET) {
      
        /* Kick the dog! */
        adi_wdt_Kick();
        
        gNumTimeoutsKick++;
    } /* ENDIF */
    
    gNumTimeouts++;
}

int main(void) 
{   ADI_TMR_CONFIG tmrConfig;
    ADI_TMR_RESULT eTimerResult;
    ADI_WDT_RESULT eResult;
    uint32_t nReadNumTimeouts;
  
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
    
   
    /* IF (WDT reset occured) */
    if ((pADI_PMG0->RST_STAT & BITM_PMG_RST_STAT_WDRST) == BITM_PMG_RST_STAT_WDRST) {
      
        /* Clear the reset status register */
    	uint32_t status = pADI_PMG0->RST_STAT;
        pADI_PMG0->RST_STAT = status;
                
        /* Example completed successfully */
        common_Pass();
        return 0;        
        
    /* ELSE(Run the test code to force a WDT reset) */
    } else {  
      
        /*------------------ GP TIMER 0 CONFIGURATION --------------------------
            To avoid resetting, a periodic event operating faster than the WDT
            is necessary. For this example we will use GP0 to "kick the dog", 
            however other peripherals like GP1, GP2, RGB, or SysTick timers
            could be used.
      
            In this example, GP0 will be configured to operate at .25 seconds,
            so the WDT should not timeout if we kick every GP0 timeout. However,
            when we decide to stop kicking the dog, it should take 4 GP0 interrupts
            before a reset occurs.
        ----------------------------------------------------------------------*/
        eTimerResult = adi_tmr_Init(ADI_TMR_DEVICE_GP0, GP0CallbackFunction, NULL, true);
        DEBUG_RESULT("Error initializing GP0.", eTimerResult, ADI_TMR_SUCCESS);
        
        /* Please see the GP timer example for timer configuration details */
        tmrConfig.bCountingUp    = false;
        tmrConfig.bPeriodic      = true;
        tmrConfig.ePrescaler     = ADI_TMR_PRESCALER_256;
        tmrConfig.eClockSource   = ADI_TMR_CLOCK_LFOSC;        
        tmrConfig.nLoad          = GP0_LOAD_VALUE_FOR_250MS_PERIOD;
        tmrConfig.nAsyncLoad     = GP0_LOAD_VALUE_FOR_250MS_PERIOD;        
        tmrConfig.bReloading     = false;
        tmrConfig.bSyncBypass    = false;        
        eTimerResult = adi_tmr_ConfigTimer(ADI_TMR_DEVICE_GP0, &tmrConfig);
        DEBUG_RESULT("Error configuring GP0.", eTimerResult, ADI_TMR_SUCCESS);
        
        eTimerResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP0, true);
        DEBUG_RESULT("Error starting GP0.", eTimerResult, ADI_TMR_SUCCESS); 
      
        /*----------------------- WDT CONFIGURATION ----------------------------
            The WDT operates at 32 kHz, setting the prescaler to 256 (2u) reduces
            the clock to 128 Hz. Using a load value of 128 gives a WDT timeout
            of 1 second. Using periodic mode allows the load value to cause a 
            timeout. So configuration equation is:
      
                    PERIOD(seconds) = (1 / (32768/PRESCALER))*LOAD
        
            This configuration is done in the local adi_wdt_config.h.
      
            Since we are in reset mode, the second parameter to this
            function call is irrevelent (no callback function).
        ----------------------------------------------------------------------*/
        eResult = adi_wdt_Enable(true, NULL);
        DEBUG_RESULT("Error enabling the WDT.", eResult, ADI_WDT_SUCCESS);
        
        /* WHILE(Forever, but a WDT should occur after 24 GP0 timeouts) */
        while(1u) {
            /* Avoid warning about volatile access order */
            nReadNumTimeouts = gNumTimeouts;
            /* IF(The WDT did not reset, error out of the example) */
            if (nReadNumTimeouts > (gNumTimeoutsKick + GP0_TIMEOUT_FREQ)) {
                common_Fail("WDT reset did not occur when expected.");
                return 1;
            } /* ENDIF */
        } /* ENDWHILE */
    } /* ENDIF */  
}
