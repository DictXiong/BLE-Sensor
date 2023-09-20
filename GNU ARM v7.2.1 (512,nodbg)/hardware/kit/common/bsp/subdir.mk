################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hardware/kit/common/bsp/bsp_stk.c 

OBJS += \
./hardware/kit/common/bsp/bsp_stk.o 

C_DEPS += \
./hardware/kit/common/bsp/bsp_stk.d 


# Each subdirectory must supply rules for building sources it contributes
hardware/kit/common/bsp/bsp_stk.o: ../hardware/kit/common/bsp/bsp_stk.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m33 -mthumb -std=c99 '-DNVM3_DEFAULT_NVM_SIZE=24576' '-D__StackLimit=0x20000000' '-D__STACK_SIZE=0x800' '-DHAL_CONFIG=1' '-D__HEAP_SIZE=0xD00' '-DEFR32BG22C224F512GM32=1' -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\Device\SiliconLabs\EFR32BG22\Source\GCC" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\CMSIS\Include" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\Device\SiliconLabs\EFR32BG22\Include" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\radio\rail_lib\common" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\hardware\kit\common\bsp" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\emlib\src" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\emlib\inc" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\common\inc" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\hardware\kit\common\halconfig" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\emdrv\nvm3\src" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\app\bluetooth\common\util" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\emdrv\nvm3\inc" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\hardware\kit\common\drivers" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\Device\SiliconLabs\EFR32BG22\Source" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\radio\rail_lib\chip\efr32\efr32xg2x" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\halconfig\inc\hal-config" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\bootloader\api" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\protocol\bluetooth\ble_stack\inc\common" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\radio\rail_lib\protocol\ble" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\radio\rail_lib\protocol\ieee802154" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\protocol\bluetooth\ble_stack\inc\soc" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\service\sleeptimer\src" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\emdrv\common\inc" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\emdrv\uartdrv\inc" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\service\sleeptimer\inc" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\emdrv\sleep\src" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\service\sleeptimer\config" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\emdrv\sleep\inc" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\emdrv\gpiointerrupt\inc" -I"C:\Users\me\SimplicityStudio\v4_workspace\Blitz\platform\bootloader" -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=hard -MMD -MP -MF"hardware/kit/common/bsp/bsp_stk.d" -MT"hardware/kit/common/bsp/bsp_stk.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


