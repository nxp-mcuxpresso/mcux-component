# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_utility.debug_console)

    mcux_add_source(
        SOURCES fsl_debug_console.h
                fsl_debug_console.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()
if (CONFIG_MCUX_COMPONENT_utility.ram_console)

    mcux_add_source(
        SOURCES fsl_debug_console.h
                fsl_ram_console.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()
if (CONFIG_MCUX_PRJSEG_utility.debug_console_transfer_non_blocking)

    mcux_add_configuration(
        AS "-DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING"
        CC "-DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING"
        CX "-DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING"
    )
endif()

if (CONFIG_MCUX_COMPONENT_utility.debug_console_template_config)

	mcux_add_source(
		SOURCES config/fsl_debug_console_conf.h
		CONFIG TRUE
	)
	
    mcux_add_include(
        INCLUDES config
        TARGET_FILES fsl_debug_console_conf.h
    )
endif()

if (CONFIG_MCUX_PRJSEG_utility.debug_console.generated_config)
endif ()