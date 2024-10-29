# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.pca6416a)
    mcux_add_include(
        INCLUDES ./pca6416a
    )

    mcux_add_source(
        SOURCES ./pca6416a/fsl_pca6416a.c
                ./pca6416a/fsl_pca6416a.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.pcal6408)
    mcux_add_include(
        INCLUDES ./pcal6408
    )

    mcux_add_source(
        SOURCES ./pcal6408/fsl_pcal6408.c
                ./pcal6408/fsl_pcal6408.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.pcal6524)
    mcux_add_include(
        INCLUDES ./pcal6524
    )

    mcux_add_source(
        SOURCES ./pcal6524/fsl_pcal6524.c
                ./pcal6524/fsl_pcal6524.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.adp5585)
    mcux_add_include(
        INCLUDES ./adp5585
    )

    mcux_add_source(
        SOURCES ./adp5585/fsl_adp5585.h
                ./adp5585/fsl_adp5585.c
    )

endif()
