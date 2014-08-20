/*
 * @brief HID generic example's callabck routines
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

#include "board.h"
#include <stdint.h>
#include <string.h>
#include "usbd_rom_api.h"
#include "cdc_i2c.h"
#include "i2c_lpc11uxx.h"
#include "lpcusbsio_i2c.h"
#include "avalon_api.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define CDC_I2C_STATE_DISCON        0
#define CDC_I2C_STATE_CONNECTED     1

/**
 * Structure containing HID_I2C control data
 */
typedef struct __HID_I2C_CTRL_T {
	USBD_HANDLE_T hUsb;		/*!< Handle to USB stack */
	USBD_HANDLE_T hCdc;		/*!< Handle to USB stack */
	LPC_I2C_T *pI2C;		/*!< I2C port this bridge is associated. */
	uint8_t reqWrIndx;		/*!< Write index for request queue. */
	uint8_t reqRdIndx;		/*!< Read index for request queue. */
	uint8_t respWrIndx;		/*!< Write index for response queue. */
	uint8_t respRdIndx;		/*!< Read index for response queue. */
	uint8_t reqPending;		/*!< Flag indicating request is pending in EP RAM */
	uint8_t state;			/*!< State of the controller */
	uint8_t resetReq;		/*!< Flag indicating if reset is requested by host */
	uint8_t epin_adr;		/*!< Interrupt IN endpoint associated with this HID instance. */
	uint8_t epout_adr;		/*!< Interrupt OUT endpoint associated with this HID instance. */
	uint16_t pad0;
	volatile uint16_t tx_flags;
	uint8_t reqQ[CDC_I2C_MAX_PACKETS][CDC_I2C_PACKET_SZ];		/*!< Requests queue */
	uint8_t respQ[CDC_I2C_MAX_PACKETS][CDC_I2C_PACKET_SZ];	/*!< Response queue */
} CDC_I2C_CTRL_T;

static const char *g_fwVersion = "FW v1.00 (" __DATE__ " " __TIME__ ")";

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

static ErrorCode_t CDC_I2C_Bind(CDC_I2C_CTRL_T *pCDCI2c, LPC_I2C_T *pI2C)
{
	ErrorCode_t ret = ERR_API_INVALID_PARAMS;

#if defined(BOARD_MANLEY_11U68) || defined(BOARD_NXP_LPCXPRESSO_11U68)
	if (pI2C == LPC_I2C0) {
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4, IOCON_FUNC1 | IOCON_FASTI2C_EN);
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5, IOCON_FUNC1 | IOCON_FASTI2C_EN);
		/* bind to I2C port */
		pHidI2c->pI2C = pI2C;
		NVIC_DisableIRQ(I2C0_IRQn);
		ret = LPC_OK;
	}

#elif (defined(BOARD_NXP_XPRESSO_11U14) || defined(BOARD_NGX_BLUEBOARD_11U24))
	Chip_SYSCTL_PeriphReset(RESET_I2C0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 4, IOCON_FUNC1 | IOCON_FASTI2C_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 5, IOCON_FUNC1 | IOCON_FASTI2C_EN);
	/* bind to I2C port */
	pCDCI2c->pI2C = pI2C;
	NVIC_DisableIRQ(I2C0_IRQn);
	ret = LPC_OK;
#else
#warning "No I2C Pin Muxing defined for this example"
#endif
	return ret;
}

static INLINE void CDC_I2C_IncIndex(uint8_t *index)
{
	*index = (*index + 1) % CDC_I2C_MAX_PACKETS;
}

/* CDC_I2C Interrupt endpoint event handler. */
static ErrorCode_t CDC_I2C_EpIn_Hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
	CDC_I2C_CTRL_T *pCDCI2c = (CDC_I2C_CTRL_T *) data;

	if (event == USB_EVT_IN) {
		DEBUGOUT("CDC_I2C_EpIn_Hdlr\r\n");
		pCDCI2c->tx_flags &= ~CDC_I2C_TX_BUSY;
	}
	return LPC_OK;
}

/* CDC_I2C Interrupt endpoint event handler. */
static ErrorCode_t CDC_I2C_EpOut_Hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
	CDC_I2C_CTRL_T *pCDCI2c = (CDC_I2C_CTRL_T *) data;
	CDC_I2C_OUT_REPORT_T *pOut;

	if (event == USB_EVT_OUT) {
		/* Read the new request received. */
		USBD_API->hw->ReadEP(hUsb, pCDCI2c->epout_adr, &pCDCI2c->reqQ[pCDCI2c->reqWrIndx][0]);
		pOut = (CDC_I2C_OUT_REPORT_T *) &pCDCI2c->reqQ[pCDCI2c->reqWrIndx][0];

		/* handle CDC_I2C_REQ_FLUSH request in IRQ context to abort current
		   transaction and reset the queue states. */
		if (pOut->req == CDC_I2C_REQ_RESET) {
			pCDCI2c->resetReq = 1;
		}

		/* normal request queue the buffer */
		CDC_I2C_IncIndex(&pCDCI2c->reqWrIndx);
	}
	return LPC_OK;
}

static uint32_t CDC_I2C_StatusCheckLoop(CDC_I2C_CTRL_T *pCDCI2c)
{
	uint8_t try = 0;

	/* wait for status change interrupt */
	while ( (Chip_I2CM_StateChanged(pCDCI2c->pI2C) == 0) &&
			(pCDCI2c->resetReq == 0) && (try++ < 200)) {
		/* loop */
	}
	return pCDCI2c->resetReq;
}

static int32_t CDC_I2C_HandleReadStates(CDC_I2C_CTRL_T *pHidI2c,
										CDC_I2C_RW_PARAMS_T *pRWParam,
										CDC_I2C_IN_REPORT_T *pIn,
										uint32_t status)
{
	int32_t ret = 0;
	uint8_t *buff = &pIn->length;
	uint32_t pos;

	switch (status) {
	case 0x58:			/* Data Received and NACK sent */
		buff[pIn->length++] = Chip_I2CM_ReadByte(pHidI2c->pI2C);
		pIn->resp = CDC_I2C_RES_OK;
		ret = 1;
		break;

	case 0x40:			/* SLA+R sent and ACK received */
		pHidI2c->pI2C->CONSET = I2C_CON_AA;

	case 0x50:			/* Data Received and ACK sent */
		if (status == 0x50) {
			buff[pIn->length++] = Chip_I2CM_ReadByte(pHidI2c->pI2C);
		}
		pos = pIn->length - CDC_I2C_HEADER_SZ;
		if (pRWParam->options & CDC_I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE) {
			if ((pRWParam->length - pos) == 1) {
				Chip_I2CM_NackNextByte(pHidI2c->pI2C);
			}
		}
		else if (pRWParam->length == pos) {
			pIn->resp = CDC_I2C_RES_OK;
		}
		ret = 1;
		break;
	}

	return ret;
}

static int32_t CDC_I2C_HandleWriteStates(CDC_I2C_CTRL_T *pCDCI2c,
										 CDC_I2C_RW_PARAMS_T *pRWParam,
										 CDC_I2C_IN_REPORT_T *pIn,
										 uint32_t status)
{
	int32_t ret = 0;
	uint8_t *buff = &pRWParam->data[0];
	uint32_t pos;

	switch (status) {
	case 0x30:			/* DATA sent NAK received */
		if (pRWParam->options & CDC_I2C_TRANSFER_OPTIONS_BREAK_ON_NACK) {
			pIn->resp = CDC_I2C_RES_NAK;
			ret = 1;
			break;
		}		/* else fall through */

	case 0x08:			/* Start condition on bus */
	case 0x10:			/* Repeated start condition */
		if ((pRWParam->options & CDC_I2C_TRANSFER_OPTIONS_NO_ADDRESS) == 0) {
			break;
		}		/* else fall-through */

	case 0x18:			/* SLA+W sent and ACK received */
	case 0x28:			/* DATA sent and ACK received */
		pos = pIn->length - CDC_I2C_HEADER_SZ;
		Chip_I2CM_WriteByte(pCDCI2c->pI2C, buff[pos]);
		pIn->length++;
		if ((pos) == pRWParam->length) {
			pIn->resp = CDC_I2C_RES_OK;
		}
		ret = 1;
		break;
	}

	return ret;
}

static void CDC_I2C_HandleRWReq(CDC_I2C_CTRL_T *pCDCI2c,
								CDC_I2C_OUT_REPORT_T *pOut,
								CDC_I2C_IN_REPORT_T *pIn,
								int32_t read)
{
	CDC_I2C_RW_PARAMS_T *pRWParam = (CDC_I2C_RW_PARAMS_T *) &pOut->data[0];
	int32_t handled = 0;
	uint32_t status;

	/* clear state change interrupt status */
	Chip_I2CM_ClearSI(pCDCI2c->pI2C);

	if (pRWParam->options & CDC_I2C_TRANSFER_OPTIONS_START_BIT) {
		Chip_I2CM_SendStart(pCDCI2c->pI2C);
	}

	while (pIn->resp == CDC_I2C_RES_INVALID_CMD) {
		/* wait for status change interrupt */
			if (CDC_I2C_StatusCheckLoop(pCDCI2c) == 0) {

			status = Chip_I2CM_GetCurState(pCDCI2c->pI2C);
			/* handle Read or write states. */
			if (read ) {
				handled = CDC_I2C_HandleReadStates(pCDCI2c, pRWParam, pIn, status);
			}
			else {
				handled = CDC_I2C_HandleWriteStates(pCDCI2c, pRWParam, pIn, status);
			}

			if (handled == 0) {
				/* check status and send data */
				switch (status) {
				case 0x08:	/* Start condition on bus */
				case 0x10:	/* Repeated start condition */
					if ((pRWParam->options & CDC_I2C_TRANSFER_OPTIONS_NO_ADDRESS) == 0) {
						Chip_I2CM_WriteByte(pCDCI2c->pI2C, (pRWParam->slaveAddr << 1) | read);
					}
					break;

				case 0x20:	/* SLA+W sent NAK received */
				case 0x48:	/* SLA+R sent NAK received */
					pIn->resp = CDC_I2C_RES_SLAVE_NAK;
					break;

				case 0x38:	/* Arbitration lost */
					pIn->resp = CDC_I2C_RES_ARBLOST;
					break;

				case 0x00:	/* Bus Error */
					pIn->resp = CDC_I2C_RES_BUS_ERROR;
					break;

				default:	/* we shouldn't be in any other state */
					pIn->resp = CDC_I2C_RES_ERROR;
					break;
				}
			}
			/* clear state change interrupt status */
			Chip_I2CM_ClearSI(pCDCI2c->pI2C);
		}
		else {
			pIn->resp = CDC_I2C_RES_ERROR;
			break;
		}
	}

	if ((pIn->resp != CDC_I2C_RES_ARBLOST) &&
		(pRWParam->options & CDC_I2C_TRANSFER_OPTIONS_STOP_BIT) ) {
		Chip_I2CM_SendStop(pCDCI2c->pI2C);
	}
}

static void CDC_I2C_HandleXferReq(CDC_I2C_CTRL_T *pCDCI2c,
								  CDC_I2C_OUT_REPORT_T *pOut,
								  CDC_I2C_IN_REPORT_T *pIn)
{
	CDC_I2C_XFER_PARAMS_T *pXfrParam = (CDC_I2C_XFER_PARAMS_T *) &pOut->data[0];
	I2CM_XFER_T xfer;
	uint32_t ret = 0;

	memset(&xfer, 0, sizeof(I2CM_XFER_T));
	xfer.slaveAddr = pXfrParam->slaveAddr;
	xfer.txBuff = &pXfrParam->data[0];
	xfer.rxBuff = &pIn->data[0];
	xfer.rxSz = pXfrParam->rxLength;
	xfer.txSz = pXfrParam->txLength;
	xfer.options = pXfrParam->options;
	DEBUGOUT("xfer.slaveAddr = %d, rxSz = %d, txSz = %d\r\n", xfer.slaveAddr, xfer.rxSz, xfer.txSz);
	/* start transfer */
	Chip_I2CM_Xfer(pCDCI2c->pI2C, &xfer);

	while (ret == 0) {
		/* wait for status change interrupt */
		if (CDC_I2C_StatusCheckLoop(pCDCI2c) == 0) {
			/* call state change handler */
			ret = Chip_I2CM_XferHandler(pCDCI2c->pI2C, &xfer);
		}
		else {
			xfer.status = CDC_I2C_RES_ERROR;
			break;
		}
	}

	/* Update the length we have to send back */
	if ((pXfrParam->rxLength - xfer.rxSz) > 0) {
		pIn->length += pXfrParam->rxLength - xfer.rxSz;
		DEBUGOUT("Write = %d\r\n", pIn->length - 4);
	}

	/* update response with the I2CM status returned. No translation
	   needed as they map directly to base LPCUSBSIO status. */
	pIn->resp = xfer.status;
}

/* Set line coding call back routine */
static ErrorCode_t CDC_SetLineCode(USBD_HANDLE_T hCDC, CDC_LINE_CODING *line_coding)
{
	return LPC_OK;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* CDC init routine */
ErrorCode_t CDC_I2C_init(USBD_HANDLE_T hUsb,
						 USB_CORE_DESCS_T *pDesc,
						 USBD_API_INIT_PARAM_T *pUsbParam,
						 LPC_I2C_T *pI2C,
						 USBD_HANDLE_T *pI2CCDC)
{
	USBD_CDC_INIT_PARAM_T cdc_param;
	uint32_t ep_indx;
	CDC_I2C_CTRL_T *pCDCI2c;
	ErrorCode_t ret = LPC_OK;
	USBD_HANDLE_T hCdc;
	USB_CDC_CTRL_T *pCDC;

	/* CDC paramas */
	memset((void *) &cdc_param, 0, sizeof(USBD_CDC_INIT_PARAM_T));
	cdc_param.mem_base = pUsbParam->mem_base;
	cdc_param.mem_size = pUsbParam->mem_size;
	cdc_param.cif_intf_desc = (uint8_t *) find_IntfDesc(pDesc->high_speed_desc, CDC_COMMUNICATION_INTERFACE_CLASS);
	cdc_param.dif_intf_desc = (uint8_t *) find_IntfDesc(pDesc->high_speed_desc, CDC_DATA_INTERFACE_CLASS);
	cdc_param.SetLineCode = CDC_SetLineCode;

	ret = USBD_API->cdc->init(hUsb, &cdc_param, &hCdc);
	if (ret == LPC_OK) {
		/* allocate memory for HID_I2C_CTRL_T instance */
		pCDCI2c = (CDC_I2C_CTRL_T *) cdc_param.mem_base;
		cdc_param.mem_base += sizeof(CDC_I2C_CTRL_T);
		cdc_param.mem_size -= sizeof(CDC_I2C_CTRL_T);
		/* update memory variables */
		pUsbParam->mem_base = cdc_param.mem_base;
		pUsbParam->mem_size = cdc_param.mem_size;

		/* Init the control structure */
		memset(pCDCI2c, 0, sizeof(CDC_I2C_CTRL_T));
		/* store stack handle*/
		pCDCI2c->hUsb = hUsb;
		pCDCI2c->hCdc = hCdc;
		/* set return handle */
		*pI2CCDC = (USBD_HANDLE_T) pCDCI2c;

		/* bind to I2C port */
		ret = CDC_I2C_Bind(pCDCI2c, pI2C);

		/* register endpoint interrupt handler */
		pCDCI2c->epin_adr = USB_CDC_IN_EP;
		ep_indx = (((USB_CDC_IN_EP & 0x0F) << 1) + 1);
		ret = USBD_API->core->RegisterEpHandler(hUsb, ep_indx, CDC_I2C_EpIn_Hdlr, pCDCI2c);
		if (ret == LPC_OK) {
			/* register endpoint interrupt handler */
			pCDCI2c->epout_adr = USB_CDC_OUT_EP;
			ep_indx = ((USB_CDC_OUT_EP & 0x0F) << 1);
			ret = USBD_API->core->RegisterEpHandler(hUsb, ep_indx, CDC_I2C_EpOut_Hdlr, pCDCI2c);
			pCDC = (USB_CDC_CTRL_T *)(pCDCI2c->hCdc);
			pCDC->line_coding.dwDTERate = 115200;
			pCDC->line_coding.bDataBits = 8;
		}

		/* update mem_base and size variables for cascading calls. */
		pUsbParam->mem_base = cdc_param.mem_base;
		pUsbParam->mem_size = cdc_param.mem_size;
	}

	return ret;
}

static unsigned int timecnt = 0;
static void AVALON_TMRID1_Fun(void)
{
	timecnt++;
}

/* Process CDC_I2C request and response queue. */
void CDC_I2C_process(USBD_HANDLE_T hI2CCDC)
{
	CDC_I2C_CTRL_T *pCDCI2c = (CDC_I2C_CTRL_T *) hI2CCDC;
	CDC_I2C_OUT_REPORT_T *pOut;
	CDC_I2C_IN_REPORT_T *pIn;
	CDC_I2C_PORTCONFIG_T *pCfg;
	uint16_t len;

	if (USB_IsConfigured(pCDCI2c->hUsb)) {
		/* set state to connected */
		pCDCI2c->state = CDC_I2C_STATE_CONNECTED;
		if (pCDCI2c->reqWrIndx != pCDCI2c->reqRdIndx) {
			DEBUGOUT("CDC_I2C_process : req %d,%d\r\n", pCDCI2c->reqWrIndx, pCDCI2c->reqRdIndx);
			/* process the current packet */
			pOut = (CDC_I2C_OUT_REPORT_T *) &pCDCI2c->reqQ[pCDCI2c->reqRdIndx][0];
			pIn = (CDC_I2C_IN_REPORT_T *) &pCDCI2c->respQ[pCDCI2c->respWrIndx][0];
			/* construct response template */
			pIn->length = CDC_I2C_HEADER_SZ;
			pIn->transId = pOut->transId;
			pIn->sesId = pOut->sesId;
			pIn->resp = CDC_I2C_RES_INVALID_CMD;

			DEBUGOUT("pOut->req = %d\r\n", pOut->req);
			switch (pOut->req) {
			case CDC_I2C_REQ_INIT_PORT:
				/* Init I2C port */
				Chip_I2CM_Init(pCDCI2c->pI2C);
				pCfg = (CDC_I2C_PORTCONFIG_T *) &pOut->data[0];
				Chip_I2CM_SetBusSpeed(pCDCI2c->pI2C, pCfg->busSpeed);

				/* TBD. Change I2C0 pads modes per bus speed requested. Do we need to?*/
				/* send back firmware version string */
				memcpy(&pIn->data[0], g_fwVersion, strlen(g_fwVersion));
				pIn->length += strlen(g_fwVersion);

				/* update response */
				pIn->resp = CDC_I2C_RES_OK;
				break;

			case CDC_I2C_REQ_DEINIT_PORT:
				Chip_I2CM_DeInit(pCDCI2c->pI2C);
				/* update response */
				pIn->resp = CDC_I2C_RES_OK;
				break;

			case CDC_I2C_REQ_DEVICE_WRITE:
			case CDC_I2C_REQ_DEVICE_READ:
				timecnt = 0;
				AVALON_TMR_Set(AVALON_TMR_ID1, 1, AVALON_TMRID1_Fun);
				CDC_I2C_HandleRWReq(pCDCI2c, pOut, pIn, (pOut->req == CDC_I2C_REQ_DEVICE_READ));
				AVALON_TMR_Kill(AVALON_TMR_ID1);
				DEBUGOUT("W/R rate : %d bps\r\n", 40 * 8 * TICKRATE_AVALON / timecnt);
				break;

			case CDC_I2C_REQ_DEVICE_XFER:
				timecnt = 0;
				AVALON_TMR_Set(AVALON_TMR_ID1, 1, AVALON_TMRID1_Fun);
				CDC_I2C_HandleXferReq(pCDCI2c, pOut, pIn);
				AVALON_TMR_Kill(AVALON_TMR_ID1);
				DEBUGOUT("XFER rate : %d bps\r\n", 40 * 8 * TICKRATE_AVALON / timecnt);
				break;

			case CDC_I2C_REQ_RESET:
				Chip_I2CM_ResetControl(pCDCI2c->pI2C);
				Chip_I2CM_SendStartAfterStop(pCDCI2c->pI2C);
				Chip_I2CM_WriteByte(pCDCI2c->pI2C, 0xFF);
				Chip_I2CM_SendStop(pCDCI2c->pI2C);
				pCDCI2c->resetReq = 0;
				break;

			}

			CDC_I2C_IncIndex(&pCDCI2c->reqRdIndx);
			CDC_I2C_IncIndex(&pCDCI2c->respWrIndx);
		}

		/* last report is successfully sent. Send next response if in queue. */
		if (pCDCI2c->respRdIndx != pCDCI2c->respWrIndx) {
			DEBUGOUT("CDC_I2C_process : resp %d,%d\r\n", pCDCI2c->respWrIndx, pCDCI2c->respRdIndx);
			if ((pCDCI2c->tx_flags & CDC_I2C_TX_BUSY) == 0) {
				pCDCI2c->tx_flags |= CDC_I2C_TX_BUSY;
				len = pCDCI2c->respQ[pCDCI2c->respRdIndx][0];
				USBD_API->hw->WriteEP(pCDCI2c->hUsb, pCDCI2c->epin_adr, &pCDCI2c->respQ[pCDCI2c->respRdIndx][0], len);
				CDC_I2C_IncIndex(&pCDCI2c->respRdIndx);
			}
		}
	}
	else {
		/* check if we just got dis-connected */
		if (pCDCI2c->state != CDC_I2C_STATE_DISCON) {
			/* reset indexes */
			pCDCI2c->reqWrIndx = pCDCI2c->reqRdIndx = 0;
			pCDCI2c->respRdIndx = pCDCI2c->respWrIndx = 0;
			pCDCI2c->resetReq = 0;
		}
		pCDCI2c->state = CDC_I2C_STATE_DISCON;
	}
}
