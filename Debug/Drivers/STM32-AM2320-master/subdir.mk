################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/STM32-AM2320-master/am2320.c 

OBJS += \
./Drivers/STM32-AM2320-master/am2320.o 

C_DEPS += \
./Drivers/STM32-AM2320-master/am2320.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32-AM2320-master/%.o: ../Drivers/STM32-AM2320-master/%.c Drivers/STM32-AM2320-master/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Tree/Desktop/PBL/PBL2_SmartClock/Drivers/ILI9341" -I"C:/Users/Tree/Desktop/PBL/PBL2_SmartClock/Drivers/STM32-AM2320-master" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-STM32-2d-AM2320-2d-master

clean-Drivers-2f-STM32-2d-AM2320-2d-master:
	-$(RM) ./Drivers/STM32-AM2320-master/am2320.d ./Drivers/STM32-AM2320-master/am2320.o

.PHONY: clean-Drivers-2f-STM32-2d-AM2320-2d-master

