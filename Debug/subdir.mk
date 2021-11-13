################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../alt_generalpurpose_io.c \
../exec_adc_readSingleChannel.c \
../exec_adc_testpattern.c 

OBJS += \
./alt_generalpurpose_io.o \
./exec_adc_readSingleChannel.o \
./exec_adc_testpattern.o 

C_DEPS += \
./alt_generalpurpose_io.d \
./exec_adc_readSingleChannel.d \
./exec_adc_testpattern.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler 4 [arm-linux-gnueabihf]'
	arm-linux-gnueabihf-gcc -Dsoc_cv_av -I"C:\intelFPGA\17.1\embedded\ip\altera\hps\altera_hps\hwlib\include\soc_cv_av" -I"C:\intelFPGA\17.1\embedded\ip\altera\hps\altera_hps\hwlib\include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


