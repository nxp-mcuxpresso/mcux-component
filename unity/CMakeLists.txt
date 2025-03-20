# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if(CONFIG_MCUX_COMPONENT_utilities.gcov)
    mcux_component_version(1.0.0)
    mcux_add_configuration(
        COMPILERS gcc
        CC  "-DSDK_USE_SYSCALL_STUB=0 -DGCOV_ERROR_FILE -DGCOV_EXIT_AT_ERROR -DUNITY_DUMP_RESULT -DUNITY_DUMP_CASE_RESULT_ONLY -DMCUX_SEGGER_SEMIHOSTING -DGCOV_DO_COVERAGE=1 -DUNIT_TEST_COMBINE -DNDEBUG"
    )
    mcux_add_armgcc_configuration(
        LD "-Xlinker --defsym=__stack_size__=0x1000 -Xlinker --defsym=__heap_size__=0x4000 --coverage"
    )

    mcux_add_source(
        SOURCES components/unity/gcov_support.c
                components/unity/gcov_support.h
                components/unity/gcov_func.c
                components/unity/gcov_func.h
                examples_int/unit_tests/sdmmc/tools/segger_semihosting.c
                examples_int/unit_tests/sdmmc/tools/segger_semihosting.h
        BASE_PATH ${SdkRootDirPath}/./
        COMPILERS gcc
    )
    mcux_add_include(
        INCLUDES examples_int/unit_tests/sdmmc/tools
        BASE_PATH ${SdkRootDirPath}/./
        COMPILERS gcc
    )
endif()

if(CONFIG_MCUX_COMPONENT_utilities.gcov.board_init)
    mcux_component_version(1.0.0)
    mcux_add_source(
        SOURCES examples/_boards/${board}/board_init_gcov.c
                examples/_boards/${board}/board_init_gcov.h
        BASE_PATH ${SdkRootDirPath}/./
        COMPILERS gcc
    )
    mcux_add_include(
        INCLUDES examples/_boards/${board}/
        BASE_PATH ${SdkRootDirPath}/./
        COMPILERS gcc
    )
endif()

if(CONFIG_MCUX_COMPONENT_utilities.unity)
    mcux_component_version(2.1.0)
    mcux_add_configuration(
        CC  "-DUNITY_CUSTOM_OUTPUT_CHAR"
    )

    mcux_add_source(
        SOURCES unity.c
                unity.h
                unity_internals.h
        BASE_PATH ${SdkRootDirPath}/components/unity/
    )
    mcux_add_include(
        INCLUDES .
        BASE_PATH ${SdkRootDirPath}/components/unity/
    )
endif()
