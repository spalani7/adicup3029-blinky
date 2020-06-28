/*********************************************************************************
   @file:    button_interrupt.h
   @brief:   Button interrupt example header file.
             This header file will have processor specific definitions.

 -------------------------------------------------------------------------------

Copyright(c) 2017 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

*********************************************************************************/

#ifndef __BUTTON_INTERRUPT_H__
#define __BUTTON_INTERRUPT_H__

/* used for exit timeout */
#define MAXCOUNT           (5000000u)
#define GREENTOGGLECOUNT   (50000U)

/* SysTick defines for 1ms interrupts from 26MHz clock */
#define SYSTEM_CORE_CLOCK  (26000000u)
#define SYSTICK_MS_DIVIDER (1000u)

/* SysTick interrupt priority macros */
#define LOWEST_PRIORITY ((1U << __NVIC_PRIO_BITS) -1)
#define SYSTICK_PRIORITY (LOWEST_PRIORITY - 1)

/* time (in ms) to wait for BootPin debounce */
/* this is a fairly long time, but it apparently is a fairly dirty switch */
#define DEBOUNCE_MS        (50u)

typedef struct {
	ADI_GPIO_PORT Port;
	ADI_GPIO_DATA Pins;
} PinMap;

#endif /* __BUTTON_INTERRUPT_H__ */
