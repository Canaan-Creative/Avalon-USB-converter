/*
 * avalon_etc.c
 *
 *  Created on: Nov 3, 2014
 *      Author: xiangfu
 */

#include <stdio.h>
#include "board.h"

void AVALON_Delay(volatile unsigned int ticks)
{
	while (ticks--)
		__NOP();
}
