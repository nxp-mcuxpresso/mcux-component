/**
 *   @file    hse_fuses.c
 *
 *   @brief   Examples of HSE UTEST  programming.
 *
 *   @addtogroup [hse_utest]
 *   @{
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
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/

#include "hse_global_variables.h"
#include "hse_host_attrs.h"
#include "hse_host_boot.h"
#include "string.h"
#include "hse_host_flashSrv.h"


/*=============================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
=============================================================================*/

/*=============================================================================
*                                       LOCAL MACROS
=============================================================================*/
#define BLOCK0_BASE_ADDRESS 0x00400000UL
#define IVT_FIXED_LENGTH 0x100U
/*=============================================================================
*                                      LOCAL CONSTANTS
=============================================================================*/

/*=============================================================================
*                                      LOCAL VARIABLES
=============================================================================*/

/*=============================================================================
*                                      GLOBAL CONSTANTS
=============================================================================*/

/*=============================================================================
*                                      GLOBAL VARIABLES
=============================================================================*/
extern volatile hseSrvResponse_t gsrvResponse;
hseAttrConfigBootAuth_t gIVTauthvalue = 0U;
extern volatile bool_t write_attr;
/*=============================================================================
*                                   LOCAL FUNCTION PROTOTYPES
=============================================================================*/

/*=============================================================================
*                                       LOCAL FUNCTIONS
=============================================================================*/

/*=============================================================================
*                                       GLOBAL FUNCTIONS
=============================================================================*/

/******************************************************************************
 * Function:     UpdateIvt
 * Description:  Enable Secure Boot by signing IVT and by setting BOOT_SEQ bit
 ******************************************************************************/
hseSrvResponse_t UpdateIvt(ivt_type_t IvtType)
{
    memcpy((void *)&IVT, (void *)BLOCK0_IVT_ADDRESS, 0xEF);
    /* erase IVT location */
    ASSERT(HostFlash_Erase(
                HOST_BLOCK0_CODE_MEMORY,
                BLOCK0_IVT_ADDRESS,
                1U) == FLS_JOB_OK);
    if (NON_SECURE_IVT == IvtType)
    {
        /*copy IVT from 0x420000 location to block 0 ivt address */
        ASSERT(HostFlash_Program(
                    HOST_BLOCK0_CODE_MEMORY,
                    (uint32_t)BLOCK0_IVT_ADDRESS,
                    (uint8_t *)(BLOCK0_IVT_ADDRESS + NON_SECURE_IVT_BACKUPADDR_OFFSET),
                    (uint32_t)0x100U) == FLS_JOB_OK);
    }
    else
    {
        if (MU_REG_READ64(UTEST_BASE_ADDRESS + IVT_GMAC_FLAG_OFFSET) == UINT64_MAX_VAL)
        {
            IVT.bootCfgWord |= IVT_BOOT_CFG_WORD_BOOT_SEQ;
            /* update IVT */
            ASSERT(HostFlash_Program(
                        HOST_BLOCK0_CODE_MEMORY,
                        (uint32_t)BLOCK0_IVT_ADDRESS,
                        (uint8_t *)&IVT,
                        (uint32_t)0x100U) == FLS_JOB_OK);
        }
        else
        {
            /*copy IVT from 0x402200 location to block 0 ivt address */
            ASSERT(HostFlash_Program(
                        HOST_BLOCK0_CODE_MEMORY,
                        (uint32_t)BLOCK0_IVT_ADDRESS,
                        (uint8_t *)(BLOCK0_IVT_ADDRESS + SECURE_IVT_BACKUPADDR_OFFSET),
                        (uint32_t)0x100U) == FLS_JOB_OK);
        }
    }
    return HSE_SRV_RSP_OK;
}

/*****************************************************************************
 * Function:    HSE_EnableIVTAuthentication
 * Description: Function for enabling IVT authentication
 *              When authentication of IVT is enabled, during startup,
 *              IVT will be authenticated
 *              and then only execution will go to HSE FW
 *****************************************************************************/
hseSrvResponse_t HSE_EnableIVTAuthentication(void)
{
        hseSrvResponse_t srvResponse;
        hseAttrConfigBootAuth_t configIvtAth;

        /* WARNING: This operation is irreversible */
        /* Enable IVT authentication */
        configIvtAth = HSE_IVT_AUTH;

        srvResponse = SetAttr(
            HSE_ENABLE_BOOT_AUTH_ATTR_ID,
            sizeof(hseAttrConfigBootAuth_t),
            (void *)&configIvtAth);
        ASSERT(HSE_SRV_RSP_OK == srvResponse);
        return srvResponse;
}

/******************************************************************************
 * Function:    HSE_GetIVTauthbit
 * Description: Function for reading IVT authentication bit
 ******************************************************************************/
hseSrvResponse_t HSE_GetIVTauthbit(hseAttrConfigBootAuth_t *pIvtValue)
{
        hseSrvResponse_t srvResponse;

        srvResponse = Get_Attr(
            HSE_ENABLE_BOOT_AUTH_ATTR_ID,
            sizeof(hseAttrConfigBootAuth_t),
            (void *)pIvtValue);
        ASSERT(HSE_SRV_RSP_OK == srvResponse);
        return srvResponse;
}

void IVT_Auth_Service()
{
    /*read first to confirm if IVT auth already enabled*/
    gsrvResponse = HSE_GetIVTauthbit(&gIVTauthvalue);
    /*check if ADKP is programmed*/
    if ((ADKP_PROGRAMMING_SUCCESS == (testStatus & ADKP_PROGRAMMING_SUCCESS)) && (TRUE == write_attr) && (0U == gIVTauthvalue))
    {
        /*
         * only if IVT auth bit is not set and ADKP is programmed,
         * will the user be allowed to change enable boot auth
         */
        if ((HSE_IVT_NO_AUTH == gIVTauthvalue) && (HSE_SRV_RSP_OK == gsrvResponse))
        {
            gsrvResponse = HSE_SRV_RSP_GENERAL_ERROR;
            gsrvResponse = UpdateIvt(NON_SECURE_IVT);

            /*
             * write IVT auth bit executed and then read back to
             * confirm that value was written as expected
             */
            gsrvResponse = HSE_EnableIVTAuthentication();
            if (HSE_SRV_RSP_OK == gsrvResponse)
            {
                (void)HSE_GetIVTauthbit(&gIVTauthvalue);
                if (HSE_IVT_AUTH == gIVTauthvalue)
                {
                    testStatus |= IVT_AUTHENTICATION_ENABLED;
                }
            }
        }
        else if (HSE_IVT_AUTH == gIVTauthvalue)
        {
            testStatus |= IVT_AUTHENTICATION_ENABLED;
            gsrvResponse = HSE_SRV_RSP_GENERAL_ERROR;
        }
    }
}

/******************************************************************************
 * Function:    generateIvtSign
 * Description: generates GMAC of IVT
 ******************************************************************************/
hseSrvResponse_t generateIvtSign(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    /*
     * make a copy of IVT in data section as code section memory address
     * is not a valid address for generating GMAC */
    (void)memcpy(
        &IVT,
        (const void *)BLOCK0_BASE_ADDRESS,
        IVT_FIXED_LENGTH);
    /* take non-secure boot IVT backup */
    srvResponse = HSE_SignBootImage(
        (uint8_t *)&IVT,
        BOOT_IMG_TAG_LEN,
        (uint8_t *)&IVT.GMAC[0]);
    ASSERT(HSE_SRV_RSP_OK == srvResponse);

    /** write IVT with GMAC appended at the offset 0xF0 of IVT location,
     *  no need of erase as flash will be erased 1st time. */
    ASSERT(HostFlash_Erase(
            HOST_BLOCK0_CODE_MEMORY,
            (BLOCK0_BASE_ADDRESS + NON_SECURE_IVT_BACKUPADDR_OFFSET),
            1U) == FLS_JOB_OK);
    ASSERT(HostFlash_Program(
            HOST_BLOCK0_CODE_MEMORY,
            (uint32_t)(BLOCK0_BASE_ADDRESS + NON_SECURE_IVT_BACKUPADDR_OFFSET),
            (uint8_t *)&IVT,
            (uint32_t)0x100) == FLS_JOB_OK);

    /* take secure boot IVT backup */
    IVT.bootCfgWord |= IVT_BOOT_CFG_WORD_BOOT_SEQ;
    srvResponse = HSE_SignBootImage(
        (uint8_t *)&IVT,
        BOOT_IMG_TAG_LEN,
        (uint8_t *)&IVT.GMAC[0]);
    ASSERT(HSE_SRV_RSP_OK == srvResponse);

    /** write IVT with GMAC appended at the offset 0xF0 of IVT location,
     *  no need of erase as flash will be erased 1st time. */
    ASSERT(HostFlash_Erase(
            HOST_BLOCK0_CODE_MEMORY,
            (BLOCK0_BASE_ADDRESS + SECURE_IVT_BACKUPADDR_OFFSET),
            1U) == FLS_JOB_OK);

    ASSERT(HostFlash_Program(
            HOST_BLOCK0_CODE_MEMORY,
            (uint32_t)(BLOCK0_BASE_ADDRESS + SECURE_IVT_BACKUPADDR_OFFSET),
            (uint8_t *)&IVT,
            (uint32_t)0x100) == FLS_JOB_OK);
    return HSE_SRV_RSP_OK;
}
#ifdef __cplusplus
}
#endif

/** @} */
