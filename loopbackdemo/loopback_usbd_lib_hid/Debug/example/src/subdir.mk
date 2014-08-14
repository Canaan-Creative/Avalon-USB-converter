################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../example/src/cr_startup_lpc11xx.c \
../example/src/hid_desc.c \
../example/src/hid_generic.c \
../example/src/hid_main.c \
../example/src/sysinit.c 

OBJS += \
./example/src/cr_startup_lpc11xx.o \
./example/src/hid_desc.o \
./example/src/hid_generic.o \
./example/src/hid_main.o \
./example/src/sysinit.o 

C_DEPS += \
./example/src/cr_startup_lpc11xx.d \
./example/src/hid_desc.d \
./example/src/hid_generic.d \
./example/src/hid_main.d \
./example/src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
example/src/cr_startup_lpc11xx.o: ../example/src/cr_startup_lpc11xx.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -DCORE_M0 -I"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/loopback_usbd_lib_hid/example/inc" -I"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/nxplib/lpc_chip_11uxx_lib/inc" -I"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/nxplib/nxp_lpcxpresso_11u14_board_lib/inc" -I"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/nxplib/lpc_chip_11uxx_lib/inc/usbd" -Os -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"example/src/cr_startup_lpc11xx.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

example/src/%.o: ../example/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -DCORE_M0 -I"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/loopback_usbd_lib_hid/example/inc" -I"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/nxplib/lpc_chip_11uxx_lib/inc" -I"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/nxplib/nxp_lpcxpresso_11u14_board_lib/inc" -I"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/nxplib/lpc_chip_11uxx_lib/inc/usbd" -Og -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


