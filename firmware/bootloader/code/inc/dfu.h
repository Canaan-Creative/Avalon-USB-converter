/*
 * @brief dfu head file
 *
 * @note
 *
 * @par
 */
#ifndef __DFU_H_
#define __DFU_H_

ErrorCode_t dfu_init(USBD_HANDLE_T hUsb, USB_INTERFACE_DESCRIPTOR* pIntfDesc, USBD_API_INIT_PARAM_T *pUsbParam);
uint8_t dfu_sig(void);
void dfu_proc(void);
uint32_t get_write_flash_addr(void);
uint8_t get_enable_write_flash_status(void);
void clear_enable_write_flash_status(void);
uint8_t get_finish_upgrade(void);
void clear_finish_upgrade(void);
#endif /* __DFU_H_ */
