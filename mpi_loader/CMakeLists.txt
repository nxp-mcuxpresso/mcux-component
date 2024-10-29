# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.mpi_loader)
    mcux_add_source(
        SOURCES fsl_mpi_loader.h
                fsl_mpi_loader.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
