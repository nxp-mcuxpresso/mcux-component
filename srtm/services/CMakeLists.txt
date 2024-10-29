# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_component.srtm_i2c_codec_adapter)
    mcux_add_source(
        SOURCES srtm_i2c_codec_adapter.h
                srtm_i2c_codec_adapter.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_pdm_edma_adapter)
    mcux_add_source(
        SOURCES srtm_pdm_edma_adapter.h
                srtm_pdm_edma_adapter.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_pdm_sdma_adapter)
    mcux_add_source(
        SOURCES srtm_pdm_sdma_adapter.h
                srtm_pdm_sdma_adapter.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_pf1550_adapter)
    mcux_add_source(
        SOURCES srtm_pf1550_adapter.h
                srtm_pf1550_adapter.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_pwm_adapter)
    mcux_add_source(
        SOURCES srtm_pwm_adapter.h
                srtm_pwm_adapter.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_rtc_adapter)
    mcux_add_source(
        SOURCES srtm_rtc_adapter.h
                srtm_rtc_adapter.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_sai_edma_adapter)
    mcux_add_source(
        SOURCES srtm_sai_edma_adapter.h
                srtm_sai_edma_adapter.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_sai_sdma_adapter)
    mcux_add_source(
        SOURCES srtm_sai_sdma_adapter.h
                srtm_sai_sdma_adapter.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_wm8524_adapter)
    mcux_add_source(
        SOURCES srtm_wm8524_adapter.h
                srtm_wm8524_adapter.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_i2c_service)
    mcux_add_source(
        SOURCES srtm_i2c_service.h
                srtm_i2c_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_audio_service)
    mcux_add_source(
        SOURCES srtm_audio_service.h
                srtm_audio_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_auto_service)
    mcux_add_source(
        SOURCES srtm_auto_service.h
                srtm_auto_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_io_service)
    mcux_add_source(
        SOURCES srtm_io_service.h
                srtm_io_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_keypad_service)
    mcux_add_source(
        SOURCES srtm_keypad_service.h
                srtm_keypad_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_lfcl_service)
    mcux_add_source(
        SOURCES srtm_lfcl_service.h
                srtm_lfcl_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_netc_service)
    mcux_add_source(
        SOURCES srtm_netc_service.h
                srtm_netc_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_pmic_service)
    mcux_add_source(
        SOURCES srtm_pmic_service.h
                srtm_pmic_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_pwm_service)
    mcux_add_source(
        SOURCES srtm_pwm_service.h
                srtm_pwm_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_rtc_service)
    mcux_add_source(
        SOURCES srtm_rtc_service.h
                srtm_rtc_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_sensor_service)
    mcux_add_source(
        SOURCES srtm_sensor_service.h
                srtm_sensor_service.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()

if (CONFIG_MCUX_COMPONENT_component.srtm_snvs_lp_rtc_adapter)
    mcux_add_source(
        SOURCES srtm_snvs_lp_rtc_adapter.h
                srtm_snvs_lp_rtc_adapter.c
    )
    mcux_add_include(
        INCLUDES .
    )
endif()
