################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/button.c \
../Core/Src/i2c-lcd.c \
../Core/Src/led.c \
../Core/Src/led_control.c \
../Core/Src/main.c \
../Core/Src/potentioRecording.c \
../Core/Src/potentiometer.c \
../Core/Src/recording.c \
../Core/Src/report_generator.c \
../Core/Src/resutlsDisplay.c \
../Core/Src/signals.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/ultrasonic.c \
../Core/Src/ultrasonicRecording.c 

OBJS += \
./Core/Src/button.o \
./Core/Src/i2c-lcd.o \
./Core/Src/led.o \
./Core/Src/led_control.o \
./Core/Src/main.o \
./Core/Src/potentioRecording.o \
./Core/Src/potentiometer.o \
./Core/Src/recording.o \
./Core/Src/report_generator.o \
./Core/Src/resutlsDisplay.o \
./Core/Src/signals.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/ultrasonic.o \
./Core/Src/ultrasonicRecording.o 

C_DEPS += \
./Core/Src/button.d \
./Core/Src/i2c-lcd.d \
./Core/Src/led.d \
./Core/Src/led_control.d \
./Core/Src/main.d \
./Core/Src/potentioRecording.d \
./Core/Src/potentiometer.d \
./Core/Src/recording.d \
./Core/Src/report_generator.d \
./Core/Src/resutlsDisplay.d \
./Core/Src/signals.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/ultrasonic.d \
./Core/Src/ultrasonicRecording.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/button.cyclo ./Core/Src/button.d ./Core/Src/button.o ./Core/Src/button.su ./Core/Src/i2c-lcd.cyclo ./Core/Src/i2c-lcd.d ./Core/Src/i2c-lcd.o ./Core/Src/i2c-lcd.su ./Core/Src/led.cyclo ./Core/Src/led.d ./Core/Src/led.o ./Core/Src/led.su ./Core/Src/led_control.cyclo ./Core/Src/led_control.d ./Core/Src/led_control.o ./Core/Src/led_control.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/potentioRecording.cyclo ./Core/Src/potentioRecording.d ./Core/Src/potentioRecording.o ./Core/Src/potentioRecording.su ./Core/Src/potentiometer.cyclo ./Core/Src/potentiometer.d ./Core/Src/potentiometer.o ./Core/Src/potentiometer.su ./Core/Src/recording.cyclo ./Core/Src/recording.d ./Core/Src/recording.o ./Core/Src/recording.su ./Core/Src/report_generator.cyclo ./Core/Src/report_generator.d ./Core/Src/report_generator.o ./Core/Src/report_generator.su ./Core/Src/resutlsDisplay.cyclo ./Core/Src/resutlsDisplay.d ./Core/Src/resutlsDisplay.o ./Core/Src/resutlsDisplay.su ./Core/Src/signals.cyclo ./Core/Src/signals.d ./Core/Src/signals.o ./Core/Src/signals.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/ultrasonic.cyclo ./Core/Src/ultrasonic.d ./Core/Src/ultrasonic.o ./Core/Src/ultrasonic.su ./Core/Src/ultrasonicRecording.cyclo ./Core/Src/ultrasonicRecording.d ./Core/Src/ultrasonicRecording.o ./Core/Src/ultrasonicRecording.su

.PHONY: clean-Core-2f-Src

