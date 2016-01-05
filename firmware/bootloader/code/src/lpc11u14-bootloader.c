/*
===============================================================================
 Name        : lpc11u14-bootloader.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#include <string.h>
#include "app_usbd_cfg.h"
#include "sbl_iap.h"
#include "dfu.h"

#include "libfunctions.h"
#include "avalon_usb.h"

// TODO: insert other definitions and declarations here

int main(void) {

	uint32_t enabled_irqs = 0;

#if defined (__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    //Board_LED_Set(0, true);
#endif
#endif

    // TODO: insert code here

    // Force the counter to be placed into memory
    // Enter an infinite loop, just incrementing a counter

    if ((*(uint32_t *)(APP_END_ADDR - 4)) != 0xAABBCCDD) {
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
			}

			if (get_finish_upgrade()) {
				enabled_irqs = NVIC->ISER[0];
				NVIC->ICER[0] = enabled_irqs;
				write_finish_upgrade_flag();
				memset((unsigned char *)&g_flash_buf[0], 0xff, FLASH_BUF_SIZE);
				NVIC->ISER[0] = enabled_irqs;
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


/*****************************************************************************
 *
 *                      Interrupt redirection functions
 *
 *****************************************************************************/
/*****************************************************************************
 ** Function name:   NMI_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void NMI_Handler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x4008");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   HardFault_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void HardFault_Handler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x400C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   SVCall_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void SVCall_Handler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x402C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   PendSV_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 ******************************************************************************/
void PendSV_Handler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x4038");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   SysTick_Handler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void SysTick_Handler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x403C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   WAKEUP_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 ******************************************************************************/
void WAKEUP_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x4040");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   I2C_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void I2C_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x407C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   TIMER16_0_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void TIMER16_0_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x4080");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   TIMER16_1_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void TIMER16_1_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x4084");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   TIMER32_0_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void TIMER32_0_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x4088");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   TIMER32_1_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void TIMER32_1_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x408C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   SSP_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void SSP_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x4090");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   UART_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void UART_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x4094");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   USB_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
//void USB_IRQHandler(void)
//{
	/* Re-direct interrupt, get handler address from application vector table */
//	asm volatile("ldr r0, =0x4098");
//	asm volatile("ldr r0, [r0]");
//	asm volatile("mov pc, r0");
//}

/*****************************************************************************
 ** Function name:   USB_FIQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void USB_FIQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x409C");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   ADC_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void ADC_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x40A0");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   WDT_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void WDT_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x40A4");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   BOD_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void BOD_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x40A8");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   FMC_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void FMC_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x40AC");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   PIOINT3_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void PIOINT3_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x40B0");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   PIOINT2_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void PIOINT2_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x40B4");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   PIOINT1_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void PIOINT1_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x40B8");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 ** Function name:   PIOINT0_IRQHandler
 **
 ** Description:	 Redirects CPU to application defined handler
 **
 ** Parameters:	     None
 **
 ** Returned value:  None
 **
 *****************************************************************************/
void PIOINT0_IRQHandler(void)
{
	/* Re-direct interrupt, get handler address from application vector table */
	asm volatile("ldr r0, =0x40BC");
	asm volatile("ldr r0, [r0]");
	asm volatile("mov pc, r0");
}

/*****************************************************************************
 **                            End Of File
 *****************************************************************************/
