# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.Legacy_flash_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_flash.h
                fsl_adapter_flash.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.JNflash_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_flash.h
                fsl_adapter_jn_flash.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flashiap_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_flash.h
                fsl_adapter_flashiap.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.k4_flash_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_flash.h
                fsl_adapter_k4_flash.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_nor_flash_adapter_rt1060)
    mcux_add_source(
        SOURCES fsl_adapter_flexspi_nor_flash.c
                fsl_adapter_flash.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.mcxw_flash_adapter)
    mcux_add_source(
        SOURCES fsl_adapter_flash.h
                fsl_adapter_mcxw_flash.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_nor_flash_adapter_rt1060evkb)
    mcux_add_source(
        SOURCES evkbmimxrt1060/fsl_adapter_flexspi_nor_flash.c
                fsl_adapter_flash.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_nor_flash_adapter_rt1170)
    mcux_add_source(
        SOURCES mimxrt1170/fsl_adapter_flexspi_nor_flash.c
                fsl_adapter_flash.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_nor_flash_adapter_rt1170evkb)
    mcux_add_source(
        SOURCES evkbmimxrt1170/fsl_adapter_flexspi_nor_flash.c
                fsl_adapter_flash.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_nor_flash_adapter_rt685evk)
    mcux_add_source(
        SOURCES octal_flash/fsl_adapter_flexspi_nor_flash.c
                fsl_adapter_flash.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_nor_flash_adapter_rt595evk)
    mcux_add_source(
        SOURCES octal_flash/RT595/fsl_adapter_flexspi_nor_flash.c
                fsl_adapter_flash.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_nor_flash_adapter_rt1040evk)
    mcux_add_source(
        SOURCES evkmimxrt1040/fsl_adapter_flexspi_nor_flash.c
                fsl_adapter_flash.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_hyper_flash_adapter_rt1050)
    mcux_add_source(
        SOURCES fsl_adapter_flash.h
                hyper_flash/fsl_adapter_flexspi_hyper_nor_flash.c
                hyper_flash/RT1050/fsl_adapter_flexspi_hyper_flash_config.c
                hyper_flash/RT1050/fsl_adapter_flexspi_hyper_flash_config.h
    )
    mcux_add_include(
        INCLUDES ./
                 ./hyper_flash/RT1050
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_nor_flash_adapter_rw61x)
    mcux_add_source(
        SOURCES rdrw612bga/fsl_adapter_flexspi_nor_flash.c
                fsl_adapter_flash.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_nor_flash_adapter_rt1020)
    mcux_add_source(
        SOURCES fsl_adapter_flash.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.flexspi_nor_flash_adapter_rt1160)
    mcux_add_source(
        SOURCES mimxrt1160/fsl_adapter_flexspi_nor_flash.c
                fsl_adapter_flash.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()