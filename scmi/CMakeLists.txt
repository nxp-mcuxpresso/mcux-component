# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.sm.scmi)
    mcux_add_source(
        SOURCES scmi_common.h
                scmi.c
                scmi.h
                scmi_clock.c
                scmi_clock.h
                scmi_base.c
                scmi_base.h
                scmi_bbm.c
                scmi_bbm.h
                scmi_cpu.c
                scmi_cpu.h
                scmi_fusa.c
                scmi_fusa.h
                scmi_internal.h
                scmi_lmm.c
                scmi_lmm.h
                scmi_misc.c
                scmi_misc.h
                scmi_perf.c
                scmi_perf.h
                scmi_pinctrl.c
                scmi_pinctrl.h
                scmi_power.c
                scmi_power.h
                scmi_reset.c
                scmi_reset.h
                scmi_sensor.c
                scmi_sensor.h
                scmi_sys.c
                scmi_sys.h
                scmi_voltage.c
                scmi_voltage.h
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()
