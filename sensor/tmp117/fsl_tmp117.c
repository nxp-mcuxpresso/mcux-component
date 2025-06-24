/*
 * Copyright 2022-2023 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @addtogroup tmp117
 * @{
 *
 * The design of the driver is straightforward. For each function,
 * the necessary registers are written or read through the registered
 * I2C callback functions.
 *
 * All register values are 16 bits and use big endian. SWAP16 is a macro
 * used to convert the values back to little endian
 */

#include "fsl_tmp117.h"

/** Default TMP117 I2C device address */
#define DEFAULT_I2C_ADDRESS 0x48U

/** TMP117 register addresses */
#define TMP117_REG_TEMP_RESULT 0x00U /*< Temperature result reg */
#define TMP117_REG_CONFIG      0x01U /*< Configuration reg */
#define TMP117_REG_THIGH_LIMIT 0x02U /*< Temperature high limit reg */
#define TMP117_REG_TLOW_LIMIT  0x03U /*< Temperature low limit reg */
#define TMP117_REG_EEPROM_UL   0x04U /*< Eeprom unlock reg */
#define TMP117_REG_EEPROM1     0x05U /*< Eeprom 1 reg */
#define TMP117_REG_EEPROM2     0x06U /*< Eeprom 2 reg */
#define TMP117_REG_TEMP_OFFSET 0x07U /*< Temperature offset reg */
#define TMP117_REG_EEPROM3     0x08U /*< Eeprom 3 reg */
#define TMP117_REG_DEVICE_ID   0x0FU /*< Device id reg */

/** Configuration register fields */
#define TMP117_CONFIG_DRALERT_POS       2                                /*< Data ready alert position */
#define TMP117_CONFIG_DRALERT_MASK      (1 << TMP117_CONFIG_DRALERT_POS) /*< Data ready alert mask */
#define TMP117_CONFIG_POL_POS           3                                /*< Alert polarity position */
#define TMP117_CONFIG_POL_MASK          (1 << TMP117_CONFIG_POL_POS)     /*< Alert polarity mask */
#define TMP117_CONFIG_AVG_POS           5                                /*< Average mode position */
#define TMP117_CONFIG_MODE_SHUTDOWN_VAL 1                                /*< Shutdown mode value */
#define TMP117_CONFIG_MODE_ONE_SHOT_VAL 3                                /*< One shot mode value */
#define TMP117_CONFIG_MODE_POS          10                               /*< Mode position */
#define TMP117_CONFIG_EEPROM_BUSY_POS   12                               /*< Eeprom busy position */

/** Macro to convert 16 bit integer from little to big endian */
#define SWAP16(x) ((((x) >> 8) & 0xff) | (((x)&0xff) << 8))

/** Holds the configuration register value */
static uint16_t s_configRegVal;

/**
 * Initializes the given config with default values
 *
 * @param config Pointer to config to initialize
 */
void TMP117_GetDefaultConfig(tmp117_config_t *config)
{
    memset(config, 0, sizeof(*config));
    config->deviceAddress = DEFAULT_I2C_ADDRESS;
}

/**
 * Initializes the TMP117 driver
 *
 * @param config Pointer to the configuration
 * @return kStatus_Success in case of success
 */
status_t TMP117_Init(tmp117_config_t *config)
{
    uint16_t data;
    s_configRegVal = (config->alertActiveHigh ? TMP117_CONFIG_POL_MASK : 0) |
                     (config->assertAlert ? TMP117_CONFIG_DRALERT_MASK : 0) |
                     (TMP117_CONFIG_MODE_SHUTDOWN_VAL << TMP117_CONFIG_MODE_POS);
    data = SWAP16(s_configRegVal);
    return config->i2cWriteFunc(config->deviceAddress, TMP117_REG_CONFIG, (uint8_t *)&data, sizeof(data));
}

/**
 * Returns the most recent temperature conversion result.
 * The unit is 1/128 degrees.
 *
 * @param config Pointer to the configuration
 * @param temperature Pointer to variable that will hold the temperature
 * @post The data ready flag is cleared
 * @return kStatus_Success in case of success
 */
status_t TMP117_GetTemperature(tmp117_config_t *config, int16_t *temperature)
{
    status_t status = config->i2cReadFunc(config->deviceAddress, TMP117_REG_TEMP_RESULT, (uint8_t *)temperature,
                                          sizeof(*temperature));
    /* Swap temperature of raw unit16 instead of int16_t to avoid wrong swap result
     if the value being cast is outside the range of int16_t. */
    uint16_t raw_temp = (uint16_t)*temperature;
    uint16_t swapped = SWAP16(raw_temp);
    *temperature = (int16_t)swapped;
    return status;
}

/**
 * Returns the status of the TMP117
 *
 * @param config Pointer to the configuration
 * @param status Pointer to current status. Combination of fields of tmp117_status_t
 * @post All status flags are cleared
 * @return kStatus_Success in case of success
 */
status_t TMP117_GetStatus(tmp117_config_t *config, uint8_t *tmp117Status)
{
    uint16_t configVal;
    status_t status =
        config->i2cReadFunc(config->deviceAddress, TMP117_REG_CONFIG, (uint8_t *)&configVal, sizeof(configVal));
    configVal     = SWAP16(configVal);
    *tmp117Status = (uint8_t)(configVal >> TMP117_CONFIG_EEPROM_BUSY_POS);
    return status;
}

/**
 * Starts a temperature measurement
 *
 * @param config Pointer to the configuration
 * @param averagingMode The averaging mode to use for this measurement
 * @post kTMP117_Status_DataReady is set when conversion is completed
 * @post If enabled, alert signal is asserted when conversion is completed
 * @post When the conversion is completed, the TMP117 automatically enters shutdown mode
 * @return kStatus_Success in case of success
 */
status_t TMP117_StartMeasurement(tmp117_config_t *config, tmp117_averaging_mode_t averagingMode)
{
    uint16_t configVal = s_configRegVal | (TMP117_CONFIG_MODE_ONE_SHOT_VAL << TMP117_CONFIG_MODE_POS) |
                         (averagingMode << TMP117_CONFIG_AVG_POS);
    configVal = SWAP16(configVal);
    return config->i2cWriteFunc(config->deviceAddress, TMP117_REG_CONFIG, (uint8_t *)&configVal, sizeof(configVal));
}

/**
 * Puts the TMP117 in shutdown mode
 *
 * @param config Pointer to the configuration
 * @post Any ongoing conversion is aborted
 * @return kStatus_Success in case of success
 */
status_t TMP117_Shutdown(tmp117_config_t *config)
{
    uint16_t configVal = s_configRegVal | (TMP117_CONFIG_MODE_SHUTDOWN_VAL << TMP117_CONFIG_MODE_POS);
    configVal          = SWAP16(configVal);
    return config->i2cWriteFunc(config->deviceAddress, TMP117_REG_CONFIG, (uint8_t *)&configVal, sizeof(configVal));
}

/*! @} */
