/*
** ###################################################################
**
** Copyright 2026 NXP
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
** o Redistributions of source code must retain the above copyright notice, this list
**   of conditions and the following disclaimer.
**
** o Redistributions in binary form must reproduce the above copyright notice, this
**   list of conditions and the following disclaimer in the documentation and/or
**   other materials provided with the distribution.
**
** o Neither the name of the copyright holder nor the names of its
**   contributors may be used to endorse or promote products derived from this
**   software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
** ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
** ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** ###################################################################
*/

/*==========================================================================*/
/*!
 * @file
 *
 * Text names for the SCMI status codes. Keeping the table here lets every
 * agent, driver and example log a failing call in a readable way instead of
 * printing a raw negative number, which is easy to misread once it has been
 * widened to an unsigned value.
 */
/*==========================================================================*/

/* Includes */

#include "scmi.h"
#include "scmi_internal.h"

/* Local defines */

/* Local types */

/* Local variables */

/*--------------------------------------------------------------------------*/
/* Return the name of an SCMI status code                                   */
/*--------------------------------------------------------------------------*/
const char *SCMI_StatusToString(int32_t status)
{
    const char *str;

    switch (status)
    {
        case SCMI_ERR_SUCCESS:
            str = "SCMI_ERR_SUCCESS";
            break;
        case SCMI_ERR_NOT_SUPPORTED:
            str = "SCMI_ERR_NOT_SUPPORTED";
            break;
        case SCMI_ERR_INVALID_PARAMETERS:
            str = "SCMI_ERR_INVALID_PARAMETERS";
            break;
        case SCMI_ERR_DENIED:
            str = "SCMI_ERR_DENIED";
            break;
        case SCMI_ERR_NOT_FOUND:
            str = "SCMI_ERR_NOT_FOUND";
            break;
        case SCMI_ERR_OUT_OF_RANGE:
            str = "SCMI_ERR_OUT_OF_RANGE";
            break;
        case SCMI_ERR_BUSY:
            str = "SCMI_ERR_BUSY";
            break;
        case SCMI_ERR_COMMS_ERROR:
            str = "SCMI_ERR_COMMS_ERROR";
            break;
        case SCMI_ERR_GENERIC_ERROR:
            str = "SCMI_ERR_GENERIC_ERROR";
            break;
        case SCMI_ERR_HARDWARE_ERROR:
            str = "SCMI_ERR_HARDWARE_ERROR";
            break;
        case SCMI_ERR_PROTOCOL_ERROR:
            str = "SCMI_ERR_PROTOCOL_ERROR";
            break;
        case SCMI_ERR_IN_USE:
            str = "SCMI_ERR_IN_USE";
            break;
        case SCMI_ERR_ABORT_ERROR:
            str = "SCMI_ERR_ABORT_ERROR";
            break;
        case SCMI_ERR_CRC_ERROR:
            str = "SCMI_ERR_CRC_ERROR";
            break;
        case SCMI_ERR_MISSING_PARAMETERS:
            str = "SCMI_ERR_MISSING_PARAMETERS";
            break;
        case SCMI_ERR_POWER:
            str = "SCMI_ERR_POWER";
            break;
        case SCMI_ERR_TEST:
            str = "SCMI_ERR_TEST";
            break;
        case SCMI_ERR_SEQ_ERROR:
            str = "SCMI_ERR_SEQ_ERROR";
            break;
        case SCMI_ERR_TIMEOUT:
            str = "SCMI_ERR_TIMEOUT";
            break;
        default:
            str = "SCMI_ERR_UNKNOWN";
            break;
    }

    return str;
}

