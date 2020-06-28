/*
 **
 ** Source file generated on September 14, 2014 at 22:01:39.
 **
 ** Copyright (C) 2016 Analog Devices Inc., All Rights Reserved.
 **
 ** This file is generated automatically based upon the options selected in
 ** the Pin Multiplexing configuration editor. Changes to the Pin Multiplexing
 ** configuration should be made by changing the appropriate options rather
 ** than editing this file.
 **
 ** Selected Peripherals
 ** --------------------
 ** SPI0 (SCLK, MISO, MOSI)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P0_00, P0_01, P0_02
 */

#include <adi_processor.h>

/* 1st three bits of Mux1 */
#define SPI0_CLK_PORTP0_MUX   ((uint16_t) ((uint16_t) 1<<0))
#define SPI0_MOSI_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<2))
#define SPI0_MISO_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<4))

int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* PORTx_MUX registers */
    *pREG_GPIO0_CFG = SPI0_CLK_PORTP0_MUX | SPI0_MOSI_PORTP0_MUX | SPI0_MISO_PORTP0_MUX;

    return 0;
}

