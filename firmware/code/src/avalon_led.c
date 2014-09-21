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
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 8);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 9);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 11);
}

void AVALON_LED_Rgb(unsigned int rgb)
{
	switch (rgb) {
	case AVALON_LED_RED:
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, true);
		break;
	case AVALON_LED_GREEN:
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		break;
	case AVALON_LED_BLUE:
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 11, true);
		break;
	case AVALON_LED_WHITE:
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 11, true);
		break;
	case AVALON_LED_BLACK:
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 8, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 11, false);
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
	AVALON_LED_Rgb(AVALON_LED_WHITE);
	AVALON_Delay(4000000);

	/* close all led */
	AVALON_LED_Rgb(AVALON_LED_BLACK);
	AVALON_Delay(4000000);

	/* open separate led and close it, r->g->b */
	AVALON_LED_Rgb(AVALON_LED_RED);
	AVALON_Delay(4000000);
	AVALON_LED_Rgb(AVALON_LED_BLACK);

	AVALON_LED_Rgb(AVALON_LED_GREEN);
	AVALON_Delay(4000000);
	AVALON_LED_Rgb(AVALON_LED_BLACK);

	AVALON_LED_Rgb(AVALON_LED_BLUE);
	AVALON_Delay(4000000);
	AVALON_LED_Rgb(AVALON_LED_BLACK);

	/* all led has been closed */
	AVALON_Delay(4000000);
}
