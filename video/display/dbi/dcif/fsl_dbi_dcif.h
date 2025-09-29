/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FSL_DBI_DCIF_H_
#define FSL_DBI_DCIF_H_

#include "fsl_dbi.h"
#include "fsl_dcif.h"

/*
 * Change log:
 *
 *   1.0.0
 *     - Initial version
 */

/*!
 * @addtogroup dbi_dcif
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief DCIF DBI interface transfer operation. */
extern const dbi_iface_xfer_ops_t g_dbiIfaceXferOps;
extern const dbi_config_ops_t g_dbiConfigOps;

typedef struct _dbi_dcif_prv_data
{
    DCIF_Type *dcif;      /*!< Pointer to DCIF peripheral. */
    uint8_t domain;       /*!< Domain used for interrupt. */
    uint16_t height;      /*!< Left height of the area to send. */
    uint16_t width;       /*!< Width of the area to send. */
    uint8_t bytePerPixel; /*!< Used to calculate stride when not using 2-D memory write. */
} dbi_dcif_prv_data_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Init DCIF DBI controller.
 *
 * @param[out] dbiIface Pointer to the DBI interface.
 * @param[in] prvData Private data for DBI controller, maintains the info DCIF needs for the memory write,
    uppwer layer only need to pass in it, DBI_DCIF driver will initialize it.
 *
 */
void DBI_DCIF_InitController(dbi_iface_t *dbiIface, dbi_dcif_prv_data_t *prvData);

status_t DBI_DCIF_WriteCommandData(dbi_iface_t *dbiIface, uint32_t command, const void *data, uint32_t len_byte);

status_t DBI_DCIF_WriteMemory(dbi_iface_t *dbiIface, uint32_t command, const void *data, uint32_t len_byte);

status_t DBI_DCIF_WriteMemory2D(
    dbi_iface_t *dbiIface, uint32_t command, const void *data, uint32_t len_byte, uint32_t stride_byte);

status_t DBI_DCIF_SetPixelFormat(dbi_iface_t *dbiIface, video_pixel_format_t format);

void DBI_DCIF_IRQHandler(dbi_iface_t *dbiIface);

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* FSL_DBI_DCIF_H_ */
