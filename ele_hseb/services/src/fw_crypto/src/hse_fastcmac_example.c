/*
 * hse_fastcmac_example.c
 * @brief Source file for Fast CMAC example in crypto services
 * @details Contains the API's for demonstrating the use of Fast CMAC crypto service in DemoApp
 */

/*==================================================================================================
*
*   Copyright-2022 NXP.
*
*   This software is owned or controlled by NXP and may only be used strictly in accordance with
*   the applicable license terms. By expressly accepting such terms or by downloading, installing,
*   activating and/or otherwise using the software, you are agreeing that you have read, and that
*   you agree to comply with and are bound by, such license terms. If you do not agree to
*   be bound by the applicable license terms, then you may not retain, install, activate or
*   otherwise use the software.
==================================================================================================*/

#include "hse_fastcmac_example.h"

#ifdef HSE_SPT_MONOTONIC_COUNTERS
#ifdef HSE_SPT_CMAC_WITH_COUNTER
static const uint8_t aesEcbPlaintext[] =
{ 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73,
        0x93, 0x17, 0x2a
};
#endif

uint8_t fastcmacoutput[BUFFER_SIZE] = {0};

/******************************************************************************
 * Function:    HSE_FastCmacwithCounter_Example
 * Description: Example of Fast CMAC with  sent Synchronously
 *
 *****************************************************************************/
hseSrvResponse_t HSE_FastCmacwithCounter_Example(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;
    uint32_t rpBitSize          = 40UL;
    uint32_t TxnodecntIdx       = HSE_NUM_OF_MONOTONIC_COUNTERS - 3UL;
    uint32_t RxnodecntIdx       = HSE_NUM_OF_MONOTONIC_COUNTERS - 2UL;
    #ifdef HSE_SPT_CMAC_WITH_COUNTER
    uint32_t volatileCnt       = 0xFFFFFFFFUL;
    uint32_t hseCounter        = 0UL;
    #endif

    /* Configure Node A */
    srvResponse = MonotonicCnt_Config(TxnodecntIdx, rpBitSize);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Configure Node B */
    srvResponse = MonotonicCnt_Config(RxnodecntIdx, rpBitSize);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Configure Node A Counter as a default value */
    srvResponse = MonotonicCnt_Increment(TxnodecntIdx, 0x800000UL);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }

    /* Configure Node B Counter as a default value */
    srvResponse = MonotonicCnt_Increment(RxnodecntIdx, 0x800000UL);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }
    #ifdef HSE_SPT_CMAC_WITH_COUNTER
    /* Generate CMAC with Counter on Node A */
    srvResponse = CmacWithCounter(AesNVMKeyHandle, HSE_AUTH_DIR_GENERATE,TxnodecntIdx,0,
    NUM_OF_ELEMS(aesEcbPlaintext)*8U, aesEcbPlaintext, (16U*8U), fastcmacoutput, &volatileCnt,HSE_SGT_OPTION_NONE);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }
    ASSERT(0x800001UL == (volatileCnt &0x00FFFFFF) );
    /* Read Counter & check for the expected value */
    srvResponse = MonotonicCnt_Read(TxnodecntIdx, (uint32_t)&hseCounter);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }
    ASSERT(0x800001UL == hseCounter);

    /* Verify CMAC with Counter on Node B */
    srvResponse = CmacWithCounter(AesNVMKeyHandle, HSE_AUTH_DIR_VERIFY,RxnodecntIdx, 0x0U,
    NUM_OF_ELEMS(aesEcbPlaintext)*8U, aesEcbPlaintext, (16U*8U), fastcmacoutput, &hseCounter,HSE_SGT_OPTION_NONE);
    if(HSE_SRV_RSP_OK != srvResponse)
    {
        goto exit;
    }
    #endif
exit:
    return srvResponse;
}
#endif
