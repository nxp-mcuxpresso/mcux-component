/*
 * @file sbaf_update.c
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


#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
*                                        INCLUDE FILES
* ================================================================================================*/
#include "hse_sbaf_update.h"
#include <string.h>

/*==================================================================================================
 *                                       GLOBAL FUNCTIONS
 * ================================================================================================*/
hseSrvResponse_t sbaf_update(void)
{
    hseSrvResponse_t hseStatus = HSE_SRV_RSP_GENERAL_ERROR;
    const uint32_t newSbafFwAddress = 0x00422000UL;
    hseSbafUpdateSrv_t *psbafUpdateSrv;
    hseSrvDescriptor_t *pHseSrvDesc;
    uint8_t u8MuChannel;

    /* Get a free channel on u8MuInstance */
    u8MuChannel = HSE_MU_GetFreeChannel(MU0);
    if (HSE_INVALID_CHANNEL == u8MuChannel)
    {
        goto exit;
    }

    pHseSrvDesc = &gHseSrvDesc[MU0][u8MuChannel];
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    pHseSrvDesc->srvId = HSE_SRV_ID_SBAF_UPDATE;
    psbafUpdateSrv = &(pHseSrvDesc->hseSrv.sbafUpdateReq);
    psbafUpdateSrv->pInFwFile = (HOST_ADDR)newSbafFwAddress;

    hseStatus = HSE_Send(MU0, u8MuChannel, gSyncTxOption, pHseSrvDesc);

exit:
    return hseStatus;
}

#ifdef __cplusplus
}

#endif

/** @} */
