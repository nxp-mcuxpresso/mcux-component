# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.gt911)
    mcux_add_source(
        SOURCES gt911/fsl_gt911.h
                gt911/fsl_gt911.c
    )
    mcux_add_include(
        INCLUDES ./gt911
    )

    mcux_add_configuration(
        CC "-DSDK_I2C_BASED_COMPONENT_USED=1"
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.ft6x06)
    mcux_add_source(
        SOURCES ft6x06/fsl_ft6x06.h
                ft6x06/fsl_ft6x06.c
    )
    mcux_add_include(
        INCLUDES ./ft6x06
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.ft5406)
    mcux_add_source(
        SOURCES ft5406/fsl_ft5406.h
                ft5406/fsl_ft5406.c
    )
    mcux_add_include(
        INCLUDES ./ft5406
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.ft5406_rt)
    mcux_add_source(
        SOURCES ft5406_rt/fsl_ft5406_rt.h
                ft5406_rt/fsl_ft5406_rt.c
    )
    mcux_add_include(
        INCLUDES ./ft5406_rt
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.ft3267)
    mcux_add_source(
        SOURCES ft3267/fsl_ft3267.h
                ft3267/fsl_ft3267.c
    )
    mcux_add_include(
        INCLUDES ./ft3267
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.ektf2k)
    mcux_add_source(
        SOURCES ektf2k/fsl_ektf2k.h
                ektf2k/fsl_ektf2k.c
    )
    mcux_add_include(
        INCLUDES ./ektf2k
    )

    mcux_add_configuration(
        CC "-DSDK_I2C_BASED_COMPONENT_USED=1"
    )
endif()


if (CONFIG_MCUX_COMPONENT_driver.tma525b)
    mcux_add_source(
        SOURCES tma525b/fsl_tma525b.h
                tma525b/fsl_tma525b.c
    )
    mcux_add_include(
        INCLUDES ./tma525b
    )
endif()
