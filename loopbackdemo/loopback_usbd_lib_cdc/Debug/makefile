################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include example/src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: loopback_usbd_lib_cdc.axf

# Tool invocations
loopback_usbd_lib_cdc.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/nxplib/nxp_lpcxpresso_11u14_board_lib/Debug" -L"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/nxplib/lpc_chip_11uxx_lib/Debug" -L"/Users/mikeqin/Documents/LPCXpresso_7.3.0/workspace/loopbackdemo/nxplib/lpc_chip_11uxx_lib/libs" -Xlinker -Map="loopback_usbd_lib_cdc.map" -Xlinker --gc-sections -mcpu=cortex-m0 -mthumb -T "loopback_usbd_lib_cdc_Debug.ld" -o "loopback_usbd_lib_cdc.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) loopback_usbd_lib_cdc.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "loopback_usbd_lib_cdc.axf"; # arm-none-eabi-objcopy -O binary "loopback_usbd_lib_cdc.axf" "loopback_usbd_lib_cdc.bin" ; checksum -p LPC11U14_201 -d "loopback_usbd_lib_cdc.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
