/*
 * @brief This file contains Avalon using USB ROM Drivers.
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
#include "app_usbd_cfg.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

#ifdef __GNUC__
#define ALIGN4 __attribute__ ((aligned(4)))
#else // Keil
#define ALIGN4 __align(4)
#endif

/**
 * USB Standard Device Descriptor
 */
ALIGNED(4) const uint8_t USB_DeviceDescriptor[] = {
	USB_DEVICE_DESC_SIZE,			/* bLength */
	USB_DEVICE_DESCRIPTOR_TYPE,		/* bDescriptorType */
	WBVAL(0x0200),					/* bcdUSB 2.0 */
	0x00,							/* bDeviceClass */
	0x00,							/* bDeviceSubClass */
	0x00,							/* bDeviceProtocol */
	USB_MAX_PACKET0,				/* bMaxPacketSize0 */
	WBVAL(0x29f1),					/* idVendor */
	WBVAL(0x33f2),					/* idProduct */
	WBVAL(0x0100),					/* bcdDevice */
	0x01,							/* iManufacturer */
	0x02,							/* iProduct */
	0x03,							/* iSerialNumber */
	0x01							/* bNumConfigurations */
};

/**
 * USB FSConfiguration Descriptor
 * All Descriptors (Configuration, Interface, Endpoint, Class, Vendor)
 */
ALIGNED(4) uint8_t USB_FsConfigDescriptor[] = {
	/* Configuration 1 */
	USB_CONFIGURATION_DESC_SIZE,			/* bLength */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,		/* bDescriptorType */
	WBVAL(									/* wTotalLength */
		USB_CONFIGURATION_DESC_SIZE   +
		USB_DFU_DESCRIPTOR_SIZE       +
		USB_INTERFACE_DESC_SIZE		  +
		0
		),
	0x01,									/* bNumInterfaces */
	0x01,									/* bConfigurationValue */
	0x00,									/* iConfiguration */
	USB_CONFIG_SELF_POWERED,				/* bmAttributes */
	USB_CONFIG_POWER_MA(100),				/* bMaxPower */

	/* Interface 0, Alternate Setting 0, DFU Class */
	USB_INTERFACE_DESC_SIZE,				/* bLength */
	USB_INTERFACE_DESCRIPTOR_TYPE,			/* bDescriptorType */
	0x00,									/* bInterfaceNumber */
	0x00,									/* bAlternateSetting */
	0x00,									/* bNumEndpoints */
	USB_DEVICE_CLASS_APP,					/* bInterfaceClass */
	USB_DFU_SUBCLASS,						/* bInterfaceSubClass */
	0x01,									/* bInterfaceProtocol */
	0x04,									/* iInterface */
	/* DFU RunTime/DFU Mode Functional Descriptor */
	USB_DFU_DESCRIPTOR_SIZE,				/* bLength */
	USB_DFU_DESCRIPTOR_TYPE,				/* bDescriptorType */
	USB_DFU_CAN_DOWNLOAD | USB_DFU_CAN_UPLOAD | USB_DFU_MANIFEST_TOL | USB_DFU_WILL_DETACH, /* bmAttributes */
	WBVAL(0xFF00),							/* wDetachTimeout */
	WBVAL(USB_DFU_XFER_SIZE),				/* wTransferSize */
	WBVAL(0x100),							/* bcdDFUVersion */
	/* Terminator */
	0										/* bLength */
};

ALIGN4 const uint8_t USB_dfuConfigDescriptor[] = {
	USB_CONFIGURATION_DESC_SIZE,		/* bLength */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType */
	WBVAL(								/* wTotalLength */
		USB_CONFIGURATION_DESC_SIZE +
		USB_INTERFACE_DESC_SIZE     +
		USB_DFU_DESCRIPTOR_SIZE
	     ),
	0x01,								/* bNumInterfaces */
	0x02,								/* bConfigurationValue */
	0x00,								/* iConfiguration */
	USB_CONFIG_SELF_POWERED,			/* bmAttributes */
	USB_CONFIG_POWER_MA(100),			/* bMaxPower */

	/* Interface 0, Alternate Setting 0, DFU Class */
	USB_INTERFACE_DESC_SIZE,			/* bLength */
	USB_INTERFACE_DESCRIPTOR_TYPE,		/* bDescriptorType */
	0x00,								/* bInterfaceNumber */
	0x00,								/* bAlternateSetting */
	0x00,								/* bNumEndpoints */
	USB_DEVICE_CLASS_APP,				/* bInterfaceClass */
	USB_DFU_SUBCLASS,					/* bInterfaceSubClass */
	0x02,								/* bInterfaceProtocol */  /* 02: DFU mode */
	0x04,								/* iInterface */
	/* DFU RunTime/DFU Mode Functional Descriptor */
	USB_DFU_DESCRIPTOR_SIZE,			/* bLength */
	USB_DFU_DESCRIPTOR_TYPE,			/* bDescriptorType */
	USB_DFU_CAN_DOWNLOAD | USB_DFU_CAN_UPLOAD | USB_DFU_MANIFEST_TOL | USB_DFU_WILL_DETACH,
	WBVAL(0xFF00),						/* wDetachTimeout */
	WBVAL(USB_DFU_XFER_SIZE),			/* wTransferSize */
	WBVAL(0x100),						/* bcdDFUVersion */
	/* Terminator */
	0									/* bLength */
};

/**
 * USB String Descriptor (optional)
 */
const uint8_t USB_StringDescriptor[] = {
	/* Index 0x00: LANGID Codes */
	0x04,							/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	WBVAL(0x0409),					/* wLANGID : US English */
	/* Index 0x01: Manufacturer */
	(6 * 2 + 2),					/* bLength (18 Char + Type + lenght) */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	'C', 0,
	'A', 0,
	'N', 0,
	'A', 0,
	'A', 0,
	'N', 0,
	/* Index 0x02: Product */
	(21 * 2 + 2),					/* bLength (21 Char + Type + lenght) */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	'A', 0,
	'v', 0,
	'a', 0,
	'l', 0,
	'o', 0,
	'n', 0,
	' ', 0,
	'U', 0,
	'S', 0,
	'B', 0,
	' ', 0,
	'B', 0,
	'o', 0,
	'o', 0,
	't', 0,
	'l', 0,
	'o', 0,
	'a', 0,
	'd', 0,
	'e', 0,
	'r', 0,
	/* Index 0x03: Serial Number */
	(8 * 2 + 2),					/* bLength (13 Char + Type + lenght) */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	'2', 0,
	'0', 0,
	'1', 0,
	'6', 0,
	'0', 0,
	'1', 0,
	'0', 0,
	'6', 0,
	/* Index 0x04: Interface 0, Alternate Setting 0 */
	(14 * 2 + 2),					/* bLength (14 Char + Type + lenght) */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	'A', 0,
	'v', 0,
	'a', 0,
	'l', 0,
	'o', 0,
	'n', 0,
	' ', 0,
	'U', 0,
	'S', 0,
	'B', 0,
	' ', 0,
	'D', 0,
	'F', 0,
	'U', 0,
};
