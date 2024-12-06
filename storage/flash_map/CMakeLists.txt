
if (CONFIG_MCUX_COMPONENT_component.storage_flash_map)
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/storage/flash_map
        SOURCES
                ./flash_map.c
                ./flash_map_default.c
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/storage
        INCLUDES 
                 flash_map/.
    )
endif()

# if (NOT CONFIG_FLASH_MAP_CUSTOM)
#     mcux_add_source(
#         BASE_PATH ${SdkRootDirPath}/components/storage/flash_map
#         SOURCES
#                 ./flash_map_default.c
#     )
# endif()

# if (CONFIG_FLASH_MAP_SHELL)
#     mcux_add_source(
#         BASE_PATH ${SdkRootDirPath}/components/storage/flash_map
#         SOURCES
#                 ./flash_map_shell.c
#     )
# endif()

# if (CONFIG_FLASH_PAGE_LAYOUT)
#     mcux_add_source(
#         BASE_PATH ${SdkRootDirPath}/components/storage/flash_map
#         SOURCES
#                 ./flash_map_layout.c
#     )
# endif()

# if (CONFIG_FLASH_AREA_CHECK_INTEGRITY)
#     mcux_add_source(
#         BASE_PATH ${SdkRootDirPath}/components/storage/flash_map
#         SOURCES
#                 ./flash_map_integrity.c
#     )
# endif()

