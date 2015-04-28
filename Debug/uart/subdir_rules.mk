################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
uart/uart.obj: ../uart/uart.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -Ooff --opt_for_speed=2 --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="C:/Users/Nick/workspace_v6_0/lumosembedded/simplelink" --include_path="C:/Users/Nick/workspace_v6_0/lumosembedded/FreeRTOS/Source/include" --include_path="C:/Users/Nick/workspace_v6_0/lumosembedded/FreeRTOS/Source/portable/CCS4/ARM_CM3" --include_path="C:/ti/TivaWare_C_Series-2.1.0.12573/" --include_path="C:/ti/CC3100SDK_1.0.0/cc3100-sdk/examples/common" --include_path="C:/ti/CC3100SDK_1.0.0/cc3100-sdk/simplelink/include" --include_path="C:/ti/CC3100SDK_1.0.0/cc3100-sdk/simplelink/source" --include_path="C:/ti/CC3100SDK_1.0.0/cc3100-sdk/platform/tiva-c-launchpad" -g --gcc --define=PART_TM4C123GH6PM --define=_USE_CLI_ --define=TARGET_IS_BLIZZARD_RA1=1 --define=ccs="ccs" --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="uart/uart.pp" --obj_directory="uart" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


