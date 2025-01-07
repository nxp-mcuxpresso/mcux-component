# Copyright 2025 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
if (CONFIG_MCUX_COMPONENT_component.gen_hal.memc.flexspi)
    mcux_component_version(2.3.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        SOURCES zephyr/drivers/memc/memc_mcux_flexspi.c
                zephyr/drivers/memc/memc_mcux_flexspi.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/gen_hal
        INCLUDES zephyr/drivers/memc/.
    )
endif()
