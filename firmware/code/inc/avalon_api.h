/*
 * @brief Programming API used with avalon
 *
 * @note
 * Copyright(C) 0xf8, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */
#ifndef __AVALON_API_H_
#define __AVALON_API_H_

#include <stdio.h>
#include "board.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AVALON_LED_RED		0
#define AVALON_LED_GREEN 	1
#define AVALON_LED_BLUE		2
#define AVALON_LED_BLACK	3
#define AVALON_LED_WHITE	4

extern void AVALON_LED_Init(void);
extern void AVALON_LED_Rgb(unsigned int rgb);
extern void AVALON_LED_Test(void);

void AVALON_ADC_Init(void);
extern uint16_t AVALON_Temp_Rd(void);

static void AVALON_Delay(unsigned int max)
{
	volatile unsigned int i;
	for(i = 0; i < max; i++);
}

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __AVALON_API_H_ */
