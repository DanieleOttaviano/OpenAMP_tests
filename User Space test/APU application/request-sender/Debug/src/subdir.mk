################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/helper.c \
../src/list-test-kernel.c \
../src/main.c \
../src/platform_info.c \
../src/rpmsg-ping.c \
../src/tasks-managment.c \
../src/zynqmp_linux_r5_proc.c 

OBJS += \
./src/helper.o \
./src/list-test-kernel.o \
./src/main.o \
./src/platform_info.o \
./src/rpmsg-ping.o \
./src/tasks-managment.o \
./src/zynqmp_linux_r5_proc.o 

C_DEPS += \
./src/helper.d \
./src/list-test-kernel.d \
./src/main.d \
./src/platform_info.d \
./src/rpmsg-ping.d \
./src/tasks-managment.d \
./src/zynqmp_linux_r5_proc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v8 Linux gcc compiler'
	aarch64-linux-gnu-gcc -Wall -O0 -g -c -fmessage-length=0 -MT"$@" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


