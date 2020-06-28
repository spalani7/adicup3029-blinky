/*

Copyright (c) 2011-2017 Analog Devices, Inc.

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

#include <stddef.h>		/* for 'NULL' */
#include <string.h>		/* for strlen */
#include "common.h"

#include <drivers/general/adi_drivers_general.h>

#include <drivers/spi/adi_spi.h>
#include <drivers/pwr/adi_pwr.h>
#include <drivers/gpio/adi_gpio.h>
#include <drivers/dma/adi_dma.h>

#include "spi_loopback.h"

 /* Transmit data buffer */
ADI_ALIGNED_PRAGMA(2)
static uint8_t overtx[BUFFERSIZE] ADI_ALIGNED_ATTRIBUTE(2);


/* Receieve data buffer a */
ADI_ALIGNED_PRAGMA(2)
static uint8_t overrx[BUFFERSIZE] ADI_ALIGNED_ATTRIBUTE(2);

/* Data buffers for RD_CTL mode of operation */
ADI_ALIGNED_PRAGMA(2)
static uint8_t overtx_rdctl[SPI_RD_CTL_NBYTES_TX] ADI_ALIGNED_ATTRIBUTE(2);
ADI_ALIGNED_PRAGMA(2)
static uint8_t overrx_rdctl[SPI_RD_CTL_NBYTES_RX] ADI_ALIGNED_ATTRIBUTE(2);

/* the "usable" buffers within the overallocated buffers housing the guard data */

ADI_ALIGNED_PRAGMA(2)
uint8_t spidevicemem[ADI_SPI_MEMORY_SIZE] ADI_ALIGNED_ATTRIBUTE(2);

/* Forward declarations */
static ADI_SPI_RESULT run_the_loopback_nonBlockingMode(ADI_SPI_HANDLE hDevice, bool useDma);
static ADI_SPI_RESULT run_the_loopback_nonBlockingMode_useRdCtl(ADI_SPI_HANDLE hDevice, bool useDma);
static ADI_SPI_RESULT run_the_loopback_blockingMode(ADI_SPI_HANDLE hDevice, bool useDma);
static ADI_SPI_RESULT run_the_loopback_blockingMode_useRdCtl(ADI_SPI_HANDLE hDevice, bool useDma);

int main(void)
{
    ADI_SPI_HANDLE hDevice;
    ADI_SPI_RESULT eResult;

    adi_initpinmux();

    /* Common system initialization */
    common_Init();

    /* Power initialization */
    if(ADI_PWR_SUCCESS != adi_pwr_Init())
    {
        DEBUG_MESSAGE("Failed to intialize the power service");
    }
    if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_HCLK,1))
    {
        DEBUG_MESSAGE("Failed to intialize the power service");
    }
    if(ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_PCLK,1))
    {
        DEBUG_MESSAGE("Failed to intialize the power service\n");
    }

    /* Initialize SPI */
    eResult = adi_spi_Open(SPI_DEVICE_NUM,spidevicemem, ADI_SPI_MEMORY_SIZE, &hDevice);
    DEBUG_RESULT("Failed to init SPI driver",eResult,ADI_SPI_SUCCESS);

    /* throttle bitrate to something the controller can reach */
    eResult = adi_spi_SetBitrate(hDevice, 2500000);
    DEBUG_RESULT("Failed to set Bitrate",eResult,ADI_SPI_SUCCESS);

    /* Set IRQMODE. In this case we are setting it to the default value  */
    /* This code sequence is just calling out the fact that this API would be required  */
    /* for short bursts (less than the size of the FIFO) in PIO (interrupt) mode        */
    eResult = adi_spi_SetIrqmode(hDevice, 0u);
    DEBUG_RESULT("Failed to set Irqmode",eResult,ADI_SPI_SUCCESS);

    /* set the chip select */
    eResult = adi_spi_SetChipSelect(hDevice, ADI_SPI_CS0);
    DEBUG_RESULT("Failed to set the chip select",eResult,ADI_SPI_SUCCESS);

#ifdef ENABLE_INTERNAL_SPI_LOOPBACK
    /* set internal loopback mode */
    eResult = adi_spi_SetLoopback(hDevice, true);
    DEBUG_RESULT("Failed to set internal loopback mode",eResult,ADI_SPI_SUCCESS);
#endif

    /* Demonstrate non-dma blocking mode with and without RD_CTL */
    eResult = run_the_loopback_blockingMode(hDevice, false);
    DEBUG_RESULT("SPI interrupt-driven blocking mode example failed 1",eResult,ADI_SPI_SUCCESS);
    eResult = run_the_loopback_blockingMode_useRdCtl(hDevice, false);
    DEBUG_RESULT("SPI interrupt-driven blocking mode example failed 2",eResult,ADI_SPI_SUCCESS);

    /* Demonstrate non-dma non-blocking mode with and without RD_CTL*/
    eResult = run_the_loopback_nonBlockingMode(hDevice, false);
    DEBUG_RESULT("SPI interrupt-driven blocking mode example failed 3",eResult,ADI_SPI_SUCCESS);
    eResult = run_the_loopback_nonBlockingMode_useRdCtl(hDevice, false);
    DEBUG_RESULT("SPI interrupt-driven blocking mode example failed 4",eResult,ADI_SPI_SUCCESS);

    /* Demonstrate dma-based blocking mode with and without RD_CTL */
    eResult = run_the_loopback_blockingMode(hDevice, true);
    DEBUG_RESULT("SPI interrupt-driven blocking mode example failed 5",eResult,ADI_SPI_SUCCESS);
    eResult = run_the_loopback_blockingMode_useRdCtl(hDevice, true);
    DEBUG_RESULT("SPI interrupt-driven blocking mode example failed 6",eResult,ADI_SPI_SUCCESS);


    /* Demonstrate dma-based non-blocking mode with and without RD_CTL */
    eResult = run_the_loopback_nonBlockingMode(hDevice, true);
    DEBUG_RESULT("SPI interrupt-driven blocking mode example failed 7",eResult,ADI_SPI_SUCCESS);
    eResult = run_the_loopback_nonBlockingMode(hDevice, true);
    DEBUG_RESULT("SPI interrupt-driven blocking mode example failed 8",eResult,ADI_SPI_SUCCESS);

    /* Close the SPI device  */
    eResult = adi_spi_Close(hDevice);
    DEBUG_RESULT("Failed to uninit SPI driver",eResult,ADI_SPI_SUCCESS);

    /* The example has successfully completed */
    common_Pass();

}



/*
 *
 * This routine will use the following fatures of the SPI driver
 *    1. Non-blocking mode of operation via the adi_spi_MasterSubmitBuffer API
 *    2. DMA is either true/false based on the value of the useDma parameter
 *    3. Blocking for transaction completion using the adi_spi_GetBuffer API
 *
 */
static ADI_SPI_RESULT run_the_loopback_nonBlockingMode(ADI_SPI_HANDLE hDevice, bool useDma)
{
    ADI_SPI_RESULT result;
    ADI_SPI_TRANSCEIVER transceive;

    /* initialize both the RX and TX buffers */
    for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
        overtx[i] = (unsigned char)i;
        overrx[i] = (uint8_t)0xdd;
    }


    /* link transceive data size to the remaining count */
    transceive.TransmitterBytes = BUFFERSIZE;
    /* link transceive data size to the remaining count */
    transceive.ReceiverBytes = BUFFERSIZE;
    /* initialize data attributes */
    transceive.pTransmitter = overtx;
    transceive.pReceiver = overrx;
    /* auto increment both buffers */
    transceive.nTxIncrement = 1;
    transceive.nRxIncrement = 1;
    transceive.bDMA = useDma;
    transceive.bRD_CTL = false;

    /* Use the non-blocking API */
    if (ADI_SPI_SUCCESS != (result = adi_spi_MasterSubmitBuffer(hDevice, &transceive)))
    {
        return result;
    }

    /* When using non-blocking APIs the application can do useful work here */


    /* When the application calls adi_spi_GetBuffer it will block           */
    /* until the transaction completes                                      */

    uint32_t HWError;
    ADI_SPI_RESULT res = adi_spi_GetBuffer(hDevice, &HWError);
    if( (res != ADI_SPI_SUCCESS) || (HWError != (uint32_t)ADI_SPI_HW_ERROR_NONE)) {
        return ADI_SPI_FAILURE;
    }


    /* verify receive data                                                   */
    /* Since the data is simply looped back from MOSI to MISO we simply need */
    /* to verify that the tx and rx buffers are identical                    */
    for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
        if (overtx[i] != overrx[i]) {
            return ADI_SPI_FAILURE;  /* failure */
        }
    }
    return ADI_SPI_SUCCESS;
}


/*
 *
 * This routine will use the following fatures of the SPI driver
 *    1. Blocking mode of operation via the adi_spi_MasterReadWrite API
 *    2. DMA is either true/false based on the value of the useDma parameter
 *
 */

static ADI_SPI_RESULT run_the_loopback_blockingMode(ADI_SPI_HANDLE hDevice, bool useDma)
{
    ADI_SPI_RESULT result;
    ADI_SPI_TRANSCEIVER transceive;

    /* initialize both the RX and TX buffers */
    for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
        overtx[i] = (unsigned char)i;
        overrx[i] = (uint8_t)0xdd;
    }

    /* link transceive data size to the remaining count */
    transceive.TransmitterBytes = BUFFERSIZE;
    /* link transceive data size to the remaining count */
    transceive.ReceiverBytes = BUFFERSIZE;
    /* initialize data attributes */
    transceive.pTransmitter = overtx;
    transceive.pReceiver = overrx;
    /* auto increment both buffers */
    transceive.nTxIncrement = 1;
    transceive.nRxIncrement = 1;
    transceive.bDMA = useDma;
    transceive.bRD_CTL = false;

    if (ADI_SPI_SUCCESS != (result = adi_spi_MasterReadWrite(hDevice, &transceive)))
    {
        return result;
    }


    /* verify receive data                                                   */
    /* Since the data is simply looped back from MOSI to MISO we simply need */
    /* to verify that the tx and rx buffers are identical                    */
    for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
        if (overtx[i] != overrx[i]) {
            return ADI_SPI_FAILURE;  /* failure */
        }
    }

    return ADI_SPI_SUCCESS;
}

/*
 *
 * This routine will use the following fatures of the SPI driver
 *    1. Non-blocking mode of operation via the adi_spi_MasterSubmitBuffer API
 *    2. The RD_CTL mode of operation is used
 *    3. DMA is either true/false based on the value of the useDma parameter
 *    4. Polling for transaction completion using the adi_spi_isBufferAvailable API
 *    5. Reclaiming compeleted transaction buffer using the adi_spi_GetBuffer API
 *       Note that we know this API will not block since the transaction is finished
 *       before the call to adi_spi_GetBuffer is made.
 *
 *
 */

static ADI_SPI_RESULT run_the_loopback_nonBlockingMode_useRdCtl(ADI_SPI_HANDLE hDevice, bool useDma)
{
    ADI_SPI_RESULT result = ADI_SPI_SUCCESS;  /* assume the best */
    ADI_SPI_TRANSCEIVER transceive;

    transceive.TransmitterBytes = SPI_RD_CTL_NBYTES_TX;
    /* link transceive data size to the remaining count */
    transceive.ReceiverBytes = SPI_RD_CTL_NBYTES_RX;
    /* initialize data attributes */
    transceive.pTransmitter = overtx_rdctl;
    transceive.pReceiver = overrx_rdctl;
    /* auto increment both buffers */
    transceive.nTxIncrement = 1;
    transceive.nRxIncrement = 1;
    transceive.bDMA = useDma;
    transceive.bRD_CTL = true;

    /* Use the non-blocking API */
    if (ADI_SPI_SUCCESS != (result = adi_spi_MasterSubmitBuffer(hDevice, &transceive)))
    {
        return result;
    }

    /* Poll until the transaction is complete                  */

    bool bMasterComplete= false;
    ADI_SPI_RESULT res;
    uint32_t HWError;

    /* Poll for transaction completion.                                */
    /* When using non-blocking APIs the application can do useful work */
    /* before/while checking to see if the transaction is done         */

    while( (bMasterComplete == false) )
    {
      res = adi_spi_isBufferAvailable(hDevice,&bMasterComplete);
      if( res != ADI_SPI_SUCCESS ) {
        return ADI_SPI_FAILURE;
      }
    }

    /* Transaction is now complete, reclaim ownership of the buffer */
    res = adi_spi_GetBuffer(hDevice, &HWError);

    if( (res != ADI_SPI_SUCCESS) || (HWError != (uint32_t)ADI_SPI_HW_ERROR_NONE)) {
        return ADI_SPI_FAILURE;
    }


    /* The SPI MOSI is looped back to SPI MISO                                                    */
    /* In RD_CTL mode                                                                             */
    /*   1. The SPI will first transmit the MOSI line                                             */
    /*   2. The SPI will stop transmitting but will still clock the slave while receiving on MISO */
    /*                                                                                            */
    /* In this example, since MOSI is connected to MISO there will be zeros on on the MISO line   */
    /* while the receieve portion of a RD_CTL transaction performed                               */
    /* Therefore, all recieved bytes will be zero                                                 */

    for (unsigned int i = 0u; i < SPI_RD_CTL_NBYTES_RX; i++) {
        if (0 != overrx_rdctl[i]) {
            return ADI_SPI_FAILURE;
        }
    }
    return ADI_SPI_SUCCESS;
}

/*
 *
 * This routine will use the following fatures of the SPI driver
 *    1. Blocking mode of operation via the adi_spi_MasterSubmitBuffer API
 *    2. The RD_CTL mode of operation is used
 *    3. DMA is either true/false based on the value of the useDma parameter
 *
 *
 */

static ADI_SPI_RESULT run_the_loopback_blockingMode_useRdCtl(ADI_SPI_HANDLE hDevice, bool useDma)
{
    ADI_SPI_RESULT result = ADI_SPI_SUCCESS;  /* assume the best */
    ADI_SPI_TRANSCEIVER transceive;

    /* initialize both the RX and TX buffers */
    memset(overtx_rdctl, -1, SPI_RD_CTL_NBYTES_TX);
    memset(overrx_rdctl, -1, SPI_RD_CTL_NBYTES_RX);

    /* Set up for a SPI RD_CTL transfer  */
    transceive.TransmitterBytes = SPI_RD_CTL_NBYTES_TX;
    /* link transceive data size to the remaining count */
    transceive.ReceiverBytes = SPI_RD_CTL_NBYTES_RX;
    /* initialize data attributes */
    transceive.pTransmitter = overtx_rdctl;
    transceive.pReceiver = overrx_rdctl;
    transceive.nTxIncrement = 1;
    transceive.nRxIncrement = 1;
    transceive.bDMA = useDma;
    transceive.bRD_CTL = true;

    /* Call the blocking mode API */
    if (ADI_SPI_SUCCESS != (result = adi_spi_MasterReadWrite(hDevice, &transceive)))
    {
        return result;
    }

    /* The SPI MOSI is looped back to SPI MISO                                                    */
    /* In RD_CTL mode                                                                             */
    /*   1. The SPI will first transmit the MOSI line                                             */
    /*   2. The SPI will stop transmitting but will still clock the slave while receiving on MISO */
    /*                                                                                            */
    /* In this example, since MOSI is connected to MISO there will be zeros on on the MISO line   */
    /* while the receieve portion of a RD_CTL transaction performed                               */
    /* Therefore, all recieved bytes will be zero                                                 */
    for (unsigned int i = 0u; i < SPI_RD_CTL_NBYTES_RX; i++) {
        if (0 != overrx_rdctl[i]) {
            return  ADI_SPI_FAILURE;
        }
    }
    return ADI_SPI_SUCCESS;
}



