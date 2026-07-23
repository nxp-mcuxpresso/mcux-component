/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pf9453.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Component ID definition, used by SDK component tracking. */
#ifndef FSL_COMPONENT_ID
#define FSL_COMPONENT_ID "driver.pf9453"
#endif

/* Per-regulator voltage encoding: V = baseMv + code * step, code in the outMask field of outReg.
 * stepMvX100 is in hundredths of a millivolt so 12.5 mV is exact (1250). minMv/maxMv bound the
 * caller's request (checked before encoding). The BUCK2 hardware also clamps DVS to its
 * MAX/MIN_LIMIT registers (0x1F/0x20). */
typedef struct _pf9453_rail_info
{
    uint8_t outReg;      /* Output-voltage register (lock-key protected). */
    uint8_t ctrlReg;     /* Register holding *_ENMODE[1:0]. */
    uint8_t adReg;       /* Register holding the active-discharge bit (== outReg for LDO_SNVS). */
    uint8_t adMask;      /* Active-discharge bit within adReg. */
    uint8_t outMask;     /* Voltage field within outReg; other bits are preserved (RMW). */
    uint16_t baseMv;     /* Voltage of code 0. */
    uint16_t stepMvX100; /* Step in 1/100 mV (1250 = 12.5 mV, 2500 = 25 mV). */
    uint16_t minMv;      /* Minimum settable voltage. */
    uint16_t maxMv;      /* Maximum settable voltage. */
} pf9453_rail_info_t;

/* Private rail index into s_railInfo. The public API is per-rail (PF9453_ConfigBuck1..4 /
 * ConfigLdoSnvs/1/2) — there is no regulator-selector enum in the header. */
enum
{
    kRailBuck1 = 0,
    kRailBuck2,
    kRailBuck3,
    kRailBuck4,
    kRailLdoSnvs,
    kRailLdo1,
    kRailLdo2,
};

/* Rail table indexed by kRail*. LDO1 has two output regs selected by the SD_VSEL pin; ConfigLdo1
 * programs both (outReg here is the "high" one). For LDO_SNVS the active-discharge bit shares the
 * lock-protected voltage register (adReg == outReg), so ConfigLdoN takes the single-unlock+RMW
 * branch; note LDO_SNVS is the ONLY rail whose ctrlReg (CFG2, holding LS_ENMODE) differs from
 * adReg/outReg (CFG1) -- that branch writes enable-mode to ctrlReg and AD+voltage to the shared
 * reg, which is correct because the shared-register branch does not touch ctrlReg for AD/voltage.
 * Buck CTRL also carries FPWM (bit2) and, for BUCK2, the DVS ramp (bits[7:6]); handled in the
 * Config functions. */
static const pf9453_rail_info_t s_railInfo[] = {
    /* reg (out / ctrl / ad)                                            adMask outMask base  stepx100 min    max   */
    /* [kRailBuck1]   */ {PF9453_BUCK1OUT, PF9453_BUCK1CTRL, PF9453_BUCK1CTRL, 0x08U, 0x7FU, 600U, 2500U, 600U, 3775U},
    /* [kRailBuck2]   */ {PF9453_BUCK2OUT, PF9453_BUCK2CTRL, PF9453_BUCK2CTRL, 0x08U, 0x7FU, 600U, 1250U, 600U, 2188U},
    /* [kRailBuck3]   */ {PF9453_BUCK3OUT, PF9453_BUCK3CTRL, PF9453_BUCK3CTRL, 0x08U, 0x7FU, 600U, 2500U, 600U, 3775U},
    /* [kRailBuck4]   */ {PF9453_BUCK4OUT, PF9453_BUCK4CTRL, PF9453_BUCK4CTRL, 0x08U, 0x7FU, 600U, 2500U, 600U, 3775U},
    /* [kRailLdoSnvs] */
    {PF9453_LDOSNVS_CFG1, PF9453_LDOSNVS_CFG2, PF9453_LDOSNVS_CFG1, 0x80U, 0x7FU, 1200U, 2500U, 1200U, 3400U},
    /* [kRailLdo1]    */ {PF9453_LDO1_OUT_H, PF9453_LDO1_CFG, PF9453_LDO1_CFG, 0x10U, 0x7FU, 800U, 2500U, 800U, 3300U},
    /* [kRailLdo2]    */ {PF9453_LDO2_OUT, PF9453_LDO2_CFG, PF9453_LDO2_CFG, 0x80U, 0x3FU, 500U, 2500U, 500U, 1950U},
};

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Validate against the rail's min/max and encode a millivolt value to its register code. */
static status_t PF9453_EncodeVoltage(const pf9453_rail_info_t *rail, uint16_t millivolt, uint8_t *code)
{
    uint32_t c;

    if ((millivolt < rail->minMv) || (millivolt > rail->maxMv))
    {
        return kStatus_InvalidArgument;
    }

    c = ((uint32_t)(millivolt - rail->baseMv) * 100U) / (uint32_t)rail->stepMvX100;

    /* Reject (do not silently mask) a code that would not fit the voltage field. */
    if (c > (uint32_t)rail->outMask)
    {
        return kStatus_InvalidArgument;
    }

    *code = (uint8_t)c;
    return kStatus_Success;
}

/* Write a voltage-field code into a lock-key-protected register, preserving the other bits.
 * The current value is read BEFORE unlocking (reads are not protected), so the unlock immediately
 * precedes the protected write. */
static status_t PF9453_WriteVoltageField(pf9453_handle_t *handle, uint8_t reg, uint8_t mask, uint8_t code)
{
    status_t status;
    uint8_t old = 0U;
    uint8_t val;

    status = PF9453_ReadReg(handle, reg, &old);
    if (status != kStatus_Success)
    {
        return status;
    }
    val = (uint8_t)((old & (uint8_t)(~mask)) | (uint8_t)(code & mask));

    status = PF9453_UnlockRegulators(handle);
    if (status == kStatus_Success)
    {
        status = PF9453_WriteReg(handle, reg, val);
    }
    return status;
}

status_t PF9453_WriteReg(pf9453_handle_t *handle, uint8_t regAddr, uint8_t val)
{
    assert(handle != NULL);
    assert(handle->I2C_SendFunc != NULL);

    return handle->I2C_SendFunc(handle->slaveAddress, regAddr, 1U, &val, 1U);
}

status_t PF9453_ReadReg(pf9453_handle_t *handle, uint8_t regAddr, uint8_t *val)
{
    assert(handle != NULL);
    assert(handle->I2C_ReceiveFunc != NULL);

    return handle->I2C_ReceiveFunc(handle->slaveAddress, regAddr, 1U, val, 1U);
}

status_t PF9453_ModifyReg(pf9453_handle_t *handle, uint8_t regAddr, uint8_t mask, uint8_t val)
{
    assert(handle != NULL);

    status_t status;
    uint8_t tmp8 = 0U;

    status = PF9453_ReadReg(handle, regAddr, &tmp8);
    if (status == kStatus_Success)
    {
        tmp8 &= (uint8_t)(~mask);
        tmp8 |= (uint8_t)(val & mask);
        status = PF9453_WriteReg(handle, regAddr, tmp8);
    }

    return status;
}

status_t PF9453_DumpReg(pf9453_handle_t *handle, uint8_t regAddr, uint8_t *buffer, uint8_t length)
{
    assert(handle != NULL);
    assert(handle->I2C_ReceiveFunc != NULL);
    assert(buffer != NULL);

    uint8_t i;
    uint32_t addr   = (uint32_t)regAddr; /* uint32_t avoids unsigned wrap (CERT INT30-C). */
    status_t status = kStatus_Success;

    for (i = 0U; i < length; i++)
    {
        status = PF9453_ReadReg(handle, (uint8_t)(addr & 0xFFU), buffer);
        buffer++;
        addr++;
        if (status != kStatus_Success)
        {
            break;
        }
    }

    return status;
}

void PF9453_CreateHandle(pf9453_handle_t *handle, const pf9453_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);

    handle->I2C_SendFunc    = config->I2C_SendFunc;
    handle->I2C_ReceiveFunc = config->I2C_ReceiveFunc;
    handle->slaveAddress    = config->slaveAddress;
}

status_t PF9453_UnlockRegulators(pf9453_handle_t *handle)
{
    assert(handle != NULL);

    return PF9453_WriteReg(handle, PF9453_REG_LOCK, PF9453_UNLOCK_KEY);
}

/* Shared implementation for BUCK1/BUCK3/BUCK4 (single voltage, no standby register, no DVS ramp). */
static status_t PF9453_ConfigBuckN(pf9453_handle_t *handle, uint32_t railIdx, const pf9453_buck_config_t *config)
{
    const pf9453_rail_info_t *rail = &s_railInfo[railIdx];
    uint8_t ctrlVal;
    uint8_t ctrlMask;
    uint8_t code = 0U;
    status_t status;

    status = PF9453_EncodeVoltage(rail, config->millivolt, &code);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* CTRL: enable-mode + forced-PWM + active-discharge. Not lock-protected. */
    ctrlVal  = (uint8_t)((uint8_t)config->enableMode & PF9453_ENMODE_MASK);
    ctrlMask = (uint8_t)(PF9453_ENMODE_MASK | PF9453_BUCK_FPWM_MASK | rail->adMask);
    if (config->forcePwm)
    {
        ctrlVal |= PF9453_BUCK_FPWM_MASK;
    }
    if (config->activeDischarge)
    {
        ctrlVal |= rail->adMask;
    }
    status = PF9453_ModifyReg(handle, rail->ctrlReg, ctrlMask, ctrlVal);
    if (status == kStatus_Success)
    {
        status = PF9453_WriteVoltageField(handle, rail->outReg, rail->outMask, code);
    }
    return status;
}

status_t PF9453_ConfigBuck1(pf9453_handle_t *handle, const pf9453_buck_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);
    return PF9453_ConfigBuckN(handle, (uint32_t)kRailBuck1, config);
}

status_t PF9453_ConfigBuck3(pf9453_handle_t *handle, const pf9453_buck_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);
    return PF9453_ConfigBuckN(handle, (uint32_t)kRailBuck3, config);
}

status_t PF9453_ConfigBuck4(pf9453_handle_t *handle, const pf9453_buck_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);
    return PF9453_ConfigBuckN(handle, (uint32_t)kRailBuck4, config);
}

status_t PF9453_ConfigBuck2(pf9453_handle_t *handle, const pf9453_buck2_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);

    const pf9453_rail_info_t *rail = &s_railInfo[kRailBuck2];
    uint8_t ctrlVal;
    uint8_t ctrlMask;
    uint8_t runCode  = 0U;
    uint8_t stbyCode = 0U;
    status_t status;

    /* Validate both voltages up front so nothing is written on a bad request. */
    status = PF9453_EncodeVoltage(rail, config->runMillivolt, &runCode);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = PF9453_EncodeVoltage(rail, config->standbyMillivolt, &stbyCode);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* CTRL: enable-mode + forced-PWM + active-discharge + DVS ramp. Not lock-protected. */
    ctrlVal  = (uint8_t)((uint8_t)config->enableMode & PF9453_ENMODE_MASK);
    ctrlMask = (uint8_t)(PF9453_ENMODE_MASK | PF9453_BUCK_FPWM_MASK | rail->adMask | PF9453_BUCK2_RAMP_MASK);
    if (config->forcePwm)
    {
        ctrlVal |= PF9453_BUCK_FPWM_MASK;
    }
    if (config->activeDischarge)
    {
        ctrlVal |= rail->adMask;
    }
    ctrlVal |= (uint8_t)(((uint8_t)config->dvsRamp << PF9453_BUCK2_RAMP_SHIFT) & PF9453_BUCK2_RAMP_MASK);
    status = PF9453_ModifyReg(handle, rail->ctrlReg, ctrlMask, ctrlVal);

    /* Run voltage. */
    if (status == kStatus_Success)
    {
        status = PF9453_WriteVoltageField(handle, rail->outReg, rail->outMask, runCode);
    }

    /* Standby voltage (BUCK2 has a dedicated standby register, applied when PMIC_STBY_REQ = H). */
    if (status == kStatus_Success)
    {
        status = PF9453_WriteVoltageField(handle, PF9453_BUCK2OUT_STBY, rail->outMask, stbyCode);
    }

    return status;
}

status_t PF9453_SetBuck2RunVoltage(pf9453_handle_t *handle, uint16_t millivolt)
{
    assert(handle != NULL);

    const pf9453_rail_info_t *rail = &s_railInfo[kRailBuck2];
    uint8_t code                   = 0U;
    status_t status;

    status = PF9453_EncodeVoltage(rail, millivolt, &code);
    if (status == kStatus_Success)
    {
        /* DVS hot path: touch only the run output register (unlock + RMW of the voltage field). */
        status = PF9453_WriteVoltageField(handle, rail->outReg, rail->outMask, code);
    }
    return status;
}

/* Shared implementation for LDO_SNVS/LDO1/LDO2. */
static status_t PF9453_ConfigLdoN(pf9453_handle_t *handle, uint32_t railIdx, const pf9453_ldo_config_t *config)
{
    const pf9453_rail_info_t *rail = &s_railInfo[railIdx];
    uint8_t code                   = 0U;
    uint8_t adVal;
    status_t status;

    adVal = config->activeDischarge ? rail->adMask : 0U;

    status = PF9453_EncodeVoltage(rail, config->millivolt, &code);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Enable-mode lives in the (non-lock-protected) CTRL register. */
    status = PF9453_ModifyReg(handle, rail->ctrlReg, PF9453_ENMODE_MASK,
                              (uint8_t)((uint8_t)config->enableMode & PF9453_ENMODE_MASK));
    if (status != kStatus_Success)
    {
        return status;
    }

    if (rail->adReg == rail->outReg)
    {
        /* LDO_SNVS: active-discharge and voltage share the lock-protected register -> one unlock+RMW. */
        uint8_t old = 0U;
        uint8_t val;
        status = PF9453_ReadReg(handle, rail->outReg, &old);
        if (status != kStatus_Success)
        {
            return status;
        }
        val = (uint8_t)((old & (uint8_t)(~(rail->adMask | rail->outMask))) | adVal | (uint8_t)(code & rail->outMask));
        status = PF9453_UnlockRegulators(handle);
        if (status == kStatus_Success)
        {
            status = PF9453_WriteReg(handle, rail->outReg, val);
        }
    }
    else
    {
        /* Active-discharge is in the (unprotected) CTRL register; voltage is a separate protected reg. */
        status = PF9453_ModifyReg(handle, rail->adReg, rail->adMask, adVal);
        if (status == kStatus_Success)
        {
            status = PF9453_WriteVoltageField(handle, rail->outReg, rail->outMask, code);
        }
        /* LDO1 output is selected by the SD_VSEL pin; program the low-select register too. */
        if ((status == kStatus_Success) && (railIdx == (uint32_t)kRailLdo1))
        {
            status = PF9453_WriteVoltageField(handle, PF9453_LDO1_OUT_L, rail->outMask, code);
        }
    }

    return status;
}

status_t PF9453_ConfigLdoSnvs(pf9453_handle_t *handle, const pf9453_ldo_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);
    return PF9453_ConfigLdoN(handle, (uint32_t)kRailLdoSnvs, config);
}

status_t PF9453_ConfigLdo1(pf9453_handle_t *handle, const pf9453_ldo_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);
    return PF9453_ConfigLdoN(handle, (uint32_t)kRailLdo1, config);
}

status_t PF9453_ConfigLdo2(pf9453_handle_t *handle, const pf9453_ldo_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);
    return PF9453_ConfigLdoN(handle, (uint32_t)kRailLdo2, config);
}

status_t PF9453_SetStandbyTarget(pf9453_handle_t *handle, pf9453_standby_mode_t target)
{
    assert(handle != NULL);

    uint8_t val = (target == kPF9453_StandbyDeepStby) ? PF9453_SYS_CFG1_STANDBY_CFG_MASK : 0U;

    return PF9453_ModifyReg(handle, PF9453_SYS_CFG1, PF9453_SYS_CFG1_STANDBY_CFG_MASK, val);
}

status_t PF9453_EnableInterrupts(pf9453_handle_t *handle, uint8_t mask)
{
    assert(handle != NULL);

    /* INT1_MASK: 1 = masked/disabled, 0 = enabled. Clear the bits to enable. */
    return PF9453_ModifyReg(handle, PF9453_INT1_MASK, mask, 0U);
}

status_t PF9453_DisableInterrupts(pf9453_handle_t *handle, uint8_t mask)
{
    assert(handle != NULL);

    return PF9453_ModifyReg(handle, PF9453_INT1_MASK, mask, mask);
}

status_t PF9453_GetInterruptStatus(pf9453_handle_t *handle, uint8_t *flags)
{
    assert(handle != NULL);
    assert(flags != NULL);

    /* Non-destructive: INT1_STATUS is a read-only snapshot of the latched sources (unlike INT1,
     * which is read-to-clear). Polling this does not consume the sources. */
    return PF9453_ReadReg(handle, PF9453_INT1_STATUS, flags);
}

status_t PF9453_ClearInterruptStatus(pf9453_handle_t *handle)
{
    uint8_t tmp8 = 0U;
    status_t status;

    assert(handle != NULL);

    /* Read-to-clear INT1; the value itself is intentionally discarded. */
    status = PF9453_ReadReg(handle, PF9453_INT1, &tmp8);
    (void)tmp8;
    return status;
}
