# Copyright 2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.power_manager_mcxw23)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/power_manager_mcxw23/device
        SOURCES fsl_pm_device.c
                fsl_pm_device.h
                fsl_pm_device_config.h
                fsl_pm_board.c
                fsl_pm_board.h
    )
    mcux_add_include(
        INCLUDES ./core
    )
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/power_manager_mcxw23/core
        SOURCES fsl_pm_core.h
                fsl_pm_core.c
                fsl_pm_config.h
                fsl_pm_app.c
                fsl_pm_app.h
    )

    mcux_add_include(
        INCLUDES ./device
    )
endif()
