/*! *****************************************************************************
 * @file    tmr_example_gp.c
 * @brief   Example showing how to use the General Purpose timer device driver.
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


#include "tmr_example_gp.h"

#if defined(__ADUCM302x__)
#define GP_TMR_CAPTURE_EVENT 14u
#elif defined(__ADUCM4x50__)
#define GP_TMR_CAPTURE_EVENT 16u
#else
#error TMR is not ported for this processor
#endif

volatile static uint32_t gNumGp0Timeouts = 0u;
volatile static uint32_t gNumGp1Timeouts = 0u;
volatile static uint32_t gNumGp1Captures = 0u;

void GP0CallbackFunction(void *pCBParam, uint32_t Event, void  * pArg)
{   
    /* IF(Interrupt occurred because of a timeout) */
    if ((Event & ADI_TMR_EVENT_TIMEOUT) == ADI_TMR_EVENT_TIMEOUT) {
        gNumGp0Timeouts++;
    } /* ENDIF */
}

void GP1CallbackFunction(void *pCBParam, uint32_t Event, void  * pArg)
{   
    /* IF(Interrupt occurred because of a timeout) */
    if ((Event & ADI_TMR_EVENT_TIMEOUT) == ADI_TMR_EVENT_TIMEOUT) {
        gNumGp1Timeouts++;
    } /* ENDIF */
    
    /* IF(Interrupt occurred because of an event capture) */
    if ((Event & ADI_TMR_EVENT_CAPTURE) == ADI_TMR_EVENT_CAPTURE) {
        gNumGp1Captures++;
    } /* ENDIF */
}

int main(void)
{   
    ADI_TMR_EVENT_CONFIG  evtConfig;
    ADI_PWR_RESULT        ePwrResult;  
    ADI_TMR_CONFIG        tmrConfig;
    ADI_TMR_RESULT        eResult;
    uint32_t              nTimeout;

    common_Init();

    ePwrResult = adi_pwr_Init();
    DEBUG_RESULT("adi_pwr_Init failed.", ePwrResult, ADI_PWR_SUCCESS);

    ePwrResult = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u);
    DEBUG_RESULT("adi_pwr_SetClockDivider (HCLK) failed.", ePwrResult, ADI_PWR_SUCCESS);

    ePwrResult = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u);
    DEBUG_RESULT("adi_pwr_SetClockDivider (PCLK) failed.", ePwrResult, ADI_PWR_SUCCESS);

    /*--------------------- GP TIMER INITIALIZATION --------------------------*/
      
    /* Set up GP0 callback function */
    eResult = adi_tmr_Init(ADI_TMR_DEVICE_GP0, GP0CallbackFunction, NULL, true);
    DEBUG_RESULT("Error initializing GP0.", eResult, ADI_TMR_SUCCESS);

    /* Set up GP1 callback function */
    eResult = adi_tmr_Init(ADI_TMR_DEVICE_GP1, GP1CallbackFunction, NULL, true);
    DEBUG_RESULT("Error initializing GP1.", eResult, ADI_TMR_SUCCESS);
    
    /*------------------ GP TIMER CONFIGURATION --------------------------------

    Timer periods are set based on the following equation:
    
    PERIOD [sec] = (1 / (eClockSource [Hz] / ePrescaler)) * LOAD [16-bit]
    
    Where LOAD is given by nLoad and nAsyncLoad depending on the clock settings. 

    PCLK will vary depending on which root clock is selected and what prescaler is 
    applied to the PCLK. The PCLK can be set-up using the PWR driver. The HFOSC and
    LFOSC are fixed values and are enabled by default at start-up. These clock sources
    are used in the timer examples in order to avoid a dependency on the PWR driver.
    The LFXTAL is a constant low frequency external clock that can also be used. Unlike
    the internal clocks, this clock is disabled by default and must be enabled with the
    PWR driver.

        ================================================
        | Clock Source |  Default    |  Equation        |
        ================================================
        |    PCLK      |   6.5 MHz   | root_clk / PDIV  |
        |    HFOSC     |   26  MHz   |  Constant        |
        |    LFOSC     |   32  kHz   |  Constant        |
        |    LFXTAL    |   32  kHz   |  Constant        |
        ================================================  

    There are several boolean options that also effect timer period and behavior:
    
    - bCountingUp : Determines the direction of the count.
                        - true  increments
                        - false decrements

    - bPeriodic   : Determines the mode of the timer.
                        - true  PERIODIC
                        - false FREE-RUNNING

        ================================================
        | bCountingUp |  bPeriodic  |  Count Sequence  |
        ================================================
        |    false    |   false     | 0xFFFF -> 0x0000 |
        |    false    |   true      |  LOAD  -> 0x0000 |
        |    true     |   false     | 0x0000 -> 0xFFFF |
        |    true     |   true      |  LOAD  -> 0xFFFF |
        ================================================
    
    - bReloading  : Only relevent for periodic mode, allows user to reset timer before timeout.
                        - true  adi_tmr_Reload can be called to reload the timer
                        - false adi_tmr_Reload can not be called to reload the timer

    
    - bSyncBypass : Only relevent if PCLK is selected, allows a prescale of 4 to be used.
                        - true  ADI_TMR_PRESCALER_1 = PCLK / 1
                        - false ADI_TMR_PRESCALER_1 = PCLK / 4 
    
    --------------------------------------------------------------------------*/

    /* Configure GP0 to have a period of 10 ms */
    tmrConfig.bCountingUp  = false;
    tmrConfig.bPeriodic    = true;
    tmrConfig.ePrescaler   = ADI_TMR_PRESCALER_64;
    tmrConfig.eClockSource = ADI_TMR_CLOCK_HFOSC;
    tmrConfig.nLoad        = GP0_LOAD_VALUE_FOR_10MS_PERIOD;
    tmrConfig.nAsyncLoad   = GP0_LOAD_VALUE_FOR_10MS_PERIOD;
    tmrConfig.bReloading   = false;
    tmrConfig.bSyncBypass  = false;                        
    eResult = adi_tmr_ConfigTimer(ADI_TMR_DEVICE_GP0, &tmrConfig);
    DEBUG_RESULT("Error configuring GP0.", eResult, ADI_TMR_SUCCESS);

    /* Configure GP1 to have a period 4 times shorter than GP0 */
    tmrConfig.nLoad        = GP0_LOAD_VALUE_FOR_10MS_PERIOD / TMR_FACTOR;
    tmrConfig.nAsyncLoad   = GP0_LOAD_VALUE_FOR_10MS_PERIOD / TMR_FACTOR;
    eResult = adi_tmr_ConfigTimer(ADI_TMR_DEVICE_GP1, &tmrConfig);
    DEBUG_RESULT("Error configuring GP1.", eResult, ADI_TMR_SUCCESS);    
        
    /*------------------------- GP TIMER ENABLE ------------------------------*/
    
    eResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP0, true);
    DEBUG_RESULT("Error starting GP0.", eResult, ADI_TMR_SUCCESS);     

    eResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP1, true);
    DEBUG_RESULT("Error starting GP1.", eResult, ADI_TMR_SUCCESS); 
    
    /*-------------------------- EXECUTE TEST --------------------------------*/

    nTimeout = 0u;
    while(gNumGp0Timeouts == 0u) {
        /* IF(Maximum number of spin loops is reached, error out to avoid infinite loop) */
        if (nTimeout == TIMEOUT_SPIN_LOOPS) {
            /* Failure will be caught later */
            break;
        } /* ENDIF */
        nTimeout++;
    } /* ENDWHILE */    
        
    /*------------------------- GP TIMER DISABLE -----------------------------*/
    
    eResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP0, false);
    DEBUG_RESULT("Error starting GP0.", eResult, ADI_TMR_SUCCESS);     

    eResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP1, false);
    DEBUG_RESULT("Error starting GP1.", eResult, ADI_TMR_SUCCESS); 

    /*--------------------------- VERIFY TEST --------------------------------*/
      
    /* IF(Unexpected outcome) */
    if (gNumGp1Timeouts != TMR_FACTOR) {
        common_Fail("GP1 did not run with the expected period.");
        return 1;
    } /* ENDIF */

    /*------------------ GP TIMER EVENT CONFIGURATION --------------------------

    The timers can be configured to latch the current count value when a specific
    event occurs. A large table of events is enumerated in the HRM, the user should
    set nEventID to the event they wish to capture.

    There are several boolean options that also effect timer period and behavior:

    - bEnable        : Enable or disable event capture.
                        - true  enable
                        - false disable

    - bPrescaleReset : Determines if the timer count and prescale should be reset when the 
                       event is captured.
                        - true  timer is reset when the event is captured
                        - false timer keeps running normally when the event is captured

    The callback function will be called with ADI_TMR_EVENT_CAPTURE when the event
    occurs. The latched value can be read by calling adi_tmr_GetCaptureCount.                      
    
    --------------------------------------------------------------------------*/    
    
    /* Configure GP1 to capture the timeout event of GP0 */
    evtConfig.bEnable        = true;
    evtConfig.bPrescaleReset = false;
    evtConfig.nEventID       = GP_TMR_CAPTURE_EVENT;
    eResult = adi_tmr_ConfigEvent(ADI_TMR_DEVICE_GP1, &evtConfig);
    DEBUG_RESULT("Error configuring GP1 event capture.", eResult, ADI_TMR_SUCCESS); 

    /*------------------------- GP TIMER ENABLE ------------------------------*/
    
    eResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP0, true);
    DEBUG_RESULT("Error starting GP0.", eResult, ADI_TMR_SUCCESS);     

    eResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP1, true);
    DEBUG_RESULT("Error starting GP1.", eResult, ADI_TMR_SUCCESS);     

    /*-------------------------- EXECUTE TEST --------------------------------*/
    
    nTimeout = 0u;
    while(gNumGp1Captures == 0u) {
        /* IF(Maximum number of spin loops is reached, error out to avoid infinite loop) */
        if (nTimeout == TIMEOUT_SPIN_LOOPS) {
            /* Failure will be caught later */
            break;
        } /* ENDIF */
        nTimeout++;
    } /* ENDWHILE */

    /*------------------------- GP TIMER DISABLE -----------------------------*/
    
    eResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP0, false);
    DEBUG_RESULT("Error closing GP0.", eResult, ADI_TMR_SUCCESS);     

    eResult = adi_tmr_Enable(ADI_TMR_DEVICE_GP1, false);
    DEBUG_RESULT("Error closing GP1.", eResult, ADI_TMR_SUCCESS); 

    /*--------------------------- VERIFY TEST --------------------------------*/
      
    /* IF(Unexpected outcome) */
    if (gNumGp1Captures != 1u) {
        common_Fail("GP1 did not run capture GP0 timeout properly.");
        return 1;
    } /* ENDIF */

    /*----------------------- GP TIMER UNINITIALIZE --------------------------*/
      
    /* Disable GP0 interrupts and callback function */
    eResult = adi_tmr_Init(ADI_TMR_DEVICE_GP0, NULL, NULL, false);
    DEBUG_RESULT("Error uninitializing GP0.", eResult, ADI_TMR_SUCCESS);

    /* Disable GP1 interrupts and callback function */
    eResult = adi_tmr_Init(ADI_TMR_DEVICE_GP1, NULL, NULL, false);
    DEBUG_RESULT("Error uninitializing GP1.", eResult, ADI_TMR_SUCCESS);    
      
    common_Pass();
    return 0;
}
