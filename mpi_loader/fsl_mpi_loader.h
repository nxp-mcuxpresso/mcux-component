/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_MPI_LOADER_H_
#define _FSL_MPI_LOADER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*!
 * @addtogroup mpi_loader
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Kick off loading of Multicore Packed images.
 */
void MPI_LoadMultiImages(void);

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* _FSL_MPI_LOADER_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
