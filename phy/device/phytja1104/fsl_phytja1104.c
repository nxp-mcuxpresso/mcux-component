/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_phytja1104.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Defines the PHY TJA1104 registers */

/* MMD1 - IEEE registers for PMA/PMD */
#define MDIO_PMA_CONTROL1                 (0x0000U)
#define MDIO_PMA_CONTROL1_REMOTE_LOOPBACK (1U << 1)
#define MDIO_PMA_CONTROL1_LOCAL_LOOPBACK  (1U << 0)

/* MMD3 - IEEE registers for PCS */
#define MDIO_PCS_CONTROL1          (0x0000U)
#define MDIO_PCS_CONTROL1_LOOPBACK (1U << 14)

/* MMD30 - Device version register */
#define TJA1104_DEVICE_IDENTIFIER3              (0x0004U)
#define TJA1104_DEVICE_IDENTIFIER3_SIL_VER_MASK (0xF000U)
#define TJA1104_DEVICE_IDENTIFIER3_SIL_VER_A0   (0x100U)

/* MMD30 - Device status register */
#define TJA1104_DEVICE_CONTROL                 (0x0040U)
#define TJA1104_DEVICE_CONTROL_RESET           (1U << 15)
#define TJA1104_DEVICE_CONTROL_GLOBAL_CFG_EN   (1U << 14)
#define TJA1104_DEVICE_CONTROL_SUPER_CFG_EN    (1U << 13)
#define TJA1104_DEVICE_CONTROL_START_OPERATION (1U << 0)

/* MMD30 - Device xMII Control register */
#define TJA1104_INFRA_CONFIG         (0xAC06U)
#define TJA1104_INFRA_CONFIG_TEST_EN (1U << 0)

/* MMD30 - Device xMII Control register */
#define TJA1104_XMII_CONTROL                 (0xAFC0U)
#define TJA1104_XMII_CONTROL_RESET           (1U << 15)
#define TJA1104_XMII_CONTROL_LOGICAL_ISOLATE (1U << 2)
#define TJA1104_XMII_CONTROL_MAC_LOOPBACK    (1U << 1)
#define TJA1104_XMII_CONTROL_PHY_LOOPBACK    (1U << 0)

/* MMD30 - Device xMII Abilities register */
#define TJA1104_XMII_ABILITIES                   (0xAFC4U)
#define TJA1104_XMII_ABILITIES_RGMII_ID_ABILITY  (1U << 15)
#define TJA1104_XMII_ABILITIES_RGMII_ABILITY     (1U << 14)
#define TJA1104_XMII_ABILITIES_RMII_MSTR_ABILITY (1U << 11)
#define TJA1104_XMII_ABILITIES_RMII_SLV_ABILITY  (1U << 10)
#define TJA1104_XMII_ABILITIES_MII_SLV_ABILITY   (1U << 9)
#define TJA1104_XMII_ABILITIES_MII_MSTR_ABILITY  (1U << 8)
#define TJA1104_XMII_ABILITIES_SGMII_ABILITY     (1U << 0)

/* MMD30 - Device xMII Basic Config*/
#define TJA1104_XMII_BASIC_CONFIG            (0xAFC6U)
#define TJA1104_XMII_BASIC_CONFIG_MODE       (1U << 0)
#define TJA1104_XMII_BASIC_CONFIG_MODE_SGMII (0x1)
#define TJA1104_XMII_BASIC_CONFIG_MODE_MII   (0x4)
#define TJA1104_XMII_BASIC_CONFIG_MODE_RMII  (0x5)
#define TJA1104_XMII_BASIC_CONFIG_MODE_RGMII (0x7)
#define TJA1104_XMII_BASIC_CONFIG_MODE_MASK  (0x7)

/* MMD30 - SGMII control register */
#define TJA1104_SGMII_BASIC_CONTROL     (0xB000U)
#define TJA1104_SGMII_BASIC_CONTROL_LPM (1U << 11)

/* Shared - Port function enables register */
#define TJA1104_PORT_FUNC_ENABLES         (0x8048U)
#define TJA1104_PORT_FUNC_ENABLES_TEST_EN (1U << 0)

/* Shared - PHY control register */
#define TJA1104_PHY_CONTROL              (0x8100U)
#define TJA1104_PHY_CONTROL_START_OP     (1U << 0)
#define TJA1104_PHY_CONTROL_GOTO_STANDBY (1U << 1)

/* Shared - PHY status register */
#define TJA1104_PHY_STATUS                 (0x8102U)
#define TJA1104_PHY_STATUS_LINK_AVAILABLE  (1U << 6)
#define TJA1104_PHY_STATUS_SENDN_OR_DATA   (1U << 5)
#define TJA1104_PHY_STATUS_REM_RCVR_STATUS (1U << 4)
#define TJA1104_PHY_STATUS_SCR_STATUS      (1U << 3)
#define TJA1104_PHY_STATUS_LINK_STATUS     (1U << 2)
#define TJA1104_PHY_STATUS_LOC_RCVR_STATUS (1U << 1)

/* Shared - PHY config register */
#define TJA1104_PHY_CONFIG             (0x8108U)
#define TJA1104_PHY_CONFIG_TEST_ENABLE (1U << 1)
#define TJA1104_PHY_CONFIG_AUTO_OP     (1U << 0)

/* Shared - PHY functional IRQ masked status register */
#define TJA1104_PHY_FUNC_IRQ_STATUS     (0x80A2U)
#define TJA1104_PHY_FUNC_IRQ_LINK_EVENT (1U << 1)
#define TJA1104_PHY_FUNC_IRQ_LINK_AVAIL (1U << 2)

/* Shared - PHY functional IRQ source & enable registers */
#define TJA1104_PHY_FUNC_IRQ_ACK           (0x80A0U)
#define TJA1104_PHY_FUNC_IRQ_EN            (0x80A1U)
#define TJA1104_PHY_FUNC_IRQ_LINK_EVENT_EN (1U << 1)
#define TJA1104_PHY_FUNC_IRQ_LINK_AVAIL_EN (1U << 2)

/* IEEE registers for PMA/PMD */
#define MDIO_PMA_BT1_CTRL            (0x0834U)
#define MDIO_PMA_BT1_CTRL_CFG_LEADER (1U << 14)

/* RGMII TX delay*/
#define TJA1104_PHY_RGMII_TXC_DELAY_CONFIG      (0xAFCC)
#define TJA1104_PHY_RGMII_TXC_DELAY_ENABLE      (1U << 15U)
#define TJA1104_PHY_RGMII_TXC_DELAY_PSHIFT      (1U << 0U)
#define TJA1104_PHY_RGMII_TXC_DELAY_PSHIFT_MASK (0x1FU)

/* RGMII RX delay*/
#define TJA1104_PHY_RGMII_RXC_DELAY_CONFIG      (0xAFCD)
#define TJA1104_PHY_RGMII_RXC_DELAY_ENABLE      (1U << 15U)
#define TJA1104_PHY_RGMII_RXC_DELAY_PSHIFT      (1U << 0U)
#define TJA1104_PHY_RGMII_RXC_DELAY_PSHIFT_MASK (0x1FU)

/*! @brief TJA1104 PHY identifier 1 */
#define TJA1104_ID_1 (0x1BU)

/*! @brief TJA1104 PHY identifier 2 */
#define TJA1104_ID_2 (0xB013U)

/*! @brief TJA1104 PHY identifier 2 mask */
#define TJA1104_ID_2_MASK (0xFFFFU)

/*! @brief Defines the timeout macro */
#define PHY_READID_TIMEOUT_COUNT (100U)

/*! @brief Converts clause 22 register address to clause 45 address */
#define CL22_TO_CL45(register) ((register) + 0x8000U)

/*!
 * @brief Execute an operation and return immediately if it fails
 *
 * This macro executes the given operation and checks its status.
 * If the operation returns anything other than kStatus_Success,
 * it returns that error code from the current function.
 *
 * @param x The operation to execute
 */
#define RETURN_ON_ERROR(x)              \
    do                                  \
    {                                   \
        status_t _result = (x);         \
        if (_result != kStatus_Success) \
        {                               \
            return _result;             \
        }                               \
    } while (0)

/*! @brief Defines the PHY resource interface */
#define PHY_TJA1104_WRITE(handle, devAddr, regAddr, data) \
    ((phy_tja1104_resource_t *)(handle)->resource)->write((handle)->phyAddr, devAddr, regAddr, data)
#define PHY_TJA1104_READ(handle, devAddr, regAddr, pData) \
    ((phy_tja1104_resource_t *)(handle)->resource)->read((handle)->phyAddr, devAddr, regAddr, pData)

/*******************************************************************************
 * Variables
 ******************************************************************************/

const phy_operations_t phytja1104_ops = {.phyInit             = PHY_TJA1104_Init,
                                         .phyWrite            = NULL,
                                         .phyRead             = NULL,
                                         .phyWriteC45         = PHY_TJA1104_Write,
                                         .phyReadC45          = PHY_TJA1104_Read,
                                         .getAutoNegoStatus   = PHY_TJA1104_GetAutoNegotiationStatus,
                                         .getLinkStatus       = PHY_TJA1104_GetLinkStatus,
                                         .getLinkSpeedDuplex  = PHY_TJA1104_GetLinkSpeedDuplex,
                                         .setLinkSpeedDuplex  = PHY_TJA1104_SetLinkSpeedDuplex,
                                         .enableLoopback      = PHY_TJA1104_EnableLoopback,
                                         .enableLinkInterrupt = PHY_TJA1104_EnableInterrupt,
                                         .clearInterrupt      = PHY_TJA1104_ClearInterrupt};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Verifies the TJA1104 PHY ID
 *
 * This function reads the PHY identifier registers and verifies if it matches
 * the expected values for a TJA1104 PHY.
 *
 * @param handle The PHY device handle.
 * @retval kStatus_Success if the PHY ID is valid
 * @retval kStatus_Fail if timeout occurs
 * @retval kStatus_Timeout PHY MDIO visit time out
 */
static status_t PHY_TJA1104_VerifyPhyId(phy_handle_t *handle)
{
    uint32_t counter = PHY_READID_TIMEOUT_COUNT;
    uint16_t reg1;
    uint16_t reg2;

    do
    {
        RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_PMAPMD, CL22_TO_CL45(PHY_ID1_REG), &reg1));
        RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_PMAPMD, CL22_TO_CL45(PHY_ID2_REG), &reg2));
        counter--;
    } while (((reg1 != TJA1104_ID_1) || ((reg2 & TJA1104_ID_2_MASK) != TJA1104_ID_2)) && (counter != 0U));

    if (counter == 0U)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

/*!
 * @brief Enables or disables configuration registers in the TJA1104 PHY
 *
 * @param handle Pointer to the PHY device handle.
 * @param enable true to enable configuration register access, false to disable it.
 * @retval kStatus_Success if successful
 * @retval kStatus_Fail or other error code otherwise.
 */
static status_t PHY_TJA1104_EnableConfigRegisters(phy_handle_t *handle, bool enable)
{
    uint16_t reg;

    if (enable)
    {
        /* Enable config registers */
        RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_DEVICE_CONTROL,
                                          TJA1104_DEVICE_CONTROL_GLOBAL_CFG_EN | TJA1104_DEVICE_CONTROL_SUPER_CFG_EN));
    }
    else
    {
        /* Clear configuration enable bits */
        RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_DEVICE_CONTROL, &reg));
        reg &= ~(TJA1104_DEVICE_CONTROL_GLOBAL_CFG_EN | TJA1104_DEVICE_CONTROL_SUPER_CFG_EN);
        RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_DEVICE_CONTROL, reg));
    }

    return kStatus_Success;
}

/*!
 * @brief Starts PHY operation
 *
 * @param handle Pointer to the PHY device handle.
 * @retval kStatus_Success if successful
 * @retval kStatus_Fail or other error code otherwise.
 */
static status_t PHY_TJA1104_StartPhyOperation(phy_handle_t *handle)
{
    uint16_t reg;

    RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_PHY_CONTROL, &reg));
    reg &= ~TJA1104_PHY_CONTROL_GOTO_STANDBY;
    reg |= TJA1104_PHY_CONTROL_START_OP;
    RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_PHY_CONTROL, reg));

    return kStatus_Success;
}

/*!
 * @brief Goes to standy mode for reconfiguration
 *
 * @param handle Pointer to the PHY device handle.
 * @retval kStatus_Success if successful
 * @retval kStatus_Fail or other error code otherwise.
 */
static status_t PHY_TJA1104_GoToStandby(phy_handle_t *handle)
{
    uint16_t reg;

    RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_PHY_CONTROL, &reg));
    reg &= ~TJA1104_PHY_CONTROL_START_OP;
    reg |= TJA1104_PHY_CONTROL_GOTO_STANDBY;
    RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_PHY_CONTROL, reg));

    return kStatus_Success;
}

/* !
 * @brief Calculates the phase shift in degree to apply on RGMII tx-rx
 * Configure RGMII delay
 * tx_delay = 1800ps = 1.80us
 * rx_delay = 1666us = 1.67us
 *
 * pico_seconds_per_degree = 8000/360
 * degree = xx_delay /pico_seconds_per_degree
 * phase_shift = degree * 10  - 738 /9
 *
 * @param  raw value of delay in picoseconds
 * @retval Phase shift in degrees corresponding to the delay
 */
static uint16_t PHY_TJA1104_GetPhaseShift(uint16_t phase_offset_raw)
{
    /* The delay in degree phase is 73.8 + phase_offset_raw * 0.9.
     * To avoid floating point operations we'll multiply by 10
     * and get 1 decimal point precision.
     */
    uint16_t degree = (phase_offset_raw * 8000) / 360;
    uint16_t result = (degree * 10 - 738) / 9;

    return result;
}

status_t PHY_TJA1104_SetDelays(phy_handle_t *handle, uint16_t tx_delay, uint16_t rx_delay)
{
    uint16_t reg;
    /* Set tx_delay*/
    reg = TJA1104_PHY_RGMII_TXC_DELAY_ENABLE |
          (TJA1104_PHY_RGMII_TXC_DELAY_PSHIFT_MASK & PHY_TJA1104_GetPhaseShift(tx_delay));

    RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_PHY_RGMII_TXC_DELAY_CONFIG, reg));

    /* Set rx_delay*/
    reg = TJA1104_PHY_RGMII_RXC_DELAY_ENABLE |
          (TJA1104_PHY_RGMII_RXC_DELAY_PSHIFT_MASK & PHY_TJA1104_GetPhaseShift(rx_delay));

    RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_PHY_RGMII_RXC_DELAY_CONFIG, reg));

    return kStatus_Success;
}

status_t PHY_TJA1104_DisableDelays(phy_handle_t *handle)
{
    uint16_t reg;

    /* Disable tx_delay*/
    RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_PHY_RGMII_TXC_DELAY_CONFIG, &reg));
    reg &= ~(TJA1104_PHY_RGMII_TXC_DELAY_ENABLE);
    RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_PHY_RGMII_TXC_DELAY_CONFIG, reg));

    /* Disable rx_delay*/
    RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_PHY_RGMII_RXC_DELAY_CONFIG, &reg));
    reg &= ~(TJA1104_PHY_RGMII_RXC_DELAY_ENABLE);
    RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_PHY_RGMII_RXC_DELAY_CONFIG, reg));

    return kStatus_Success;
}

status_t PHY_TJA1104_Init(phy_handle_t *handle, const phy_config_t *config)
{
    assert(config != NULL);

    uint16_t reg;

    /* Assign PHY address and operation resource */
    handle->phyAddr  = config->phyAddr;
    handle->resource = config->resource;

    /* Check PHY ID */
    RETURN_ON_ERROR(PHY_TJA1104_VerifyPhyId(handle));

    /* Enable configuration registers */
    RETURN_ON_ERROR(PHY_TJA1104_EnableConfigRegisters(handle, true));

    /* Enable PHY autonomous operation */
    RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_PHY_CONFIG, &reg));
    reg |= TJA1104_PHY_CONFIG_AUTO_OP;
    RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_PHY_CONFIG, reg));

    /* Configure leader/follower mode */
    RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_PMAPMD, MDIO_PMA_BT1_CTRL, &reg));
    if (config->master)
    {
        /* Enable leader */
        reg |= MDIO_PMA_BT1_CTRL_CFG_LEADER;
    }
    else
    {
        /* Disable leader */
        reg &= ~MDIO_PMA_BT1_CTRL_CFG_LEADER;
    }
    RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_PMAPMD, MDIO_PMA_BT1_CTRL, reg));

    /* Enable interrupts */
    RETURN_ON_ERROR(PHY_TJA1104_EnableInterrupt(handle, config->intrType));

    /* Start PHY operation */
    RETURN_ON_ERROR(PHY_TJA1104_StartPhyOperation(handle));

    return kStatus_Success;
}

status_t PHY_TJA1104_Write(phy_handle_t *handle, uint8_t devAddr, uint16_t phyReg, uint16_t data)
{
    return PHY_TJA1104_WRITE(handle, devAddr, phyReg, data);
}

status_t PHY_TJA1104_Read(phy_handle_t *handle, uint8_t devAddr, uint16_t phyReg, uint16_t *pData)
{
    return PHY_TJA1104_READ(handle, devAddr, phyReg, pData);
}

status_t PHY_TJA1104_GetAutoNegotiationStatus(phy_handle_t *handle, bool *status)
{
    /* For TJA1104, the link being up means auto-negotiation is complete
     * since it's a 100BASE-T1 PHY with fixed speed/duplex settings */
    return PHY_TJA1104_GetLinkStatus(handle, status);
}

status_t PHY_TJA1104_GetLinkStatus(phy_handle_t *handle, bool *status)
{
    assert(status);

    uint16_t reg;

    if (status == NULL)
    {
        return kStatus_InvalidArgument;
    }

    /* Check if internal (PCS) loopback is enabled */
    RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_PCS, MDIO_PCS_CONTROL1, &reg));

    if (reg & MDIO_PCS_CONTROL1_LOOPBACK)
    {
        PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_PHY_STATUS, &reg);
        /* Link available not set when activated but always available when
         * following registers are set */
        if ((reg & TJA1104_PHY_STATUS_LINK_AVAILABLE) && (reg & TJA1104_PHY_STATUS_SENDN_OR_DATA) &&
            (reg & TJA1104_PHY_STATUS_REM_RCVR_STATUS) && (reg & TJA1104_PHY_STATUS_SCR_STATUS) &&
            (reg & TJA1104_PHY_STATUS_LINK_STATUS) && (reg & TJA1104_PHY_STATUS_LOC_RCVR_STATUS))
        {
            *status = true;
        }
        else
        {
            *status = false;
        }

        return kStatus_Success;
    }

    /* Read PHY status register */
    RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_PHY_STATUS, &reg));
    *status = (reg & TJA1104_PHY_STATUS_LINK_STATUS) | (reg & TJA1104_PHY_STATUS_LINK_AVAILABLE);

    return kStatus_Success;
}

status_t PHY_TJA1104_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex)
{
    assert(!((speed == NULL) && (duplex == NULL)));

    if ((speed == NULL) && (duplex == NULL))
    {
        return kStatus_InvalidArgument;
    }

    /* No need to read settings, speed and duplex is fixed on this PHY */
    if (speed != NULL)
    {
        *speed = kPHY_Speed100M;
    }

    if (duplex != NULL)
    {
        *duplex = kPHY_FullDuplex;
    }

    return kStatus_Success;
}

status_t PHY_TJA1104_SetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t speed, phy_duplex_t duplex)
{
    if ((speed != kPHY_Speed100M) || (duplex != kPHY_FullDuplex))
    {
        return kStatus_InvalidArgument;
    }

    /* No need to write settings, speed and duplex is fixed on this PHY */
    return kStatus_Success;
}

status_t PHY_TJA1104_EnableLoopback(phy_handle_t *handle, phy_loop_t mode, phy_speed_t speed, bool enable)
{
    assert(speed == kPHY_Speed100M);

    uint16_t reg;
    bool pcs_loopback = false;
    bool pma_loopback = false;

    /* The TJA1104 only supports 100 Mbps speed */
    if (speed != kPHY_Speed100M)
    {
        return kStatus_InvalidArgument;
    }

    RETURN_ON_ERROR(PHY_TJA1104_GoToStandby(handle));

    if (enable)
    {
        /* Enable PCS loopback */
        if (pcs_loopback)
        {
            RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_PCS, MDIO_PCS_CONTROL1, &reg));
            reg |= MDIO_PCS_CONTROL1_LOOPBACK;
            RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_PCS, MDIO_PCS_CONTROL1, reg));
        }
        else if (pma_loopback)
        {
            RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_PMAPMD, MDIO_PMA_CONTROL1, &reg));
            switch (mode)
            {
                case kPHY_LocalLoop:
                    /* Enable PMA local loopback */
                    reg &= ~MDIO_PMA_CONTROL1_REMOTE_LOOPBACK;
                    reg |= MDIO_PMA_CONTROL1_LOCAL_LOOPBACK;
                    break;

                case kPHY_RemoteLoop:
                    /* Enable PMA remote loopback */
                    reg &= ~MDIO_PMA_CONTROL1_LOCAL_LOOPBACK;
                    reg |= MDIO_PMA_CONTROL1_REMOTE_LOOPBACK;
                    break;

                case kPHY_ExternalLoop:
                    /* Enable PMA local loopback */
                    reg |= MDIO_PMA_CONTROL1_LOCAL_LOOPBACK;
                    break;

                default:
                    return kStatus_InvalidArgument;
            }

            /* Enable PMAPMD loopback mode */
            RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_PMAPMD, MDIO_PMA_CONTROL1, reg));
        }
        else
        {
            /* Enable infra config test*/
            RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_INFRA_CONFIG, &reg));
            reg |= TJA1104_INFRA_CONFIG_TEST_EN;
            RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_INFRA_CONFIG, reg));

            /* Enable xMII MAC loopback*/
            RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_XMII_CONTROL, &reg));
            reg |= TJA1104_XMII_CONTROL_MAC_LOOPBACK;
            RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_XMII_CONTROL, reg));
        }
    }
    else
    {
        /* Disable infra config test*/
        RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_INFRA_CONFIG, &reg));
        reg &= ~TJA1104_INFRA_CONFIG_TEST_EN;
        RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_INFRA_CONFIG, reg));

        /* Disable PCS loopback */
        RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_PCS, MDIO_PCS_CONTROL1, &reg));
        reg &= ~MDIO_PCS_CONTROL1_LOOPBACK;
        RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_PCS, MDIO_PCS_CONTROL1, reg));

        /* Disable PMAPMD loopback mode */
        RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_PMAPMD, MDIO_PMA_CONTROL1, &reg));
        reg &= ~MDIO_PMA_CONTROL1_REMOTE_LOOPBACK;
        reg &= ~MDIO_PMA_CONTROL1_LOCAL_LOOPBACK;
        RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_PMAPMD, MDIO_PMA_CONTROL1, reg));

        /* Disable xMII MAC loopback*/
        RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_XMII_CONTROL, &reg));
        reg &= ~TJA1104_XMII_CONTROL_MAC_LOOPBACK;
        RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_XMII_CONTROL, reg));
    }

    /* Start PHY operation */
    RETURN_ON_ERROR(PHY_TJA1104_StartPhyOperation(handle));

    return kStatus_Success;
}

status_t PHY_TJA1104_EnableInterrupt(phy_handle_t *handle, phy_interrupt_type_t type)
{
    assert((type == kPHY_IntrDisable) || (type == kPHY_IntrActiveLow));

    if (type == kPHY_IntrDisable)
    {
        /* Disable all interrupts in PHY interrupt enable register */
        RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_PHY_FUNC_IRQ_EN, 0U));
    }
    else if (type == kPHY_IntrActiveLow)
    {
        /* Clear any pending interrupts */
        RETURN_ON_ERROR(PHY_TJA1104_ClearInterrupt(handle));

        /* Enable link status change interrupts */
        RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_PHY_FUNC_IRQ_EN,
                                          TJA1104_PHY_FUNC_IRQ_LINK_EVENT_EN | TJA1104_PHY_FUNC_IRQ_LINK_AVAIL_EN));
    }
    else
    {
        /* Interrupt type not supported */
        return kStatus_InvalidArgument;
    }

    return kStatus_Success;
}

status_t PHY_TJA1104_ClearInterrupt(phy_handle_t *handle)
{
    uint16_t reg;

    /* Read interrupt status register */
    RETURN_ON_ERROR(PHY_TJA1104_READ(handle, PHY_MMD_VEND1, TJA1104_PHY_FUNC_IRQ_STATUS, &reg));

    /* If there are active interrupts, acknowledge/clear them */
    if (reg & (TJA1104_PHY_FUNC_IRQ_LINK_EVENT | TJA1104_PHY_FUNC_IRQ_LINK_AVAIL))
    {
        /* Write the read value to the acknowledge register to clear the interrupts */
        RETURN_ON_ERROR(PHY_TJA1104_WRITE(handle, PHY_MMD_VEND1, TJA1104_PHY_FUNC_IRQ_ACK,
                                          (reg & (TJA1104_PHY_FUNC_IRQ_LINK_EVENT | TJA1104_PHY_FUNC_IRQ_LINK_AVAIL))));
    }

    return kStatus_Success;
}
