/*! *****************************************************************************
 * @file    blinky_example.c
 * @brief   Example showing how to use the GPIO driver to blink LEDs as outputs.
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

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-
INFRINGEMENT, TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF
CLAIMS OF INTELLECTUAL PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/


#include "blinky_example.h"

typedef struct {
	ADI_GPIO_PORT Port;
	ADI_GPIO_DATA Pins;
} PinMap;

/* LED GPIO assignments */

#ifdef __EVCOG__
PinMap MSB = {ADI_GPIO_PORT1, ADI_GPIO_PIN_10};  /*   Red LED on GPIO31 (DS4) */
PinMap LSB = {ADI_GPIO_PORT2, ADI_GPIO_PIN_2};   /* Green LED on GPIO32 (DS3) */
#else
PinMap MSB = {ADI_GPIO_PORT1, ADI_GPIO_PIN_15};  /*   Red LED on GPIO31 (DS4) */
PinMap LSB = {ADI_GPIO_PORT2, ADI_GPIO_PIN_0};   /* Green LED on GPIO32 (DS3) */
#endif

uint8_t sunMemory[32] __attribute__ ((section (".isram_retained"))) = { 0 };

int main(void)
{
	uint8_t         gpioMemory[ADI_GPIO_MEMORY_SIZE] = {0};
	uint32_t        count = 0;
	ADI_PWR_RESULT  ePwrResult;
	ADI_GPIO_RESULT eGpioResult;

	/* common init */
	common_Init();

	ePwrResult = adi_pwr_Init();
	DEBUG_RESULT("adi_pwr_Init failed.", ePwrResult, ADI_PWR_SUCCESS);

	ePwrResult = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u);
	DEBUG_RESULT("adi_pwr_SetClockDivider (HCLK) failed.", ePwrResult, ADI_PWR_SUCCESS);

	ePwrResult = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u);
	DEBUG_RESULT("adi_pwr_SetClockDivider (PCLK) failed.", ePwrResult, ADI_PWR_SUCCESS);

    /* Initialize GPIO driver */
    eGpioResult= adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE);
    DEBUG_RESULT("adi_GPIO_Init failed.", eGpioResult, ADI_GPIO_SUCCESS);

    /* Enable MSB output */
    eGpioResult = adi_gpio_OutputEnable(MSB.Port, MSB.Pins, true);
    DEBUG_RESULT("adi_GPIO_SetOutputEnable failed on MSB.", eGpioResult, ADI_GPIO_SUCCESS);

    /* Enable LSB output */
    eGpioResult = adi_gpio_OutputEnable(LSB.Port, LSB.Pins, true);
    DEBUG_RESULT("adi_GPIO_SetOutputEnable failed on LSB.", eGpioResult, ADI_GPIO_SUCCESS);

	/* Loop indefinitely */
    while (count <= MAX_COUNT)  {

		/* Delay between iterations */
		for (volatile uint32_t i = 0; i < 1000000; i++)
            ;

		/* Blink count (mod4) on the LEDs */
		switch (count%4) {
			case 3:
				eGpioResult = adi_gpio_SetHigh (MSB.Port, MSB.Pins);
				DEBUG_RESULT("adi_gpio_SetHigh (MSB).", eGpioResult, ADI_GPIO_SUCCESS);

				eGpioResult = adi_gpio_SetHigh(LSB.Port,  LSB.Pins);
				DEBUG_RESULT("adi_gpio_SetHigh (LSB).", eGpioResult, ADI_GPIO_SUCCESS);
				break;

			case 2:
				eGpioResult = adi_gpio_SetHigh (MSB.Port, MSB.Pins);
				DEBUG_RESULT("adi_gpio_SetHigh (MSB).", eGpioResult, ADI_GPIO_SUCCESS);

				eGpioResult = adi_gpio_SetLow(LSB.Port,  LSB.Pins);
 				DEBUG_RESULT("adi_gpio_SetLow (LSB).", eGpioResult, ADI_GPIO_SUCCESS);
				break;

			case 1:
				eGpioResult = adi_gpio_SetLow (MSB.Port, MSB.Pins);
				DEBUG_RESULT("adi_gpio_SetLow (MSB).", eGpioResult, ADI_GPIO_SUCCESS);

				eGpioResult = adi_gpio_SetHigh(LSB.Port,  LSB.Pins);
 				DEBUG_RESULT("adi_gpio_SetHigh (LSB).", eGpioResult, ADI_GPIO_SUCCESS);
				break;

			case 0:
				eGpioResult = adi_gpio_SetLow (MSB.Port, MSB.Pins);
				DEBUG_RESULT("adi_gpio_SetLow (MSB).", eGpioResult, ADI_GPIO_SUCCESS);

				eGpioResult = adi_gpio_SetLow(LSB.Port,  LSB.Pins);
				DEBUG_RESULT("adi_gpio_SetLow (LSB).", eGpioResult, ADI_GPIO_SUCCESS);
				break;

		}
		count++;
	}

    common_Pass();

	return 0;
}
