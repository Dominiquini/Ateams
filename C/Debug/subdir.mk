################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ateams.c \
../genetico.c \
../io.c \
../lista.c \
../makespan.c \
../memoria.c \
../solucoes.c \
../tabu.c 

OBJS += \
./ateams.o \
./genetico.o \
./io.o \
./lista.o \
./makespan.o \
./memoria.o \
./solucoes.o \
./tabu.o 

C_DEPS += \
./ateams.d \
./genetico.d \
./io.d \
./lista.d \
./makespan.d \
./memoria.d \
./solucoes.d \
./tabu.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


