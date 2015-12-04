/*
===============================================================================
 Name        : avalon_led.c
 Author      : Mikeqin
 Version     : 0.1
 Copyright   : GPL
 Description : avalon led api
===============================================================================
*/
#include "avalon_api.h"

void AVALON_LED_Init(void)
{
	/* Set LED's GPIO High */
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, true);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 11, true);

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 8);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 9);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 11);
}

void AVALON_LED_Rgb(unsigned int rgb, bool on)
{
	/* LED Level reversal ,and LEDs mutex light */

	switch (rgb) {
	case AVALON_LED_RED:
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, (on ^ true));
		if (true == on) {
			Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, on);
			Chip_GPIO_SetPinState(LPC_GPIO, 0, 11, on);
		}
		break;
	case AVALON_LED_GREEN:
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, (on ^ true));
		if (true == on) {
			Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, on);
			Chip_GPIO_SetPinState(LPC_GPIO, 0, 11, on);
		}
		break;
	case AVALON_LED_BLUE:
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 11, (on ^ true));
		if (true == on) {
			Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, on);
			Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, on);
		}
		break;
	default:
		break;
	}
}

void AVALON_LED_Test(void)
{
    /* Initialize GPIO */
	Chip_GPIO_Init(LPC_GPIO);
	AVALON_LED_Init();

	/* open all led */
	AVALON_LED_Rgb(AVALON_LED_RED, true);
	AVALON_LED_Rgb(AVALON_LED_GREEN, true);
	AVALON_LED_Rgb(AVALON_LED_BLUE, true);
	AVALON_Delay(4000000);

	/* close all led */
	AVALON_LED_Rgb(AVALON_LED_RED, false);
	AVALON_LED_Rgb(AVALON_LED_GREEN, false);
	AVALON_LED_Rgb(AVALON_LED_BLUE, false);
	AVALON_Delay(4000000);

	/* red led test */
	AVALON_LED_Rgb(AVALON_LED_RED, true);
	AVALON_Delay(4000000);
	AVALON_LED_Rgb(AVALON_LED_RED, false);

	/* green led test */
	AVALON_LED_Rgb(AVALON_LED_GREEN, true);
	AVALON_Delay(4000000);
	AVALON_LED_Rgb(AVALON_LED_GREEN, false);

	/* blue led test */
	AVALON_LED_Rgb(AVALON_LED_BLUE, true);
	AVALON_Delay(4000000);
	AVALON_LED_Rgb(AVALON_LED_BLUE, false);

	AVALON_Delay(4000000);
}
