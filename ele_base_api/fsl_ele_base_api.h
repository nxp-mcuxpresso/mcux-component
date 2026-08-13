/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_ELE_BASE_API_H_
#define _FSL_ELE_BASE_API_H_

#include "fsl_common.h"
#include "fsl_device_registers.h"

/*! @addtogroup ele_base_api */
/*! @{ */

/*! @file */

/*******************************************************************************
 * Configurations
 ******************************************************************************/

/*!
 * @brief Enables the EdgeLock enclave (ELE) V2X firmware bring-up API.
 *
 * Not all SoCs integrate a V2X firmware core. The bring-up API is compiled in
 * only when the ELE_HAS_V2X_FW_BRINGUP Kconfig option is selected (disabled by
 * default; SoCs with a V2X firmware core select it in their device Kconfig). A
 * board/app may override this by defining the macro before including this file.
 */
#if !defined(FSL_FEATURE_ELE_HAS_V2X_FW_BRINGUP)
#if defined(ELE_HAS_V2X_FW_BRINGUP)
#define FSL_FEATURE_ELE_HAS_V2X_FW_BRINGUP (1)
#else
#define FSL_FEATURE_ELE_HAS_V2X_FW_BRINGUP (0)
#endif
#endif


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RESPONSE_SUCCESS                                                   (0xd6u)
#define SHIFT_16                                                           (16u)
#define SHIFT_8                                                            (8u)
#define MSG_RESPONSE_MAX                                                   (16u)
#define MSG_TAG_CMD                                                        (0x17u)
#define MSG_TAG_RESP                                                       (0xE1u)

#define PING                                                               (0x17010106u)
#define PING_SIZE                                                          (0x1u)
#define PING_RESPONSE_HDR                                                  (0xe1010206u)

#define GET_FW_VERSION                                                     (0x179D0106u)
#define GET_FW_VERSION_SIZE                                                (0x1u)
#define GET_FW_VERSION_RESPONSE_HDR                                        (0xe19D0406u)

#define CLOCK_CHANGE_START                                                 (0x17100106u)
#define CLOCK_CHANGE_START_SIZE                                            (0x1u)
#define CLOCK_CHANGE_START_RESPONSE_HDR                                    (0xe1100206u)

#define CLOCK_CHANGE_FINISH                                                (0x17110206u)
#define CLOCK_CHANGE_FINISH_SIZE                                           (0x2u)
#define CLOCK_CHANGE_FINISH_RESPONSE_HDR                                   (0xe1110206u)

#define VOLTAGE_CHANGE_START                                               (0x17120106u)
#define VOLTAGE_CHANGE_START_SIZE                                          (0x1u)
#define VOLTAGE_CHANGE_START_RESPONSE_HDR                                  (0xe1120206u)

#define VOLTAGE_CHANGE_FINISH                                              (0x17130106u)
#define VOLTAGE_CHANGE_FINISH_SIZE                                         (0x1u)
#define VOLTAGE_CHANGE_FINISH_RESPONSE_HDR                                 (0xe1130206u)

#define GET_FW_STATUS                                                      (0x17C50106u)
#define GET_FW_STATUS_SIZE                                                 (0x1u)
#define GET_FW_STATUS_RESPONSE_HDR                                         (0xe1C50306u)

#define ENABLE_APC                                                         (0x17D20106u)
#define ENABLE_APC_SIZE                                                    (0x1u)
#define ENABLE_APC_RESPONSE_HDR                                            (0xe1D20206u)

#define START_RNG                                                          (0x17a30106u)
#define START_RNG_SIZE                                                     (0x1u)
#define START_RNG_RESPONSE_HDR                                             (0xe1a30206u)

#define FORWARD_LIFECYCLE                                                  (0x17950206u)
#define FORWARD_LIFECYCLE_SIZE                                             (0x2u)
#define FORWARD_LIFECYCLE_RESPONSE_HDR                                     (0xe1950206u)

#if defined(FSL_FEATURE_SOC_OTFAD_COUNT) && (FSL_FEATURE_SOC_OTFAD_COUNT > 1)
#define ENABLE_OTFAD                                                       (0x17c60206u)
#define ENABLE_OTFAD_SIZE                                                  (2u)
#else
#define ENABLE_OTFAD                                                       (0x17c60106u)
#define ENABLE_OTFAD_SIZE                                                  (1u)
#endif  /* FSL_FEATURE_SOC_OTFAD_COUNT */
#define ENABLE_OTFAD_RESPONSE_HDR                                          (0xe1c60206u)

#define RELEASE_RDC                                                        (0x17C40206u)
#define RELEASE_RDC_SIZE                                                   (0x2u)
#define RELEASE_RDC_RESPONSE_HDR                                           (0xe1C40206u)

/*! @name Driver version */
/*@{*/
/*! @brief Defines ELE Base API version 1.0.1.*/
#define FSL_ELE_BASE_API_DRIVER_VERSION (MAKE_VERSION(1, 0, 1))

#if FSL_FEATURE_ELE_HAS_V2X_FW_BRINGUP
/*! @name V2X firmware bring-up (ELE) command wire format */
/*@{*/
#define ELE_V2X_FW_AUTH_CMD  (0xB1u) /*!< ELE_V2X_FW_AUTH command byte. */
#define ELE_V2X_GET_STATE_CMD (0xB2u) /*!< ELE get-V2X-FW-state command byte. */
#define ELE_V2X_MSG_VERSION  (0x06u) /*!< ELE message version for V2X bring-up. */
#define ELE_V2X_FW_AUTH_SIZE (4u)    /*!< ELE_V2X_FW_AUTH request word count. */
#define ELE_V2X_GET_STATE_SIZE (1u)  /*!< ELE get-state request word count. */
#define ELE_V2X_FW_STATE_MASK (0xFFu) /*!< V2X FW state byte mask. */
#define ELE_V2X_FW_STATE_NORM_RUNNING (0x13u) /*!< V2X FW running (normal). */
#define ELE_V2X_FW_STATE_DBG_RUNNING  (0x15u) /*!< V2X FW running (debug). */
/*@}*/
#endif /* FSL_FEATURE_ELE_HAS_V2X_FW_BRINGUP */

/*@}*/


/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Ping ELE
 *
 * This function Ping EdgeLock Enclave, can be sent at any time to verify ELE is alive.
 * Additionally, this command reloads the fuse shadow registers and kick the Sentinel active bit.
 * This active bit must be kicked at least once every day (24 hours).
 *
 * @param mu MU peripheral base address
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_Ping(S3MU_Type *mu);

/*!
 * @brief Get ELE FW Version
 *
 * This function is used to retrieve the Sentinel FW version.
 *
 * @param mu MU peripheral base address
 * @param EleFwVersion Pointer where ElE firmware version will be stored
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_GetFwVersion(S3MU_Type *mu, uint32_t *EleFwVersion);

/*!
 * @brief Get ELE FW Status
 *
 * This function is used to retrieve the Sentinel FW status.
 *
 * @param mu MU peripheral base address
 * @param EleFwStatus Pointer where ElE firmware status will be stored
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_GetFwStatus(S3MU_Type *mu, uint32_t *EleFwStatus);

/*!
 * @brief Enable APC (Application core)
 *
 * This function is used by RTC (real time core) to release APC (Application core) when needed.
 *
 * @param mu MU peripheral base address
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_EnableAPC(S3MU_Type *mu);

/*!
 * @brief Forward Lifecycle update
 *
 * This function is to change chip lifecycle
 *  0x01U for NXP provisoned
 *  0x02U for OEM Open
 *  0x08U for OEM Closed
 *  0x80U for OEM Locked
 *
 * @param mu MU peripheral base address
 * @param Lifecycle to switch
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_ForwardLifecycle(S3MU_Type *mu, uint32_t Lifecycle);

/*!
 * @brief Release RDC
 *
 * This function is used to release specifed RDC to the core identified in this function.
 * The RDC will be released only if the FW of the core to which is the RDC ownership is going to be
 * transferred has been properly authenticated and verified.
 *
 * @param mu MU peripheral base address
 * @param RdcID Resource Domain Control identifier
 * @param CoreID Core identifier
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_ReleaseRDC(S3MU_Type *mu, uint32_t RdcID, uint32_t CoreID);

/*!
 * @brief Start the initialization of the RNG context.
 *
 * The RNG must be started before using some of the ELE services.
 *
 * @param mu MU peripheral base address
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_StartRng(S3MU_Type *mu);

/*!
 * @brief Enable an instance of OTFAD.
 *
 * @param mu MU peripheral base address
 * @param OtfadID ID of the OTFAD instance to enable - used only if there are
 * multiple instances on the SoC
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_EnableOtfad(S3MU_Type *mu, uint8_t OtfadID);

/*!
 * @brief Start the clock change process
 *
 * @param mu MU peripheral base address
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_ClockChangeStart(S3MU_Type *mu);

/*!
 * @brief Change ELE and/or CM33 clock
 *
 * It is valid to pass both parameters at the same time if the SoC supports both.
 *
 * @param mu MU peripheral base address
 * @param NewClockRateELE the new clock rate for ELE
 * @param NewClockRateCM33 the new clock rate for the CM33 core
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_ClockChangeFinish(S3MU_Type *mu, uint8_t NewClockRateELE, uint8_t NewClockRateCM33);

/*!
 * @brief Start the voltage change process
 *
 * @param mu MU peripheral base address
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_VoltageChangeStart(S3MU_Type *mu);

/*!
 * @brief Finish the voltage change process
 *
 * @param mu MU peripheral base address
 *
 * @return Status kStatus_Success if success, kStatus_Fail if fail
 * Possible errors: kStatus_S3MU_InvalidArgument, kStatus_S3MU_AgumentOutOfRange
 */
status_t ELE_BaseAPI_VoltageChangeFinish(S3MU_Type *mu);


#if FSL_FEATURE_ELE_HAS_V2X_FW_BRINGUP
/*!
 * @brief Ask the EdgeLock enclave (ELE) to authenticate and release the V2X FW.
 *
 * Sends ELE_V2X_FW_AUTH (0xB1) over the ELE MU with the DDR address of the V2X
 * firmware image. On i.MX943 the V2X firmware core is not auto-started; the
 * enclave must authenticate/load it first, otherwise the V2X MU never answers.
 *
 * @param mu        ELE message unit base.
 * @param v2xFwAddr DDR address of the V2X firmware image.
 * @retval kStatus_Success        Authentication request accepted.
 * @retval kStatus_InvalidArgument Argument invalid.
 * @retval kStatus_Fail           Command failed.
 */
status_t ELE_BaseAPI_V2xFwAuthenticate(S3MU_Type *mu, uint32_t v2xFwAddr);

/*!
 * @brief Query the V2X firmware state through the ELE.
 *
 * Sends ELE_GET_STATE (0xB2) over the ELE MU and returns the V2X FW state byte.
 *
 * @param mu    ELE message unit base.
 * @param state Output V2X firmware state (low byte of the ELE response).
 * @retval kStatus_Success        State read.
 * @retval kStatus_InvalidArgument Argument invalid.
 * @retval kStatus_Fail           Command failed.
 */
status_t ELE_BaseAPI_GetV2xFwState(S3MU_Type *mu, uint32_t *state);

/*!
 * @brief Test whether a V2X firmware state value means the V2X FW is running.
 *
 * @param state State value from ELE_BaseAPI_GetV2xFwState().
 * @return true if the state is a running state (0x13 or 0x15).
 */
bool ELE_BaseAPI_IsV2xFwRunning(uint32_t state);

/*!
 * @brief Bring the V2X firmware up through the ELE (authenticate + poll state).
 *
 * If the V2X FW is not already running, this authenticates it via
 * ELE_BaseAPI_V2xFwAuthenticate() then polls ELE_BaseAPI_GetV2xFwState() until
 * the state is running or the retry budget is exhausted.
 *
 * @param mu        ELE message unit base.
 * @param v2xFwAddr DDR address of the V2X firmware image.
 * @param retries   Maximum number of get-state polls.
 * @retval kStatus_Success V2X firmware is running.
 * @retval kStatus_Timeout V2X firmware did not reach a running state.
 * @retval kStatus_Fail    A bring-up command failed.
 */
status_t ELE_BaseAPI_BringUpV2xFw(S3MU_Type *mu, uint32_t v2xFwAddr, uint32_t retries);
#endif /* FSL_FEATURE_ELE_HAS_V2X_FW_BRINGUP */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*! @} */


#endif /* _FSL_ELE_BASE_API_H_ */
