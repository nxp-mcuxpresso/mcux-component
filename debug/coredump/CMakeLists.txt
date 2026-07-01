# Copyright 2024-2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause


if (CONFIG_MCUX_COMPONENT_component.debug_coredump)
    mcux_component_version(1.0.0)

    # Coredump core (backend-agnostic dispatcher, memory-region descriptors,
    # public coredump() entry point, and the Cortex-M fault ESF builder). These
    # are required by every backend, including out-of-tree BACKEND_OTHER
    # implementations (e.g. the wireless board_debug backend) that select
    # component.debug_coredump without selecting mflash or flash_partition.
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        SOURCES
                ./coredump_core.c
                ./coredump_memory_regions.c
                ./coredump.c
                ./error_stack_frame.c
                ./coredump_internal.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        INCLUDES ./
    )

    mcux_add_configuration(
        # Suppress such warnings in gen_hal SDK code:
        # Pe111: statement is unreachable
        # Pe021: type qualifiers are meaningless in this declaration
        # Pe1153: function 'xxx' declared implicitly
        # Pe223: function 'xxx' declared implicitly

        TOOLCHAINS iar
        CC "--diag_suppress=Pe111,Pe021,Pe1153,Pe223"
        CX "--diag_suppress=Pe111,Pe021,Pe1153,Pe223"
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.debug_coredump_backend.flash_partition)
    mcux_component_version(1.0.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        SOURCES
                ./coredump_backend_flash_partition.c
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.debug_coredump_backend.mflash)
    mcux_component_version(1.0.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        SOURCES
                ./coredump_backend_mflash.c
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
