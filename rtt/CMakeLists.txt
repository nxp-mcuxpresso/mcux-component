# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.rtt.template)
    mcux_add_source(
        SOURCES template/SEGGER_RTT_Conf.h
        CONFIG TRUE
    )
    mcux_add_include(
        INCLUDES template
        TARGET_FILES SEGGER_RTT_Conf.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.rtt)
    mcux_add_source(
        SOURCES RTT/SEGGER_RTT.c
                RTT/SEGGER_RTT.h
                RTT/SEGGER_RTT_ASM_ARMv7M.S
                RTT/SEGGER_RTT_printf.c
                Syscalls/SEGGER_RTT_Syscalls_GCC.c
                Syscalls/SEGGER_RTT_Syscalls_IAR.c
                Syscalls/SEGGER_RTT_Syscalls_KEIL.c
                Syscalls/SEGGER_RTT_Syscalls_SES.c
                License.txt
                README.txt
    )
    mcux_add_include(
        INCLUDES RTT
    )
endif()
