/*********************************************************************************
   @file:    button_interrupt.c
   @brief:   Wakeup button example source file.

 -------------------------------------------------------------------------------

Copyright(c) 2017 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/

/*
* Use the XINT and GPIO driver to toggle LED when the wakeup button is pressed
* on the Kit.
*/

#include <drivers/pwr/adi_pwr.h>
#include <drivers/gpio/adi_gpio.h>
#include <drivers/xint/adi_xint.h>
#include <common.h>

#include "xint_example.h"


/* globals */
uint8_t gpioMemory[ADI_GPIO_MEMORY_SIZE];
uint8_t xintMemory[ADI_XINT_MEMORY_SIZE];

volatile uint64_t loopcount;
volatile ADI_GPIO_RESULT gpioXINT0 = ADI_GPIO_FAILURE;
volatile uint32_t milliseconds;
#ifdef __EVCOG__
/*LED GPIO assignments */
PinMap ledRed           = {ADI_GPIO_PORT1, ADI_GPIO_PIN_10};  /*   Red LED on GPIO26  */
PinMap ledGreen         = {ADI_GPIO_PORT2, ADI_GPIO_PIN_2 }; /* Green LED on GPIO34  */
#else
/* LED GPIO assignments */
PinMap ledRed           = {ADI_GPIO_PORT1, ADI_GPIO_PIN_15};  /*   Red LED on GPIO31 (DS4) */
PinMap ledGreen         = {ADI_GPIO_PORT2, ADI_GPIO_PIN_0 };  /* Green LED on GPIO32 (DS3) */
#endif
/* Other GPIO pin assignments */
PinMap xint0InputPin    = {ADI_GPIO_PORT0, ADI_GPIO_PIN_15};  /* Wake0 (XINT0) pin (GPIO15) */
PinMap xintSimOutputPin = {ADI_GPIO_PORT0, ADI_GPIO_PIN_13};  /* Wake2 (XINT2) pin (GPIO13) */
PinMap BootPin          = {ADI_GPIO_PORT1, ADI_GPIO_PIN_1 };  /* "Boot" pushbutton (GPIO17) */


/*
 * SysTick interrupt handler
 */
void SysTick_Handler(void)
{
	/* simply bump the counter every millisecond */
	milliseconds++;
}


/*
 * Simulate an external XINT interrupt event in software that would normally
 * come from a hard pushbutton or such.  NOTE: because the EVAL-ADICUP3029
 * board has no pushbuttons tied up to any of the fixed XINT pins (XINT0,
 * XINT1, XINT2, or XINT3), we take the XINT simulation approach to
 * demonstrate the XINT hardware feature and Device Driver functionality.
 */
static void simulateXINT(void)
{
    /* Simulate an artificial XINT event manually by driving the XINT input pin
     * with a GPIO line, simulating an external interrupt event.
	 */

	ADI_GPIO_RESULT result = ADI_GPIO_FAILURE;

	result = adi_gpio_SetLow(xintSimOutputPin.Port, xintSimOutputPin.Pins);
    DEBUG_RESULT("adi_gpio_SetLow failed.", result, ADI_GPIO_SUCCESS);

	result = adi_gpio_SetHigh(xintSimOutputPin.Port, xintSimOutputPin.Pins);
    DEBUG_RESULT("adi_gpio_SetLow failed.", result, ADI_GPIO_SUCCESS);
}


/*
 * XINT Callback event handler
 */
static void xintCallback(void* pCBParam, uint32_t nEvent,  void* pEventData)
{
    uint16_t *pPortInterruptStatus;

    /* update flag if we got XINT0... */
    if ((ADI_XINT_EVENT)nEvent == ADI_XINT_EVENT_INT0)
    {
        /* toggle red LED */
        gpioXINT0 = adi_gpio_Toggle(ledRed.Port, ledRed.Pins);

		/* reset any latched pin interrupt in GPIO controller ... */
		pPortInterruptStatus = (uint16_t*)&(pADI_GPIO1->INT);
		*pPortInterruptStatus = (uint16_t)BootPin.Pins;  /* W1C */

		/* also clear any pending GroupA interrupt in NVIC */
		NVIC_ClearPendingIRQ(SYS_GPIO_INTA_IRQn);

		/* reset the loop count */
		loopcount = 0u;
    }
}


/*
 * GPIO pin interrupt Callback handler
 */
static void pinCallback(void* pCBParam, uint32_t Port,  void* PinIntData)
{
	/* test for Boot pushbutton pin (GPIO17) */
	if ( (Port == BootPin.Port) && (*(uint32_t*)PinIntData & BootPin.Pins) )
	{
		/* BootPin got the low-to-high transition... */

		/* reset millisecond counter */
		milliseconds = 0;

		/* wait some number of ms for Boot pin to debounce... */
		while (milliseconds < DEBOUNCE_MS)
			;

		/* simulate an XINT "input" event upon Boot button push */
		simulateXINT();
	}
}


/*
 * main
 */
int main(void)
{
	/* assume the worst... */
    ADI_GPIO_RESULT gpioResult = ADI_GPIO_FAILURE;
    ADI_XINT_RESULT xintResult = ADI_XINT_FAILURE;
    ADI_PWR_RESULT  pwrResult  = ADI_PWR_FAILURE;

    /* test system initialization */
    common_Init();

    pwrResult = adi_pwr_Init();
    DEBUG_RESULT("adi_pwr_Init failed.", pwrResult, ADI_PWR_SUCCESS);

    pwrResult = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u);
    DEBUG_RESULT("adi_pwr_SetClockDivider (HCLK) failed.", pwrResult, ADI_PWR_SUCCESS);

    pwrResult = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u);
    DEBUG_RESULT("adi_pwr_SetClockDivider (PCLK) failed.", pwrResult, ADI_PWR_SUCCESS);

    /* setup SyTick for 1ms interrupts for pushbutton debouncing */
	SysTick_Config(SYSTEM_CORE_CLOCK  / SYSTICK_MS_DIVIDER);

	/* demote all programmable interrupts so we can promote SysTick interrupt above them all in priority */
    for (IRQn_Type irq = RTC1_EVT_IRQn; irq < DMA0_CH23_DONE_IRQn; irq++) {
        NVIC_SetPriority(irq, LOWEST_PRIORITY);
    }

    /* ...raise SysTick priority so it can update millisecond debounce counter during other active interrupts */
    NVIC_SetPriority(SysTick_IRQn, SYSTICK_PRIORITY);

	/* configure hardware */
    do
    {
        /* init the GPIO service */
        if(ADI_GPIO_SUCCESS != (gpioResult = adi_gpio_Init(gpioMemory, ADI_GPIO_MEMORY_SIZE)))
        {
            DEBUG_MESSAGE("adi_gpio_Init failed\n");
            break;
        }

        /* Configure Boot pushbutton pin for input */
        if(ADI_GPIO_SUCCESS != (gpioResult = adi_gpio_InputEnable(BootPin.Port, BootPin.Pins, true)))
        {
            DEBUG_MESSAGE("adi_gpio_InputEnable failed\n");
            break;
        }

        /* Configure XINT0 pin for "input" (caught by xintCallback handler) */
        if(ADI_GPIO_SUCCESS != (gpioResult = adi_gpio_InputEnable(xint0InputPin.Port, xint0InputPin.Pins, true)))
        {
            DEBUG_MESSAGE("adi_gpio_InputEnable failed\n");
            break;
        }

        /* Configure XINT1 pin for "output" (driven by Boot pin interrupt handler) */
        if(ADI_GPIO_SUCCESS != (gpioResult = adi_gpio_OutputEnable(xintSimOutputPin.Port, xintSimOutputPin.Pins, true)))
        {
            DEBUG_MESSAGE("adi_gpio_InputEnable failed\n");
            break;
        }

        /* set XINT1 output simulation pin initially HI */
        if(ADI_GPIO_SUCCESS != (gpioResult = adi_gpio_SetHigh(xintSimOutputPin.Port, xintSimOutputPin.Pins)))
        {
            DEBUG_MESSAGE("adi_gpio_SetHigh failed\n");
            break;
        }

        /* Configure red LED for output */
        if(ADI_GPIO_SUCCESS != (gpioResult = adi_gpio_OutputEnable(ledRed.Port, ledRed.Pins, true)))
        {
            DEBUG_MESSAGE("adi_gpio_OutputEnable failed\n");
            break;
        }

        /* Configure green LED for output */
        if(ADI_GPIO_SUCCESS != (gpioResult = adi_gpio_OutputEnable(ledGreen.Port, ledGreen.Pins, true)))
        {
            DEBUG_MESSAGE("adi_gpio_OutputEnable failed\n");
            break;
        }

        /* configure Group pin interrupt polarity to interrupt on low-to-high pin transition */
        if(ADI_GPIO_SUCCESS != (gpioResult = adi_gpio_SetGroupInterruptPolarity(BootPin.Port, BootPin.Pins)))
        {
            DEBUG_MESSAGE("adi_gpio_SetGroupInterruptPolarity failed\n");
            break;
        }

        /* configure GroupA pin set to interrupt on */
        if(ADI_GPIO_SUCCESS != (gpioResult = adi_gpio_SetGroupInterruptPins(BootPin.Port, ADI_GPIO_INTA_IRQ, BootPin.Pins)))
        {
            DEBUG_MESSAGE("adi_gpio_SetGroupInterruptPins failed\n");
            break;
        }

		/* register a GroupA pin set interrupt callback handler */
        if(ADI_GPIO_SUCCESS != (gpioResult = adi_gpio_RegisterCallback (ADI_GPIO_INTA_IRQ, pinCallback, (void*)ADI_GPIO_INTA_IRQ)))
        {
            DEBUG_MESSAGE("adi_gpio_RegisterCallback failed\n");
            break;
        }


        /* Now configure XINT driver... */

        /* Initialize the XINT driver */
        if(ADI_XINT_SUCCESS != (xintResult = adi_xint_Init(xintMemory, ADI_XINT_MEMORY_SIZE)))
        {
            DEBUG_MESSAGE("adi_xint_Init failed\n");
            break;
        }

        /* Register the callback for XINT0 external interrupts  */
        if(ADI_XINT_SUCCESS != (xintResult = adi_xint_RegisterCallback (ADI_XINT_EVENT_INT0, xintCallback, NULL)))
        {
            DEBUG_MESSAGE("adi_xint_RegisterCallback failed\n");
            break;
        }

        /* Enable XINT0 for falling edge interrupt */
        if(ADI_XINT_SUCCESS != (xintResult = adi_xint_EnableIRQ (ADI_XINT_EVENT_INT0, ADI_XINT_IRQ_FALLING_EDGE)))
        {
            DEBUG_MESSAGE("adi_xint_EnableExIRQ failed\n");
            break;
        }

    } while(0);

    /* pre-flight checks... */
    if (ADI_GPIO_SUCCESS != gpioResult) {
        common_Fail("GPIO configuration failed.");
    }

	if (ADI_XINT_SUCCESS != xintResult) {
		common_Fail("XINT configuration failed.");
	}

	/* run the test... */

    loopcount = 0u;
    DEBUG_MESSAGE("Watch for periodic flash on green LED (DS3) through normal GPIO driver methods.\n");
    DEBUG_MESSAGE("Optionally, press \"S3\" (\"3029 BOOT\") pushbutton on EVAL-ADICUP3029 board to");
    DEBUG_MESSAGE("synthesize an XINT0 interrupt through XINT driver and observe red LED (DS4) toggle.\n");
    DEBUG_MESSAGE("Note: XINT interrupt capture requires physical loopback jumper across P7, pins IO13 and IO15.\n");

    /* wait for push button interrupts - exit the loop after a while */
    while (loopcount < MAXCOUNT) {
		/* toggle the green LED periodically to show life */
		if (0 == (loopcount % GREENTOGGLECOUNT)) {
			adi_gpio_Toggle(ledGreen.Port, ledGreen.Pins);
		}
        loopcount++;
    }


    /* check XINT interrupt flag */
    if (ADI_GPIO_SUCCESS != gpioXINT0) {
		DEBUG_MESSAGE("NO XINT INTERRUPTS DETECTED, check loopback jumper across P7 connector, pins IO13 and IO15");
		DEBUG_MESSAGE("... and/or press BOOT pushbutton.");
    } else {
		DEBUG_MESSAGE("XINT event detected.");
    }

    common_Pass();

    return 0;
}
