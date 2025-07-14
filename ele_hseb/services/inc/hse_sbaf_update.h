/*
 * hse_sbaf_update.h
*
 * @brief Update the SBAF via demo APP
 * @description API for updating SBAF via Demo APp
 */


/*==================================================================================================
*
*   Copyright 2022 NXP.
*
*   This software is owned or controlled by NXP and may only be used strictly in accordance with
*   the applicable license terms. By expressly accepting such terms or by downloading, installing,
*   activating and/or otherwise using the software, you are agreeing that you have read, and that
*   you agree to comply with and are bound by, such license terms. If you do not agree to
*   be bound by the applicable license terms, then you may not retain, install, activate or
*   otherwise use the software.
==================================================================================================*/



#ifndef SERVICES_SRC_SBAF_UPDATE_SBAF_UPDATE_H_
#define SERVICES_SRC_SBAF_UPDATE_SBAF_UPDATE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hse_srv_sbaf_update.h"


#include "hse_host_attrs.h"
#include "hse_host_boot.h"
#include "hse_demo_app_services.h"
#include "hse_mu.h"
#include "hse_host.h"

hseSrvResponse_t sbaf_update(void);


#ifdef __cplusplus
}

#endif

#endif /* SERVICES_SRC_SBAF_UPDATE_SBAF_UPDATE_H_ */

/** @} */
