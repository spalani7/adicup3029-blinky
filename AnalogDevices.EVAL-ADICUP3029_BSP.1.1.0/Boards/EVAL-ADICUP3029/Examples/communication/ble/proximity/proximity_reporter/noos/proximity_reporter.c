/*!
 *****************************************************************************
   @file:    proximity_reporter.c

   @brief:   Proximity Reporter Example

   @details: Example program demonstrating basic use of the Data Exchange
             profile to send a string from the EM9304 to a host.
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


#include "proximity_reporter.h"


extern int32_t adi_initpinmux(void);


static void ApplicationCallback(void * pCBParam, uint32_t Event, void * pArg);
static void SetAdvertisingMode (void);

static bool               gbConnected;
static ADI_BLE_GAP_MODE   geMode;
#define PERIPHERAL_ADV_MODE      ((ADI_BLE_GAP_MODE)(ADI_BLE_GAP_MODE_CONNECTABLE | ADI_BLE_GAP_MODE_DISCOVERABLE))


/*!
 * @brief      Main
 *
 * @details    Application entry point.
 *
 * @param [in] argc : Number of arguments (unused)
 *
 * @param [in] argv : Arguments (unused)
 *
 */
int main(int argc, char *argv[])
{
    ADI_BLER_RESULT eResult;
    ADI_PWR_RESULT  ePwr;
    uint8_t *       aDataExchangeName = (unsigned char *) "ADI_BLE_PROXIMITY_REPORTER";
  
    /* Explicitly disable the watchdog timer */ 
    *pREG_WDT0_CTL = 0x0u;

    /* Pinmux */
    adi_initpinmux();

    /* Initialize clocks */
    ePwr = adi_pwr_Init(); 
    DEBUG_RESULT("Error initializing the power service.\r\n", ePwr, ADI_PWR_SUCCESS);

    ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u);     
    DEBUG_RESULT("Error configuring the core clock.\r\n", ePwr, ADI_PWR_SUCCESS);

    ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u);
    DEBUG_RESULT("Error configuring the peripheral clock.\r\n", ePwr, ADI_PWR_SUCCESS);

    common_Init();

    DEBUG_MESSAGE("Starting Proximity Monitor Example.\r\n");
    
    /* Initialize radio and framework layer */
    eResult = adi_ble_Init(ApplicationCallback, NULL);
    DEBUG_RESULT("Error initializing the radio.\r\n", eResult, ADI_BLER_SUCCESS);
    
    /* Configure radio */
    eResult = adi_radio_RegisterDevice(ADI_BLE_ROLE_PERIPHERAL);
    DEBUG_RESULT("Error registering the radio.\r\n", eResult, ADI_BLER_SUCCESS);
    
    eResult = adi_radio_SetLocalBluetoothDevName(aDataExchangeName, strlen((const char *) aDataExchangeName), 0u, 0u);
    DEBUG_RESULT("Error setting local device name.\r\n", eResult, ADI_BLER_SUCCESS);

    SetAdvertisingMode();

    /* Initialize Proximity Reporter Profile which contains the link loss service */
    eResult = adi_radio_Register_ProximityReporter();
    DEBUG_RESULT("Error registering proximity reporter.\r\n", eResult, ADI_BLER_SUCCESS);

    /* Initialize FindmeTarget. The reason we do this is to enable the immediate alert service.  */
    eResult = adi_radio_Register_FindmeTarget();
    DEBUG_RESULT("Error registering Findme target.\r\n", eResult, ADI_BLER_SUCCESS);

    /* Set Tx power level to 0 db. The Android host application will use this in the path loss calculation */
    eResult = adi_radio_SetTxPowerLevel(0u);
    DEBUG_RESULT("Error setting tx power level.\r\n", eResult, ADI_BLER_SUCCESS);

    /* Now enter infinite loop waiting for connection and then data exchange events */
    DEBUG_MESSAGE("Waiting for connection. Initiate connection on central device please.\r\n");

    /* WHILE(forever) */
    while(1u)
    {
        /* Dispatch events for two seconds - they will arrive in the application callback */
        eResult = adi_ble_DispatchEvents(2000u);
        DEBUG_RESULT("Error dispatching events to the callback.\r\n", eResult, ADI_BLER_SUCCESS);

        if (geMode != PERIPHERAL_ADV_MODE)
        {
            SetAdvertisingMode();
        }
    } /* ENDWHILE */
}


/*!
 * @brief      Application Callback
 *
 * @details    Called by the framework layer (adi_ble_noos.c) when an event occurs that the application did NOT
 *             explicity expect by calling #adi_ble_WaitForEventWithTimeout.
 *
 * @param [in] pCBParam : Callback parameter (unused)
 *
 * @param [in] Event : Event of type #ADI_BLER_EVENT.
 *
 * @param [in] pArg : Callback argument (unused)
 *
 * @note       The application should NOT call other radio functions (adi_ble_radio.c) from this callback that issue
 *             a command to the radio. The application may call radio functions that simply extract data from the
 *             companion module, these are located below #adi_ble_GetEvent in adi_ble_radio.c. Ideally this callback
 *             should just be used for flags and semaphores.
 */
static void ApplicationCallback(void * pCBParam, uint32_t Event, void * pArg)
{
    ADI_BLE_ALERT_LEVEL AlertLevel;

    switch(Event)
    {
        case GAP_EVENT_CONNECTED:
            DEBUG_MESSAGE("Connected!\r\n");
            gbConnected = true;
            break;

        case GAP_EVENT_DISCONNECTED:
            DEBUG_MESSAGE("Disconnected!\r\n");
            geMode      = ADI_BLE_GAP_MODE_NOTCONNECTABLE;
            gbConnected = false;
            break;

        case DATA_EXCHANGE_RX_EVENT:
            DEBUG_MESSAGE("Data received!\r\n");
            break;

        case DATA_EXCHANGE_TX_COMPLETE:
            DEBUG_MESSAGE("Data sent!\r\n");
            break;

        case GAP_EVENT_MODE_CHANGE:
            DEBUG_MESSAGE("GAP mode changed.\r\n");
            break;

        case IMMEDIATE_ALERT_EVENT:
            AlertLevel = *((ADI_BLE_ALERT_LEVEL *) pArg); 

            if (AlertLevel == 0)
            {
              DEBUG_MESSAGE("Immediate Alert Event. Level = No Alert\r\n");
            }
            else if (AlertLevel == 1)
            {
              DEBUG_MESSAGE("Immediate Alert Event. Level = Medium Alert\r\n");
            }
            else if (AlertLevel == 2)
            {
              DEBUG_MESSAGE("Immediate Alert Event. Level = High Alert\r\n");
            }
            break;

        case LINKLOSS_ALERT_EVENT:
            AlertLevel = *((ADI_BLE_ALERT_LEVEL *) pArg); 

            if (AlertLevel == 0)
            {
                DEBUG_MESSAGE("LinkLoss Alert Event. Level = No Alert\r\n");
            }
            else if (AlertLevel == 1)
            {
                DEBUG_MESSAGE("LinkLoss Alert Event. Level = Medium Alert\r\n");
            }
            else if (AlertLevel == 2)
            {
                DEBUG_MESSAGE("LinkLoss Alert Event. Level = High Alert\r\n");
            }
            break;

        case GAP_EVENT_CONNECTION_UPDATED:
            DEBUG_MESSAGE("Connection interval updated.\r\n");
            break;              

        case BLE_RADIO_ERROR_READING:
            /* If you want to enable this print statement, please be aware that the first
             * packet sent from the radio on startup will cause this error. It is a known bug
             * and will not have any adverse effects on the application.
             *
             *DEBUG_MESSAGE("Failed to read a packet from the radio.\r\n");
             *
             */
            break;

        case BLE_RADIO_ERROR_PARSING:
            DEBUG_MESSAGE("Failed to parse a packet from the radio.\r\n");
            break;      
            
        case BLE_RADIO_ERROR_PROCESSING:
            DEBUG_MESSAGE("Failed to process a packet from the radio.\r\n");
            break;                    

        default:
            DEBUG_MESSAGE("Unexpected event received.\r\n");
            break;
    }
}


/*!
 * @brief      Set Advertising Mode
 *
 * @details    Helper function to avoid repeated code in main.
 *
 */
static void SetAdvertisingMode(void)
{
    ADI_BLER_RESULT eResult;

    eResult = adi_radio_SetMode(PERIPHERAL_ADV_MODE, 0u, 0u);
    DEBUG_RESULT("Error setting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

    eResult = adi_ble_WaitForEventWithTimeout(GAP_EVENT_MODE_CHANGE, 5000u);
    DEBUG_RESULT("Error waiting for GAP_EVENT_MODE_CHANGE.\r\n", eResult, ADI_BLER_SUCCESS);    

    eResult = adi_radio_GetMode(&geMode);
    DEBUG_RESULT("Error setting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

    if (geMode != PERIPHERAL_ADV_MODE)
    {
        DEBUG_MESSAGE("Error in SetAdvertisingMode.\r\n");
    }
}
