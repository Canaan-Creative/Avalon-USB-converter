/*
===============================================================================
 Name        : avalon_adc.c
 Author      : Mikeqin
 Version     : 0.1
 Copyright   : GPL
 Description : avalon adc api
===============================================================================
*/
#include <math.h>
#include "avalon_api.h"

void AVALON_ADC_Init(void)
{
	ADC_CLOCK_SETUP_T ADCSetup;
	/*
	 * PIO0_23 AD7 Thermistor
	 * */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, FUNC1);
	Chip_ADC_Init(LPC_ADC, &ADCSetup);
}

/* see https://learn.adafruit.com/thermistor/using-a-thermistor */
uint16_t AVALON_Temp_Rd(void)
{
	uint16_t temp;

	Chip_ADC_EnableChannel(LPC_ADC, ADC_CH7, ENABLE);
	/* Start A/D conversion */
	Chip_ADC_SetStartMode(LPC_ADC, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
	/* Waiting for A/D conversion complete */
	while (Chip_ADC_ReadStatus(LPC_ADC, ADC_CH7, ADC_DR_DONE_STAT) != SET);
	/* Read ADC value */
	Chip_ADC_ReadValue(LPC_ADC, ADC_CH7, &temp);
	Chip_ADC_EnableChannel(LPC_ADC, ADC_CH7, DISABLE);

	return temp;
}
