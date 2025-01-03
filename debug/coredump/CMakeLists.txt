if (CONFIG_MCUX_COMPONENT_component.debug_coredump_backend.flash_partition)
    mcux_component_version(2.3.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        SOURCES
                ./coredump_core.c
                ./coredump_memory_regions.c
                ./coredump_backend_flash_partition.c
                ./coredump.c
                ./error_stack_frame.c
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/debug/coredump
        INCLUDES ./
    )
endif()











