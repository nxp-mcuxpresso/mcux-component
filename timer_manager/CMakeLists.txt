# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.timer_manager)

    mcux_add_source(
        SOURCES fsl_component_timer_manager.h
                fsl_component_timer_manager.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_PRJSEG_configure.timer_manager_enable_time_stamp)

    mcux_add_configuration(
        AS "-DTM_ENABLE_TIME_STAMP=1"
        CC "-DTM_ENABLE_TIME_STAMP=1"
        CX "-DTM_ENABLE_TIME_STAMP=1"
    )

endif()
