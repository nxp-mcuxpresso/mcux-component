if (CONFIG_MCUX_PRJSEG_project.hw_app_project_folder)
    mcux_add_include(
        INCLUDES ./
    )

    mcux_add_source(
        SOURCES
            ./devicetree_generated.h
    )
endif()
