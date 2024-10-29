# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.ili9341)

    mcux_add_source(
        SOURCES ili9341/fsl_ili9341.h
                ili9341/fsl_ili9341.c
    )

    mcux_add_include(
        INCLUDES ./ili9341
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.st7796s)

    mcux_add_source(
        SOURCES st7796s/fsl_st7796s.h
                st7796s/fsl_st7796s.c
    )

    mcux_add_include(
        INCLUDES ./st7796s
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.ssd1963)

    mcux_add_source(
        SOURCES ssd1963/fsl_ssd1963.h
                ssd1963/fsl_ssd1963.c
    )

    mcux_add_include(
        INCLUDES ./ssd1963
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.psp27801)

    mcux_add_source(
        SOURCES psp27801/fsl_psp27801.h
                psp27801/fsl_psp27801.c
    )

    mcux_add_include(
        INCLUDES ./psp27801
    )

endif()
