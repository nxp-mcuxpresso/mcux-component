# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause

if (CONFIG_MCUX_COMPONENT_driver.srtm)
    mcux_add_source(
        SOURCES include/srtm_channel.h
                include/srtm_defs.h
                include/srtm_dispatcher.h
                include/srtm_message.h
                include/srtm_peercore.h
                include/srtm_service.h
                srtm/srtm_channel.c
                srtm/srtm_channel_struct.h
                srtm/srtm_dispatcher.c
                srtm/srtm_dispatcher_struct.h
                srtm/srtm_message.c
                srtm/srtm_message_struct.h
                srtm/srtm_peercore.c
                srtm/srtm_peercore_struct.h
                srtm/srtm_service.c
                srtm/srtm_service_struct.h
                srtm/srtm_heap.h
                srtm/srtm_message_pool.h
                srtm/srtm_list.h
                srtm/srtm_mutex.h
                srtm/srtm_sem.h
                port/srtm_message_pool.c
    )
    mcux_add_include(
        INCLUDES include
                 srtm
                 port
    )
endif()

if (CONFIG_MCUX_COMPONENT_driver.srtm.freertos)
    mcux_add_source(
        SOURCES port/srtm_heap_freertos.c
                port/srtm_mutex_freertos.c
                port/srtm_sem_freertos.c
    )
endif()

mcux_add_cmakelists(${SdkRootDirPath}/components/srtm/channels)
mcux_add_cmakelists(${SdkRootDirPath}/components/srtm/services)