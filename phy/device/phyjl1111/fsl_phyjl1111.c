/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_phyjl1111.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Defines the JL1111 device ID information from the JL1111BI datasheet. */
#define PHY_CONTROL_ID1           (0x937CU) /*!< PHY Identifier 1 (OUI MSB). */
#define PHY_CONTROL_ID2_MASK      (0xFFF0U) /*!< PHY Identifier 2 OUI + model bits. */
#define PHY_CONTROL_ID2_OUI_MODEL (0x4020U) /*!< PHY Identifier 2 expected OUI + model value. */

/*! @brief Defines the JL1111 interrupt and vendor defined registers. */
#define PHY_JL1111_INTERRUPT_STATUS_REG (0x1EU) /*!< Interrupt Status and SNR Display Register. */
#define PHY_JL1111_PAGE_SELECT_REG      (0x1FU) /*!< Page Select Register. */
#define PHY_JL1111_RMII_PAGE            (0x07U) /*!< Page containing RMII, interrupt, and loopback controls. */
#define PHY_JL1111_INTR_ENABLE_REG      (0x13U) /*!< Interrupt WOL Enable and LEDs Function Register. */
#define PHY_JL1111_ECAT_LOOPBACK_REG    (0x19U) /*!< EtherCAT and Remote Loopback Register. */

/*! @brief Defines the JL1111 interrupt status/enable bit masks. */
#define PHY_JL1111_INTR_AN_COMPLETE_MASK ((uint16_t)0x8000U)
#define PHY_JL1111_INTR_SPEED_CHG_MASK   ((uint16_t)0x4000U)
#define PHY_JL1111_INTR_LINK_CHG_MASK    ((uint16_t)0x2000U)
#define PHY_JL1111_INTR_DUPLEX_CHG_MASK  ((uint16_t)0x1000U)
#define PHY_JL1111_INTR_AN_ERR_MASK      ((uint16_t)0x0800U)
#define PHY_JL1111_INTR_LINK_EVENT_MASK                                                                  \
    (PHY_JL1111_INTR_AN_COMPLETE_MASK | PHY_JL1111_INTR_SPEED_CHG_MASK | PHY_JL1111_INTR_LINK_CHG_MASK | \
     PHY_JL1111_INTR_DUPLEX_CHG_MASK | PHY_JL1111_INTR_AN_ERR_MASK)

/*! @brief Defines the JL1111 remote loopback enable bit. */
#define PHY_JL1111_REMOTE_LOOPBACK_MASK ((uint16_t)0x0001U)

/*! @brief Defines all 10/100M advertisement capabilities. */
#define PHY_ALL_CAPABLE_MASK                                                                        \
    (PHY_100BASETX_FULLDUPLEX_MASK | PHY_100BASETX_HALFDUPLEX_MASK | PHY_10BASETX_FULLDUPLEX_MASK | \
     PHY_10BASETX_HALFDUPLEX_MASK)

/*! @brief Defines the timeout macro. */
#define PHY_TIMEOUT_COUNT             (500000U)
#define PHY_RESET_COMPLETE_POLL_COUNT PHY_TIMEOUT_COUNT

/*! @brief Defines the PHY resource interface. */
#define PHY_JL1111_WRITE(handle, regAddr, data) \
    ((phy_jl1111_resource_t *)((handle)->resource))->write((handle)->phyAddr, regAddr, data)
#define PHY_JL1111_READ(handle, regAddr, pData) \
    ((phy_jl1111_resource_t *)((handle)->resource))->read((handle)->phyAddr, regAddr, pData)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void PHY_JL1111_GetLinkSpeedDuplexFromBasicControl(uint16_t regValue, phy_speed_t *speed, phy_duplex_t *duplex);
static status_t PHY_JL1111_GetLinkSpeedDuplexFromAutoNegotiation(phy_handle_t *handle,
                                                                 phy_speed_t *speed,
                                                                 phy_duplex_t *duplex);
static status_t PHY_JL1111_MMD_SetDevice(phy_handle_t *handle,
                                         uint8_t device,
                                         uint16_t addr,
                                         phy_mmd_access_mode_t mode);
static inline status_t PHY_JL1111_MMD_ReadData(phy_handle_t *handle, uint16_t *pData);
static inline status_t PHY_JL1111_MMD_WriteData(phy_handle_t *handle, uint16_t data);
static status_t PHY_JL1111_MMD_Read(phy_handle_t *handle, uint8_t device, uint16_t addr, uint16_t *pData);
static status_t PHY_JL1111_MMD_Write(phy_handle_t *handle, uint8_t device, uint16_t addr, uint16_t data);
static status_t PHY_JL1111_SelectPage(phy_handle_t *handle, uint8_t page);

/*******************************************************************************
 * Variables
 ******************************************************************************/

const phy_operations_t phyjl1111_ops = {.phyInit             = PHY_JL1111_Init,
                                        .phyWrite            = PHY_JL1111_Write,
                                        .phyRead             = PHY_JL1111_Read,
                                        .phyWriteC45         = PHY_JL1111_WriteC45,
                                        .phyReadC45          = PHY_JL1111_ReadC45,
                                        .getAutoNegoStatus   = PHY_JL1111_GetAutoNegotiationStatus,
                                        .getLinkStatus       = PHY_JL1111_GetLinkStatus,
                                        .getLinkSpeedDuplex  = PHY_JL1111_GetLinkSpeedDuplex,
                                        .setLinkSpeedDuplex  = PHY_JL1111_SetLinkSpeedDuplex,
                                        .enableLoopback      = PHY_JL1111_EnableLoopback,
                                        .enableLinkInterrupt = PHY_JL1111_EnableLinkInterrupt,
                                        .clearInterrupt      = PHY_JL1111_ClearInterrupt};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t PHY_JL1111_Init(phy_handle_t *handle, const phy_config_t *config)
{
    uint32_t counter  = PHY_TIMEOUT_COUNT;
    status_t result   = kStatus_Success;
    uint16_t regValue = 0U;
    uint16_t id1      = 0U;
    uint16_t id2      = 0U;

    /* Assign PHY address and operation resource. */
    handle->phyAddr  = config->phyAddr;
    handle->resource = config->resource;

    /* Check PHY ID. */
    do
    {
        result = PHY_JL1111_READ(handle, PHY_ID1_REG, &id1);
        if (result != kStatus_Success)
        {
            return result;
        }

        result = PHY_JL1111_READ(handle, PHY_ID2_REG, &id2);
        if (result != kStatus_Success)
        {
            return result;
        }

        counter--;
    } while (((id1 != PHY_CONTROL_ID1) || ((id2 & PHY_CONTROL_ID2_MASK) != PHY_CONTROL_ID2_OUI_MODEL)) &&
             (counter != 0U));

    if (counter == 0U)
    {
        return kStatus_Fail;
    }

    /* Reset PHY and wait until completion. Always perform at least one read. */
    result = PHY_JL1111_WRITE(handle, PHY_BASICCONTROL_REG, PHY_BCTL_RESET_MASK);
    if (result != kStatus_Success)
    {
        return result;
    }

    counter = PHY_RESET_COMPLETE_POLL_COUNT;

    while (true)
    {
        result = PHY_JL1111_READ(handle, PHY_BASICCONTROL_REG, &regValue);
        if (result != kStatus_Success)
        {
            return result;
        }

        if ((regValue & PHY_BCTL_RESET_MASK) == 0U)
        {
            break;
        }

        if (counter == 0U)
        {
            return kStatus_Fail;
        }

        counter--;
    }

    result = PHY_JL1111_ClearInterrupt(handle);
    if (result != kStatus_Success)
    {
        return result;
    }

    if (config->autoNeg)
    {
        /* JL1111BI is a 10/100 PHY, advertise all 10/100 capabilities. */
        result =
            PHY_JL1111_WRITE(handle, PHY_AUTONEG_ADVERTISE_REG, PHY_ALL_CAPABLE_MASK | PHY_IEEE802_3_SELECTOR_MASK);
        if (result == kStatus_Success)
        {
            result = PHY_JL1111_READ(handle, PHY_BASICCONTROL_REG, &regValue);
            if (result == kStatus_Success)
            {
                result = PHY_JL1111_WRITE(handle, PHY_BASICCONTROL_REG,
                                          regValue | PHY_BCTL_AUTONEG_MASK | PHY_BCTL_RESTART_AUTONEG_MASK);
            }
        }
    }
    else
    {
        /* JL1111BI is a 10/100 PHY. */
        assert(config->speed <= kPHY_Speed100M);

        /* Disable isolate mode. */
        result = PHY_JL1111_READ(handle, PHY_BASICCONTROL_REG, &regValue);
        if (result != kStatus_Success)
        {
            return result;
        }
        regValue &= ~PHY_BCTL_ISOLATE_MASK;
        result = PHY_JL1111_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
        if (result != kStatus_Success)
        {
            return result;
        }

        /* Disable the auto-negotiation and set user-defined speed/duplex configuration. */
        result = PHY_JL1111_SetLinkSpeedDuplex(handle, config->speed, config->duplex);
    }

    if (result != kStatus_Success)
    {
        return result;
    }

    return PHY_JL1111_EnableLinkInterrupt(handle, config->intrType);
}

status_t PHY_JL1111_Write(phy_handle_t *handle, uint8_t phyReg, uint16_t data)
{
    return PHY_JL1111_WRITE(handle, phyReg, data);
}

status_t PHY_JL1111_Read(phy_handle_t *handle, uint8_t phyReg, uint16_t *pData)
{
    return PHY_JL1111_READ(handle, phyReg, pData);
}

status_t PHY_JL1111_WriteC45(phy_handle_t *handle, uint8_t devAddr, uint16_t regAddr, uint16_t data)
{
    return PHY_JL1111_MMD_Write(handle, devAddr, regAddr, data);
}

status_t PHY_JL1111_ReadC45(phy_handle_t *handle, uint8_t devAddr, uint16_t regAddr, uint16_t *pData)
{
    return PHY_JL1111_MMD_Read(handle, devAddr, regAddr, pData);
}

status_t PHY_JL1111_GetAutoNegotiationStatus(phy_handle_t *handle, bool *status)
{
    assert(status);

    status_t result;
    uint16_t regValue;

    *status = false;

    result = PHY_JL1111_READ(handle, PHY_BASICSTATUS_REG, &regValue);
    if (result == kStatus_Success)
    {
        if ((regValue & PHY_BSTATUS_AUTONEGCOMP_MASK) != 0U)
        {
            *status = true;
        }
    }
    return result;
}

status_t PHY_JL1111_GetLinkStatus(phy_handle_t *handle, bool *status)
{
    assert(status);

    uint16_t regValue;
    status_t result;

    /* Clause 22 BMSR link bit is latch-low on many PHYs; read twice for current state. */
    result = PHY_JL1111_READ(handle, PHY_BASICSTATUS_REG, &regValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    result = PHY_JL1111_READ(handle, PHY_BASICSTATUS_REG, &regValue);
    if (result == kStatus_Success)
    {
        *status = ((regValue & PHY_BSTATUS_LINKSTATUS_MASK) != 0U);
    }
    return result;
}

status_t PHY_JL1111_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex)
{
    uint16_t regValue;
    status_t result;

    assert(!((speed == NULL) && (duplex == NULL)));

    result = PHY_JL1111_READ(handle, PHY_BASICCONTROL_REG, &regValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    if ((regValue & PHY_BCTL_AUTONEG_MASK) != 0U)
    {
        return PHY_JL1111_GetLinkSpeedDuplexFromAutoNegotiation(handle, speed, duplex);
    }
    else
    {
        PHY_JL1111_GetLinkSpeedDuplexFromBasicControl(regValue, speed, duplex);
        return kStatus_Success;
    }
}

status_t PHY_JL1111_SetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t speed, phy_duplex_t duplex)
{
    status_t result;
    uint16_t regValue;

    /* JL1111BI is a 10/100 PHY. */
    assert(speed <= kPHY_Speed100M);

    result = PHY_JL1111_READ(handle, PHY_BASICCONTROL_REG, &regValue);
    if (result == kStatus_Success)
    {
        /* Disable the auto-negotiation and set according to user-defined configuration. */
        regValue &= ~PHY_BCTL_AUTONEG_MASK;
        regValue &= ~(PHY_BCTL_SPEED0_MASK | PHY_BCTL_SPEED1_MASK);

        if (speed == kPHY_Speed100M)
        {
            regValue |= PHY_BCTL_SPEED0_MASK;
        }

        if (duplex == kPHY_FullDuplex)
        {
            regValue |= PHY_BCTL_DUPLEX_MASK;
        }
        else
        {
            regValue &= ~PHY_BCTL_DUPLEX_MASK;
        }
        result = PHY_JL1111_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
    }
    return result;
}

status_t PHY_JL1111_EnableLoopback(phy_handle_t *handle, phy_loop_t mode, phy_speed_t speed, bool enable)
{
    status_t result;
    uint16_t regValue;

    /* JL1111BI only supports 10/100M speed. */
    assert(speed <= kPHY_Speed100M);

    if (enable)
    {
        if (mode == kPHY_LocalLoop)
        {
            if (speed == kPHY_Speed100M)
            {
                regValue = PHY_BCTL_SPEED0_MASK | PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
            }
            else
            {
                regValue = PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
            }
            result = PHY_JL1111_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
        }
        else if (mode == kPHY_RemoteLoop)
        {
            result = PHY_JL1111_SelectPage(handle, PHY_JL1111_RMII_PAGE);
            if (result == kStatus_Success)
            {
                result = PHY_JL1111_READ(handle, PHY_JL1111_ECAT_LOOPBACK_REG, &regValue);
                if (result == kStatus_Success)
                {
                    result = PHY_JL1111_WRITE(handle, PHY_JL1111_ECAT_LOOPBACK_REG,
                                              regValue | PHY_JL1111_REMOTE_LOOPBACK_MASK);
                }
            }
            if (result == kStatus_Success)
            {
                result = PHY_JL1111_SelectPage(handle, 0U);
            }
        }
        else
        {
            result = kStatus_InvalidArgument;
        }
    }
    else
    {
        if (mode == kPHY_LocalLoop)
        {
            result = PHY_JL1111_READ(handle, PHY_BASICCONTROL_REG, &regValue);
            if (result == kStatus_Success)
            {
                regValue &= ~PHY_BCTL_LOOP_MASK;
                if ((regValue & PHY_BCTL_AUTONEG_MASK) != 0U)
                {
                    regValue |= PHY_BCTL_RESTART_AUTONEG_MASK;
                }
                result = PHY_JL1111_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
            }
        }
        else if (mode == kPHY_RemoteLoop)
        {
            result = PHY_JL1111_SelectPage(handle, PHY_JL1111_RMII_PAGE);
            if (result == kStatus_Success)
            {
                result = PHY_JL1111_READ(handle, PHY_JL1111_ECAT_LOOPBACK_REG, &regValue);
                if (result == kStatus_Success)
                {
                    result = PHY_JL1111_WRITE(handle, PHY_JL1111_ECAT_LOOPBACK_REG,
                                              regValue & ~PHY_JL1111_REMOTE_LOOPBACK_MASK);
                }
            }
            if (result == kStatus_Success)
            {
                result = PHY_JL1111_SelectPage(handle, 0U);
            }
        }
        else
        {
            result = kStatus_InvalidArgument;
        }
    }
    return result;
}

status_t PHY_JL1111_EnableLinkInterrupt(phy_handle_t *handle, phy_interrupt_type_t type)
{
    assert(type != kPHY_IntrActiveHigh);

    status_t result;
    uint16_t regValue;

    result = PHY_JL1111_SelectPage(handle, PHY_JL1111_RMII_PAGE);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Read operation will clear pending interrupt before enable interrupt. */
    result = PHY_JL1111_READ(handle, PHY_JL1111_INTR_ENABLE_REG, &regValue);
    if (result != kStatus_Success)
    {
        (void)PHY_JL1111_SelectPage(handle, 0U);
        return result;
    }

    regValue &= ~PHY_JL1111_INTR_LINK_EVENT_MASK;

    if (type == kPHY_IntrActiveLow)
    {
        regValue |= PHY_JL1111_INTR_LINK_EVENT_MASK;
    }
    else if (type == kPHY_IntrActiveHigh)
    {
        result = kStatus_InvalidArgument;
    }
    else
    {
        /* Disable interrupt. */
    }

    if (result == kStatus_Success)
    {
        result = PHY_JL1111_WRITE(handle, PHY_JL1111_INTR_ENABLE_REG, regValue);
    }

    if (result == kStatus_Success)
    {
        result = PHY_JL1111_SelectPage(handle, 0U);
    }
    else
    {
        (void)PHY_JL1111_SelectPage(handle, 0U);
    }

    return result;
}

status_t PHY_JL1111_ClearInterrupt(phy_handle_t *handle)
{
    uint16_t regValue;

    /* Just read interrupt status register (page 0, reg 30) to clear interrupt status. */
    return PHY_JL1111_READ(handle, PHY_JL1111_INTERRUPT_STATUS_REG, &regValue);
}

static void PHY_JL1111_GetLinkSpeedDuplexFromBasicControl(uint16_t regValue, phy_speed_t *speed, phy_duplex_t *duplex)
{
    if (speed != NULL)
    {
        if ((regValue & PHY_BCTL_SPEED0_MASK) != 0U)
        {
            *speed = kPHY_Speed100M;
        }
        else
        {
            *speed = kPHY_Speed10M;
        }
    }

    if (duplex != NULL)
    {
        if ((regValue & PHY_BCTL_DUPLEX_MASK) != 0U)
        {
            *duplex = kPHY_FullDuplex;
        }
        else
        {
            *duplex = kPHY_HalfDuplex;
        }
    }
}

static status_t PHY_JL1111_GetLinkSpeedDuplexFromAutoNegotiation(phy_handle_t *handle,
                                                                 phy_speed_t *speed,
                                                                 phy_duplex_t *duplex)
{
    uint16_t advertiseValue;
    uint16_t partnerValue;
    uint16_t commonAbility;
    uint16_t statusValue;
    status_t result;

    result = PHY_JL1111_READ(handle, PHY_BASICSTATUS_REG, &statusValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Clause 22 BMSR link bit is latch-low on many PHYs; read twice for current state. */
    result = PHY_JL1111_READ(handle, PHY_BASICSTATUS_REG, &statusValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    if ((statusValue & PHY_BSTATUS_LINKSTATUS_MASK) == 0U)
    {
        return kStatus_Fail;
    }

    if ((statusValue & PHY_BSTATUS_AUTONEGCOMP_MASK) == 0U)
    {
        return kStatus_Fail;
    }

    result = PHY_JL1111_READ(handle, PHY_AUTONEG_ADVERTISE_REG, &advertiseValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    result = PHY_JL1111_READ(handle, PHY_AUTONEG_LINKPARTNER_REG, &partnerValue);
    if (result != kStatus_Success)
    {
        return result;
    }

    commonAbility = advertiseValue & partnerValue;

    /* Resolve speed/duplex using IEEE 802.3 Clause 28 priority resolution:
     * 100BASE-TX Full Duplex
     * 100BASE-TX Half Duplex
     * 10BASE-T Full Duplex
     * 10BASE-T Half Duplex
     */
    if ((commonAbility & PHY_100BASETX_FULLDUPLEX_MASK) != 0U)
    {
        if (speed != NULL)
        {
            *speed = kPHY_Speed100M;
        }
        if (duplex != NULL)
        {
            *duplex = kPHY_FullDuplex;
        }
    }
    else if ((commonAbility & PHY_100BASETX_HALFDUPLEX_MASK) != 0U)
    {
        if (speed != NULL)
        {
            *speed = kPHY_Speed100M;
        }
        if (duplex != NULL)
        {
            *duplex = kPHY_HalfDuplex;
        }
    }
    else if ((commonAbility & PHY_10BASETX_FULLDUPLEX_MASK) != 0U)
    {
        if (speed != NULL)
        {
            *speed = kPHY_Speed10M;
        }
        if (duplex != NULL)
        {
            *duplex = kPHY_FullDuplex;
        }
    }
    else if ((commonAbility & PHY_10BASETX_HALFDUPLEX_MASK) != 0U)
    {
        if (speed != NULL)
        {
            *speed = kPHY_Speed10M;
        }
        if (duplex != NULL)
        {
            *duplex = kPHY_HalfDuplex;
        }
    }
    else
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

static status_t PHY_JL1111_MMD_SetDevice(phy_handle_t *handle,
                                         uint8_t device,
                                         uint16_t addr,
                                         phy_mmd_access_mode_t mode)
{
    status_t result = kStatus_Success;

    /* Set function mode of address access (b00) and device address. */
    result = PHY_JL1111_WRITE(handle, PHY_MMD_ACCESS_CONTROL_REG, device);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Set register address. */
    result = PHY_JL1111_WRITE(handle, PHY_MMD_ACCESS_DATA_REG, addr);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Set function mode of data access (b01~11) and device address. */
    result = PHY_JL1111_WRITE(handle, PHY_MMD_ACCESS_CONTROL_REG, (uint16_t)mode | (uint16_t)device);
    return result;
}

static inline status_t PHY_JL1111_MMD_ReadData(phy_handle_t *handle, uint16_t *pData)
{
    return PHY_JL1111_READ(handle, PHY_MMD_ACCESS_DATA_REG, pData);
}

static inline status_t PHY_JL1111_MMD_WriteData(phy_handle_t *handle, uint16_t data)
{
    return PHY_JL1111_WRITE(handle, PHY_MMD_ACCESS_DATA_REG, data);
}

static status_t PHY_JL1111_MMD_Read(phy_handle_t *handle, uint8_t device, uint16_t addr, uint16_t *pData)
{
    status_t result = kStatus_Success;

    result = PHY_JL1111_MMD_SetDevice(handle, device, addr, kPHY_MMDAccessNoPostIncrement);
    if (result == kStatus_Success)
    {
        result = PHY_JL1111_MMD_ReadData(handle, pData);
    }
    return result;
}

static status_t PHY_JL1111_MMD_Write(phy_handle_t *handle, uint8_t device, uint16_t addr, uint16_t data)
{
    status_t result = kStatus_Success;

    result = PHY_JL1111_MMD_SetDevice(handle, device, addr, kPHY_MMDAccessNoPostIncrement);
    if (result == kStatus_Success)
    {
        result = PHY_JL1111_MMD_WriteData(handle, data);
    }
    return result;
}

static status_t PHY_JL1111_SelectPage(phy_handle_t *handle, uint8_t page)
{
    return PHY_JL1111_WRITE(handle, PHY_JL1111_PAGE_SELECT_REG, page);
}
