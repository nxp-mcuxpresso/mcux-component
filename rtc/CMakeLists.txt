# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.rtc_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_rtc.h
                fsl_adapter_rtc.c
    )

    mcux_add_include(
        INCLUDES ./
    )
	
    mcux_add_configuration(
        AS "-DRTC_LEGACY_FUNCTION_PROTOTYPE=1"
        CC "-DRTC_LEGACY_FUNCTION_PROTOTYPE=1"
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.lpcrtc)

    mcux_add_source(
        SOURCES fsl_adapter_rtc.h
                fsl_adapter_lpcrtc.c
    )

    mcux_add_include(
        INCLUDES ./
    )

    mcux_add_configuration(
        AS "-DRTC_LEGACY_FUNCTION_PROTOTYPE=1"
        CC "-DRTC_LEGACY_FUNCTION_PROTOTYPE=1"
    )
	
endif()

if (CONFIG_MCUX_COMPONENT_component.rtc_bbnsm_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_rtc.h
                fsl_adapter_rtc_bbnsm.c
    )

    mcux_add_include(
        INCLUDES ./
    )

    mcux_add_configuration(
        AS "-DRTC_LEGACY_FUNCTION_PROTOTYPE=0"
        CC "-DRTC_LEGACY_FUNCTION_PROTOTYPE=0"
    )
endif()
