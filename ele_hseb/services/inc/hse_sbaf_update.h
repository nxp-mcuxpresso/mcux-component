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
*   License: LA_OPT_Online Code Hosting NXP_Software_License - v1.4 May 2025
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
