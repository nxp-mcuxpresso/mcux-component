/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * EdgeLock V2X base (debug) service driver.
 *
 * Ported from the Linux driver drivers/firmware/imx/v2x_base_msg.c. Implements
 * the four base operations (start_rng, pwr_state, get_fw_version, debug_dump)
 * over its message unit, driven directly with the generic MU hardware
 * driver (fsl_mu), reusing the same enclave message framing as ele_crypto
 * / v2x_crypto. See v2x_base_internal.h for the wire format.
 */

#include "v2x_base.h"
#include "v2x_base_internal.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Comfortable fixed message buffer size in 32-bit words for req and resp.
 * The largest response handled here is the debug dump (23 words). */
#define V2X_BASE_MSG_MAX_WORDS (24u)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static status_t V2X_Base_Transceive(MU_Type *mu,
                                     const uint32_t *msg,
                                     uint32_t msgWords,
                                     uint32_t *resp,
                                     uint8_t command);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Send a V2X base message and wait for its response.
 *
 * Sends msgWords 32-bit words through the MU, retrieves the response into
 * resp, then validates the response tag and echoed command. The status word
 * is NOT validated here so that callers can inspect the failure indication
 * (e.g. TRNG-in-progress for start_rng, permission-denied for pwr_state).
 */
static status_t V2X_Base_Transceive(MU_Type *mu,
                                    const uint32_t *msg,
                                    uint32_t msgWords,
                                    uint32_t *resp,
                                    uint8_t command)
{
    status_t status;
    uint32_t respWords;
    uint32_t regIdx;
    uint32_t i;

    /* Send the request word-by-word through the MU transmit registers,
     * round-robining across the available TR registers (regIndex = word
     * counter modulo MU_TR_COUNT). MU_SendMsg blocks until the target TR
     * register is empty before writing. */
    for (i = 0u; i < msgWords; i++)
    {
        regIdx = i % (uint32_t)MU_TR_COUNT;
        status = MU_SendMsg(mu, regIdx, msg[i]);
        if (status != kStatus_Success)
        {
            return status;
        }
    }

    /* Collect the response. The header always arrives in RR[0]; its Size field
     * (bits 15-8) gives the total word count (including the header). The
     * remaining payload words are read round-robin across the RR registers
     * (regIndex = word counter modulo MU_RR_COUNT). */
    resp[0]   = MU_ReceiveMsg(mu, 0u);
    respWords = (resp[0] >> 8) & 0xFFu;
    if ((respWords == 0u) || (respWords > V2X_BASE_MSG_MAX_WORDS))
    {
        respWords = V2X_BASE_MSG_MAX_WORDS;
    }
    for (i = 1u; i < respWords; i++)
    {
        regIdx  = i % (uint32_t)MU_RR_COUNT;
        resp[i] = MU_ReceiveMsg(mu, regIdx);
    }

    /* Validate response framing: tag and echoed command. */
    if (V2X_BASE_HDR_TAG(resp[0]) != V2X_BASE_MSG_TAG_RESP)
    {
        return kStatus_Fail;
    }
    if (V2X_BASE_HDR_CMD(resp[0]) != command)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

status_t V2X_Base_StartRng(MU_Type *mu)
{
    uint32_t msg[V2X_BASE_START_RNG_REQ_WORDS];
    uint32_t resp[V2X_BASE_MSG_MAX_WORDS];
    status_t status;

    if (mu == NULL)
    {
        return kStatus_InvalidArgument;
    }

    msg[0] = V2X_BASE_CMD_HEADER(V2X_BASE_CMD_START_RNG, V2X_BASE_START_RNG_REQ_WORDS);

    status = V2X_Base_Transceive(mu, msg, V2X_BASE_START_RNG_REQ_WORDS, resp, V2X_BASE_CMD_START_RNG);
    if (status != kStatus_Success)
    {
        return status;
    }

    if (V2X_BASE_RESP_STATUS(resp[1]) != V2X_BASE_STATUS_SUCCESS)
    {
        /* Non-success: if the P-TRNG (bit 0) or S-TRNG (bit 1) initialization
         * is still in progress the caller may retry. */
        if ((resp[1] & (V2X_BASE_PTRNG_IN_PROGRESS | V2X_BASE_STRNG_IN_PROGRESS)) != 0u)
        {
            return kStatus_Busy;
        }
        return kStatus_Fail;
    }

    return kStatus_Success;
}

status_t V2X_Base_SetPowerState(MU_Type *mu, v2x_pwr_action_t action)
{
    uint32_t msg[V2X_BASE_PWR_STATE_REQ_WORDS];
    uint32_t resp[V2X_BASE_MSG_MAX_WORDS];
    status_t status;

    if (mu == NULL)
    {
        return kStatus_InvalidArgument;
    }
    if ((action != kV2X_PowerOn) && (action != kV2X_PowerOff))
    {
        return kStatus_InvalidArgument;
    }

    msg[0] = V2X_BASE_CMD_HEADER(V2X_BASE_CMD_PWR_STATE, V2X_BASE_PWR_STATE_REQ_WORDS);
    msg[1] = (uint32_t)action;

    status = V2X_Base_Transceive(mu, msg, V2X_BASE_PWR_STATE_REQ_WORDS, resp, V2X_BASE_CMD_PWR_STATE);
    if (status != kStatus_Success)
    {
        return status;
    }

    if (V2X_BASE_RESP_STATUS(resp[1]) != V2X_BASE_STATUS_SUCCESS)
    {
        /* Permission denied or invalid operation indications occupy the full
         * 16-bit status/rating field. */
        return kStatus_Fail;
    }

    return kStatus_Success;
}

status_t V2X_Base_GetFwVersion(MU_Type *mu, v2x_fw_version_t *version)
{
    uint32_t msg[V2X_BASE_GET_FW_VER_REQ_WORDS];
    uint32_t resp[V2X_BASE_MSG_MAX_WORDS];
    status_t status;

    if ((mu == NULL) || (version == NULL))
    {
        return kStatus_InvalidArgument;
    }

    msg[0] = V2X_BASE_CMD_HEADER(V2X_BASE_CMD_GET_FW_VER, V2X_BASE_GET_FW_VER_REQ_WORDS);

    status = V2X_Base_Transceive(mu, msg, V2X_BASE_GET_FW_VER_REQ_WORDS, resp, V2X_BASE_CMD_GET_FW_VER);
    if (status != kStatus_Success)
    {
        return status;
    }

    if (V2X_BASE_RESP_STATUS(resp[1]) != V2X_BASE_STATUS_SUCCESS)
    {
        return kStatus_Fail;
    }

    /* resp[2] holds the packed version; resp[3] the commit id. */
    version->major    = (resp[2] >> V2X_BASE_FW_MAJOR_SHIFT) & V2X_BASE_FW_MAJOR_MASK;
    version->minor    = (resp[2] >> V2X_BASE_FW_MINOR_SHIFT) & V2X_BASE_FW_MINOR_MASK;
    version->patch    = resp[2] & V2X_BASE_FW_PATCH_MASK;
    version->commitId = resp[3];

    return kStatus_Success;
}

status_t V2X_Base_DebugDump(MU_Type *mu, uint32_t *dump, uint32_t capacity, uint32_t *outWords)
{
    uint32_t msg[V2X_BASE_DBG_DUMP_REQ_WORDS];
    uint32_t resp[V2X_BASE_MSG_MAX_WORDS];
    status_t status;
    uint32_t words;
    uint32_t i;

    if ((mu == NULL) || (dump == NULL) || (outWords == NULL))
    {
        return kStatus_InvalidArgument;
    }

    *outWords = 0u;

    msg[0] = V2X_BASE_CMD_HEADER(V2X_BASE_CMD_DBG_DUMP, V2X_BASE_DBG_DUMP_REQ_WORDS);
    msg[1] = 0x1u; /* dump trigger: ask the V2X FW to stream its debug buffer
                    * (matches Linux v2x_debug_dump: tx_msg->data[0] = 0x1). */

    status = V2X_Base_Transceive(mu, msg, V2X_BASE_DBG_DUMP_REQ_WORDS, resp, V2X_BASE_CMD_DBG_DUMP);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* The debug dump is a special streaming reply. Unlike the other base
     * commands it does NOT carry the 0xD6 success indication in word 1, and
     * its Size field is variable. The Linux ELE driver validates it through a
     * dedicated exception_for_size() path that skips the ordinary status-byte
     * check and only bounds the Size field. Do the same here: accept any reply
     * whose Size is within the valid dump range and treat word 1 onward as
     * dump payload rather than a status word. */
    words = (resp[0] >> 8) & 0xFFu;
    if ((words < 2u) || (words > V2X_BASE_DBG_DUMP_RSP_WORDS))
    {
        return kStatus_Fail;
    }

    /* The Size field (header bits 15-8) gives the total word count, including
     * the header and the first V2X_BASE_NON_DUMP_BUFFER_SZ non-dump words. The
     * dump payload starts after them. */
    if (words <= V2X_BASE_NON_DUMP_BUFFER_SZ)
    {
        return kStatus_Success; /* nothing to dump */
    }
    words -= V2X_BASE_NON_DUMP_BUFFER_SZ;
    if (words > capacity)
    {
        words = capacity;
    }

    for (i = 0u; i < words; i++)
    {
        dump[i] = resp[V2X_BASE_NON_DUMP_BUFFER_SZ + i];
    }
    *outWords = words;

    return kStatus_Success;
}
