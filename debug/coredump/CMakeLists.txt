# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause


if (CONFIG_MCUX_COMPONENT_component.debug_coredump_backend.flash_partition)
    mcux_component_version(1.0.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        SOURCES
                ./coredump_core.c
                ./coredump_memory_regions.c
                ./coredump_backend_flash_partition.c
                ./coredump.c
                ./error_stack_frame.c
                ./coredump_internal.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.debug_coredump_backend.mflash)
    mcux_component_version(1.0.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        SOURCES
                ./coredump_core.c
                ./coredump_memory_regions.c
                ./coredump_backend_mflash.c
                ./coredump.c
                ./error_stack_frame.c
                ./coredump_internal.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.debug_thread_info)
    mcux_component_version(1.0.0)
    
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        SOURCES
            ./thread_info.c
            ./FreeRTOSConfig_debug_coredump.h
    )

endif()
