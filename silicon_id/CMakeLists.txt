# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.silicon_id)
    mcux_add_include(
        INCLUDES
            .
    )

    mcux_add_source(
        SOURCES
            fsl_silicon_id.h
            fsl_silicon_id.c
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.silicon_id_imxrt10xx)
    mcux_add_include(
        INCLUDES
            ./socs
    )

    mcux_add_source(
        SOURCES
            ./socs/rt10xx/fsl_silicon_id_soc.c
            ./socs/fsl_silicon_id_soc.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.silicon_id_imxrt1170)
    mcux_add_include(
        INCLUDES
            ./socs
    )

    mcux_add_source(
        SOURCES
            ./socs/imxrt1170/fsl_silicon_id_soc.c
            ./socs/fsl_silicon_id_soc.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.silicon_id_imxrt1180)
    mcux_add_include(
        INCLUDES
            ./socs
    )

    mcux_add_source(
        SOURCES
            ./socs/imxrt1180/fsl_silicon_id_soc.c
            ./socs/fsl_silicon_id_soc.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.silicon_id_imx8)
    mcux_add_include(
        INCLUDES
            ./socs
    )

    mcux_add_source(
        SOURCES
            ./socs/imx8/fsl_silicon_id_soc.c
            ./socs/fsl_silicon_id_soc.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.silicon_id_imx93)
    mcux_add_include(
        INCLUDES
            ./socs
    )

    mcux_add_source(
        SOURCES
            ./socs/imx93/fsl_silicon_id_soc.c
            ./socs/fsl_silicon_id_soc.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.silicon_id_scfw)
    mcux_add_include(
        INCLUDES
            ./socs
    )

    mcux_add_source(
        SOURCES
            ./socs/scfw/fsl_silicon_id_soc.c
            ./socs/fsl_silicon_id_soc.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.silicon_id_rw610)
    mcux_add_include(
        INCLUDES
            ./socs
    )

    mcux_add_source(
        SOURCES
            ./socs/rw610/fsl_silicon_id_soc.c
            ./socs/fsl_silicon_id_soc.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.silicon_id_mcxn)
    mcux_add_include(
        INCLUDES
            ./socs
    )

    mcux_add_source(
        SOURCES
            ./socs/mcxn/fsl_silicon_id_soc.c
            ./socs/fsl_silicon_id_soc.h
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.silicon_id_mcxe31x)
    mcux_add_include(
        INCLUDES
            ./socs
    )

    mcux_add_source(
        SOURCES
            ./socs/mcxe31x/fsl_silicon_id_soc.c
            ./socs/fsl_silicon_id_soc.h
    )
endif()
