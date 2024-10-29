# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_utility.assert_lite)

    mcux_add_source(
        SOURCES fsl_assert.h
                fsl_assert.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_utility.assert)

    mcux_add_source(
        SOURCES fsl_assert.h
                fsl_assert.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_utility.assert_lite_min)

    mcux_add_source(
        SOURCES fsl_assert_min.c
    )

endif()