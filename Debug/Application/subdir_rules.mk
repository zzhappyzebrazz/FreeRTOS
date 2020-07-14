################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Application/%.obj: ../Application/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1000/ccs/tools/compiler/ti-cgt-arm_20.2.0.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/user/Documents/Embedded programming/TVALab_freertos/FreeRTOSExample1" --include_path="C:/Users/user/Documents/Embedded programming/FreRTOS/utils" --include_path="C:/Users/user/Documents/Embedded programming/FreRTOS" --include_path="C:/Users/user/Documents/Embedded programming/FreRTOS/FreeRTOS/FreeRTOSv10.0.1/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/Users/user/Documents/Embedded programming/TVALab_freertos/FreeRTOSExample1/Application" --include_path="C:/Users/user/Documents/Embedded programming/FreRTOS/FreeRTOS/FreeRTOSv10.0.1/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178" --include_path="C:/ti/TivaWare_C_Series-2.1.4.178/utils" --include_path="C:/Users/user/Documents/Embedded programming/FreRTOS/drivers" --include_path="C:/ti/ccs1000/ccs/tools/compiler/ti-cgt-arm_20.2.0.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_BLIZZARD_RB1 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="Application/$(basename $(<F)).d_raw" --obj_directory="Application" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


