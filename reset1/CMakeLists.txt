
if (CONFIG_MCUX_COMPONENT_component.hal_reset)
    mcux_add_source(
        SOURCES hal_reset.h
                # TODO please change to relative dir
                hal_reset.c
    )
    mcux_add_include(
        INCLUDES ./
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.hal_reset.imx943)
    mcux_add_source(
        SOURCES porting/platform/imx943/hal_reset_platform.h
                porting/platform/imx943/hal_reset_platform.c
    )
    mcux_add_include(
        INCLUDES porting/platform/imx943
    )
endif()
