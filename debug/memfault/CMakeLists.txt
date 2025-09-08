# Copyright 2024-2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.memfault_sdk)

    mcux_add_armgcc_configuration(
        # For the compile warning:
        # cc1: warning: variable tracking requested, but useless unless producing debug info
        #
        # Debug info is needed when debugging the fault, so should always generate it,
        # otherwise there is compile warning.
        CC "-g"
    )

    mcux_add_iar_configuration(
        # Suppress such warnings in memfault SDK code:
        # Pe188: enumerated type mixed with another type
        # Pe1305: function declared with "noreturn" does return
        # Pe128: loop is not reachable
        CC "--diag_suppress=Pe188,Pe1305,Pe128"
    )

    mcux_add_include(
        INCLUDES ./memfault-firmware-sdk/components/include
                 ./memfault-firmware-sdk/ports/include
    )

    mcux_add_source(
        SOURCES ./memfault-firmware-sdk/components/core/src/*.c
                ./memfault-firmware-sdk/components/core/src/*.h
                ./memfault-firmware-sdk/components/panics/src/*.c
                ./memfault-firmware-sdk/components/panics/src/*.h
                ./memfault-firmware-sdk/components/util/src/*.c
                ./memfault-firmware-sdk/components/util/src/*.h
                ./memfault-firmware-sdk/components/include/*.h
                ./memfault-firmware-sdk/components/include/memfault/*.h
                ./memfault-firmware-sdk/components/include/memfault/core/*.h
                ./memfault-firmware-sdk/components/include/memfault/core/platform/*.h
                ./memfault-firmware-sdk/components/include/memfault/panics/*.h
                ./memfault-firmware-sdk/components/include/memfault/panics/platform/*.h
                ./memfault-firmware-sdk/components/include/memfault/panics/arch/*.h
                ./memfault-firmware-sdk/components/include/memfault/panics/arch/arm/*.h
                ./memfault-firmware-sdk/components/include/memfault/panics/arch/riscv/*.h
                ./memfault-firmware-sdk/components/include/memfault/panics/arch/xtensa/*.h
                ./memfault-firmware-sdk/components/include/memfault/http/*.h
                ./memfault-firmware-sdk/components/include/memfault/http/platform/*.h
                ./memfault-firmware-sdk/components/include/memfault/metrics/*.h
                ./memfault-firmware-sdk/components/include/memfault/metrics/platform/*.h
                ./memfault-firmware-sdk/components/include/memfault/demo/*.h
                ./memfault-firmware-sdk/components/include/memfault/util/*.h
                ./memfault-firmware-sdk/ports/include/memfault/ports/*.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.memfault_sdk.http)
    mcux_add_source(
        SOURCES ./memfault-firmware-sdk/components/http/src/*.c
                ./memfault-firmware-sdk/components/http/src/*.h
                ./memfault-firmware-sdk/components/include/memfault/http/*.h
                ./memfault-firmware-sdk/components/include/memfault/http/platform/*.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.memfault_sdk.metrics)
    mcux_add_source(
        SOURCES ./memfault-firmware-sdk/components/metrics/src/*.c
                ./memfault-firmware-sdk/components/metrics/src/*.h
                ./memfault-firmware-sdk/components/include/memfault/metrics/*.h
                ./memfault-firmware-sdk/components/include/memfault/metrics/*.def
                ./memfault-firmware-sdk/components/include/memfault/metrics/platform/*.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.memfault_sdk.ports.freertos)
    mcux_add_include(
        INCLUDES ./memfault-firmware-sdk/ports/include
                 ./memfault-firmware-sdk
    )

    mcux_add_source(
        SOURCES ./memfault-firmware-sdk/ports/freertos/src/*.c
                ./memfault-firmware-sdk/ports/freertos/config/memfault_metrics_heartbeat_freertos_config.def
                ./memfault-firmware-sdk/ports/include/memfault/ports/freertos/*.h
                ./memfault-firmware-sdk/ports/include/memfault/ports/freertos.h
                ./memfault-firmware-sdk/ports/include/memfault/ports/freertos_coredump.h
                ./memfault-firmware-sdk/ports/include/memfault/ports/freertos_trace.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.memfault_sdk.ports.mcuxsdk)
    mcux_add_include(
        INCLUDES ./sdk_port/include
    )

    mcux_add_source(
        SOURCES ./sdk_port/src/*.c
                ./sdk_port/include/*.h
                ./sdk_port/include/*.def
    )
endif()
