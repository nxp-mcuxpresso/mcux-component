# Copyright 2024-2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.fxas21002cq)

    mcux_add_source(
        SOURCES ./fxas21002cq/fsl_fxas.h
                ./fxas21002cq/fsl_fxas.c
    )

    mcux_add_include(
        INCLUDES ./fxas21002cq
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.fxls8974cf)

    mcux_add_source(
        SOURCES ./fxls8974cf/fsl_fxls.h
                ./fxls8974cf/fsl_fxls.c
    )

    mcux_add_include(
        INCLUDES ./fxls8974cf
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.fxos8700cq)

    mcux_add_source(
        SOURCES ./fxos8700cq/fsl_fxos.h
                ./fxos8700cq/fsl_fxos.c
    )

    mcux_add_include(
        INCLUDES ./fxos8700cq
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.htu21d)

    mcux_add_source(
        SOURCES ./htu21d/fsl_htu21d.h
                ./htu21d/fsl_htu21d.c
    )

    mcux_add_include(
        INCLUDES ./htu21d
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.icm42688p)

    mcux_add_source(
        SOURCES ./icm42688p/fsl_icm42688p.h
                ./icm42688p/fsl_icm42688p.c
    )

    mcux_add_include(
        INCLUDES ./icm42688p
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.max30101)

    mcux_add_source(
        SOURCES ./max30101/fsl_max.h
                ./max30101/fsl_max.c
    )

    mcux_add_include(
        INCLUDES ./max30101
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.mma8451q)

    mcux_add_source(
        SOURCES ./mma8451q/fsl_mma.h
                ./mma8451q/fsl_mma.c
    )

    mcux_add_include(
        INCLUDES ./mma8451q
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.mma8652fc)

    mcux_add_source(
        SOURCES ./mma8652fc/fsl_mma.h
                ./mma8652fc/fsl_mma.c
    )

    mcux_add_include(
        INCLUDES ./mma8652fc
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.mpl3115a2)

    mcux_add_source(
        SOURCES ./mpl3115a2/fsl_mpl3115a2.h
                ./mpl3115a2/fsl_mpl3115a2.c
    )

    mcux_add_include(
        INCLUDES ./mpl3115a2
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.nmh1000)

    mcux_add_source(
        SOURCES ./nmh1000/fsl_nmh1000.h
                ./nmh1000/fsl_nmh1000.c
    )

    mcux_add_include(
        INCLUDES ./nmh1000
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.p3t1755)

    mcux_add_source(
        SOURCES ./p3t1755/fsl_p3t1755.h
                ./p3t1755/fsl_p3t1755.c
    )

    mcux_add_include(
        INCLUDES ./p3t1755
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.tsl2561)

    mcux_add_source(
        SOURCES ./tsl2561/fsl_tsl2561.h
                ./tsl2561/fsl_tsl2561.c
    )

    mcux_add_include(
        INCLUDES ./tsl2561
    )

endif()

if (CONFIG_MCUX_COMPONENT_driver.lsm6dso)

    mcux_add_source(
        SOURCES ./lsm6dso/fsl_lsm.h
                ./lsm6dso/fsl_lsm.c
    )

    mcux_add_include(
        INCLUDES ./lsm6dso
    )

endif()
