# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.log)

    mcux_add_source(
        SOURCES fsl_component_log.h
                fsl_component_log.c
                fsl_component_log_config.h
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.log.backend.debug_console)

    mcux_add_source(
        SOURCES fsl_component_log_backend_debugconsole.h
                fsl_component_log_backend_debugconsole.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()
if (CONFIG_MCUX_COMPONENT_component.log.backend.debug_console_lite)

    mcux_add_source(
        SOURCES fsl_component_log_backend_debugconsole.h
                fsl_component_log_backend_debugconsole.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_component.log.backend.ringbuffer)

    mcux_add_source(
        SOURCES fsl_component_log_backend_ringbuffer.h
                fsl_component_log_backend_ringbuffer.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()