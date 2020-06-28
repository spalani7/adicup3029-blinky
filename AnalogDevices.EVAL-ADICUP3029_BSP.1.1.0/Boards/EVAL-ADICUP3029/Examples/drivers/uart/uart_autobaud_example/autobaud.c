/*
 *****************************************************************************
 * @file:    autobaud.c
 * @brief:   File which contain "main" for testing UART Device Driver.
 *****************************************************************************

Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*!
* @file      autobaud.c
*
* @brief     This is file contains an autobaud example which demonstrate how to configure
*            a UART device to detect the baudrate.
*
*/

#include <stdio.h>
#include <drivers/uart/adi_uart.h>
#include <drivers/pwr/adi_pwr.h>
#include "autobaud.h"
#include "common.h"
#include <drivers/general/adi_drivers_general.h>


extern int32_t adi_initpinmux(void);

/* Handle for UART device. */
ADI_ALIGNED_PRAGMA(4)
static ADI_UART_HANDLE hDevice ADI_ALIGNED_ATTRIBUTE(4);

/* Memory for UART driver. */
ADI_ALIGNED_PRAGMA(4)
static uint8_t UartDeviceMem[ADI_UART_MEMORY_SIZE] ADI_ALIGNED_ATTRIBUTE(4);

ADI_ALIGNED_PRAGMA(4)
static char InfoBuffer[150] ADI_ALIGNED_ATTRIBUTE(4)={0} ;

/* Boolean to check if we entered the callback. */
static bool volatile bCallback = false;
/*********************************************************************

    Function:       UARTCallback

    Description:    In the example, we've configured the autobaud to
                    notify the API when it is done.

*********************************************************************/
static void UARTCallback(
    void        *pAppHandle,
    uint32_t     nEvent,
    void        *pArg
)
{
    switch (nEvent)
    {
        case ADI_UART_EVENT_AUTOBAUD_COMPLETE:
            bCallback = true;
            break;
    }
}


int main( void )
{
    /* Variable to store the baudrate. */
    static uint32_t nBaudrate;

    /* Variable to store any autobaud errors. */
    uint32_t nAutobaudError;

    /* Variable to store any hardware errors. */
    uint32_t nHardwareError;

    /* Boolean to tell the driver if we will be using the callback or not. */
    bool bAutobaudCallback = true;

    /* Boolean to notify user of a failure during run-time. */
    bool bFailure = false;

    uint8_t nChar=0x00;

    uint32_t nAutobaudCounter = 0u;

    bool bTxComplete = false;

    /* Pinmux initialization. */
    adi_initpinmux();

    do
    {
        /* Power initialization. */
        if(ADI_PWR_SUCCESS != adi_pwr_Init())
        {
            DEBUG_MESSAGE("Failed to intialize the power service.");
            bFailure = true;
            break;
        }

        /* System clock initialization. */
        if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u))
        {
            DEBUG_MESSAGE("Failed to intialize the system clock.");
            bFailure = true;
            break;
        }

        /* Peripheral clock initialization. */
        if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u))
        {
            DEBUG_MESSAGE("Failed to intialize the peripheral clock.");
            bFailure = true;
            break;
        }

        /* Open the UART device. */
        if(ADI_UART_SUCCESS != adi_uart_Open(UART_DEVICE_NUM, ADI_UART_DIR_BIDIRECTION, UartDeviceMem, ADI_UART_MEMORY_SIZE, &hDevice))
        {
            DEBUG_MESSAGE("Failed to open the UART device.");
            bFailure = true;
            break;
        }

        /* Register a callback to test callback mode. */
        if(adi_uart_RegisterCallback(hDevice, UARTCallback, hDevice) != ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Call back registration failed");
            bFailure = true;
            break;
        }

        /* Enable autobaud using the callback. */
        if(adi_uart_EnableAutobaud(hDevice, true, bAutobaudCallback) != ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Failed to enable autobaud");
            bFailure = true;
            break;
        }

      #ifndef __CC_ARM
       DEBUG_MESSAGE("Autobaud started, press Enter/return key in the terminal\n");
      #endif

        /* Wait until the baudrate has been detected. */
        do
        {
            adi_uart_GetBaudRate(hDevice, &nBaudrate, &nAutobaudError);
            nAutobaudCounter++;

        }while((nBaudrate == 0) && (nAutobaudCounter < UART_AUTOBAUD_TIMEOUT));

        if(nAutobaudCounter == UART_AUTOBAUD_TIMEOUT)
        {
            DEBUG_MESSAGE("Timeout due to no incoming key character");
            bFailure = true;
            break;
        }

        /* Disable autobaud. */
        if(adi_uart_EnableAutobaud(hDevice, false, false) != ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Failed to disable autobaud");
            bFailure = true;
            break;
        }

        /* Let the user know that the baudrate has been detected. */
        sprintf(InfoBuffer,"\n\t\r If you can read this then the baudrate was successfully detected!");
        if (adi_uart_Write(hDevice,&InfoBuffer[0],strlen(InfoBuffer), 0, &nHardwareError) != ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Failed to write  the specified data to UART device");
            bFailure = true;
            break;
        }

        sprintf(InfoBuffer,"\n\t\r The baudrate is: %08d.",(int) nBaudrate);
        if (adi_uart_Write(hDevice,&InfoBuffer[0],strlen(InfoBuffer), 0, &nHardwareError) != ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Failed to write  the specified data to UART device");
            bFailure = true;
            break;
        }

        sprintf(InfoBuffer,"\n\t\r Note: This is using integer precision so it could vary slightly from the actual baudrate.");
        if (adi_uart_Write(hDevice,&InfoBuffer[0],strlen(InfoBuffer), 0, &nHardwareError) != ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Failed to write  the specified data to UART device");
            bFailure = true;
            break;
        }

        sprintf(InfoBuffer,"\n\t\r Please enter any character to echo back on terminal\n");
        if (adi_uart_Write(hDevice,&InfoBuffer[0],strlen(InfoBuffer), 0, &nHardwareError) != ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Failed to write  the specified data to UART device");
            bFailure = true;
            break;
        }

        sprintf(InfoBuffer,"\n\t\r Enter 'Q' to end the example:\n");
        if (adi_uart_Write(hDevice,&InfoBuffer[0],strlen(InfoBuffer), 0, &nHardwareError) != ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Failed to write  the specified data to UART device");
            bFailure = true;
            break;
        }

        /* Wait for the character "Q" to terminate the example. */

        while(nChar != 0x51)
        {
            if (adi_uart_Read(hDevice,&nChar,1, 0, &nHardwareError) != ADI_UART_SUCCESS)
            {
               DEBUG_MESSAGE("Failed to read the input data");
               bFailure = true;
               break;
            }
            if (adi_uart_Write(hDevice,&nChar,1, 0, &nHardwareError) != ADI_UART_SUCCESS)
            {
                DEBUG_MESSAGE("Failed to write  the specified data to UART device");
                bFailure = true;
                break;
            }

        }

        sprintf(InfoBuffer,"\n\t\r Detected char 'Q' and terminating the autobaud example");
        if (adi_uart_Write(hDevice,&InfoBuffer[0],strlen(InfoBuffer), 0, &nHardwareError) != ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Failed to write the specified data to UART device");
            bFailure = true;
            break;

        }

        /* Wait until Tx buffer is completly empty before closing the device */
        while(bTxComplete == false)
        {
            if(adi_uart_IsTxComplete(hDevice, &bTxComplete) != ADI_UART_SUCCESS)
            {
               DEBUG_MESSAGE("Failed to query Tx Complete");
               bFailure = true;
               break;
            }
        }

        /* Disable autobaud. */
        if(adi_uart_EnableAutobaud(hDevice, false, bAutobaudCallback) != ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Failed to disable autobaud");
            bFailure = true;
            break;
        }

        /* Close the device. */
        if(adi_uart_Close(hDevice)!= ADI_UART_SUCCESS)
        {
            DEBUG_MESSAGE("Failed to close the device");
            bFailure = true;
        }

    }while(0);

    /* Check to make sure we reached the callback, if we chose to use it. */
    if((bCallback == false) && (bAutobaudCallback == true) && (bFailure == false))
    {
      DEBUG_MESSAGE("Failed to reach the autobaud success callback.");
      bFailure = true;
    }

    if(bFailure == false)
    {
      common_Pass();
    }
    else
    {
      common_Fail("Autobaud example failed");
    }

    return(0);
}
