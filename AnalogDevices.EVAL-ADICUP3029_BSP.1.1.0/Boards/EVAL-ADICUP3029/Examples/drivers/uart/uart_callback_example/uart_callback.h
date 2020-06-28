/*
 *****************************************************************************
 * @file:    uart_callback.h
 * @brief:   Header file for uart_callback.c
 *****************************************************************************

Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*!
* @file      uart_callback.h
*
* @brief     This is a header file for uart_callback.c
*
*/
#ifndef UART_CALLBACK_H
#define UART_CALLBACK_H

static void UARTCallback(void        *pAppHandle,
                         uint32_t     nEvent,
                         void        *pArg);

/* Memory required by the driver for bidirectional mode of operation. */
#define ADI_UART_MEMORY_SIZE    (ADI_UART_BIDIR_MEMORY_SIZE)

/* Size of the data buffers that will be processed. */
#define SIZE_OF_BUFFER  26u

/* UART device number. There are 2 devices for ADUCM4x50,so this can be 0 or 1.
   But ADUCM302x has only 1 device ,so it should be "0" */
   
#if defined (__ADUCM302x__)
#define 	UART_DEVICE_NUM 0u
#elif defined(__ADUCM4x50__)
#define 	UART_DEVICE_NUM 1u
#else
#error UART driver is not ported for this processor
#endif


/* Internal loopback mode. Defined for internal loopback (no jumpers required),
   undefined for external loopback (see Readme_uart_loopback.txt) */
#define UART_INTERNAL_LOOPBACK


/* Timeout value for receiving data. */
#define UART_GET_BUFFER_TIMEOUT 1000000u

#endif /* UART_CALLBACK_H */
