# Copyright 2025-2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.mflash_onchip)
    mcux_add_include(
        BASE_PATH ${SdkRootDirPath}/components/flash/mflash
        INCLUDES
            .
            ./frdmmcxc162
    )
    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/flash/mflash
        SOURCES
            mflash_common.h
            mflash_file.c
            mflash_file.h
            ./frdmmcxc162/mflash_drv.c
            ./frdmmcxc162/mflash_drv.h
    )
    # Use the last flash sector for mflash file storage
    if (${board} STREQUAL frdmmcxc353)
        mcux_add_macro(
            "-DMFLASH_FILE_BASEADDR=122880"
        )
    else()
        mcux_add_macro(
            "-DMFLASH_FILE_BASEADDR=57344"
        )
    endif()
endif()
