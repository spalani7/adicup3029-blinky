/*********************************************************************************

Copyright(c) 2016 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/
/*
*
*           Primary header file for SPI driver example which contains the
*           processor specific defines.
*
*/

#ifndef _SPI_LOOPBACK_H_
#define _SPI_LOOPBACK_H_


/* Choose a SPI device */
#define SPI_DEVICE_NUM    0U

/** define size of data buffers, DMA max size is 255 */
#define BUFFERSIZE 251

/* In RD_CTL mode the master will first transmit between 1 and 16 bytes */
#define SPI_RD_CTL_NBYTES_TX   3
#define SPI_RD_CTL_NBYTES_RX   5

/* enable internal loopback for testing without physical loopback jumper */
#define ENABLE_INTERNAL_SPI_LOOPBACK

/* Pin muxing */
extern int32_t adi_initpinmux(void);

#endif /* _SPI_LOOPBACK_H_ */
