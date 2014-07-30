/*
 * @brief LPC11u1x I2C master driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
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

#include "chip.h"
#include "i2c_lpc11uxx.h"
#include "avalon_api.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Control flags */
#define I2C_CON_FLAGS (I2C_CON_AA | I2C_CON_SI | I2C_CON_STO | I2C_CON_STA)

#undef DEBUGOUT
#define DEBUGOUT(...) {		\
		char str[255];						\
		memset(str, 0, 255);				\
		m_sprintf(str, __VA_ARGS__);		\
		UART_Write((uint8_t*)str,strlen(str));	\
	}

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Initializes the LPC_I2C peripheral with specified parameter */
void Chip_I2CM_Init(LPC_I2C_T *pI2C)
{
	Chip_I2C_Init(I2C0);
}

/* De-initializes the I2C peripheral registers to their default reset values */
void Chip_I2CM_DeInit(LPC_I2C_T *pI2C)
{
	Chip_I2C_DeInit(I2C0);
}

/* Set up bus speed for LPC_I2C interface */
void Chip_I2CM_SetBusSpeed(LPC_I2C_T *pI2C, uint32_t busSpeed)
{
	uint32_t clockDiv = (Chip_Clock_GetMainClockRate() / busSpeed);

	//Chip_I2CM_SetDutyCycle(pI2C, 12, 12);
	Chip_I2CM_SetDutyCycle(pI2C, (clockDiv >> 1), (clockDiv - (clockDiv >> 1)));
}

/* Master transfer state change handler handler */
uint32_t Chip_I2CM_XferHandler(LPC_I2C_T *pI2C, I2CM_XFER_T *xfer)
{
	uint32_t cclr = I2C_CON_FLAGS;

	switch (Chip_I2CM_GetCurState(pI2C)) {
	case 0x08:		/* Start condition on bus */
	case 0x10:		/* Repeated start condition */
		pI2C->DAT = (xfer->slaveAddr << 1) | (xfer->txSz == 0);
		break;

	/* Tx handling */
	case 0x20:		/* SLA+W sent NAK received */
	case 0x30:		/* DATA sent NAK received */
		if ((xfer->options & I2CM_XFER_OPTION_IGNORE_NACK) == 0) {
			xfer->status = I2CM_STATUS_NAK;
			cclr &= ~I2C_CON_STO;
			break;
		}

	case 0x18:		/* SLA+W sent and ACK received */
	case 0x28:		/* DATA sent and ACK received */
		if (!xfer->txSz) {
			if (xfer->rxSz) {
				cclr &= ~I2C_CON_STA;
			}
			else {
				xfer->status = I2CM_STATUS_OK;
				cclr &= ~I2C_CON_STO;
			}

		}
		else {
			pI2C->DAT = *xfer->txBuff++;
			xfer->txSz--;
		}
		break;

	/* Rx handling */
	case 0x58:		/* Data Received and NACK sent */
	case 0x50:		/* Data Received and ACK sent */
		*xfer->rxBuff++ = pI2C->DAT;
		xfer->rxSz--;

	case 0x40:		/* SLA+R sent and ACK received */
		if ((xfer->rxSz > 1) || (xfer->options & I2CM_XFER_OPTION_LAST_RX_ACK)) {
			cclr &= ~I2C_CON_AA;
		}
		if (xfer->rxSz == 0) {
			xfer->status = I2CM_STATUS_OK;
			cclr &= ~I2C_CON_STO;
		}
		break;

	/* NAK Handling */
	case 0x48:		/* SLA+R sent NAK received */
		xfer->status = I2CM_STATUS_SLAVE_NAK;
		cclr &= ~I2C_CON_STO;
		break;

	case 0x38:		/* Arbitration lost */
		xfer->status = I2CM_STATUS_ARBLOST;
		break;

	case 0x00:		/* Bus Error */
		xfer->status = I2CM_STATUS_BUS_ERROR;
		cclr &= ~I2C_CON_STO;
		break;

	default:
		xfer->status = I2CM_STATUS_ERROR;
		cclr &= ~I2C_CON_STO;
		break;
	}

	/* Set clear control flags */
	pI2C->CONSET = cclr ^ I2C_CON_FLAGS;
	pI2C->CONCLR = cclr;

	return xfer->status != I2CM_STATUS_BUSY;
}

/* Transmit and Receive data in master mode */
void Chip_I2CM_Xfer(LPC_I2C_T *pI2C, I2CM_XFER_T *xfer)
{
	/* set the transfer status as busy */
	xfer->status = I2CM_STATUS_BUSY;
	/* Clear controller state. */
	Chip_I2CM_ResetControl(pI2C);
	/* Enter to Master Transmitter mode */
	Chip_I2CM_SendStart(pI2C);
}

/* Transmit and Receive data in master mode */
uint32_t Chip_I2CM_XferBlocking(LPC_I2C_T *pI2C, I2CM_XFER_T *xfer)
{
	uint32_t ret = 0;
	/* start transfer */
	Chip_I2CM_Xfer(pI2C, xfer);

	while (ret == 0) {
		/* wait for status change interrupt */
		while ( Chip_I2CM_StateChanged(pI2C) == 0) {}
		/* call state change handler */
		ret = Chip_I2CM_XferHandler(pI2C, xfer);
	}
	return ret;
}

/* Master tx only */
uint32_t Chip_I2CM_Write(LPC_I2C_T *pI2C, const uint8_t *buff, uint32_t len)
{
	uint32_t txLen = 0, err = 0;

	/* clear state change interrupt status */
	Chip_I2CM_ClearSI(pI2C);
	/* generate START condition */
	Chip_I2CM_SendStart(pI2C);

	while ((txLen < len) && (err == 0)) {
		/* wait for status change interrupt */
		while ( Chip_I2CM_StateChanged(pI2C) == 0) {}

		/* check status and send data */
		switch (Chip_I2CM_GetCurState(pI2C)) {
		case 0x08:		/* Start condition on bus */
		case 0x10:		/* Repeated start condition */
		case 0x18:		/* SLA+W sent and ACK received */
		case 0x28:		/* DATA sent and ACK received */
			Chip_I2CM_WriteByte(pI2C, buff[txLen++]);
			break;

		case 0x38:		/* Arbitration lost */
			break;

		default:		/* we shouldn't be in any other state */
			err = 1;
			break;
		}
		/* clear state change interrupt status */
		Chip_I2CM_ClearSI(pI2C);
	}

	return txLen;
}

/* Sequential master read */
uint32_t Chip_I2CM_Read(LPC_I2C_T *pI2C, uint8_t *buff, uint32_t len)
{
	uint32_t rxLen = 0, err = 0;

	/* clear state change interrupt status */
	Chip_I2CM_ClearSI(pI2C);
	/* generate START condition and auto-ack data received */
	pI2C->CONSET = I2C_CON_AA | I2C_CON_STA;

	while ((rxLen < len) && (err == 0)) {
		/* wait for status change interrupt */
		while ( Chip_I2CM_StateChanged(pI2C) == 0) {}

		/* check status and send data */
		switch (Chip_I2CM_GetCurState(pI2C)) {
		case 0x08:		/* Start condition on bus */
		case 0x10:		/* Repeated start condition */
		case 0x40:		/* SLA+R sent and ACK received */
		case 0x50:		/* Data Received and ACK sent */
			buff[rxLen++] = Chip_I2CM_ReadByte(pI2C);
			break;

		case 0x38:		/* Arbitration lost */
			break;

		default:		/* we shouldn't be in any other state */
			err = 1;
			break;
		}
		/* clear state change interrupt status */
		Chip_I2CM_ClearSI(pI2C);
	}

	return rxLen;
}

static unsigned int timecnt = 0;
static void AVALON_TMRID1_Fun(void)
{
	timecnt++;
}

void Chip_I2CM_Test(void)
{
#define I2C_DEVADDR			0
#define I2C_DATSIZE			40

	char	rxdat[I2C_DATSIZE], txdat[I2C_DATSIZE];
	int	rxsize, verifyok, i, txsize, ret;
	I2CM_XFER_T xfer;

	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4, IOCON_FUNC1 | IOCON_FASTI2C_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5, IOCON_FUNC1 | IOCON_FASTI2C_EN);

	Chip_I2CM_Init(LPC_I2C);
	Chip_I2CM_SetBusSpeed(LPC_I2C, 1000000);
	AVALON_TMR_Init();

	/* ACK */
	DEBUGOUT("ACK Test:\r\n");

	memset(&xfer, 0, sizeof(I2CM_XFER_T));
	xfer.slaveAddr = I2C_DEVADDR;
	xfer.txBuff = NULL;
	xfer.rxBuff = rxdat;
	xfer.rxSz = I2C_DATSIZE;
	xfer.txSz = 0;
	xfer.options = 0;
	ret = Chip_I2CM_XferBlocking(LPC_I2C, &xfer);
	if (ret) {
		rxsize = I2C_DATSIZE - xfer.rxSz;
		if (rxsize) {
			DEBUGOUT("ACK Read(%d):", rxsize);
			for (i = 0; i < rxsize; i++) {
				DEBUGOUT("%x", rxdat[i]);
				if (((i + 1) % 4) == 0)
					DEBUGOUT(" ");
			}
			DEBUGOUT("\r\n");
		} else
			DEBUGOUT("ACK Read: None(origin)\r\n");
	} else
		DEBUGOUT("ACK Read: Err(origin)\r\n");

	for (i = 0; i < I2C_DATSIZE; i++)
			txdat[i] = i;

	memset(&xfer, 0, sizeof(I2CM_XFER_T));
	xfer.slaveAddr = I2C_DEVADDR;
	xfer.txBuff = txdat;
	xfer.rxBuff = NULL;
	xfer.rxSz = 0;
	xfer.txSz = I2C_DATSIZE;
	xfer.options = 0;
	ret = Chip_I2CM_XferBlocking(LPC_I2C, &xfer);
	if (ret) {
		txsize = I2C_DATSIZE - xfer.txSz;
		DEBUGOUT("ACK Write: Finish (%d)\r\n", txsize);
	} else
		DEBUGOUT("ACK Write: Failed \r\n");

	verifyok = 0;
	memset(rxdat, 0, I2C_DATSIZE);
	memset(&xfer, 0, sizeof(I2CM_XFER_T));
	xfer.slaveAddr = I2C_DEVADDR;
	xfer.txBuff = NULL;
	xfer.rxBuff = rxdat;
	xfer.rxSz = I2C_DATSIZE;
	xfer.txSz = 0;
	xfer.options = 0;
	ret = Chip_I2CM_XferBlocking(LPC_I2C, &xfer);
	if (ret) {
		rxsize = I2C_DATSIZE - xfer.rxSz;
		DEBUGOUT("ACK Read(%d): Finish\r\n", rxsize);
		for (i = 0; i < rxsize; i++) {
			if (rxdat[i] != txdat[i]) {
				break;
			}
		}
		if (i == I2C_DATSIZE)
			verifyok = 1;
	} else
		DEBUGOUT("ACK Read: None(new)\r\n");

	if (verifyok) {
		DEBUGOUT("ACK Verify: W/R Success!\r\n");
	} else {
		DEBUGOUT("ACK Verify: W/R Failed!\r\n");
	}

	/* bps test */
	for (i = 0; i < I2C_DATSIZE; i++)
				txdat[i] = i;

	timecnt = 0;
	AVALON_TMR_Set(AVALON_TMR_ID1, 1, AVALON_TMRID1_Fun);
	memset(&xfer, 0, sizeof(I2CM_XFER_T));
	xfer.slaveAddr = 0;
	xfer.txBuff = txdat;
	xfer.rxBuff = NULL;
	xfer.rxSz = 0;
	xfer.txSz = I2C_DATSIZE;
	xfer.options = 0;
	ret = Chip_I2CM_XferBlocking(LPC_I2C, &xfer);
	AVALON_TMR_Kill(AVALON_TMR_ID1);
	DEBUGOUT("W rate : %d bps\r\n", 40 * 8 * TICKRATE_AVALON / timecnt);

	timecnt = 0;
	AVALON_TMR_Set(AVALON_TMR_ID1, 1, AVALON_TMRID1_Fun);
	memset(&xfer, 0, sizeof(I2CM_XFER_T));
	xfer.slaveAddr = I2C_DEVADDR;
	xfer.txBuff = NULL;
	xfer.rxBuff = rxdat;
	xfer.rxSz = I2C_DATSIZE;
	xfer.txSz = 0;
	xfer.options = 0;
	ret = Chip_I2CM_XferBlocking(LPC_I2C, &xfer);
	AVALON_TMR_Kill(AVALON_TMR_ID1);
	DEBUGOUT("R rate : %d bps\r\n", 40 * 8 * TICKRATE_AVALON / timecnt);
}
