# Copyright 2025 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.coremark)
    mcux_add_source(
        SOURCES core_list_join.c
                core_main.c
                core_matrix.c
                core_state.c
                core_util.c
                coremark.h
    )
    mcux_add_include(
        INCLUDES ./
    )
    mcux_add_source(
        SOURCES porting/${soc_series}/core_portme.c
        SOURCES porting/${soc_series}/core_portme.h
    )
    mcux_add_include(
        INCLUDES ./porting/${soc_series}
    )
endif()
