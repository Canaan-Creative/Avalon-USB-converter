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
#include "hid_i2c.h"
#include "i2c_lpc11uxx.h"
#include "lpcusbsio_i2c.h"
#include "avalon_api.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define HID_I2C_STATE_DISCON        0
#define HID_I2C_STATE_CONNECTED     1

/**
 * Structure containing HID_I2C control data
 */
typedef struct __HID_I2C_CTRL_T {
	USBD_HANDLE_T hUsb;		/*!< Handle to USB stack */
	LPC_I2C_T *pI2C;		/*!< I2C port this bridge is associated. */
	uint8_t reqWrIndx;		/*!< Write index for request queue. */
	uint8_t reqRdIndx;		/*!< Read index for request queue. */
	uint8_t respWrIndx;		/*!< Write index for response queue. */
	uint8_t respRdIndx;		/*!< Read index for response queue. */
	uint8_t reqPending;		/*!< Flag indicating request is pending in EP RAM */
	uint8_t respIdle;		/*!< Flag indicating EP_IN/response interrupt is idling */
	uint8_t state;			/*!< State of the controller */
	uint8_t resetReq;		/*!< Flag indicating if reset is requested by host */
	uint8_t epin_adr;		/*!< Interrupt IN endpoint associated with this HID instance. */
	uint8_t epout_adr;		/*!< Interrupt OUT endpoint associated with this HID instance. */
	uint16_t pad0;

	uint8_t reqQ[HID_I2C_MAX_PACKETS][HID_I2C_PACKET_SZ];		/*!< Requests queue */
	uint8_t respQ[HID_I2C_MAX_PACKETS][HID_I2C_PACKET_SZ];	/*!< Response queue */
} HID_I2C_CTRL_T;

static const char *g_fwVersion = "FW v1.00 (" __DATE__ " " __TIME__ ")";

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

extern const uint8_t HID_I2C_ReportDescriptor[];
extern const uint16_t HID_I2C_ReportDescSize;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

static ErrorCode_t HID_I2C_Bind(HID_I2C_CTRL_T *pHidI2c, LPC_I2C_T *pI2C)
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
	pHidI2c->pI2C = pI2C;
	NVIC_DisableIRQ(I2C0_IRQn);
	ret = LPC_OK;
#else
#warning "No I2C Pin Muxing defined for this example"
#endif
	return ret;
}

static INLINE void HID_I2C_IncIndex(uint8_t *index)
{
	*index = (*index + 1) & (HID_I2C_MAX_PACKETS - 1);
}

/*  HID get report callback function. */
static ErrorCode_t HID_I2C_GetReport(USBD_HANDLE_T hHid, USB_SETUP_PACKET *pSetup,
									 uint8_t * *pBuffer, uint16_t *plength)
{
	return LPC_OK;
}

/* HID set report callback function. */
static ErrorCode_t HID_I2C_SetReport(USBD_HANDLE_T hHid, USB_SETUP_PACKET *pSetup,
									 uint8_t * *pBuffer, uint16_t length)
{
	return LPC_OK;
}

/* HID_I2C Interrupt endpoint event handler. */
static ErrorCode_t HID_I2C_EpIn_Hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
	HID_I2C_CTRL_T *pHidI2c = (HID_I2C_CTRL_T *) data;
	uint16_t len;

	/* last report is successfully sent. Send next response if in queue. */
	if (pHidI2c->respRdIndx != pHidI2c->respWrIndx) {
		pHidI2c->respIdle = 0;
		len = pHidI2c->respQ[pHidI2c->respRdIndx][0];
		USBD_API->hw->WriteEP(pHidI2c->hUsb, pHidI2c->epin_adr, &pHidI2c->respQ[pHidI2c->respRdIndx][0], len);
		HID_I2C_IncIndex(&pHidI2c->respRdIndx);
	}
	else {
		pHidI2c->respIdle = 1;
	}
	return LPC_OK;
}

/* HID_I2C Interrupt endpoint event handler. */
static ErrorCode_t HID_I2C_EpOut_Hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
	HID_I2C_CTRL_T *pHidI2c = (HID_I2C_CTRL_T *) data;
	HID_I2C_OUT_REPORT_T *pOut;

	if (event == USB_EVT_OUT) {
		/* Read the new request received. */
		USBD_API->hw->ReadEP(hUsb, pHidI2c->epout_adr, &pHidI2c->reqQ[pHidI2c->reqWrIndx][0]);
		pOut = (HID_I2C_OUT_REPORT_T *) &pHidI2c->reqQ[pHidI2c->reqWrIndx][0];

		/* handle HID_I2C_REQ_FLUSH request in IRQ context to abort current
		   transaction and reset the queue states. */
		if (pOut->req == HID_I2C_REQ_RESET) {
			pHidI2c->resetReq = 1;
		}
		/* normal request queue the buffer */
		HID_I2C_IncIndex(&pHidI2c->reqWrIndx);
	}
	return LPC_OK;
}

static uint32_t HID_I2C_StatusCheckLoop(HID_I2C_CTRL_T *pHidI2c)
{
	/* wait for status change interrupt */
	while ( (Chip_I2CM_StateChanged(pHidI2c->pI2C) == 0) &&
			(pHidI2c->resetReq == 0)) {
		/* loop */
	}
	return pHidI2c->resetReq;
}

static int32_t HID_I2C_HandleReadStates(HID_I2C_CTRL_T *pHidI2c,
										HID_I2C_RW_PARAMS_T *pRWParam,
										HID_I2C_IN_REPORT_T *pIn,
										uint32_t status)
{
	int32_t ret = 0;
	uint8_t *buff = &pIn->length;
	uint32_t pos;

	switch (status) {
	case 0x58:			/* Data Received and NACK sent */
		buff[pIn->length++] = Chip_I2CM_ReadByte(pHidI2c->pI2C);
		pIn->resp = HID_I2C_RES_OK;
		ret = 1;
		break;

	case 0x40:			/* SLA+R sent and ACK received */
		pHidI2c->pI2C->CONSET = I2C_CON_AA;

	case 0x50:			/* Data Received and ACK sent */
		if (status == 0x50) {
			buff[pIn->length++] = Chip_I2CM_ReadByte(pHidI2c->pI2C);
		}
		pos = pIn->length - HID_I2C_HEADER_SZ;
		if (pRWParam->options & HID_I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE) {
			if ((pRWParam->length - pos) == 1) {
				Chip_I2CM_NackNextByte(pHidI2c->pI2C);
			}
		}
		else if (pRWParam->length == pos) {
			pIn->resp = HID_I2C_RES_OK;
		}
		ret = 1;
		break;
	}

	return ret;
}

static int32_t HID_I2C_HandleWriteStates(HID_I2C_CTRL_T *pHidI2c,
										 HID_I2C_RW_PARAMS_T *pRWParam,
										 HID_I2C_IN_REPORT_T *pIn,
										 uint32_t status)
{
	int32_t ret = 0;
	uint8_t *buff = &pRWParam->data[0];
	uint32_t pos;

	switch (status) {
	case 0x30:			/* DATA sent NAK received */
		if (pRWParam->options & HID_I2C_TRANSFER_OPTIONS_BREAK_ON_NACK) {
			pIn->resp = HID_I2C_RES_NAK;
			ret = 1;
			break;
		}		/* else fall through */

	case 0x08:			/* Start condition on bus */
	case 0x10:			/* Repeated start condition */
		if ((pRWParam->options & HID_I2C_TRANSFER_OPTIONS_NO_ADDRESS) == 0) {
			break;
		}		/* else fall-through */

	case 0x18:			/* SLA+W sent and ACK received */
	case 0x28:			/* DATA sent and ACK received */
		pos = pIn->length - HID_I2C_HEADER_SZ;
		Chip_I2CM_WriteByte(pHidI2c->pI2C, buff[pos]);
		pIn->length++;
		if ((pos) == pRWParam->length) {
			pIn->resp = HID_I2C_RES_OK;
		}
		ret = 1;
		break;
	}

	return ret;
}

static void HID_I2C_HandleRWReq(HID_I2C_CTRL_T *pHidI2c,
								HID_I2C_OUT_REPORT_T *pOut,
								HID_I2C_IN_REPORT_T *pIn,
								int32_t read)
{
	HID_I2C_RW_PARAMS_T *pRWParam = (HID_I2C_RW_PARAMS_T *) &pOut->data[0];
	int32_t handled = 0;
	uint32_t status;

	/* clear state change interrupt status */
	Chip_I2CM_ClearSI(pHidI2c->pI2C);

	if (pRWParam->options & HID_I2C_TRANSFER_OPTIONS_START_BIT) {
		Chip_I2CM_SendStart(pHidI2c->pI2C);
	}

	while (pIn->resp == HID_I2C_RES_INVALID_CMD) {
		/* wait for status change interrupt */
		if (HID_I2C_StatusCheckLoop(pHidI2c) == 0) {

			status = Chip_I2CM_GetCurState(pHidI2c->pI2C);
			/* handle Read or write states. */
			if (read ) {
				handled = HID_I2C_HandleReadStates(pHidI2c, pRWParam, pIn, status);
			}
			else {
				handled = HID_I2C_HandleWriteStates(pHidI2c, pRWParam, pIn, status);
			}

			if (handled == 0) {
				/* check status and send data */
				switch (status) {
				case 0x08:	/* Start condition on bus */
				case 0x10:	/* Repeated start condition */
					if ((pRWParam->options & HID_I2C_TRANSFER_OPTIONS_NO_ADDRESS) == 0) {
						Chip_I2CM_WriteByte(pHidI2c->pI2C, (pRWParam->slaveAddr << 1) | read);
					}
					break;

				case 0x20:	/* SLA+W sent NAK received */
				case 0x48:	/* SLA+R sent NAK received */
					pIn->resp = HID_I2C_RES_SLAVE_NAK;
					break;

				case 0x38:	/* Arbitration lost */
					pIn->resp = HID_I2C_RES_ARBLOST;
					break;

				case 0x00:	/* Bus Error */
					pIn->resp = HID_I2C_RES_BUS_ERROR;
					break;

				default:	/* we shouldn't be in any other state */
					pIn->resp = HID_I2C_RES_ERROR;
					break;
				}
			}
			/* clear state change interrupt status */
			Chip_I2CM_ClearSI(pHidI2c->pI2C);
		}
		else {
			pIn->resp = HID_I2C_RES_ERROR;
			break;
		}
	}

	if ((pIn->resp != HID_I2C_RES_ARBLOST) &&
		(pRWParam->options & HID_I2C_TRANSFER_OPTIONS_STOP_BIT) ) {
		Chip_I2CM_SendStop(pHidI2c->pI2C);
	}
}

static void HID_I2C_HandleXferReq(HID_I2C_CTRL_T *pHidI2c,
								  HID_I2C_OUT_REPORT_T *pOut,
								  HID_I2C_IN_REPORT_T *pIn)
{
	HID_I2C_XFER_PARAMS_T *pXfrParam = (HID_I2C_XFER_PARAMS_T *) &pOut->data[0];
	I2CM_XFER_T xfer;
	uint32_t ret = 0;

	memset(&xfer, 0, sizeof(I2CM_XFER_T));
	xfer.slaveAddr = pXfrParam->slaveAddr;
	xfer.txBuff = &pXfrParam->data[0];
	xfer.rxBuff = &pIn->data[0];
	xfer.rxSz = pXfrParam->rxLength;
	xfer.txSz = pXfrParam->txLength;
	xfer.options = pXfrParam->options;

	/* start transfer */
	Chip_I2CM_Xfer(pHidI2c->pI2C, &xfer);

	while (ret == 0) {
		/* wait for status change interrupt */
		if (HID_I2C_StatusCheckLoop(pHidI2c) == 0) {
			/* call state change handler */
			ret = Chip_I2CM_XferHandler(pHidI2c->pI2C, &xfer);
		}
		else {
			xfer.status = HID_I2C_RES_ERROR;
			break;
		}
	}

	/* Update the length we have to send back */
	if ((pXfrParam->rxLength - xfer.rxSz) > 0) {
		pIn->length += pXfrParam->rxLength - xfer.rxSz;
		DEBUGOUT("In(0x%02x,0x%02x), Out(0x%02x,0x%02x)\r\n", pIn->data[0], pIn->data[1], pXfrParam->rxLength, pXfrParam->txLength);
	}

	/* update response with the I2CM status returned. No translation
	   needed as they map directly to base LPCUSBSIO status. */
	pIn->resp = xfer.status;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* HID init routine */
ErrorCode_t HID_I2C_init(USBD_HANDLE_T hUsb,
						 USB_INTERFACE_DESCRIPTOR *pIntfDesc,
						 USBD_API_INIT_PARAM_T *pUsbParam,
						 LPC_I2C_T *pI2C,
						 USBD_HANDLE_T *pI2CHid)
{
	USBD_HID_INIT_PARAM_T hid_param;
	USB_HID_REPORT_T reports_data[1];
	USB_ENDPOINT_DESCRIPTOR *pEpDesc;
	uint32_t new_addr, i, ep_indx;
	HID_I2C_CTRL_T *pHidI2c;
	ErrorCode_t ret = LPC_OK;

	if ((pIntfDesc == 0) || (pIntfDesc->bInterfaceClass != USB_DEVICE_CLASS_HUMAN_INTERFACE)) {
		return ERR_FAILED;
	}

	/* HID paramas */
	memset((void *) &hid_param, 0, sizeof(USBD_HID_INIT_PARAM_T));
	hid_param.max_reports = 1;
	/* Init reports_data */
	reports_data[0].len = HID_I2C_ReportDescSize;
	reports_data[0].idle_time = 0;
	reports_data[0].desc = (uint8_t *) &HID_I2C_ReportDescriptor[0];

	hid_param.mem_base = pUsbParam->mem_base;
	hid_param.mem_size = pUsbParam->mem_size;
	hid_param.intf_desc = (uint8_t *) pIntfDesc;
	/* user defined functions */
	hid_param.HID_GetReport = HID_I2C_GetReport;
	hid_param.HID_SetReport = HID_I2C_SetReport;
	hid_param.report_data  = reports_data;

	ret = USBD_API->hid->init(hUsb, &hid_param);
	if (ret == LPC_OK) {
		/* allocate memory for HID_I2C_CTRL_T instance */
		pHidI2c = (HID_I2C_CTRL_T *) hid_param.mem_base;
		hid_param.mem_base += sizeof(HID_I2C_CTRL_T);
		hid_param.mem_size -= sizeof(HID_I2C_CTRL_T);
		/* update memory variables */
		pUsbParam->mem_base = hid_param.mem_base;
		pUsbParam->mem_size = hid_param.mem_size;

		/* Init the control structure */
		memset(pHidI2c, 0, sizeof(HID_I2C_CTRL_T));
		/* store stack handle*/
		pHidI2c->hUsb = hUsb;
		/* set return handle */
		*pI2CHid = (USBD_HANDLE_T) pHidI2c;
		/* set response is idling. For HID_I2C_process() to kickstart transmission if response
		   data is pending. */
		pHidI2c->respIdle = 1;

		/* bind to I2C port */
		ret = HID_I2C_Bind(pHidI2c, pI2C);

		/* move to next descriptor */
		new_addr = (uint32_t) pIntfDesc + pIntfDesc->bLength;
		/* loop through descriptor to find EPs */
		for (i = 0; i < pIntfDesc->bNumEndpoints; ) {
			pEpDesc = (USB_ENDPOINT_DESCRIPTOR *) new_addr;
			new_addr = (uint32_t) pEpDesc + pEpDesc->bLength;

			/* parse endpoint descriptor */
			if ((pEpDesc->bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE) &&
				(pEpDesc->bmAttributes == USB_ENDPOINT_TYPE_INTERRUPT)) {

				/* find next endpoint */
				i++;

				if (pEpDesc->bEndpointAddress & USB_ENDPOINT_DIRECTION_MASK) {
					/* store Interrupt IN endpoint */
					pHidI2c->epin_adr = pEpDesc->bEndpointAddress;
					ep_indx = ((pHidI2c->epin_adr & 0x0F) << 1) + 1;
					/* register endpoint interrupt handler if provided*/
					ret = USBD_API->core->RegisterEpHandler(hUsb, ep_indx, HID_I2C_EpIn_Hdlr, pHidI2c);
				}
				else {
					/* store Interrupt OUT endpoint */
					pHidI2c->epout_adr = pEpDesc->bEndpointAddress;
					ep_indx = ((pHidI2c->epout_adr & 0x0F) << 1);
					/* register endpoint interrupt handler if provided*/
					ret = USBD_API->core->RegisterEpHandler(hUsb, ep_indx, HID_I2C_EpOut_Hdlr, pHidI2c);
				}
			}
		}

	}

	return ret;
}

static unsigned int timecnt = 0;
static void AVALON_TMRID1_Fun(void)
{
	timecnt++;
}

/* Process HID_I2C request and response queue. */
void HID_I2C_process(USBD_HANDLE_T hI2CHid)
{
	HID_I2C_CTRL_T *pHidI2c = (HID_I2C_CTRL_T *) hI2CHid;
	HID_I2C_OUT_REPORT_T *pOut;
	HID_I2C_IN_REPORT_T *pIn;
	HID_I2C_PORTCONFIG_T *pCfg;

	if (USB_IsConfigured(pHidI2c->hUsb)) {

		/* set state to connected */
		pHidI2c->state = HID_I2C_STATE_CONNECTED;
		if (pHidI2c->reqWrIndx != pHidI2c->reqRdIndx) {

			/* process the current packet */
			pOut = (HID_I2C_OUT_REPORT_T *) &pHidI2c->reqQ[pHidI2c->reqRdIndx][0];
			pIn = (HID_I2C_IN_REPORT_T *) &pHidI2c->respQ[pHidI2c->respWrIndx][0];
			/* construct response template */
			pIn->length = HID_I2C_HEADER_SZ;
			pIn->transId = pOut->transId;
			pIn->sesId = pOut->sesId;
			pIn->resp = HID_I2C_RES_INVALID_CMD;

			DEBUGOUT("pOut->req = 0x%02x\r\n", pOut->req);
			switch (pOut->req) {
			case HID_I2C_REQ_INIT_PORT:
				/* Init I2C port */
				Chip_I2CM_Init(pHidI2c->pI2C);
				pCfg = (HID_I2C_PORTCONFIG_T *) &pOut->data[0];
				Chip_I2CM_SetBusSpeed(pHidI2c->pI2C, pCfg->busSpeed);

				/* TBD. Change I2C0 pads modes per bus speed requested. Do we need to?*/
				/* send back firmware version string */
				memcpy(&pIn->data[0], g_fwVersion, strlen(g_fwVersion));
				pIn->length += strlen(g_fwVersion);

				/* update response */
				pIn->resp = HID_I2C_RES_OK;
				break;

			case HID_I2C_REQ_DEINIT_PORT:
				Chip_I2CM_DeInit(pHidI2c->pI2C);
				/* update response */
				pIn->resp = HID_I2C_RES_OK;
				break;

			case HID_I2C_REQ_DEVICE_WRITE:
			case HID_I2C_REQ_DEVICE_READ:
				HID_I2C_HandleRWReq(pHidI2c, pOut, pIn, (pOut->req == HID_I2C_REQ_DEVICE_READ));
				break;

			case HID_I2C_REQ_DEVICE_XFER:
				HID_I2C_HandleXferReq(pHidI2c, pOut, pIn);
				break;

			case HID_I2C_REQ_RESET:
				Chip_I2CM_ResetControl(pHidI2c->pI2C);
				Chip_I2CM_SendStartAfterStop(pHidI2c->pI2C);
				Chip_I2CM_WriteByte(pHidI2c->pI2C, 0xFF);
				Chip_I2CM_SendStop(pHidI2c->pI2C);
				pHidI2c->resetReq = 0;
				break;

			}

			HID_I2C_IncIndex(&pHidI2c->reqRdIndx);
			HID_I2C_IncIndex(&pHidI2c->respWrIndx);
		}

		/* Kick-start response tx if it is idling and we have something to send. */
		if ((pHidI2c->respIdle) && (pHidI2c->respRdIndx != pHidI2c->respWrIndx)) {

			pHidI2c->respIdle = 0;
			USBD_API->hw->WriteEP(pHidI2c->hUsb,
								  pHidI2c->epin_adr,
								  &pHidI2c->respQ[pHidI2c->respRdIndx][0],
								  HID_I2C_PACKET_SZ);
			HID_I2C_IncIndex(&pHidI2c->respRdIndx);
		}
	}
	else {
		/* check if we just got dis-connected */
		if (pHidI2c->state != HID_I2C_STATE_DISCON) {
			/* reset indexes */
			pHidI2c->reqWrIndx = pHidI2c->reqRdIndx = 0;
			pHidI2c->respRdIndx = pHidI2c->respWrIndx = 0;
			pHidI2c->respIdle = 1;
			pHidI2c->resetReq = 0;
		}
		pHidI2c->state = HID_I2C_STATE_DISCON;
	}
}
