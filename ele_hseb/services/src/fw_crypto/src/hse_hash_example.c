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
#include "hse_hash_example.h"
/*
 * ============================================================================
 *                            LOCAL CONSTANTS
 * ============================================================================*/
static const uint8_t sha1Message[] =
{ 0x97, 0x77, 0xcf, 0x90, 0xdd, 0x7c, 0x7e, 0x86, 0x35, 0x06 };
static const uint8_t sha1Hash[] =
{ 0x05, 0xc9, 0x15, 0xb5, 0xed, 0x4e, 0x4c, 0x4a, 0xff, 0xfc, 0x20, 0x29, 0x61,
        0xf3, 0x17, 0x43,0x71, 0xe9, 0x0b, 0x5c
};
static const uint8_t sha256Message[] =
{ 0x59, 0xeb, 0x45, 0xbb, 0xbe, 0xb0, 0x54, 0xb0, 0xb9, 0x73, 0x34, 0xd5, 0x35,
        0x80, 0xce, 0x03, 0xf6, 0x99
};
static const uint8_t sha256Hash[] =
{ 0x32, 0xc3, 0x8c, 0x54, 0x18, 0x9f, 0x23, 0x57, 0xe9, 0x6b, 0xd7, 0x7e, 0xb0,
        0x0c, 0x2b, 0x9c, 0x34, 0x1e, 0xbe, 0xba, 0xcc, 0x29, 0x45, 0xf9, 0x78,
        0x04, 0xf5, 0x9a, 0x93, 0x23, 0x82, 0x88
};

static const uint8_t sha512Message[] =
{ 0x16, 0xb1, 0x70, 0x74, 0xd3, 0xe3, 0xd9, 0x75, 0x57, 0xf9, 0xed, 0x77, 0xd9,
        0x20, 0xb4, 0xb1, 0xbf, 0xf4, 0xe8, 0x45, 0xb3, 0x45, 0xa9, 0x22
};
static const uint8_t sha512Hash[] =
{ 0x68, 0x84, 0x13, 0x45, 0x82, 0xa7, 0x60, 0x04, 0x64, 0x33, 0xab, 0xcb, 0xd5,
        0x3d, 0xb8, 0xff, 0x1a, 0x89, 0x99, 0x58, 0x62, 0xf3, 0x05, 0xb8, 0x87,
        0x02, 0x0f, 0x6d, 0xa6, 0xc7, 0xb9, 0x03, 0xa3, 0x14, 0x72, 0x1e, 0x97,
        0x2b, 0xf4, 0x38, 0x48, 0x3f, 0x45, 0x2a, 0x8b, 0x09, 0x59, 0x62, 0x98,
        0xa5, 0x76, 0xc9, 0x03, 0xc9, 0x1d, 0xf4, 0xa4, 0x14, 0xc7, 0xbd, 0x20,
        0xfd, 0x1d, 0x07
};
static const hseHashAlgo_t hashAlgos[] =
{
    HSE_HASH_ALGO_SHA_1,
    HSE_HASH_ALGO_SHA2_256,
    HSE_HASH_ALGO_SHA2_512
};

static const uint8_t *hashMessages[] =
{
    sha1Message,
    sha256Message,
    sha512Message
};

static const uint32_t hashMessagesLengths[] =
{
    ARRAY_SIZE(sha1Message),
    ARRAY_SIZE(sha256Message),
    ARRAY_SIZE(sha512Message)
};

static const uint8_t *hashExpectedOutput[] =
{
    sha1Hash,
    sha256Hash,
    sha512Hash
};

static const uint32_t hashLength[] =
{
    ARRAY_SIZE(sha1Hash),
    ARRAY_SIZE(sha256Hash),
    ARRAY_SIZE(sha512Hash),
};

/*
 * ============================================================================
 *                               GLOBAL VARIABLES
 * ============================================================================
*/
uint8_t hashTestOutput[NUMBER_OF_ASYNC_REQ][HASH_BUFFER_SIZE] = {0};
uint32_t hashTestOutputLength[NUMBER_OF_ASYNC_REQ] = {HASH_BUFFER_SIZE, HASH_BUFFER_SIZE, HASH_BUFFER_SIZE};
hashCallbackParams_t callbackParams[HSE_NUM_OF_MU_INSTANCES][HSE_NUM_OF_CHANNELS_PER_MU];
uint32_t numberOfResponses = 0;
volatile hseSrvResponse_t asyncResponses[HSE_NUM_OF_MU_INSTANCES]
                                         [HSE_NUM_OF_CHANNELS_PER_MU] = {0};

/*
 * ============================================================================
 *                               LOCAL FUNCTIONS DEFINITION
 * ============================================================================
*/
/******************************************************************************
 * Function:    VerifyOutput
 * Description: Check hash requests outputs match the expected.
 *****************************************************************************/
static void VerifyOutput()
{
    /*Check output length and data are matching expected hash*/
    for (uint8_t i = 0U; i < NUMBER_OF_ASYNC_REQ; i++)
    {
        if( (hashLength[i] != hashTestOutputLength[i]) ||
            (0 != memcmp(hashTestOutput[i], hashExpectedOutput[i], hashLength[i])) )
        {
            goto exit;
        }
    }
 exit:
    return;
}


/******************************************************************************
 * Function:    HSE_HashRequestCallback
 * Description: Callback of Hash Request. Logs and checks the HSE response.
 ******************************************************************************/
static void HSE_HashRequestCallback(hseSrvResponse_t srvResponse,void* pArg)
{
    /* Argument is a callbackInfo_t pointer which contains the MU interface
     * and the channel on which the request was sent */
    hashCallbackParams_t *pCallbackParams = (hashCallbackParams_t *) pArg;
    uint8_t u8MuInstance = pCallbackParams->u8MuInstance;
    uint8_t u8MuChannel = pCallbackParams->u8MuChannel;

     /*Log the response*/
    asyncResponses[u8MuInstance][u8MuChannel] = srvResponse;
    numberOfResponses += 1;

     /*Check the HSE response*/
    if (HSE_SRV_RSP_OK != srvResponse)
    {

    }
}
/******************************************************************************
 * Function:    HSE_HashDataBlocking
 * Description: Blocking hash request.
 *****************************************************************************/
hseSrvResponse_t HSE_HashDataBlocking
(
    uint8_t u8MuInstance,
    hseAccessMode_t accessMode,
    uint32_t streamId,
    hseHashAlgo_t hashAlgo,
    const uint8_t *pInput,
    uint32_t inputLength,
    uint8_t *pHash,
    uint32_t *pHashLength
)
{
    uint8_t u8MuChannel = 1U;
    hseTxOptions_t asyncTxOptions;
    hseSrvDescriptor_t *pHseSrvDesc;
    hseHashSrv_t *pHashSrv;
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    /*Complete the service descriptor placed in shared memory*/
    pHseSrvDesc = &gHseSrvDesc[0U][u8MuChannel];
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    pHashSrv = &(pHseSrvDesc->hseSrv.hashReq);

    pHseSrvDesc->srvId      = HSE_SRV_ID_HASH;
    pHashSrv->accessMode    = accessMode;
    pHashSrv->streamId      = streamId;
    pHashSrv->hashAlgo      = hashAlgo;
    pHashSrv->inputLength   = inputLength;
    pHashSrv->pInput        = PTR_TO_HOST_ADDR(pInput);
    pHashSrv->pHash         = PTR_TO_HOST_ADDR(pHash);
    pHashSrv->pHashLength   = PTR_TO_HOST_ADDR(pHashLength);

    /*Complete callback info and async txOptions*/
    callbackParams[u8MuInstance][u8MuChannel].u8MuInstance  = u8MuInstance;
    callbackParams[u8MuInstance][u8MuChannel].u8MuChannel   = u8MuChannel;
    asyncTxOptions.txOp              = HSE_TX_SYNCHRONOUS;
    asyncTxOptions.pfAsyncCallback   = &(HSE_HashRequestCallback);
    asyncTxOptions.pCallbackpArg     =
    (void *)(&(callbackParams[u8MuInstance][u8MuChannel]));

    /*Send the request asynchronously*/
    srvResponse = HSE_Send(u8MuInstance,u8MuChannel,asyncTxOptions,pHseSrvDesc);
    return srvResponse;
}


/******************************************************************************
 * Function:    HSE_HashDataNonBlocking
 * Description: Non-blocking hash request.
 *****************************************************************************/
hseSrvResponse_t HSE_HashDataNonBlocking
(
    uint8_t u8MuInstance,
    hseAccessMode_t accessMode,
    uint32_t streamId,
    hseHashAlgo_t hashAlgo,
    const uint8_t *pInput,
    uint32_t inputLength,
    uint8_t *pHash,
    uint32_t *pHashLength
)
{
    uint8_t u8MuChannel = 1U;
    hseTxOptions_t asyncTxOptions;
    hseSrvDescriptor_t *pHseSrvDesc;
    hseHashSrv_t *pHashSrv;
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    //Complete the service descriptor placed in shared memory
    pHseSrvDesc = &gHseSrvDesc[0U][u8MuChannel];
    memset(pHseSrvDesc, 0, sizeof(hseSrvDescriptor_t));

    pHashSrv = &(pHseSrvDesc->hseSrv.hashReq);

    pHseSrvDesc->srvId      = HSE_SRV_ID_HASH;
    pHashSrv->accessMode    = accessMode;
    pHashSrv->streamId      = streamId;
    pHashSrv->hashAlgo      = hashAlgo;
    pHashSrv->inputLength   = inputLength;
    pHashSrv->pInput        = PTR_TO_HOST_ADDR(pInput);
    pHashSrv->pHash         = PTR_TO_HOST_ADDR(pHash);
    pHashSrv->pHashLength   = PTR_TO_HOST_ADDR(pHashLength);

    //Complete callback info and async txOptions*/
    callbackParams[u8MuInstance][u8MuChannel].u8MuInstance  = u8MuInstance;
    callbackParams[u8MuInstance][u8MuChannel].u8MuChannel   = u8MuChannel;
    asyncTxOptions.txOp              = HSE_TX_ASYNCHRONOUS;
    asyncTxOptions.pfAsyncCallback   = &(HSE_HashRequestCallback);
    asyncTxOptions.pCallbackpArg     =
            (void *)(&(callbackParams[u8MuInstance][u8MuChannel]));

    //Send the request asynchronously
    srvResponse = HSE_Send(u8MuInstance,u8MuChannel,asyncTxOptions,pHseSrvDesc);
    return srvResponse;
}



/******************************************************************************
 * Function:    HSE_HashAsync_Example
 * Description: Example of Hash requests sent asynchronously,
 *              Hashes - SHA-1, SHA2-256, SHA2-512.
 *****************************************************************************/
 hseSrvResponse_t HSE_HashAsync_Example(void)
{
    hseSrvResponse_t srvResponse = HSE_SRV_RSP_GENERAL_ERROR;

    //EnableIRQ(MU0_B_TX_IRQn); /**< ORed TX interrupt to MU-0 */
    EnableIRQ(MU0_B_RX_IRQn); /**< ORed RX interrupt to MU-0 */
    EnableIRQ(MU0_B_IRQn);    /**< ORed general purpose interrupt request to MU-0 */
    //EnableIRQ(MU1_B_TX_IRQn); /**< ORed TX interrupt to MU-1 */
    //EnableIRQ(MU1_B_RX_IRQn); /**< ORed RX interrupt to MU-1 */
    EnableIRQ(MU1_B_IRQn);    /**< ORed general purpose interrupt request to MU-1 */

    /* Enable Receive Interrupt on all channels */
    for(uint8_t mu = 0U; mu < HSE_NUM_OF_MU_INSTANCES; ++mu)
        HSE_MU_EnableInterrupts(mu, HSE_INT_RESPONSE, 0xFFFFUL);

    /* -------- Hash Requests --------- */

    /* Send requests asynchronously */
    for (uint8_t i = 0U; i < NUMBER_OF_ASYNC_REQ; i++)
    {
        srvResponse = HSE_HashDataNonBlocking(MU0, HSE_ACCESS_MODE_ONE_PASS,
            0, hashAlgos[i], hashMessages[i], hashMessagesLengths[i],
            hashTestOutput[i], &(hashTestOutputLength[i]));
        //Verify that transmission was successful
        if (HSE_SRV_RSP_OK != srvResponse)
        {
            break;
        }

        /* Adding some delay in between */
        SDK_DelayAtLeastUs(2000U, CLOCK_GetCoreClkFreq());
    }
    /* Wait all responses */
    while (numberOfResponses < NUMBER_OF_ASYNC_REQ) {};

    /* Responses are checked in callback - Check outputs */
    VerifyOutput();
    return srvResponse;
}
