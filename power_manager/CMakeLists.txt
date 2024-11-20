# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if ((${device} STREQUAL MCXN947) OR (${device} STREQUAL MCXN547) OR (${device} STREQUAL MCXN946) OR (${device} STREQUAL MCXN546))
    mcux_set_variable(pm_device_foldername MCXN94x)
endif()

if ((${device} STREQUAL MIMXRT1176) OR (${device} STREQUAL MIMXRT1175) OR (${device} STREQUAL MIMXRT1173) OR (${device} STREQUAL MIMXRT1172) OR (${device} STREQUAL MIMXRT1171))
    mcux_set_variable(pm_device_foldername RT1170)
endif()

if ((${device} STREQUAL KW45B41Z83) OR (${device} STREQUAL K32W1480) OR (${device} STREQUAL MCXW716C) 
    OR (${device} STREQUAL MCXW716A))
    mcux_set_variable(pm_device_foldername KW45B41Z83)
endif()

if ((${device} STREQUAL RW610) OR (${device} STREQUAL RW612))
    mcux_set_variable(pm_device_foldername RW612)
endif()

if ((${device} STREQUAL MIMXRT595S) OR (${device} STREQUAL MIMXRT555S) OR (${device} STREQUAL MIMXRT533S))
    mcux_set_variable(pm_device_foldername MIMXRT595S)
endif()

if ((${device} STREQUAL MIMXRT633S) OR (${device} STREQUAL MIMXRT685S))
    mcux_set_variable(pm_device_foldername MIMXRT685S)
endif()

if ((${device} STREQUAL MIMXRT735S) OR (${device} STREQUAL MIMXRT758S) OR (${device} STREQUAL MIMXRT798S))
    mcux_set_variable(pm_device_foldername MIMXRT798S)
endif()

if ((${device} STREQUAL KW47B42ZB7) OR (${device} STREQUAL KW47B42Z83) OR (${device} STREQUAL KW47B42Z96) 
    OR (${device} STREQUAL KW47B42Z97) OR (${device} STREQUAL KW47B42ZB2) OR (${device} STREQUAL KW47B42ZB3) 
    OR (${device} STREQUAL KW47B42ZB6) OR (${device} STREQUAL KW47Z42082) OR (${device} STREQUAL KW47Z42092) 
    OR (${device} STREQUAL KW47Z420B2) OR (${device} STREQUAL KW47Z420B3) OR (${device} STREQUAL MCXW727C)
    OR (${device} STREQUAL MCXW727A) OR (${device} STREQUAL MCXW727D))
    mcux_set_variable(pm_device_foldername KW47B42ZB7)
endif()

if (${device} STREQUAL MCXA276)
    mcux_set_variable(pm_device_foldername MCXA276)
endif()

if (CONFIG_MCUX_COMPONENT_component.power_manager_framework)

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/power_manager/core
        SOURCES fsl_pm_core.h
                fsl_pm_core.c
                fsl_pm_config.h
    )

    mcux_add_include(
        INCLUDES ./core
    )

    mcux_add_source(
        BASE_PATH ${SdkRootDirPath}/components/power_manager/devices/${pm_device_foldername}
        SOURCES fsl_pm_device.c
                fsl_pm_device.h
                fsl_pm_device_config.h
    )

    mcux_add_include(
        INCLUDES ./devices/${pm_device_foldername}
    )
    mcux_add_configuration(
        CC "-DGENERIC_LIST_LIGHT=1"
    )
endif()
