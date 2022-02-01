################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/RPU_task.c \
../src/helper.c \
../src/platform_info.c \
../src/rpmsg-echo.c \
../src/rsc_table.c \
../src/simple.c \
../src/zynqmp_r5_a53_rproc.c 

OBJS += \
./src/RPU_task.o \
./src/helper.o \
./src/platform_info.o \
./src/rpmsg-echo.o \
./src/rsc_table.o \
./src/simple.o \
./src/zynqmp_r5_a53_rproc.o 

C_DEPS += \
./src/RPU_task.d \
./src/helper.d \
./src/platform_info.d \
./src/rpmsg-echo.d \
./src/rsc_table.d \
./src/simple.d \
./src/zynqmp_r5_a53_rproc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM R5 gcc compiler'
	armr5-none-eabi-gcc -DARMR5 -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-r5 -mfloat-abi=hard  -mfpu=vfpv3-d16 -I/home/daniele/workspace/zcu102_1/export/zcu102_1/sw/zcu102_1/domain_psu_cortexr5_0/bspinclude/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


