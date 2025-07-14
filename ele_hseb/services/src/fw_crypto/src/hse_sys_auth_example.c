/*
 * @file hse_sys_auth_example.c
 * @brief Source file for SYS_AUTH examples in Crypto services
 * @details Contains the API's for demonstrating the use of SYS_AUTH services in demo APP
 */
/*--------------------------------------------------------------------------*/
/* Copyright 2022 NXP                                                       */
/*                                                                          */
/* NXP Proprietary. This software is owned or controlled by NXP and may     */
/* only be used strictly in accordance with the applicable license terms.   */
/* By expressly accepting such terms or by downloading, installing,         */
/* activating and/or otherwise using the software, you are agreeing that    */
/* you have read, and that you agree to comply with and are bound by, such  */
/* license terms. If you do not agree to be bound by the applicable license */
/* terms, then you may not retain, install, activate or otherwise use the   */
/* software.                                                                */
/*--------------------------------------------------------------------------*/
#include "hse_sys_auth_example.h"
/*
 * ============================================================================
 *                              CONST VARIABLES
 * ============================================================================
*/

/*
 * @brief CUST_SUPER_USER authorization key (RSA-2048 Key pair)
 */
static const uint8_t rsa2048CustAuthKeyModulus[] =
{ 0xcc, 0x83, 0x0b, 0x10, 0xc5, 0xf2, 0xa6, 0x2d, 0xd5, 0xd7, 0x07, 0x55, 0x4f,
        0x14, 0x9b, 0x6f, 0x52, 0x94, 0xe4, 0x82, 0x88, 0x9d, 0x8a, 0x48, 0xe7,
        0x5e, 0x25, 0x39, 0x08, 0xcf, 0xb6, 0x4e, 0x4d, 0xf4, 0xae, 0x22, 0x5e,
        0xee, 0x00, 0x31, 0x9e, 0xe7, 0x89, 0xc1, 0xba, 0x57, 0x21, 0xef, 0x0a,
        0x64, 0x9a, 0x8c, 0xd4, 0x01, 0x8b, 0x1d, 0xe0, 0xaf, 0x06, 0xbb, 0xd7,
        0x7a, 0xd3, 0x28, 0x94, 0xb0, 0x36, 0xb2, 0x1e, 0xc7, 0x29, 0xbb, 0xd1,
        0x09, 0x7b, 0x34, 0xf2, 0x0e, 0x07, 0x0e, 0xbb, 0x29, 0xf9, 0x18, 0xda,
        0xb1, 0x02, 0xe0, 0x60, 0x4f, 0x19, 0x19, 0xc4, 0x58, 0x05, 0x80, 0xbf,
        0x69, 0x2e, 0xad, 0x54, 0x14, 0x05, 0x39, 0x4d, 0xa0, 0xa9, 0xad, 0x79,
        0x1a, 0xe8, 0x39, 0xd0, 0xae, 0x60, 0x4b, 0x48, 0x0b, 0x5d, 0xe5, 0xe9,
        0x0f, 0x88, 0xa4, 0xdc, 0x14, 0x43, 0x4f, 0xb6, 0x9c, 0xa9, 0xb4, 0x05,
        0xc7, 0xe9, 0xc0, 0x4a, 0x70, 0x85, 0x51, 0x5c, 0x8a, 0x21, 0x85, 0x2a,
        0x61, 0xf6, 0xd3, 0x6e, 0xd7, 0x66, 0x23, 0x81, 0x4c, 0x42, 0x3b, 0xcc,
        0x4f, 0x22, 0xea, 0xfb, 0xa0, 0x95, 0x46, 0x14, 0x60, 0x2b, 0xa5, 0xe9,
        0x48, 0xaa, 0x75, 0x31, 0x07, 0xb3, 0x14, 0xad, 0x1c, 0xe6, 0x94, 0xea,
        0x99, 0xf0, 0x90, 0xbc, 0xb4, 0xe2, 0x00, 0x74, 0x1a, 0xf5, 0x44, 0x5f,
        0xfa, 0x55, 0xc9, 0x76, 0x34, 0x80, 0xea, 0x6c, 0xee, 0x5d, 0xac, 0x0b,
        0xe3, 0x0c, 0x31, 0xe7, 0xd1, 0xc5, 0xeb, 0xae, 0x20, 0x6a, 0x3c, 0x2d,
        0x59, 0x55, 0xca, 0x29, 0x17, 0xa7, 0x4b, 0xec, 0x76, 0xea, 0xe0, 0x29,
        0x37, 0x67, 0xf0, 0x09, 0x0f, 0x92, 0x93, 0x92, 0x46, 0x1a, 0x6d, 0x86,
        0x89, 0x6c, 0x9e, 0xce, 0x56, 0xad, 0xa9, 0x12, 0xe6, 0x79, 0x56, 0x6c,
        0x23, 0x67, 0x29
};
static const uint8_t rsa2048CustAuthKeyPubExp[] = { 0x01, 0x00, 0x01 };
static const uint8_t rsa2048CustAuthKeyPrivExp[] =
{ 0x4b, 0x60, 0x85, 0x44, 0x92, 0x60, 0xe3, 0xe5, 0x02, 0xea, 0xf5, 0x35, 0xf9,
        0x26, 0xb4, 0x3f, 0x6c, 0xa5, 0x9a, 0x2f, 0x8e, 0x60, 0xda, 0x66, 0x12,
        0x78, 0xbd, 0xa9, 0x6e, 0xd8, 0xf7, 0xed, 0x96, 0x87, 0xf7, 0x62, 0x8d,
        0xb0, 0x94, 0xfb, 0xb5, 0x00, 0xcb, 0xe2, 0x4e, 0x66, 0xed, 0x07, 0xe8,
        0xc1, 0xb9, 0xc0, 0xec, 0x27, 0x01, 0x7a, 0x6a, 0xa4, 0xb8, 0x0b, 0xa5,
        0xd9, 0x0c, 0x58, 0xe9, 0xac, 0xa5, 0x0c, 0x33, 0x91, 0xde, 0x8c, 0xed,
        0xf7, 0xdb, 0xa4, 0x61, 0x0e, 0xd0, 0x85, 0x1a, 0x70, 0xd6, 0xf1, 0xd5,
        0x30, 0xe6, 0xc9, 0x82, 0x55, 0xc4, 0xff, 0x9d, 0x41, 0x80, 0x0d, 0xba,
        0xfe, 0xee, 0x88, 0x4d, 0xf3, 0xef, 0x14, 0xf6, 0xca, 0x9f, 0x9f, 0x63,
        0x82, 0x8a, 0x53, 0x3c, 0xd9, 0xf8, 0x7d, 0xe6, 0xb8, 0xe1, 0xcb, 0xea,
        0x81, 0xa2, 0x30, 0x79, 0x39, 0x32, 0xb1, 0x56, 0xca, 0x0e, 0x9d, 0xbe,
        0xe5, 0x3d, 0x02, 0x09, 0x2f, 0x4c, 0xa0, 0x68, 0x43, 0x04, 0xd6, 0xad,
        0xe8, 0x26, 0xd8, 0x32, 0x7c, 0x8d, 0x68, 0x73, 0x7d, 0xdb, 0x00, 0xf0,
        0x63, 0x37, 0xc6, 0x1d, 0x63, 0xc1, 0x0e, 0x57, 0x4f, 0xeb, 0xcf, 0x22,
        0x96, 0xaf, 0x1f, 0xdc, 0xf4, 0x7a, 0x96, 0xc3, 0xfd, 0xb9, 0x18, 0xe7,
        0x5f, 0x2c, 0x43, 0x05, 0xad, 0xa4, 0x20, 0x84, 0x60, 0x04, 0x48, 0x54,
        0xd2, 0x0c, 0x45, 0x45, 0xb1, 0x5f, 0xac, 0x59, 0x0a, 0xff, 0xc7, 0x7d,
        0x9f, 0x47, 0xca, 0xa6, 0x04, 0xe5, 0xc7, 0xa4, 0x05, 0xd9, 0x07, 0x75,
        0xd0, 0x0f, 0xef, 0x70, 0x8b, 0x8c, 0xca, 0x73, 0x2b, 0x78, 0xc4, 0x9c,
        0x2d, 0xab, 0x84, 0xb7, 0x24, 0x05, 0x4a, 0x04, 0xce, 0x41, 0x7b, 0x32,
        0xcf, 0x9e, 0x5c, 0x1a, 0x4c, 0x1a, 0x27, 0x1d, 0x19, 0x96, 0x47, 0x5b,
        0x21, 0xd5, 0x81
};
/*
 * ============================================================================
 *                               LOCAL FUNCTIONS DEFINATIONS
 * ============================================================================
*/

/******************************************************************************
 * Function:    SetCustAuthorizationKey
 * Description: Sets the authorization key for CUST_SUPER_USER.
 *****************************************************************************/
 hseSrvResponse_t SetCustAuthorizationKey( void )
{
    hseSrvResponse_t hseResponse;

    /* Set the CUST_SUPER_USER authorization key
     * This is an example as NVM keys CANNOT be imported
     * in plain unless the slot is empty (see `HSE_UpdateNvmKey_Example`)
     */
    hseResponse = ImportPlainRsaKeyReq(
            HSE_DEMO_NVM_RSA2048_PUB_CUSTAUTH_HANDLE0,
            HSE_KEY_TYPE_RSA_PUB,
            (HSE_KF_USAGE_VERIFY | HSE_KF_USAGE_AUTHORIZATION),
            (HOST_ADDR)rsa2048CustAuthKeyModulus,
           (ARRAY_SIZE(rsa2048CustAuthKeyModulus)*8),
            (HOST_ADDR)rsa2048CustAuthKeyPubExp,
            ARRAY_SIZE(rsa2048CustAuthKeyPubExp),
            (HOST_ADDR) NULL
            );
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    /* Set a copy of authorization key for signature generation using HSE
     * (over challenge) Intended use case - Sign challenge via external means
     */
    hseResponse = ImportPlainRsaKeyReq(
            HSE_DEMO_NVM_RSA2048_PAIR_CUSTAUTH_HANDLE0,
            HSE_KEY_TYPE_RSA_PAIR,
            HSE_KF_USAGE_SIGN,
            (HOST_ADDR)rsa2048CustAuthKeyModulus,
            (ARRAY_SIZE(rsa2048CustAuthKeyModulus)*8),
            (HOST_ADDR)rsa2048CustAuthKeyPubExp,
            ARRAY_SIZE(rsa2048CustAuthKeyPubExp),
            (HOST_ADDR)rsa2048CustAuthKeyPrivExp
            );

    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

exit:
    return hseResponse;
}

/******************************************************************************
 * Function:    AuthorizeCust
 * Description: Authorization Request/Response sequence for CUST SU
 *****************************************************************************/
 hseSrvResponse_t AuthorizeCust(void)
{
    hseSrvResponse_t hseResponse;

    uint8_t challenge[CHALLENGE_LENGTH] = {0U};
    uint8_t sign0[SIGN_LENGTH_MAX] = {0U};
    uint32_t signLen0;

    /*  Auth scheme used for CUST SU authorization */
    hseAuthScheme_t rsaPssSignScheme = {
        .sigScheme.signSch = HSE_SIGN_RSASSA_PSS,
        .sigScheme.sch.rsaPss.hashAlgo = HSE_HASH_ALGO_SHA2_512,
        .sigScheme.sch.rsaPss.saltLength = PSS_SALT_LENGTH
    };

    /* Send the authorization request */
    memset(challenge, 0x00U, CHALLENGE_LENGTH);
    hseResponse = HSE_SysAuthorizationReq(HSE_SYS_AUTH_ALL, HSE_RIGHTS_SUPER_USER,
        HSE_DEMO_NVM_RSA2048_PUB_CUSTAUTH_HANDLE0, &rsaPssSignScheme, challenge);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    /* Sign the challenge using the pair private key of the authorization key
     * Intended use case - Sign challenge via external means
     */
    signLen0 = SIGN_LENGTH_MAX;
    memset(sign0, 0x00U, SIGN_LENGTH_MAX);
    hseResponse = RsaPssSignSrv(HSE_DEMO_NVM_RSA2048_PAIR_CUSTAUTH_HANDLE0, PSS_SALT_LENGTH,
            HSE_HASH_ALGO_SHA2_512, CHALLENGE_LENGTH,
            challenge, FALSE, 0U, &signLen0, sign0);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    /* Send the authorization response */
    hseResponse = HSE_SysAuthorizationResp(sign0, NULL, signLen0, 0UL);
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
exit:
    return hseResponse;
}
/******************************************************************************
 * Function:    HSE_SysAuthorization_Example
 * Description: SYS Authorization Example
 *              Import RSA keys, Authorization Request/Response,
 *              RSA Sign generation, Erase Key
******************************************************************************/
hseSrvResponse_t HSE_SysAuthorization_Example(void)
{
    uint16_t hseStatus = 0U;
    hseSrvResponse_t hseResponse;
    //Set the CUST_SUPER_USER authorization key
    hseResponse = SetCustAuthorizationKey();
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    //Switch to IN_FIELD User
    hseResponse = HSE_SysAuthorizationReq
            (
                    HSE_SYS_AUTH_ALL,
                    HSE_RIGHTS_USER,
                    0UL,
                    NULL,
                    NULL
            );
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }
    //Check user: IN_FIELD User mode
    hseStatus = HSE_MU_GetHseStatus(MU0);
    if(!IN_FIELD_USER_MODE(hseStatus))
    {
        goto exit;
    }
    // hseResponse =  GetKeyInfo(HSE_DEMO_NVM_RSA2048_PAIR_CUSTAUTH_HANDLE0, &keyinfo);
    //Operations with USER rights

    //Try erase operation - NOT_ALLOWED
    hseResponse = EraseKeyReq
            (
                HSE_DEMO_NVM_RSA2048_PAIR_CUSTAUTH_HANDLE0,
                HSE_ERASE_NOT_USED
            );

    if(HSE_SRV_RSP_NOT_ALLOWED != hseResponse)
    {
        goto exit;
    }
    //hseResponse =  GetKeyInfo(HSE_DEMO_NVM_RSA2048_PAIR_CUSTAUTH_HANDLE0, &keyinfo);
    //Authorize as CUST_SUPER_USER using CUST authorization key
    hseResponse = AuthorizeCust();
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    //Check user: CUST_SUPER_USER mode
    hseStatus = HSE_MU_GetHseStatus(MU0);
    if(!CUST_SUPER_USER_MODE(hseStatus))
    {
        goto exit;
    }

    //Operations with SU rights
    //Try erase operation - should work now - HSE_SRV_RSP_OK
    hseResponse = EraseKeyReq
            (
                    HSE_DEMO_NVM_RSA2048_PAIR_CUSTAUTH_HANDLE0,
                    HSE_ERASE_NOT_USED
            );
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

exit:
    return hseResponse;
}

/******************************************************************************
 * Function:    Grant_SuperUser_Rights
 * Description: main function for granting super user rights.
 *****************************************************************************/
hseSrvResponse_t Grant_SuperUser_Rights(void)
{
    uint16_t hseStatus = 0U;
    hseSrvResponse_t hseResponse;

    /*Set the CUST_SUPER_USER authorization key*/
    hseResponse = SetCustAuthorizationKey();
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    /*Switch to IN_FIELD User*/
    hseResponse = HSE_SysAuthorizationReq
            (
                    HSE_SYS_AUTH_ALL,
                    HSE_RIGHTS_USER,
                    0UL,
                    NULL,
                    NULL
                    );
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    /*Check user: IN_FIELD User mode*/
    hseStatus = HSE_MU_GetHseStatus(MU0);
    if(!IN_FIELD_USER_MODE(hseStatus))
    {
        goto exit;
    }

    /*Authorize as CUST_SUPER_USER using CUST authorization key*/
    hseResponse = AuthorizeCust();
    if(HSE_SRV_RSP_OK != hseResponse)
    {
        goto exit;
    }

    /*Check user: CUST_SUPER_USER mode*/
    hseStatus = HSE_MU_GetHseStatus(MU0);
    if(!CUST_SUPER_USER_MODE(hseStatus))
    {
        goto exit;
    }
exit:
    return hseResponse;
}
