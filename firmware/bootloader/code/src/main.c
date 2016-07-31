/*
 * @brief
 *
 * @note
 * Author: fanzixiao@canaan-creative.com
 *
 * @par
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include "board.h"
#include <string.h>
#include "app_usbd_cfg.h"
#include "sbl_iap.h"
#include "dfu.h"
#include "libfunctions.h"
#include "avalon_usb.h"

#if defined (__CODE_RED)
#include <NXP/crp.h>
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;
#endif

int main(void) {

	uint32_t enabled_irqs = 0;

    SystemCoreClockUpdate();
    Board_Init();

    if ((*(uint32_t *)(APP_END_ADDR - 4)) != UPGRADE_FLAG) {
		usb_init();
		while(1) {
			if (dfu_sig()) {
				dfu_proc();
				usb_reconnect();
			}

			if (get_enable_write_flash_status()) {
				enabled_irqs = NVIC->ISER[0];
				NVIC->ICER[0] = enabled_irqs;
				write_flash(get_write_flash_addr(), g_flash_buf, FLASH_BUF_SIZE);
				memset((unsigned char *)&g_flash_buf[0], 0xff, FLASH_BUF_SIZE);
				NVIC->ISER[0] = enabled_irqs;
				clear_enable_write_flash_status();
			} else if (get_last_upgrade_data_packet()) {
				enabled_irqs = NVIC->ISER[0];
				NVIC->ICER[0] = enabled_irqs;
				write_finish_upgrade_flag();
				memset((unsigned char *)&g_flash_buf[0], 0xff, FLASH_BUF_SIZE);
				NVIC->ISER[0] = enabled_irqs;
				clear_last_upgrade_data_packet();
			} else if (get_finish_upgrade()) {
				clear_finish_upgrade();
				NVIC_SystemReset();
			}
		}

    } else {
		/* Valid application located in the next sector(s) of flash so execute */

		/* Load main stack pointer with application stack pointer initial value,
		stored at first location of application area */
		asm volatile("ldr r0, =0x4000");
		asm volatile("ldr r0, [r0]");
		asm volatile("mov sp, r0");

		/* Load program counter with application reset vector address, located at
		second word of application area. */
		asm volatile("ldr r0, =0x4004");
		asm volatile("ldr r0, [r0]");
		asm volatile("mov pc, r0");

		/* User application execution should now start and never return here.... */
    }

    return 0 ;
}
