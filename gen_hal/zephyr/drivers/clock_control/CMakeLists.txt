# Copyright 2025 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
if (CONFIG_MCUX_COMPONENT_component.gen_hal.clock_control.mcux_syscon)
    mcux_component_version(2.3.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        SOURCES zephyr/drivers/clock_control/clock_control_mcux_syscon.c
                zephyr/include/zephyr/dt-bindings/clock/mcux_lpc_syscon_clock.h
    )
endif()
