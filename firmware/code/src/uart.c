/*
 * @brief UART Comm port call back routines
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
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
#include <string.h>
#include "board.h"
#include "app_usbd_cfg.h"
#include "uart.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Ring buffer size */
#define UART_RX_BUF_SZ		128
#define UART_TX_BUF_SZ		1024

/*
 * uart only use rx ringbuf, tx use send block
 * */
STATIC RINGBUFF_T uart_rxrb, uart_txrb;
static uint8_t uart_rxbuff[UART_RX_BUF_SZ], uart_txbuff[UART_TX_BUF_SZ];

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

static void Init_UART_PinMux(void)
{
#if (defined(BOARD_NXP_XPRESSO_11U14) || defined(BOARD_NGX_BLUEBOARD_11U24))
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_MODE_PULLUP);	/* PIO0_18 used for RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, IOCON_FUNC1 | IOCON_MODE_INACT);	/* PIO0_19 used for TXD */
#else
#error "No Pin muxing defined for UART operation"
#endif
}

/* UART port init routine */
void UART_Init(void)
{
	/* Board specific muxing */
	Init_UART_PinMux();

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaudFDR(LPC_USART, 111111);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	RingBuffer_Init(&uart_rxrb, uart_rxbuff, 1, UART_RX_BUF_SZ);
	RingBuffer_Init(&uart_txrb, uart_txbuff, 1, UART_TX_BUF_SZ);

	/* Enable receive data and line status interrupt */
	Chip_UART_IntEnable(LPC_USART, (UART_IER_RBRINT | UART_IER_RLSINT));

	/* Enable Interrupt for UART channel */
	/* Priority = 1 */
	NVIC_SetPriority(UART0_IRQn, 1);
	/* Enable Interrupt for UART channel */
	NVIC_EnableIRQ(UART0_IRQn);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	UART interrupt handler sub-routine
 * @return	Nothing
 */
void UART_IRQHandler(void)
{
	Chip_UART_IRQRBHandler(LPC_USART, &uart_rxrb, &uart_txrb);
}

/* Gets current read count. */
uint32_t UART_Read_Cnt(void)
{
	return RingBuffer_GetCount(&uart_rxrb);
}


/* Read data from uart */
uint32_t UART_Read(uint8_t *pBuf, uint32_t buf_len)
{
	uint16_t cnt = 0;

	if(pBuf)
	{
		cnt = Chip_UART_ReadRB(LPC_USART, &uart_rxrb, pBuf, buf_len);
	}

	return cnt;
}

/* Send data to uart */
uint32_t UART_Write(uint8_t *pBuf, uint32_t len)
{
	uint32_t ret = 0;

	if(pBuf)
	{
		ret = Chip_UART_SendRB(LPC_USART, &uart_txrb, pBuf, len);
	}

	return ret;
}

/* clear UART tx ringbuffer */
void UART_FlushTxRB(void)
{
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2 | UART_FCR_TX_RS));
	RingBuffer_Flush(&uart_txrb);
}

/* clear UART rx ringbuffer */
void UART_FlushRxRB(void)
{
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2 | UART_FCR_RX_RS));
	RingBuffer_Flush(&uart_rxrb);
}
