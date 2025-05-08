/*
 * Copyright 2022 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @addtogroup tmp117
 * @{
 *
 * The TMP117 driver allows the user to take temperature measurements.
 * Taking a measurement is an asynchronous process. To track the progress,
 * the status register can be read. Optionally, the ALERT signal can be
 * configured to assert each time a measurement is ready.
 *
 * The TMP117 can be configured to take multiple measurement and return
 * the average temperature to increase robustness against noise in the
 * conversion results.
 *
 * During initialization, an I2C read and a write function has to be
 * passed that the driver can use to communicate with the TMP117 sensor.
 */

#include "fsl_common.h"

/** Enum of different states */
typedef enum _tmp117_status
{
    /** Eeprom is busy */
    kTMP117_Status_EepromBusy = 0x01,
    /** Conversion completed. Data is ready */
    kTMP117_Status_DataReady = 0x02,
    /** Temperature lower than lower limit */
    kTMP117_Status_LowAlert = 0x04,
    /** Temperature higher than high limit */
    kTMP117_Status_HighAlert = 0x08
} tmp117_status_t;

/** Averaging mode */
typedef enum _tmp117_averaging_mode
{
    kTMP117_Averaging_None = 0, /*< Single measurement is done */
    kTMP117_Averaging_8    = 1, /*< Average of 8 measurement is returned */
    kTMP117_Averaging_32   = 2, /*< Average of 32 measurement is returned */
    kTMP117_Averaging_64   = 3  /*< Average of 64 measurement is returned */
} tmp117_averaging_mode_t;

/** TMP117 configuration */
typedef struct _tmp117_config
{
    /** Function used to send data over I2C */
    status_t (*i2cWriteFunc)(
        uint8_t deviceAddress, uint32_t subAddress, uint8_t *txBuff, size_t txBuffSize);
    /** Function used to read data over I2C */
    status_t (*i2cReadFunc)(
        uint8_t deviceAddress, uint32_t subAddress, uint8_t *rxBuff, size_t rxBuffSize);
    /** I2C device address of the TMP117 */
    uint8_t deviceAddress;
    /** True when the ALERT signal must be asserted upon conversion completion */
    bool assertAlert;
    /** True when the ALERT signal must be active high, false for active low */
    bool alertActiveHigh;
} tmp117_config_t;

/**
 * Initializes the given config with default values
 *
 * @param config Pointer to config to initialize
 */
void TMP117_GetDefaultConfig(tmp117_config_t *config);

/**
 * Initializes the TMP117 driver
 *
 * @param config Pointer to the configuration
 * @return kStatus_Success in case of success
 */
status_t TMP117_Init(tmp117_config_t *config);

/**
 * Returns the most recent temperature conversion result.
 * The unit is 1/128 degrees.
 *
 * @param config Pointer to the configuration
 * @param temperature Pointer to variable that will hold the temperature
 * @post The data ready flag is cleared
 * @return kStatus_Success in case of success
 */
status_t TMP117_GetTemperature(tmp117_config_t *config, int16_t *temperature);

/**
 * Returns the status of the TMP117
 *
 * @param config Pointer to the configuration
 * @param tmp117Status Pointer to current status. Combination of fields of tmp117_status_t
 * @post All status flags are cleared
 * @return kStatus_Success in case of success
 */
status_t TMP117_GetStatus(tmp117_config_t *config, uint8_t *tmp117Status);

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
status_t TMP117_StartMeasurement(tmp117_config_t *config, tmp117_averaging_mode_t averagingMode);

/**
 * Puts the TMP117 in shutdown mode
 *
 * @param config Pointer to the configuration
 * @post Any ongoing conversion is aborted
 * @return kStatus_Success in case of success
 */
status_t TMP117_Shutdown(tmp117_config_t *config);

/*! @} */
