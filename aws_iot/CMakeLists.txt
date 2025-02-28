#
# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(CONFIG_MCUX_COMPONENT_middleware.iot_reference.kvstore)
    mcux_add_source(
        SOURCES kvstore.c
                kvstore.h
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/kvstore/
    )
    mcux_add_include(
        INCLUDES .
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/kvstore/
    )
endif()

if(CONFIG_MCUX_COMPONENT_middleware.iot_reference.logging)
    mcux_add_source(
        SOURCES logging.c
                logging.h
                logging_levels.h
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/logging/
    )
    mcux_add_include(
        INCLUDES .
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/logging/
    )
endif()

if(CONFIG_MCUX_COMPONENT_middleware.iot_reference.mqtt_agent)
    mcux_add_source(
        SOURCES mqtt_agent_task.c
                mqtt_agent_task.h
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/mqtt_agent/
    )
    mcux_add_include(
        INCLUDES .
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/mqtt_agent/
    )
endif()

if(CONFIG_MCUX_COMPONENT_middleware.iot_reference.mqtt_agent_interface)
    mcux_add_source(
        SOURCES freertos_agent_message.c
                freertos_command_pool.c
                include/freertos_agent_message.h
                include/freertos_command_pool.h
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/mqtt-agent-interface/
    )
    mcux_add_include(
        INCLUDES include
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/mqtt-agent-interface/
    )
endif()

if(CONFIG_MCUX_COMPONENT_middleware.iot_reference.transport_mbedtls)
    mcux_add_source(
        SOURCES using_mbedtls.c
                using_mbedtls.h
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/using_mbedtls_pkcs11/
    )
    mcux_add_include(
        INCLUDES .
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/using_mbedtls_pkcs11/
    )
endif()

if(CONFIG_MCUX_COMPONENT_middleware.iot_reference.transport_mbedtls3x)
    mcux_add_source(
        SOURCES using_mbedtls.c
                using_mbedtls.h
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/using_mbedtls_pkcs11/
    )
    mcux_add_include(
        INCLUDES .
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/using_mbedtls_pkcs11/
    )
endif()

if(CONFIG_MCUX_COMPONENT_middleware.iot_reference.transport_mbedtls_psa)
    mcux_add_source(
        SOURCES using_mbedtls.c
                using_mbedtls.h
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/using_mbedtls_psa/
    )
    mcux_add_include(
        INCLUDES .
        BASE_PATH ${SdkRootDirPath}/components/aws_iot/using_mbedtls_psa/
    )
endif()
