/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_s3mu.h"
#include "fsl_ele_base_api.h"
#include "fsl_debug_console.h"

/* Debug switch: set to 1 (e.g. -DELE_BASE_API_DEBUG=1) to enable the
 * verbose ELE V2X transceive trace prints. Default is quiet. */
#ifndef ELE_BASE_API_DEBUG
#define ELE_BASE_API_DEBUG (0)
#endif

#if ELE_BASE_API_DEBUG
#define ELE_DBG(...) PRINTF(__VA_ARGS__)
#else
#define ELE_DBG(...) ((void)0)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static status_t nvm_storage_handle_req_baseapi(S3MU_Type *mu, uint32_t *buf, uint32_t wordCount)
{
    return kStatus_Fail;
}

/*!
 * brief Get response from MU
 *
 * This function reads response data from EdgeLock Enclave if available.
 *
 * param mu MU peripheral base address
 * param buf buffer to store read data
 * param wordCount size of data in words
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
static status_t ele_mu_get_response_baseapi(S3MU_Type *mu, uint32_t *buf)
{
    status_t status                 = kStatus_Fail;
    uint32_t rmsg[MSG_RESPONSE_MAX] = {0u};
    mu_hdr_t *msg                   = (mu_hdr_t *)rmsg;

    do
    {
        status = S3MU_GetResponse(mu, rmsg);
        if (status != kStatus_Success)
        {
            break;
        }

        if (msg->hdr_byte.tag == MSG_TAG_RESP)
        {
            (void)memcpy((void *)buf, (void *)msg, (uint32_t)(msg->hdr_byte.size * sizeof(uint32_t)));
            break;
        }
        else if (msg->hdr_byte.tag == MSG_TAG_CMD)
        {
            status = nvm_storage_handle_req_baseapi(mu, rmsg, msg->hdr_byte.size);
            if (status != kStatus_Success)
            {
                break;
            }
        }
        else
        {
            return kStatus_Fail;
        }
    } while (msg->hdr_byte.tag != MSG_TAG_RESP);

    return status;
}

/*!
 * brief Ping ELE
 *
 * This function Ping EdgeLock Enclave, can be sent at any time to verify ELE is alive.
 * Additionally, this command reloads the fuse shadow registers and kick the Sentinel active bit.
 * This active bit must be kicked at least once every day (24 hours).
 *
 * param mu MU peripheral base address
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_Ping(S3MU_Type *mu)
{
    status_t status              = kStatus_Fail;
    uint32_t tmsg[PING_SIZE]     = {0u};
    uint32_t rmsg[S3MU_RR_COUNT] = {0u};

    /****************** PING ELE message ***********************/
    tmsg[0] = PING; // PING Command Header
    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, PING_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == PING_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Get ELE FW Version
 *
 * This function is used to retrieve the Sentinel FW version.
 *
 * param mu MU peripheral base address
 * param EleFwVersion Pointer where ElE firmware version will be stored
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_GetFwVersion(S3MU_Type *mu, uint32_t *EleFwVersion)
{
    status_t status                    = kStatus_Fail;
    uint32_t tmsg[GET_FW_VERSION_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]       = {0u};

    /****************** Get firmware version ELE message ***********************/
    tmsg[0] = GET_FW_VERSION; // Get firmware version Command Header
    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, GET_FW_VERSION_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == GET_FW_VERSION_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        /* read FW version */
        *EleFwVersion = rmsg[2];
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Enable APC (Application core)
 *
 * This function is used by RTC (real time core) to release APC (Application core) when needed.
 *
 * param mu MU peripheral base address
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_EnableAPC(S3MU_Type *mu)
{
    status_t status                = kStatus_Fail;
    uint32_t tmsg[ENABLE_APC_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]   = {0u};

    /****************** Enable APC ELE message ***********************/
    tmsg[0] = ENABLE_APC; // Enable APC Command Header
    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, ENABLE_APC_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == ENABLE_APC_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Forward Lifecycle update
 *
 * This function is to change chip lifecycle
 *  0x01U for NXP provisoned
 *  0x02U for OEM Open
 *  0x08U for OEM Closed
 *  0x80U for OEM Locked
 *
 * param mu MU peripheral base address
 * param Lifecycle to switch
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_ForwardLifecycle(S3MU_Type *mu, uint32_t Lifecycle)
{
    status_t status                       = kStatus_Fail;
    uint32_t tmsg[FORWARD_LIFECYCLE_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]          = {0u};

    /****************** Forward lifecycle ELE message ***********************/
    tmsg[0] = FORWARD_LIFECYCLE; // Forward lifecycle Command Header
    tmsg[1] = Lifecycle;         // Lifecycle to switch to
    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, FORWARD_LIFECYCLE_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == FORWARD_LIFECYCLE_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Start the clock change process
 *
 * param mu MU peripheral base address
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_ClockChangeStart(S3MU_Type *mu)
{
    status_t status                        = kStatus_Fail;
    uint32_t tmsg[CLOCK_CHANGE_START_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]           = {0u};

    /****************** Clock change start message ***********************/
    tmsg[0] = CLOCK_CHANGE_START; // Clock Change Start Command Header

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, CLOCK_CHANGE_START_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == CLOCK_CHANGE_START_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Change ELE and/or CM33 clock
 *
 * It is valid to pass both parameters at the same time if the SoC supports both.
 *
 * param mu MU peripheral base address
 * param NewClockRateELE the new clock rate for ELE
 * param NewClockRateCM33 the new clock rate for the CM33 core
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_ClockChangeFinish(S3MU_Type *mu, uint8_t NewClockRateELE, uint8_t NewClockRateCM33)
{
    status_t status                         = kStatus_Fail;
    uint32_t tmsg[CLOCK_CHANGE_FINISH_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]            = {0u};

    /****************** Clock change finish message ***********************/
    tmsg[0] = CLOCK_CHANGE_FINISH;                                                // Clock Change Finish Command Header
    tmsg[1] = (uint32_t)NewClockRateCM33 << SHIFT_16 | (uint32_t)NewClockRateELE; // New CM33 and ELE clock rates

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, CLOCK_CHANGE_FINISH_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == CLOCK_CHANGE_FINISH_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Start the voltage change process
 *
 * param mu MU peripheral base address
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_VoltageChangeStart(S3MU_Type *mu)
{
    status_t status                          = kStatus_Fail;
    uint32_t tmsg[VOLTAGE_CHANGE_START_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]             = {0u};

    /****************** Voltage change start message ***********************/
    tmsg[0] = VOLTAGE_CHANGE_START; // Voltage Change Start Command Header

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, VOLTAGE_CHANGE_START_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if ((rmsg[0] == VOLTAGE_CHANGE_START_RESPONSE_HDR) && (rmsg[1] == RESPONSE_SUCCESS))
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Finish the voltage change process
 *
 * param mu MU peripheral base address
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_VoltageChangeFinish(S3MU_Type *mu)
{
    status_t status                           = kStatus_Fail;
    uint32_t tmsg[VOLTAGE_CHANGE_FINISH_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]              = {0u};

    /****************** Voltage change finish message ***********************/
    tmsg[0] = VOLTAGE_CHANGE_FINISH; // Voltage Change finish Command Header

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, VOLTAGE_CHANGE_FINISH_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if ((rmsg[0] == VOLTAGE_CHANGE_FINISH_RESPONSE_HDR) && (rmsg[1] == RESPONSE_SUCCESS))
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Get ELE FW Status
 *
 * This function is used to retrieve the Sentinel FW status.
 * If value in EleFwStatus is 0 there is no loaded ELE FW, if 1 ELE FW is authenticated and operational.
 *
 * param mu MU peripheral base address
 * param EleFwStatus Pointer where ElE firmware status will be stored
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_GetFwStatus(S3MU_Type *mu, uint32_t *EleFwStatus)
{
    status_t status                   = kStatus_Fail;
    uint32_t tmsg[GET_FW_STATUS_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]      = {0u};

    /****************** Get firmware status ELE message ***********************/
    tmsg[0] = GET_FW_STATUS; // Get firmware status Command Header
    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, GET_FW_STATUS_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == GET_FW_STATUS_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        /* read FW status */
        *EleFwStatus = rmsg[2];
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Enable an instance of OTFAD.
 *
 * param mu MU peripheral base address
 * param OtfadID ID of the OTFAD instance to enable - used only if there are
 * multiple instances on the SoC
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_EnableOtfad(S3MU_Type *mu, uint8_t OtfadID)
{
    status_t status                  = kStatus_Fail;
    uint32_t tmsg[ENABLE_OTFAD_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]     = {0u};

    /****************** Enable OTFAD message ***********************/
    tmsg[0] = ENABLE_OTFAD;      // Enable OTFAD message Command Header
#if defined(FSL_FEATURE_SOC_OTFAD_COUNT) && (FSL_FEATURE_SOC_OTFAD_COUNT > 1)
    tmsg[1] = (uint32_t)OtfadID; // ID of the OTFAD instance
#else
    (void)OtfadID; /* Prevent an unused variable warning */
#endif

    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, ENABLE_OTFAD_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == ENABLE_OTFAD_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}
/*!
 * brief Release RDC
 *
 * This function is used to release specifed RDC to the core identified in this function.
 * The RDC will be released only if the FW of the core to which is the RDC ownership is going to be
 * transferred has been properly authenticated and verified.
 *
 * param mu MU peripheral base address
 * param RdcID Resource Domain Control identifier
 * param CoreID Core identifier
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_ReleaseRDC(S3MU_Type *mu, uint32_t RdcID, uint32_t CoreID)
{
    status_t status                 = kStatus_Fail;
    uint32_t tmsg[RELEASE_RDC_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]    = {0u};

    /****************** Release RDC ELE message ***********************/
    tmsg[0] = RELEASE_RDC;               // Release RDC Command Header
    tmsg[1] = RdcID << SHIFT_8 | CoreID; // RDC_ID | CoreID
    /* Send message Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, RELEASE_RDC_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == RELEASE_RDC_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

/*!
 * brief Start the initialization of the RNG context.
 *
 * The RNG must be started before using some of the ELE services.
 *
 * param mu MU peripheral base address
 *
 * return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_StartRng(S3MU_Type *mu)
{
    status_t status               = kStatus_Fail;
    uint32_t tmsg[START_RNG_SIZE] = {0u};
    uint32_t rmsg[S3MU_RR_COUNT]  = {0u};

    /****************** Start RNG ***********************/
    tmsg[0] = START_RNG; // Start RNG message Command Header

    /* Send message to Security Sub-System */
    status = S3MU_SendMessage(mu, tmsg, START_RNG_SIZE);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Wait for response from Security Sub-System */
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check that response corresponds to the sent command */
    if (rmsg[0] == START_RNG_RESPONSE_HDR && rmsg[1] == RESPONSE_SUCCESS)
    {
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}


#if FSL_FEATURE_ELE_HAS_V2X_FW_BRINGUP
/*!
 * @brief Send an ELE V2X bring-up command and validate the response header.
 */
static status_t ele_v2x_transceive_baseapi(S3MU_Type *mu,
                                           const uint32_t *tmsg,
                                           uint32_t words,
                                           uint32_t *rmsg,
                                           uint8_t command)
{
    status_t status = S3MU_SendMessage(mu, (uint32_t *)(uintptr_t)tmsg, words);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = ele_mu_get_response_baseapi(mu, rmsg);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Response header word0: tag (0xE1) in bits [31:24], command in [23:16]. */
    if ((uint8_t)((rmsg[0] >> 24) & 0xFFu) != MSG_TAG_RESP)
    {
        return kStatus_Fail;
    }
    if ((uint8_t)((rmsg[0] >> 16) & 0xFFu) != command)
    {
        return kStatus_Fail;
    }
    return kStatus_Success;
}

status_t ELE_BaseAPI_V2xFwAuthenticate(S3MU_Type *mu, uint32_t v2xFwAddr)
{
    uint32_t tmsg[ELE_V2X_FW_AUTH_SIZE];
    uint32_t rmsg[S3MU_RR_COUNT] = {0u};
    status_t status;

    if (mu == NULL)
    {
        return kStatus_InvalidArgument;
    }

    tmsg[0] = ((uint32_t)MSG_TAG_CMD << 24) | ((uint32_t)ELE_V2X_FW_AUTH_CMD << 16) |
              ((uint32_t)ELE_V2X_FW_AUTH_SIZE << 8) | (uint32_t)ELE_V2X_MSG_VERSION;
    tmsg[1] = v2xFwAddr;
    tmsg[2] = 0u;
    tmsg[3] = v2xFwAddr;

    status = ele_v2x_transceive_baseapi(mu, tmsg, ELE_V2X_FW_AUTH_SIZE, rmsg, ELE_V2X_FW_AUTH_CMD);
    ELE_DBG("[dbg-ele] AUTH xcv status=0x%x r0=0x%08x r1=0x%08x r2=0x%08x r3=0x%08x\r\n",
           (unsigned int)status, (unsigned int)rmsg[0], (unsigned int)rmsg[1],
           (unsigned int)rmsg[2], (unsigned int)rmsg[3]);
    if (status != kStatus_Success)
    {
        return status;
    }
    if ((uint8_t)(rmsg[1] & 0xFFu) != RESPONSE_SUCCESS)
    {
        return kStatus_Fail;
    }
    return kStatus_Success;
}

status_t ELE_BaseAPI_GetV2xFwState(S3MU_Type *mu, uint32_t *state)
{
    uint32_t tmsg[ELE_V2X_GET_STATE_SIZE];
    uint32_t rmsg[S3MU_RR_COUNT] = {0u};
    status_t status;

    if ((mu == NULL) || (state == NULL))
    {
        return kStatus_InvalidArgument;
    }

    tmsg[0] = ((uint32_t)MSG_TAG_CMD << 24) | ((uint32_t)ELE_V2X_GET_STATE_CMD << 16) |
              ((uint32_t)ELE_V2X_GET_STATE_SIZE << 8) | (uint32_t)ELE_V2X_MSG_VERSION;

    status = ele_v2x_transceive_baseapi(mu, tmsg, ELE_V2X_GET_STATE_SIZE, rmsg, ELE_V2X_GET_STATE_CMD);
    ELE_DBG("[dbg-ele] STATE xcv status=0x%x r0=0x%08x r1=0x%08x r2=0x%08x r3=0x%08x\r\n",
           (unsigned int)status, (unsigned int)rmsg[0], (unsigned int)rmsg[1],
           (unsigned int)rmsg[2], (unsigned int)rmsg[3]);
    if (status != kStatus_Success)
    {
        return status;
    }
    *state = rmsg[2] & ELE_V2X_FW_STATE_MASK;
    return kStatus_Success;
}

bool ELE_BaseAPI_IsV2xFwRunning(uint32_t state)
{
    return (state == ELE_V2X_FW_STATE_NORM_RUNNING) || (state == ELE_V2X_FW_STATE_DBG_RUNNING);
}

status_t ELE_BaseAPI_BringUpV2xFw(S3MU_Type *mu, uint32_t v2xFwAddr, uint32_t retries)
{
    uint32_t state = 0u;
    status_t status;
    uint32_t i;

    if (mu == NULL)
    {
        return kStatus_InvalidArgument;
    }

    status = ELE_BaseAPI_GetV2xFwState(mu, &state);
    if ((status == kStatus_Success) && ELE_BaseAPI_IsV2xFwRunning(state))
    {
        return kStatus_Success;
    }

    status = ELE_BaseAPI_V2xFwAuthenticate(mu, v2xFwAddr);
    if (status != kStatus_Success)
    {
        return status;
    }

    for (i = 0u; i < retries; i++)
    {
        status = ELE_BaseAPI_GetV2xFwState(mu, &state);
        if (status != kStatus_Success)
        {
            return status;
        }
        if (ELE_BaseAPI_IsV2xFwRunning(state))
        {
            return kStatus_Success;
        }
    }
    return kStatus_Timeout;
}
#endif /* FSL_FEATURE_ELE_HAS_V2X_FW_BRINGUP */
