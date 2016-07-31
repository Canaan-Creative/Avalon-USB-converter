#
# Author: Mikeqin <Fengling.Qin@gmail.com>
#         Johnson <fanzixiao@canaan-creative.com>
#
# This is free and unencumbered software released into the public domain.
# For details see the UNLICENSE file at the root of the source tree.
#

SHELL = /bin/bash

CROSS_COMPILE ?= arm-none-eabi-

CC	:= $(CROSS_COMPILE)gcc
LD	:= $(CROSS_COMPILE)ld
SIZE	:= $(CROSS_COMPILE)size
AR	:= $(CROSS_COMPILE)ar
OBJCOPY	:= $(CROSS_COMPILE)objcopy

# ----- Verbosity control -----------------------------------------------------

CC_normal       := $(CC)
CPP_normal      := $(CPP)
DEPEND_normal   = $(CPP_normal) $(CFLAGS) -MM -MG

ifeq ($(V),1)
    CC          = $(CC_normal)
    BUILD       =
    DEPEND      = $(DEPEND_normal)
else
    CC          = @echo "  CC       " $@ && $(CC_normal)
    BUILD       = @echo "  BUILD    " $@ &&
    DEPEND      = @$(DEPEND_normal)
endif

