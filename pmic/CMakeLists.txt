# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.pca9420)
    mcux_add_configuration(
        CC "-DSDK_I2C_BASED_COMPONENT_USED=1"
    )

    mcux_add_include(
        INCLUDES ./pca9420
    )

    mcux_add_source(
        SOURCES ./pca9420/fsl_pca9420.c
                ./pca9420/fsl_pca9420.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.pca9422)
    mcux_add_configuration(
        CC "-DSDK_I2C_BASED_COMPONENT_USED=1"
    )

    mcux_add_include(
        INCLUDES ./pca9422
    )

    mcux_add_source(
        SOURCES ./pca9422/fsl_pca9422.c
                ./pca9422/fsl_pca9422.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.pf1550)
    mcux_add_include(
        INCLUDES ./pf1550
    )

    mcux_add_source(
        SOURCES ./pf1550/fsl_pf1550.c
                ./pf1550/fsl_pf1550.h
                ./pf1550/fsl_pf1550_charger.c
                ./pf1550/fsl_pf1550_charger.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.pf3000)
    mcux_add_include(
        INCLUDES ./pf3000
    )

    mcux_add_source(
        SOURCES ./pf3000/fsl_pf3000.c
                ./pf3000/fsl_pf3000.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.pf5020)
    mcux_add_include(
        INCLUDES ./pf5020
    )

    mcux_add_source(
        SOURCES ./pf5020/fsl_pf5020.c
                ./pf5020/fsl_pf5020.h
    )
endif()
