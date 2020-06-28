/*!
 *****************************************************************************
   @file:    findme_target.c

   @brief:   

   @details:
  -----------------------------------------------------------------------------

Copyright (c) 2017 Analog Devices, Inc.

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


#include "findme_target.h"


static void ApplicationCallback(void * pCBParam, uint32_t Event, void * pArg);
static void SetAdvertisingMode (void);

#define PERIPHERAL_ADV_MODE      ((ADI_BLE_GAP_MODE)(ADI_BLE_GAP_MODE_CONNECTABLE |  \
			                             ADI_BLE_GAP_MODE_DISCOVERABLE))
static bool               gbConnected;
static bool               gPrintAddress=true;
static ADI_BLE_GAP_MODE   gGapMode;
static char               alertLevelBuffer[40];

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
    ADI_PWR_RESULT       ePwr;
    ADI_BLER_CONN_INFO   connInfo;
    ADI_BLER_RESULT      eResult;
    uint8_t *       pDeviceName = (unsigned char *) "ADI_BLE_FINDME_TARGET";
  
    /* Explicitly disable the watchdog timer */ 
    *pREG_WDT0_CTL = 0x0u;

    /* Pinmux */
    adi_initpinmux();

    /* Initialize clocks */
    ePwr = adi_pwr_Init(); 
    DEBUG_RESULT("Error initializing the power service.", ePwr, ADI_PWR_SUCCESS);         
    ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u);     
    DEBUG_RESULT("Error initializing the power service.", ePwr, ADI_PWR_SUCCESS);     
    ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u);
    DEBUG_RESULT("Error initializing the power service.", ePwr, ADI_PWR_SUCCESS);

    common_Init();

    DEBUG_MESSAGE("Starting Findme Target Example.\r\n");

    /* Initialize radio and framework layer */
    eResult = adi_ble_Init(ApplicationCallback, NULL);
    DEBUG_RESULT("Error initializing the radio.\r\n", eResult, ADI_BLER_SUCCESS);
    
    /* Configure radio */
    eResult = adi_radio_RegisterDevice(ADI_BLE_ROLE_PERIPHERAL);
    DEBUG_RESULT("Error registering the radio.\r\n", eResult, ADI_BLER_SUCCESS);
    
    eResult = adi_radio_SetLocalBluetoothDevName((uint8_t* const)pDeviceName, strlen((const char*)pDeviceName), 0u, 0u);
    DEBUG_RESULT("Error setting local device name.\r\n", eResult, ADI_BLER_SUCCESS);

    /* Disable standard i/o buffering */
    setvbuf(stdout,NULL,_IONBF,0);

    /* Start advertising */
    SetAdvertisingMode();        

    /* Initialize data exchange profile */
    eResult = adi_radio_Register_FindmeTarget();
    DEBUG_RESULT("Error registering findme target.\r\n", eResult, ADI_BLER_SUCCESS);

    /* Now enter infinite loop waiting for connection and then data exchange events */
    DEBUG_MESSAGE("Waiting for connection. Initiate connection on central device please.\r\n");

    while(1u)
    {
        /* Dispatch events for two seconds - they will arrive in the application callback */
        eResult = adi_ble_DispatchEvents(2000u);
        DEBUG_RESULT("Error dispatching events to the callback.\r\n", eResult, ADI_BLER_SUCCESS);

        /* If connected, send data */
        if (gbConnected == true)
        {
          uint8_t buffer[80];

	    if (gPrintAddress) 
	    {
                adi_ble_GetConnectionInfo(&connInfo);
                sprintf((char*)buffer,"Connected Address 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\r\n",
				connInfo.eConnAddr.aBD_ADDR[0],
				connInfo.eConnAddr.aBD_ADDR[1],
				connInfo.eConnAddr.aBD_ADDR[2],
                                connInfo.eConnAddr.aBD_ADDR[3],
				connInfo.eConnAddr.aBD_ADDR[4],
				connInfo.eConnAddr.aBD_ADDR[5]);
                DEBUG_MESSAGE((char*)buffer);
                DEBUG_MESSAGE("\r\n");
	        gPrintAddress = false;
	    }

	}
        /* If disconnected, make sure we are in the right mode */
        else
        {
            if (gGapMode != PERIPHERAL_ADV_MODE)
            {
                SetAdvertisingMode();           
	        gPrintAddress = true;
            }
        }
    }
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
    switch(Event)
    {
        case GAP_EVENT_CONNECTED:
            DEBUG_MESSAGE("Connected!\r\n");
            gbConnected = true;
            break;

        case GAP_EVENT_DISCONNECTED:
            DEBUG_MESSAGE("Disconnected!\r\n");
            gGapMode      = ADI_BLE_GAP_MODE_NOTCONNECTABLE;
            gbConnected = false;
            break;

         case GAP_EVENT_MODE_CHANGE:
            DEBUG_MESSAGE("GAP mode changed.\r\n");
            break;

        case GAP_EVENT_CONNECTION_UPDATED:
            DEBUG_MESSAGE("Connection interval updated.\r\n");
            break;

        case IMMEDIATE_ALERT_EVENT:
  	     {
              ADI_BLE_ALERT_LEVEL *pAlertLevel = (ADI_BLE_ALERT_LEVEL *) pArg;
              if      (*pAlertLevel == 0) strcpy(alertLevelBuffer,"No Alert\r\n");
              else if (*pAlertLevel == 1) strcpy(alertLevelBuffer,"Medium Alert\r\n");
              else if (*pAlertLevel == 2) strcpy(alertLevelBuffer,"High Alert\r\n");

              DEBUG_MESSAGE("FindMe Target Alert Event. Level = ");
              DEBUG_MESSAGE(alertLevelBuffer);

  	      }
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
 * @details    Sets the bluetooth radio mode to discoverable and connectable mode.
 *             In this mode bluetooth device is visible to other central devices. 
 *             
 */
static void SetAdvertisingMode(void)
{
    ADI_BLER_RESULT eResult;

    eResult = adi_radio_SetMode(PERIPHERAL_ADV_MODE, 0u, 0u);
    DEBUG_RESULT("Error setting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

    eResult = adi_ble_WaitForEventWithTimeout(GAP_EVENT_MODE_CHANGE, 5000u);
    DEBUG_RESULT("Error waiting for GAP_EVENT_MODE_CHANGE.\r\n", eResult, ADI_BLER_SUCCESS);

    eResult = adi_radio_GetMode(&gGapMode);
    DEBUG_RESULT("Error getting the mode.\r\n", eResult, ADI_BLER_SUCCESS);

    if (gGapMode != PERIPHERAL_ADV_MODE)
    {
        DEBUG_MESSAGE("Error in SetAdvertisingMode.\r\n");
    } 
}
