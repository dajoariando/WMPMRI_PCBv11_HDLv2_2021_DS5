################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../exec_adc_readSingleChannel.c \
../exec_adc_testPattern.c \
../exec_bstream_pcharge_n_dump.c \
../exec_dac_gradWrite.c 

OBJS += \
./exec_adc_readSingleChannel.o \
./exec_adc_testPattern.o \
./exec_bstream_pcharge_n_dump.o \
./exec_dac_gradWrite.o 

C_DEPS += \
./exec_adc_readSingleChannel.d \
./exec_adc_testPattern.d \
./exec_bstream_pcharge_n_dump.d \
./exec_dac_gradWrite.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler 4 [arm-linux-gnueabihf]'
	arm-linux-gnueabihf-gcc -Dsoc_cv_av -I"C:\intelFPGA\17.1\embedded\ip\altera\hps\altera_hps\hwlib\include\soc_cv_av" -I"C:\intelFPGA\17.1\embedded\ip\altera\hps\altera_hps\hwlib\include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


