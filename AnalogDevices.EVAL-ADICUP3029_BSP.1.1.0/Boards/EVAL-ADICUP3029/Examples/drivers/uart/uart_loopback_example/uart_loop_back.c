/*
 *****************************************************************************
 * @file:    uart_loop_back.c
 * @brief:   File which contain "main" for testing UART Device Driver.
 *****************************************************************************

Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*!
* @file      uart_loop_back.c
*
* @brief     This file contains an example that demonstrates loopback with
*            UART device driver APIs in nonblocking mode.
*
*/
#include <stdio.h>
#include <drivers/uart/adi_uart.h>
#include <drivers/pwr/adi_pwr.h>
#include "uart_loop_back.h"
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

/* Variable to get the address of the processed buffer.*/
void *pProcTxBuff0, *pProcRxBuff0, *pProcTxBuff1, *pProcRxBuff1;

/*********************************************************************
*
*   Function:   main
*
*********************************************************************/
int main(void)
{
	      /* Variable used to check if the Rx buffer has been filled. */
	      bool bRxBufferComplete = false;

        /* Variable that keeps track of any errors found. */
        bool bResult = true;

        /* Variable to catch any errors when there is not a callback registered. */
        uint32_t pHwError;

        /* Variable to get the buffer size to compare the buffer contents at the end of this test. */
        uint32_t nCounter = 0u;

        /* Timeout nCounter for waiting on a receive buffer to be filled. */
        uint32_t nTimeout = 0u;

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

#ifdef UART_INTERNAL_LOOPBACK
              /* configure driver for internal loopback mode. */
              if(adi_uart_EnableLoopBack(hDevice, true) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to configure UART driver for internal loopback mode.");
                  bResult = false;
                  break;
              }
#endif

              /* Submit an empty buffer to the driver for receiving data using interrupt mode. */
              if(adi_uart_SubmitRxBuffer(hDevice, nBufferRx0, SIZE_OF_BUFFER, 0u) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to submit the Rx buffer 0 using interrupt mode.");
                  bResult = false;
                  break;
              }

              /* Submit an empty buffer to the driver for receiving data using DMA mode. */
              if(adi_uart_SubmitRxBuffer(hDevice, nBufferRx1, SIZE_OF_BUFFER, 1u) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to submit the Rx buffer 1 using dma mode.");
                  bResult = false;
                  break;
              }

              /* Submit a filled buffer to the driver using DMA mode. This data will be what fills an empty Rx buffer. */
              if(adi_uart_SubmitTxBuffer(hDevice, nBufferTx0, SIZE_OF_BUFFER, 1u) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to submit Tx buffer 0 using dma mode.");
                  bResult = false;
                  break;
              }

              /* Submit a filled buffer to the driver using interrupt mode. This data will be what
                 fills an empty Rx buffer.
              */
              if(adi_uart_SubmitTxBuffer(hDevice, nBufferTx1, SIZE_OF_BUFFER, 0u) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to submit Tx buffer 1 using interrupt mode.");
                  bResult = false;
                  break;
              }

              /* Return the buffer back to the API. "pProcTxBuff0" should contain the address
                 of "nBufferTx0".
              */
              if(adi_uart_GetTxBuffer(hDevice, &pProcTxBuff0, &pHwError) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to get Tx buffer 0.");
                  bResult = false;
                  break;
              }

              /* Wait here until the processing of the first Rx buffer has completed or a timeout occured. */
              while((bRxBufferComplete == false) && (nTimeout != UART_GET_BUFFER_TIMEOUT))
              {

                  if(adi_uart_IsRxBufferAvailable(hDevice, &bRxBufferComplete) != ADI_UART_SUCCESS)
                  {
                      DEBUG_MESSAGE("Failed to check if the Rx buffer 0 is available.");
                      bResult = false;
                      break;
                  }
                  nTimeout++;
              }

              /* Make sure there was not a timeout. */
              if(nTimeout == UART_GET_BUFFER_TIMEOUT)
              {
                  DEBUG_MESSAGE("Timeout. Check loopback connection.");
                  bResult = false;
                  break;
              }
              else
              {
                nTimeout = 0u;
              }

              /* Return the buffer back to the API. "pProcRxBuff0" should contain the address
                 of "nBufferRx0" and content of size "nBufferRx0". (i.e SIZE_OF_BUFFER)
              */
              if(adi_uart_GetRxBuffer(hDevice, &pProcRxBuff0, &pHwError) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to get Rx buffer 0.");
                  bResult = false;
                  break;
              }

              /* Wait here until the processing of the second Rx buffer has completed or a timeout occured. */
              while((bRxBufferComplete == false) && (nTimeout != UART_GET_BUFFER_TIMEOUT))
              {

                  if(adi_uart_IsRxBufferAvailable(hDevice, &bRxBufferComplete) != ADI_UART_SUCCESS)
                  {
                      DEBUG_MESSAGE("Failed to check if the Rx buffer 1 is available.");
                      bResult = false;
                      break;
                  }
                  nTimeout++;
              }

              /* Make sure there was not a timeout. */
              if(nTimeout == UART_GET_BUFFER_TIMEOUT)
              {
                  DEBUG_MESSAGE("Timeout. Check loopback connection.");
                  bResult = false;
                  break;
              }
              else
              {
                nTimeout = 0u;
              }

              /* Return the buffer back to the API. "pProcRxBuff1" should contain the address
                 of "nBufferRx1" and content of size "nBufferRx1". (i.e SIZE_OF_BUFFER)
              */
              if(adi_uart_GetRxBuffer(hDevice, &pProcRxBuff1, &pHwError) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to get Rx buffer 1.");
                  bResult = false;
                  break;
              }

              /* Return the buffer back to the API. "pProcTxBuff1" should contain the address
                 of "nBufferTx1".
              */
              if(adi_uart_GetTxBuffer(hDevice, &pProcTxBuff1, &pHwError) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to get Tx buffer 1.");
                  bResult = false;
                  break;
              }

#if defined(__CC_ARM)
              /* Close the device. Leave it open for now in the Keil environment */
              if((adi_uart_Close(hDevice)) != ADI_UART_SUCCESS)
              {
                  DEBUG_MESSAGE("Failed to close the device");
                  bResult = false;
              }
#endif
        }while(0);

        if(bResult == true)
        {
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
            DEBUG_MESSAGE("\nUART loop back example completed successfully.");
            DEBUG_MESSAGE("\nSuccessfully tested:");
            DEBUG_MESSAGE("\n\t* Nonblocking mode");
            DEBUG_MESSAGE("\n\t* Interrupt transfers");
            DEBUG_MESSAGE("\n\t* DMA transfers");
#if defined(__CC_ARM)
            /* For the Keil port we are supressing I/O via Retarget.c */
					  {
							 char passedMsg[] = "Passed\n";
							 uint32_t hwError;
               adi_uart_Write(hDevice, passedMsg, strlen(passedMsg), 0, &hwError);
					  }
#endif
        }
        else
        {
          common_Fail("UART loop back example failed.");
        }

        return(0);
}
