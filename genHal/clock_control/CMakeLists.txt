
if (CONFIG_MCUX_COMPONENT_component.genHal.clock_control.mcux_syscon)
    mcux_component_version(2.3.0)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components
        SOURCES /genHal/clock_control/clock_control_mcux_syscon.c
                /zephyr_headers/zephyr/dt-bindings/clock/mcux_lpc_syscon_clock.h
    )
endif()
