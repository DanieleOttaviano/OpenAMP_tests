################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += \
firmware_rpu.elf.size \


# All Target
all: firmware_rpu.elf secondary-outputs

# Tool invocations
firmware_rpu.elf: $(OBJS) ../src/lscript.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM R5 gcc linker'
	armr5-none-eabi-gcc -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -Wl,-T -Wl,../src/lscript.ld -L/home/daniele/workspace/zcu102_1/export/zcu102_1/sw/zcu102_1/domain_psu_cortexr5_0/bsplib/lib -o "firmware_rpu.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

firmware_rpu.elf.size: firmware_rpu.elf
	@echo 'Invoking: ARM R5 Print Size'
	armr5-none-eabi-size firmware_rpu.elf  |tee "firmware_rpu.elf.size"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(S_UPPER_DEPS)$(C_DEPS)$(ELFSIZE) firmware_rpu.elf
	-@echo ' '

secondary-outputs: $(ELFSIZE)

.PHONY: all clean dependents

-include ../makefile.targets
