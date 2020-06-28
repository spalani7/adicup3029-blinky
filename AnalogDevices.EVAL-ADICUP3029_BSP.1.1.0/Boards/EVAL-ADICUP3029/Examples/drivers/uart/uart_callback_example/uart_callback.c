/*
 *****************************************************************************
 * @file:    uart_callback.c
 * @brief:   File which contain "main" for testing UART Device Driver.
 *****************************************************************************

Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*!
* @file      uart_callback.c
*
* @brief     This file contains an example that demonstrates loopback with
*            UART device driver APIs in callback mode.
*
*/
#include <stdio.h>
#include <drivers/uart/adi_uart.h>
#include <drivers/pwr/adi_pwr.h>
#include "uart_callback.h"
#include "common.h"

extern void adi_initpinmux(void);

/* Handle for the UART device. */
static ADI_UART_HANDLE hDevice;

/* Memory for the UART driver. */
static uint8_t UartDeviceMem[ADI_UART_MEMORY_SIZE];

/* First Tx  Buffer. */
static char  nBufferTx0[SIZE_OF_BUFFER]="ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* First Rx  Buffer. */
static uint8_t nBufferRx0[SIZE_OF_BUFFER];

/* Second Tx  Buffer. */
static char  nBufferTx1[SIZE_OF_BUFFER]="abcdefghijklmnopqrstuvwxyz";

/* Second Rx  Buffer. */
static uint8_t nBufferRx1[SIZE_OF_BUFFER];

static volatile uint32_t   nRxCallbackCounter=0u;
static volatile uint32_t   nTxCallbackCounter=0u;
/*********************************************************************

    Function:       UARTCallback

    Description:    In the example, we've configured the inbound buffer
                    to generate a callback when it is full and an
                    outbound buffer to generate a callback when it is
                    empty.

*********************************************************************/
static void UARTCallback(
    void        *pAppHandle,
    uint32_t     nEvent,
    void        *pArg
)
{

    switch (nEvent)
    {
        case ADI_UART_EVENT_TX_BUFFER_PROCESSED:
        	nTxCallbackCounter++;
        	 break;
        case ADI_UART_EVENT_RX_BUFFER_PROCESSED:
        	nRxCallbackCounter++;
                break;
        default:
                break;


    }
}

/*********************************************************************
*
*   Function:   main
*
*********************************************************************/
int main(void)
{

        /* Variable that keeps track of any errors found. */
        bool bResult = true;

        /* Variable to confirm the transmit shift register is empty before closing the device. */
        bool bTxComplete = false;

        /* Variable to get the buffer size to compare the buffer contents at the end of this test. */
        uint32_t nCounter = 0u;

        /* Pinmux initialization. */
        adi_initpinmux();

        do{
              /* Power initialization. */
              if(adi_pwr_Init() != ADI_PWR_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to intialize the power service.");
                  bResult = false;
                  break;
              }

              /* System clock initialization. */
              if(adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u) != ADI_PWR_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to intialize the system clock.");
                  bResult = false;
                  break;
              }

              /* Peripheral clock initialization. */
              if(adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u) != ADI_PWR_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to intialize the peripheral clock.");
                  bResult = false;
                  break;
              }

              /* Open the bidirectional UART device. */
              if(adi_uart_Open(UART_DEVICE_NUM, ADI_UART_DIR_BIDIRECTION, UartDeviceMem, ADI_UART_MEMORY_SIZE, &hDevice) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to open the UART device.");
                  bResult = false;
                  break;
              }

              /* Register a callback. */
              if(adi_uart_RegisterCallback(hDevice, UARTCallback, NULL) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Call back registration failed");
                  bResult = false;
                  break;
              }

#ifdef UART_INTERNAL_LOOPBACK
              /* configure driver for internal loopback mode. */
              if(adi_uart_EnableLoopBack(hDevice, true) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to configure UART driver for internal loopback mode.");
                  bResult = false;
                  break;
              }
#endif

              /* Submit an empty buffer to the driver for receiving data using DMA mode. */
              if(adi_uart_SubmitRxBuffer(hDevice, nBufferRx0, SIZE_OF_BUFFER, 1u) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to submit the Rx buffer 0 using DMA mode.");
                  bResult = false;
                  break;
              }

              /* Submit an empty buffer to the driver for receiving data using interrupt mode. */
              if(adi_uart_SubmitRxBuffer(hDevice, nBufferRx1, SIZE_OF_BUFFER, 0u) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to submit the Rx buffer 1 using interrupt mode.");
                  bResult = false;
                  break;
              }

              /* Submit a filled buffer to the driver using interrupt mode. This data will be what fills an empty Rx buffer. */
              if(adi_uart_SubmitTxBuffer(hDevice, nBufferTx0, SIZE_OF_BUFFER, 0u) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to submit Tx buffer 0 using interrupt mode.");
                  bResult = false;
                  break;
              }

              /* Submit a filled buffer to the driver using DMA mode. This data will be what
                 fills an empty Rx buffer.
              */
              if(adi_uart_SubmitTxBuffer(hDevice, nBufferTx1, SIZE_OF_BUFFER, 1u) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to submit Tx buffer 1 using DMA mode.");
                  bResult = false;
                  break;
              }

              /* Make sure the transmit shift register is empty before closing the device. */
              while(bTxComplete == false)
              {
                  if(adi_uart_IsTxComplete(hDevice, &bTxComplete) != ADI_UART_SUCCESS)
                  {
                      DEBUG_MESSAGE("Failed to check if the transmit holding register is empty.");
                      bResult = false;
                      break;
                  }
              }

              /* Close the device. */
              if((adi_uart_Close(hDevice)) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to close the device");
                  bResult = false;
              }
        }while(0);

        if(bResult == true)
        {
            if((nRxCallbackCounter != 2) || (nTxCallbackCounter != 2))
            {
               bResult = false;
               DEBUG_MESSAGE("\nThe callback was entered an incorrect number of times.");
            }

            /* Make sure the data transfers all worked properly. */
            for(nCounter = 0u; nCounter < SIZE_OF_BUFFER; nCounter++)
            {
               if((nBufferTx0[nCounter] != nBufferRx0[nCounter]) || ( nBufferTx1[nCounter] != nBufferRx1[nCounter]))
               {
                  bResult = false;
                  DEBUG_MESSAGE("\nDetected a mismatch in Rx and Tx buffers.");
                  break;
               }
            }
        }

        if(bResult == true)
        {
            common_Pass();
            DEBUG_MESSAGE("\nUART callback example completed successfully.");
            DEBUG_MESSAGE("\nSuccessfully tested:");
            DEBUG_MESSAGE("\n\t* Callback mode");
            DEBUG_MESSAGE("\n\t* Interrupt transfers");
            DEBUG_MESSAGE("\n\t* DMA transfers");
        }
        else
        {
          common_Fail("UART callback example failed.");
        }

        return(0);
}
