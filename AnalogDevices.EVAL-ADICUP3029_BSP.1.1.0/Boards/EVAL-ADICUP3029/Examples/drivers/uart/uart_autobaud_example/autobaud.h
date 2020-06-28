/*
 *****************************************************************************
 * @file:    autobaud.h
 * @brief:   Header file for autobaud.c
 *****************************************************************************

Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*!
* @file      autobaud.h
*
* @brief     This is file is include file for contain an autobaud example.
*
*/
#include <stdio.h>


/* Memory required by the driver for DMA mode of operation */
#define ADI_UART_MEMORY_SIZE    (ADI_UART_BIDIR_MEMORY_SIZE)

/* UART device number. There are 2 devices for ADUCM4x50,so this can be 0 or 1.
   But ADUCM302x has only 1 device ,so it should be '0' */
#define UART_DEVICE_NUM 0u

/* Timeout value for waiting to calculate autobaud. */
#define UART_AUTOBAUD_TIMEOUT 6000000u
