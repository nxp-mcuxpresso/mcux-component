# Copyright 2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_utility.semihost)

    mcux_add_source(
        SOURCES fsl_semihost.h
                fsl_semihost.c
    )

    mcux_add_include(
        INCLUDES .
    )

endif()

if (CONFIG_MCUX_COMPONENT_utility.semihost_console)

    mcux_add_source(
        SOURCES console/semihost_console.c
    )

endif()

if (CONFIG_MCUX_COMPONENT_utility.semihost_file)

    mcux_add_source(
        SOURCES file/semihost_file.c
    )

endif()

if (CONFIG_MCUX_COMPONENT_utility.debug_console_semihost)

    mcux_add_source(
        SOURCES debug_console/fsl_debug_console.c
                debug_console/fsl_debug_console.h
    )

    # Isolate this fsl_debug_console.h in its own subdir; only on -I when the
    # backend is selected, so it never collides with the UART debug_console's
    # header on projects that also enable the base semihost unit.
    mcux_add_include(
        INCLUDES debug_console
    )

endif()
