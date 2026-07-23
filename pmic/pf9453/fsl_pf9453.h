/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef FSL_PF9453_H_
#define FSL_PF9453_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"

/*!
 * @addtogroup pf9453
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @name Driver version */
/*! @{ */
/*! @brief PF9453 driver version. */
#define FSL_PF9453_DRIVER_VERSION (MAKE_VERSION(1, 0, 0))
/*! @} */

/*!
 * @note Thread-safety: every API performs a BLOCKING I2C transfer through the caller-supplied
 * functions, so no API is ISR-safe (call from thread/main context only) and none is re-entrant on a
 * shared handle. The read-modify-write paths (ModifyReg, SetRegulatorVoltage, SetRegulatorMode,
 * SetStandbyTarget, Enable/DisableInterrupts) are especially sensitive: the caller must serialize
 * access to a given PF9453 (e.g. an I2C-bus mutex) if more than one context touches it.
 */

/*!
 * @name PF9453 register map (addresses).
 *
 * Complete register map per the PF9453 Product Data Sheet Rev 2.0 (Nov 2025), Table 17 (reserved
 * registers omitted). NOTE: the 7-bit I2C slave address is OTP-dependent (Default 0x32 / OTP1 0x33 /
 * OTP2 0x36 / OTP3 0x37, DS Table 16); the caller supplies it via pf9453_config_t.slaveAddress.
 * @{
 */
#define PF9453_DEV_ID             (0x01U) /*!< CHIP_ID[7:4] / CHIP_REV[3:0]. */
#define PF9453_INT1               (0x02U) /*!< Interrupt source (read/clear). */
#define PF9453_INT1_MASK          (0x03U) /*!< Interrupt mask (1 = masked). */
#define PF9453_INT1_STATUS        (0x04U) /*!< Interrupt status (read only). */
#define PF9453_VRFLT1_INT         (0x05U) /*!< Regulator-fault interrupt (write-1-clear). */
#define PF9453_VRFLT1_MASK        (0x06U) /*!< Regulator-fault interrupt mask. */
#define PF9453_PWR_STATE          (0x07U) /*!< PWR_STAT[7:4] / PWR_MODE[3:0]. */
#define PF9453_RESET_CTRL         (0x08U) /*!< Reset behaviour config. */
#define PF9453_SW_RST             (0x09U) /*!< Software reset (key). */
#define PF9453_PWR_SEQ_CTRL       (0x0AU) /*!< Debounce / sequence step timers. */
#define PF9453_SYS_CFG1           (0x0BU) /*!< STANDBY_CFG[5] selects STANDBY vs DPSTANDBY. */
#define PF9453_SYS_CFG2           (0x0CU) /*!< ONKEY timer / VSYS UVLO. */
#define PF9453_CLK_32K_CFG        (0x0DU) /*!< CLK_32K_DIS[0]. */
#define PF9453_BUCK1CTRL          (0x10U) /*!< BUCK1 discharge/FPWM/enable-mode. */
#define PF9453_BUCK1OUT           (0x11U) /*!< BUCK1 voltage (25 mV, code0 = 0.6 V). */
#define PF9453_BUCK2CTRL          (0x14U) /*!< BUCK2 ramp/discharge/FPWM/enable-mode. */
#define PF9453_BUCK2OUT           (0x15U) /*!< BUCK2 run voltage (12.5 mV) - PMIC_STBY_REQ = L. */
#define PF9453_BUCK2OUT_STBY      (0x1DU) /*!< BUCK2 standby voltage - PMIC_STBY_REQ = H. */
#define PF9453_BUCK2OUT_MAX_LIMIT (0x1FU) /*!< BUCK2 DVS upper clamp. */
#define PF9453_BUCK2OUT_MIN_LIMIT (0x20U) /*!< BUCK2 DVS lower clamp. */
#define PF9453_BUCK3CTRL          (0x21U) /*!< BUCK3 discharge/FPWM/enable-mode. */
#define PF9453_BUCK3OUT           (0x22U) /*!< BUCK3 voltage (25 mV, code0 = 0.6 V). */
#define PF9453_BUCK4CTRL          (0x2EU) /*!< BUCK4 discharge/FPWM/enable-mode. */
#define PF9453_BUCK4OUT           (0x2FU) /*!< BUCK4 voltage (25 mV, code0 = 0.6 V). */
#define PF9453_LDO1_OUT_L         (0x36U) /*!< LDO1 voltage when SD_VSEL = L (25 mV, code0 = 0.8 V). */
#define PF9453_LDO1_CFG           (0x37U) /*!< LDO1 enable/discharge. */
#define PF9453_LDO1_OUT_H         (0x38U) /*!< LDO1 voltage when SD_VSEL = H. */
#define PF9453_LDOSNVS_CFG1       (0x39U) /*!< LDO_SNVS: LSNVS_AD[7], L_SNVS_OUT[6:0] (always-on). */
#define PF9453_LDOSNVS_CFG2       (0x3AU) /*!< LDO_SNVS config 2 (LS_ENMODE). */
#define PF9453_LDO2_CFG           (0x3BU) /*!< LDO2 enable/discharge/mode (L2_ENMODE[1:0]). */
#define PF9453_LDO2_OUT           (0x3CU) /*!< LDO2: L2_INL2_MDET[7], L2_INL2_VSEL[6], L2_OUT[5:0]. */
#define PF9453_BUCK_POK_F_CFG     (0x3DU) /*!< Per-buck POK fault config. */
/* Load switch: no typed API (the MIMXRT2660-EVK does not use it) - reach via PF9453_WriteReg/ReadReg. */
#define PF9453_LSW_CTRL1 (0x40U) /*!< Load-switch discharge (LSW_AD) / enable (LSW_EN). */
#define PF9453_LSW_CTRL2 (0x41U) /*!< Load-switch short-circuit / over-current cfg. */
#define PF9453_REG_LOCK  (0x4EU) /*!< Write PF9453_UNLOCK_KEY to unlock the *OUT regs. */
/*! @} */

/*! @brief Key written to PF9453_REG_LOCK to unlock the lock-key-protected output registers. */
#define PF9453_UNLOCK_KEY (0x5CU)

/*! @brief CHIP_ID nibble (PF9453_DEV_ID[7:4]) for the PF9453 QFN part. */
#define PF9453_CHIP_ID_MASK   (0xF0U)
#define PF9453_CHIP_ID_PF9453 (0xB0U) /*!< CHIP_ID = 0b1011 (PF9453 QFN). */

/*! @brief Per-regulator enable-mode field (`B*_ENMODE` / `L*_ENMODE`, bits[1:0] of the CTRL reg). */
#define PF9453_ENMODE_MASK (0x03U)
/*! @brief SYS_CFG1 STANDBY_CFG bit: 0 = STANDBY mode, 1 = DPSTANDBY, when PMIC_STBY_REQ is high. */
#define PF9453_SYS_CFG1_STANDBY_CFG_MASK (0x20U)

/*! @brief Regulator enable-mode encoding (`*_ENMODE`). */
typedef enum _pf9453_enable_mode
{
    kPF9453_EnableOff             = 0U, /*!< OFF in every state. */
    kPF9453_EnableOnRun           = 1U, /*!< ON in RUN only. */
    kPF9453_EnableOnActiveStandby = 2U, /*!< ON in ACTIVE/STANDBY, OFF in DPSTANDBY. */
    kPF9453_EnableOnActive        = 3U, /*!< ON in ACTIVE, OFF in STANDBY/DPSTANDBY. */
} pf9453_enable_mode_t;

/* Each buck/LDO has its own Config API (PF9453_ConfigBuck1..4 / ConfigLdoSnvs/1/2) — there is no
 * regulator-selector enum. On the MIMXRT2660-EVK: BUCK1->VDD_IOx, BUCK2->VDD_CORE (DVS),
 * BUCK3->1.8 V group, BUCK4->WL_3V3, LDO_SNVS->VBAT_AON (always-on), LDO1->VDD_0V8, LDO2->VREFP_ANA. */

/*! @brief BUCK2 standby target selected by SYS_CFG1.STANDBY_CFG. */
typedef enum _pf9453_standby_mode
{
    kPF9453_StandbySbyt     = 0U, /*!< STANDBY: regulators stay on at their standby voltage (deep sleep). */
    kPF9453_StandbyDeepStby = 1U, /*!< DPSTANDBY: regulators marked ACTIVE-only turn off (power down). */
} pf9453_standby_mode_t;

/*! @name CTRL-register field masks (used by the per-regulator Config APIs) */
/*! @{ */
#define PF9453_BUCK_FPWM_MASK   (0x04U) /*!< B*_FPWM (buck CTRL bit2). */
#define PF9453_BUCK_AD_MASK     (0x08U) /*!< B*_AD active discharge (buck CTRL bit3). */
#define PF9453_BUCK2_RAMP_MASK  (0xC0U) /*!< B2_RAMP DVS speed (BUCK2CTRL bits[7:6]). */
#define PF9453_BUCK2_RAMP_SHIFT (6U)
#define PF9453_LDO1_AD_MASK     (0x10U) /*!< L1_AD (LDO1_CFG bit4). */
#define PF9453_LDO2_AD_MASK     (0x80U) /*!< L2_AD (LDO2_CFG bit7). */
#define PF9453_LDOSNVS_AD_MASK  (0x80U) /*!< LSNVS_AD (LDOSNVS_CFG1 bit7). */
/*! @} */

/*! @brief BUCK2 DVS ramp rate (BUCK2CTRL `B2_RAMP`[7:6]). */
typedef enum _pf9453_buck2_ramp
{
    kPF9453_Buck2Ramp_25mVPer1us = 0U, /*!< 25 mV / 1 us. */
    kPF9453_Buck2Ramp_25mVPer2us = 1U, /*!< 25 mV / 2 us (reset default). */
    kPF9453_Buck2Ramp_25mVPer4us = 2U, /*!< 25 mV / 4 us. */
    kPF9453_Buck2Ramp_25mVPer8us = 3U, /*!< 25 mV / 8 us. */
} pf9453_buck2_ramp_t;

/*!
 * @brief Buck configuration for BUCK1 / BUCK3 / BUCK4 (all 0.6-3.775 V, 25 mV).
 *
 * `millivolt` is in millivolts and validated against the rail's range.
 */
typedef struct _pf9453_buck_config
{
    uint16_t millivolt;              /*!< Output voltage in mV. */
    pf9453_enable_mode_t enableMode; /*!< Which power states the rail is enabled in. */
    bool forcePwm;                   /*!< true = forced PWM; false = auto PFM/PWM. */
    bool activeDischarge;            /*!< Active-discharge resistor when the rail is OFF. */
} pf9453_buck_config_t;

/*!
 * @brief BUCK2 configuration (VDD_CORE, DVS-capable; 0.6-2.1875 V, 12.5 mV).
 *
 * BUCK2 additionally has a standby output voltage (applied when PMIC_STBY_REQ = H) and a DVS ramp
 * rate. `runMillivolt` / `standbyMillivolt` are in millivolts and validated.
 */
typedef struct _pf9453_buck2_config
{
    uint16_t runMillivolt;           /*!< Run output voltage in mV (PMIC_STBY_REQ = L). */
    uint16_t standbyMillivolt;       /*!< Standby output voltage in mV (PMIC_STBY_REQ = H). */
    pf9453_enable_mode_t enableMode; /*!< Which power states the rail is enabled in. */
    bool forcePwm;                   /*!< true = forced PWM; false = auto PFM/PWM. */
    bool activeDischarge;            /*!< Active-discharge resistor when the rail is OFF. */
    pf9453_buck2_ramp_t dvsRamp;     /*!< DVS ramp rate. */
} pf9453_buck2_config_t;

/*! @brief LDO configuration (LDO_SNVS / LDO1 / LDO2). */
typedef struct _pf9453_ldo_config
{
    uint16_t millivolt;              /*!< Output voltage in mV (LDO1 programs both SD_VSEL banks). */
    pf9453_enable_mode_t enableMode; /*!< Which power states the rail is enabled in. */
    bool activeDischarge;            /*!< Active-discharge resistor when the rail is OFF. */
} pf9453_ldo_config_t;

/*!
 * @brief PF9453 runtime handle: I2C access functions + slave address.
 */
typedef struct _pf9453_handle
{
    status_t (*I2C_SendFunc)(uint8_t deviceAddress,
                             uint32_t subAddress,
                             uint8_t subAddressSize,
                             uint8_t *txBuff,
                             uint8_t txBuffSize); /*!< Pointer to I2C send function. */
    status_t (*I2C_ReceiveFunc)(uint8_t deviceAddress,
                                uint32_t subAddress,
                                uint8_t subAddressSize,
                                uint8_t *rxBuff,
                                uint8_t rxBuffSize); /*!< Pointer to I2C receive function. */
    uint8_t slaveAddress;                            /*!< Device slave address. */
} pf9453_handle_t;

/*!
 * @brief PF9453 initialize configuration.
 */
typedef struct _pf9453_config
{
    status_t (*I2C_SendFunc)(uint8_t deviceAddress,
                             uint32_t subAddress,
                             uint8_t subAddressSize,
                             uint8_t *txBuff,
                             uint8_t txBuffSize); /*!< Pointer to I2C send function. */
    status_t (*I2C_ReceiveFunc)(uint8_t deviceAddress,
                                uint32_t subAddress,
                                uint8_t subAddressSize,
                                uint8_t *rxBuff,
                                uint8_t rxBuffSize); /*!< Pointer to I2C receive function. */
    uint8_t slaveAddress;                            /*!< Device slave address. */
} pf9453_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*! @name Basic interfaces
 * @{
 */

/*!
 * @brief Write to a PF9453 register via I2C.
 * @param handle  Pointer to @ref pf9453_handle_t.
 * @param regAddr Register address.
 * @param val     Value to write.
 * @return kStatus_Success on success, otherwise the I2C send function's status.
 */
status_t PF9453_WriteReg(pf9453_handle_t *handle, uint8_t regAddr, uint8_t val);

/*!
 * @brief Read a PF9453 register via I2C.
 * @param handle  Pointer to @ref pf9453_handle_t.
 * @param regAddr Register address.
 * @param val     Pointer that receives the read value.
 * @return kStatus_Success on success, otherwise the I2C receive function's status.
 */
status_t PF9453_ReadReg(pf9453_handle_t *handle, uint8_t regAddr, uint8_t *val);

/*!
 * @brief Read-modify-write a PF9453 register.
 * @param handle  Pointer to @ref pf9453_handle_t.
 * @param regAddr Register address.
 * @param mask    Bits to modify.
 * @param val     New value for the masked bits.
 * @return kStatus_Success on success, otherwise the failing transaction's status.
 */
status_t PF9453_ModifyReg(pf9453_handle_t *handle, uint8_t regAddr, uint8_t mask, uint8_t val);

/*!
 * @brief Dump a run of consecutive registers.
 * @param handle  Pointer to @ref pf9453_handle_t.
 * @param regAddr First register address.
 * @param buffer  Destination buffer.
 * @param length  Number of registers to read.
 * @return kStatus_Success on success, otherwise the failing read's status.
 */
status_t PF9453_DumpReg(pf9453_handle_t *handle, uint8_t regAddr, uint8_t *buffer, uint8_t length);

/*!
 * @brief Initialize a runtime handle from a config.
 *
 * The caller supplies every field: both I2C access functions and the device's 7-bit slave address
 * (board/part knowledge — the driver assumes no default). The driver keeps no state beyond the handle.
 * @param handle Pointer to @ref pf9453_handle_t.
 * @param config Pointer to @ref pf9453_config_t (all fields set by the caller).
 */
void PF9453_CreateHandle(pf9453_handle_t *handle, const pf9453_config_t *config);

/*! @} */

/*! @name Regulator control
 * @{
 */

/*!
 * @brief Unlock the lock-key-protected output-voltage registers.
 *
 * Writes @ref PF9453_UNLOCK_KEY to @ref PF9453_REG_LOCK. The lock re-arms, so this must be called
 * before every write to a *OUT register.
 * @param handle Pointer to @ref pf9453_handle_t.
 * @return kStatus_Success on success.
 */
status_t PF9453_UnlockRegulators(pf9453_handle_t *handle);

/*!
 * @brief Configure BUCK1 (0.6-3.775 V, 25 mV) — one call sets its CTRL + run voltage.
 *
 * Programs enable-mode / forced-PWM / active-discharge (CTRL, not lock-protected) and the run output
 * voltage (unlock + read-modify-write of the voltage field). `millivolt` is in mV and validated.
 * @param handle Pointer to @ref pf9453_handle_t.
 * @param config Caller-filled @ref pf9453_buck_config_t.
 * @retval kStatus_Success / kStatus_InvalidArgument (voltage out of range; nothing written).
 */
status_t PF9453_ConfigBuck1(pf9453_handle_t *handle, const pf9453_buck_config_t *config);

/*! @brief Configure BUCK3 (0.6-3.775 V, 25 mV). @copydetails PF9453_ConfigBuck1 */
status_t PF9453_ConfigBuck3(pf9453_handle_t *handle, const pf9453_buck_config_t *config);

/*! @brief Configure BUCK4 (0.6-3.775 V, 25 mV). @copydetails PF9453_ConfigBuck1 */
status_t PF9453_ConfigBuck4(pf9453_handle_t *handle, const pf9453_buck_config_t *config);

/*!
 * @brief Configure BUCK2 (VDD_CORE, DVS; 0.6-2.1875 V, 12.5 mV).
 *
 * Sets CTRL (enable-mode / forced-PWM / active-discharge / DVS ramp), the run voltage, and the standby
 * voltage (BUCK2OUT_STBY, applied when PMIC_STBY_REQ = H). Both voltages are validated up front.
 * @param handle Pointer to @ref pf9453_handle_t.
 * @param config Caller-filled @ref pf9453_buck2_config_t.
 * @retval kStatus_Success / kStatus_InvalidArgument (a voltage out of range; nothing written).
 */
status_t PF9453_ConfigBuck2(pf9453_handle_t *handle, const pf9453_buck2_config_t *config);

/*!
 * @brief Set ONLY BUCK2's run output voltage — the run-mode DVS hot path (no CTRL/standby touch).
 *
 * Encodes @p millivolt (mV, NOT a register code) as 0.6-2.1875 V / 12.5 mV (2188 -> top step
 * 2.1875 V) and writes just BUCK2OUT (unlock + read-modify-write of the voltage field).
 * @param handle    Pointer to @ref pf9453_handle_t.
 * @param millivolt Target run voltage in millivolts.
 * @retval kStatus_Success / kStatus_InvalidArgument (out of range; nothing written).
 */
status_t PF9453_SetBuck2RunVoltage(pf9453_handle_t *handle, uint16_t millivolt);

/*!
 * @brief Configure LDO_SNVS (always-on; VBAT_AON on the EVK; 1.2-3.4 V, 25 mV).
 * @param handle Pointer to @ref pf9453_handle_t.
 * @param config Caller-filled @ref pf9453_ldo_config_t.
 * @retval kStatus_Success / kStatus_InvalidArgument.
 * @note LDO_SNVS is board-critical — disabling it drops the always-on supply.
 */
status_t PF9453_ConfigLdoSnvs(pf9453_handle_t *handle, const pf9453_ldo_config_t *config);

/*!
 * @brief Configure LDO1 (0.8-3.3 V, 25 mV; programs both SD_VSEL banks).
 * @param handle Pointer to @ref pf9453_handle_t.
 * @param config Caller-filled @ref pf9453_ldo_config_t.
 * @retval kStatus_Success / kStatus_InvalidArgument.
 */
status_t PF9453_ConfigLdo1(pf9453_handle_t *handle, const pf9453_ldo_config_t *config);

/*!
 * @brief Configure LDO2 (0.5-1.95 V, 25 mV).
 * @param handle Pointer to @ref pf9453_handle_t.
 * @param config Caller-filled @ref pf9453_ldo_config_t.
 * @retval kStatus_Success / kStatus_InvalidArgument.
 */
status_t PF9453_ConfigLdo2(pf9453_handle_t *handle, const pf9453_ldo_config_t *config);

/*!
 * @brief Select the standby target (STANDBY vs DPSTANDBY) via SYS_CFG1.STANDBY_CFG.
 * @param handle Pointer to @ref pf9453_handle_t.
 * @param target @ref pf9453_standby_mode_t.
 * @return kStatus_Success on success.
 */
status_t PF9453_SetStandbyTarget(pf9453_handle_t *handle, pf9453_standby_mode_t target);

/*! @} */

/*! @name Interrupts
 * @{
 */

/*!
 * @brief Enable interrupt sources (clear the corresponding INT1_MASK bits).
 * @param handle Pointer to @ref pf9453_handle_t.
 * @param mask   INT1 bit mask to enable.
 * @return kStatus_Success on success.
 */
status_t PF9453_EnableInterrupts(pf9453_handle_t *handle, uint8_t mask);

/*!
 * @brief Disable interrupt sources (set the corresponding INT1_MASK bits).
 * @param handle Pointer to @ref pf9453_handle_t.
 * @param mask   INT1 bit mask to disable.
 * @return kStatus_Success on success.
 */
status_t PF9453_DisableInterrupts(pf9453_handle_t *handle, uint8_t mask);

/*!
 * @brief Read the latched interrupt sources without clearing them.
 *
 * Reads the read-only INT1_STATUS register (a non-destructive snapshot), so polling this does not
 * consume the latched sources. Use PF9453_ClearInterruptStatus() to acknowledge/clear them.
 * @param handle Pointer to @ref pf9453_handle_t.
 * @param flags  Pointer that receives the INT1_STATUS value.
 * @return kStatus_Success on success.
 */
status_t PF9453_GetInterruptStatus(pf9453_handle_t *handle, uint8_t *flags);

/*!
 * @brief Clear latched interrupt sources by reading INT1 (read-to-clear).
 * @param handle Pointer to @ref pf9453_handle_t.
 * @return kStatus_Success on success.
 */
status_t PF9453_ClearInterruptStatus(pf9453_handle_t *handle);

/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* FSL_PF9453_H_ */
