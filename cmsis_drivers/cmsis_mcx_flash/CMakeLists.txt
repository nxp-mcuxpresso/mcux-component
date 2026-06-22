# Copyright 2026 NXP
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_MCUX_COMPONENT_driver.cmsis_mcx_flash)
    mcux_component_version(1.0.0)

    mcux_add_source(SOURCES fsl_flash_cmsis.h fsl_flash_cmsis.c)

    mcux_add_include(INCLUDES .)

endif()
