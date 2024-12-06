if (CONFIG_MCUX_COMPONENT_component.storage_stream)
    mcux_component_version(2.3.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/storage/stream
        SOURCES
                ./stream_flash.c
                ./stream_flash.h
    )

    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/storage
        INCLUDES stream/.
    )
endif()
