# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.dspi_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_dspi.h
                fsl_adapter_dspi.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.flexcomm_spi_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_spi.h
                fsl_adapter_flexcomm_spi.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.lpspi_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_spi.h
                fsl_adapter_lpspi.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.spi_adapter)

    mcux_add_source(
        SOURCES fsl_adapter_spi.h
                fsl_adapter_spi.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()
