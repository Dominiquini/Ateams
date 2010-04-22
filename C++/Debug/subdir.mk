################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../Ateams.o \
../Controle.o \
../JobShop.o 

CPP_SRCS += \
../Ateams.cpp \
../Controle.cpp \
../JobShop.cpp 

OBJS += \
./Ateams.o \
./Controle.o \
./JobShop.o 

CPP_DEPS += \
./Ateams.d \
./Controle.d \
./JobShop.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


