if (CONFIG_MCUX_PRJSEG_project.hw_app_project_folder)
    mcux_add_include(
        INCLUDES ./
    )

    mcux_add_source(
        SOURCES
            ./pinctrl_defs.h
            ./pinctrl_soc.h
            ./flexspi_clock_setup.c
            ./flexspi_clock_setup.h
            ./soc.h
    )
endif()
