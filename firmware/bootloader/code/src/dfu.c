/*
 * @brief
 *
 * @note
 * Author: Mikeqin Fengling.Qin@gmail.com
 *         fanzixiao@canaan-creative.com
 *
 * @par
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "app_usbd_cfg.h"
#include "board.h"
#include "dfu.h"
#include "sbl_iap.h"

#define DFU_DEST_BASE		0x4000
#define DFU_MAX_IMAGE_LEN	(16 * 1024)
#define DFU_MAX_BLOCKS		(DFU_MAX_IMAGE_LEN/USB_DFU_XFER_SIZE)

#define TIMEOUT_POLL_LONG	1000
#define TIMEOUT_POLL_SHORT	500

static volatile uint8_t dfu_detach_sig = 0;
extern const uint8_t USB_dfuConfigDescriptor[];

static volatile uint8_t enable_write_flash_status = 0;
static volatile uint8_t last_upgrade_data_packet = 0;
static volatile uint8_t finish_upgrade = 0;
static volatile uint32_t write_flash_addr = 0;
static volatile uint32_t receive_data_len = 0;

static uint32_t dfu_read(uint32_t block_num, uint8_t **pBuff, uint32_t length)
{
	uint32_t src_addr = DFU_DEST_BASE;

	if (length) {
		if (block_num == DFU_MAX_BLOCKS)
			return 0;

		if (block_num > DFU_MAX_BLOCKS)
			return DFU_STATUS_errADDRESS;

		src_addr += (block_num * USB_DFU_XFER_SIZE);
		memcpy((void*)(*pBuff), (void*)src_addr, length);
	}
	return length;
}

static uint8_t dfu_write(uint32_t block_num, uint8_t **pBuff, uint32_t length, uint8_t *bwPollTimeout)
{
	bwPollTimeout[0] = (TIMEOUT_POLL_SHORT & 0xff);
	bwPollTimeout[1] = ((TIMEOUT_POLL_SHORT >> 8) & 0xff);
	bwPollTimeout[2] = ((TIMEOUT_POLL_SHORT >> 16) & 0xff);

	if (length != 0) {
		if (block_num >= DFU_MAX_BLOCKS) {
			receive_data_len = 0;
			return DFU_STATUS_errADDRESS;
		}

		memcpy((unsigned char *)&g_flash_buf[receive_data_len], (unsigned char*)&((*pBuff)[0]), length);
		receive_data_len += length;
		if (receive_data_len > FLASH_BUF_SIZE) {
			receive_data_len = 0;
			return DFU_STATUS_errADDRESS;
		}
		if (receive_data_len == FLASH_BUF_SIZE) {
			receive_data_len = 0;
			bwPollTimeout[0] = (TIMEOUT_POLL_LONG & 0xff);
			bwPollTimeout[1] = ((TIMEOUT_POLL_LONG >> 8) & 0xff);
			bwPollTimeout[2] = ((TIMEOUT_POLL_LONG >> 16) & 0xff);
			enable_write_flash_status = 1;
			write_flash_addr = DFU_DEST_BASE + ((block_num + 1) * USB_DFU_XFER_SIZE) - FLASH_BUF_SIZE;
		}
	}
	return DFU_STATUS_OK;
}

static void dfu_done(void)
{
	if (receive_data_len) {
		enable_write_flash_status = 1;
		write_flash_addr += FLASH_BUF_SIZE;
	}

	last_upgrade_data_packet = 1;
	finish_upgrade = 1;
}

static void dfu_detach(USBD_HANDLE_T hUsb)
{
	USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;

	pCtrl->full_speed_desc = (uint8_t *) &USB_dfuConfigDescriptor[0];
	pCtrl->high_speed_desc = (uint8_t *) &USB_dfuConfigDescriptor[0];

	dfu_detach_sig = 1;

	enable_write_flash_status = 0;
	last_upgrade_data_packet = 0;
	finish_upgrade = 0;
	receive_data_len = 0;
}

ErrorCode_t dfu_init(USBD_HANDLE_T hUsb, USB_INTERFACE_DESCRIPTOR* pIntfDesc, USBD_API_INIT_PARAM_T *pUsbParam)
{
	USBD_DFU_INIT_PARAM_T dfu_param;
	ErrorCode_t ret = LPC_OK;

	/* DFU paramas */
	memset((void*)&dfu_param, 0, sizeof(USBD_DFU_INIT_PARAM_T));
	dfu_param.mem_base = pUsbParam->mem_base;
	dfu_param.mem_size = pUsbParam->mem_size;
	dfu_param.wTransferSize = USB_DFU_XFER_SIZE;

	if ((pIntfDesc == 0) ||
			(pIntfDesc->bInterfaceClass != USB_DEVICE_CLASS_APP) ||
			(pIntfDesc->bInterfaceSubClass != USB_DFU_SUBCLASS) )
		return ERR_FAILED;

	dfu_param.intf_desc = (uint8_t*)pIntfDesc;
	/* user defined functions */
	dfu_param.DFU_Write = dfu_write;
	dfu_param.DFU_Read = dfu_read;
	dfu_param.DFU_Done = dfu_done;
	dfu_param.DFU_Detach = dfu_detach;

	ret = USBD_API->dfu->init(hUsb, &dfu_param, 0);
	/* update memory variables */
	pUsbParam->mem_base = dfu_param.mem_base;
	pUsbParam->mem_size = dfu_param.mem_size;
	return ret;
}

uint8_t dfu_sig(void)
{
	return dfu_detach_sig;
}

void dfu_proc(void)
{
	dfu_detach_sig = 0;
}

uint32_t get_write_flash_addr(void)
{
	return write_flash_addr;
}

uint8_t get_enable_write_flash_status(void)
{
	return enable_write_flash_status;
}

void clear_enable_write_flash_status(void)
{
	enable_write_flash_status = 0;
}

uint8_t get_finish_upgrade(void)
{
	return finish_upgrade;
}

void clear_finish_upgrade(void)
{
	finish_upgrade = 0;
}

uint8_t get_last_upgrade_data_packet(void)
{
	return last_upgrade_data_packet;
}

void clear_last_upgrade_data_packet(void)
{
	last_upgrade_data_packet = 0;
}
