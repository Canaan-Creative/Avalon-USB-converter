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
OBJDUMP	:= $(CROSS_COMPILE)objdump

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

LIBNAME	 = $(shell pwd |sed 's/^\(.*\)[/]//' )

SRCS    ?= $(wildcard src/*.c)
OBJS	 = $(patsubst %.c,%.o,$(SRCS))

LIBDIR	 = ./libs
SLIB	 = lib$(LIBNAME).a

INCLUDES	+= -I./inc

CFLAGS_WARN = -Wall -Wextra -Wshadow -Wmissing-prototypes \
		-Wmissing-declarations -Wno-format-zero-length \
		-Wno-unused-parameter

CFLAGS_PLATFORM = -D__REDLIB__ -D__CODE_RED -DCORE_M0 \
	-fmessage-length=0 -fno-builtin -ffunction-sections \
	-fdata-sections -mcpu=cortex-m0 -mthumb \
	-specs=redlib.specs

# -Os -Og will not work with spi, it has a side effect
# Judge RELEASE OR DEBUG
ifeq "$(FW_RELEASE)" "DEBUG"
CFLAGS_DEBUG	+= -DDEBUG -O0 -g3 -Wall -c
else
CFLAGS_DEBUG	+= -DNDEBUG -Os -g -Wall -c
endif

CFLAGS += $(CFLAGS_WARN) $(CFLAGS_PLATFORM) $(CFLAGS_DEBUG) $(INCLUDES)
