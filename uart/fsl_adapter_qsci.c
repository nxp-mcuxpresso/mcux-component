/*
 * Copyright 2020,2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_qsci.h"

#include "fsl_adapter_uart.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef NDEBUG
#if (defined(DEBUG_CONSOLE_ASSERT_DISABLE) && (DEBUG_CONSOLE_ASSERT_DISABLE > 0U))
#undef assert
#define assert(n)
#endif
#endif

#if (defined(UART_ADAPTER_NON_BLOCKING_MODE) && (UART_ADAPTER_NON_BLOCKING_MODE > 0U))
/*! @brief uart RX state structure. */
typedef struct _hal_uart_receive_state
{
    volatile uint8_t *buffer;
    volatile uint32_t bufferLength;
    volatile uint32_t bufferSofar;
} hal_uart_receive_state_t;

/*! @brief uart TX state structure. */
typedef struct _hal_uart_send_state
{
    volatile uint8_t *buffer;
    volatile uint32_t bufferLength;
    volatile uint32_t bufferSofar;
} hal_uart_send_state_t;
#endif
/*! @brief uart state structure. */
typedef struct _hal_uart_state
{
#if (defined(UART_ADAPTER_NON_BLOCKING_MODE) && (UART_ADAPTER_NON_BLOCKING_MODE > 0U))
    hal_uart_transfer_callback_t callback;
    void *callbackParam;
#if (defined(HAL_UART_TRANSFER_MODE) && (HAL_UART_TRANSFER_MODE > 0U))
    qsci_transfer_handle_t hardwareHandle;
#endif
    hal_uart_receive_state_t rx;
    hal_uart_send_state_t tx;
#endif
    uint8_t instance;
} hal_uart_state_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static QSCI_Type *const s_QsciAdapterBase[] = QSCI_BASE_PTRS;

#if (defined(UART_ADAPTER_NON_BLOCKING_MODE) && (UART_ADAPTER_NON_BLOCKING_MODE > 0U))

#if !(defined(HAL_UART_TRANSFER_MODE) && (HAL_UART_TRANSFER_MODE > 0U))
/* Array of UART IRQ number. */
static const IRQn_Type s_QsciRxIRQ[]    = QSCI_RX_IRQS;
static const IRQn_Type s_QsciTxIRQ[]    = QSCI_TX_IRQS;
static const IRQn_Type s_QsciRxErrIRQ[] = QSCI_RX_ERR_IRQS;

static hal_uart_state_t *s_UartState[sizeof(s_QsciAdapterBase) / sizeof(QSCI_Type *)];
#endif

#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

#if (defined(HAL_UART_TRANSFER_MODE) && (HAL_UART_TRANSFER_MODE > 0U))
static hal_uart_status_t HAL_UartGetStatus(status_t status)
{
    hal_uart_status_t uartStatus = kStatus_HAL_UartError;
    switch (status)
    {
        case kStatus_Success:
            uartStatus = kStatus_HAL_UartSuccess;
            break;
        case kStatus_QSCI_TxBusy:
            uartStatus = kStatus_HAL_UartTxBusy;
            break;
        case kStatus_QSCI_RxBusy:
            uartStatus = kStatus_HAL_UartRxBusy;
            break;
        case kStatus_QSCI_TxIdle:
            uartStatus = kStatus_HAL_UartTxIdle;
            break;
        case kStatus_QSCI_RxIdle:
            uartStatus = kStatus_HAL_UartRxIdle;
            break;
        case kStatus_QSCI_BaudrateNotSupport:
            uartStatus = kStatus_HAL_UartBaudrateNotSupport;
            break;
        case kStatus_QSCI_NoiseError:
        case kStatus_QSCI_FramingError:
        case kStatus_QSCI_ParityError:
            uartStatus = kStatus_HAL_UartProtocolError;
            break;
        default:
            /* MISRA Rule 16.4 */
            break;
    }
    return uartStatus;
}
#else
static hal_uart_status_t HAL_UartGetStatus(status_t status)
{
    if (kStatus_Success == status)
    {
        return kStatus_HAL_UartSuccess;
    }
    else
    {
        return kStatus_HAL_UartError;
    }
}
#endif

#if (defined(UART_ADAPTER_NON_BLOCKING_MODE) && (UART_ADAPTER_NON_BLOCKING_MODE > 0U))

#if (defined(HAL_UART_TRANSFER_MODE) && (HAL_UART_TRANSFER_MODE > 0U))
static void HAL_UartCallback(qsci_transfer_handle_t *handle)
{
    hal_uart_state_t *uartHandle;
    hal_uart_status_t uartStatus = HAL_UartGetStatus(QSCI_GET_BUS_STATUS(handle));
    assert(QSCI_GET_TRANSFER_USER_DATA(handle));

    uartHandle = (hal_uart_state_t *)QSCI_GET_TRANSFER_USER_DATA(handle);

    if (kStatus_HAL_UartProtocolError == uartStatus)
    {
        if (uartHandle->hardwareHandle.u32RxDataSize)
        {
            uartStatus = kStatus_HAL_UartError;
        }
    }

    if (uartHandle->callback)
    {
        uartHandle->callback(uartHandle, uartStatus, uartHandle->callbackParam);
    }
}

#else

static void HAL_UartInterruptHandle(uint8_t instance)
{
    hal_uart_state_t *uartHandle = s_UartState[instance];
    uint32_t status;
    uint32_t clearStatus = 0;

    if (NULL == uartHandle)
    {
        return;
    }

    status = QSCI_GetStatusFlags(s_QsciAdapterBase[instance]);

    /* Receive data register full */
    if ((0U != (((uint32_t)kQSCI_RxDataRegFullFlag) & status)) &&
        (0U != (QSCI_GetEnabledInterrupts(s_QsciAdapterBase[instance]) & ((uint8_t)kQSCI_RxFullInterruptEnable))))
    {
        clearStatus |= (((uint32_t)kQSCI_RxDataRegFullFlag) | ((uint32_t)kQSCI_Group1Flags));
        if (NULL != uartHandle->rx.buffer)
        {
            uartHandle->rx.buffer[uartHandle->rx.bufferSofar++] = QSCI_ReadByte(s_QsciAdapterBase[instance]);
            if (uartHandle->rx.bufferSofar >= uartHandle->rx.bufferLength)
            {
                QSCI_DisableInterrupts(s_QsciAdapterBase[instance], ((uint8_t)kQSCI_RxFullInterruptEnable) |
                                                                        ((uint8_t)kQSCI_RxErrorInterruptEnable));
                uartHandle->rx.buffer = NULL;
                if (NULL != uartHandle->callback)
                {
                    uartHandle->callback(uartHandle, kStatus_HAL_UartRxIdle, uartHandle->callbackParam);
                }
            }
        }
    }

    /* Send data register empty and the interrupt is enabled. */
    if ((0U != (((uint32_t)kQSCI_TxDataRegEmptyFlag) & status)) &&
        (0U != (QSCI_GetEnabledInterrupts(s_QsciAdapterBase[instance]) & ((uint8_t)kQSCI_TxEmptyInterruptEnable))))
    {
        clearStatus |= (((uint32_t)kQSCI_TxDataRegEmptyFlag) | ((uint32_t)kQSCI_TxIdleFlag));
        if (NULL != uartHandle->tx.buffer)
        {
            QSCI_WriteByte(s_QsciAdapterBase[instance], uartHandle->tx.buffer[uartHandle->tx.bufferSofar++]);
            if (uartHandle->tx.bufferSofar >= uartHandle->tx.bufferLength)
            {
                QSCI_DisableInterrupts(s_QsciAdapterBase[uartHandle->instance], (uint8_t)kQSCI_TxEmptyInterruptEnable);
                uartHandle->tx.buffer = NULL;
                if (NULL != uartHandle->callback)
                {
                    uartHandle->callback(uartHandle, kStatus_HAL_UartTxIdle, uartHandle->callbackParam);
                }
            }
        }
    }

#if 1
    QSCI_ClearStatusFlags(s_QsciAdapterBase[instance], (uint16_t)clearStatus);
#endif
}
#endif

#endif

hal_uart_status_t HAL_UartInit(hal_uart_handle_t handle, const hal_uart_config_t *uart_config)
{
    hal_uart_state_t *uartHandle;
    qsci_config_t qsciConfig;
    status_t status;
    assert(handle);
    assert(uart_config);
    assert(uart_config->instance < (sizeof(s_QsciAdapterBase) / sizeof(QSCI_Type *)));
    assert(s_QsciAdapterBase[uart_config->instance]);
    assert(HAL_UART_HANDLE_SIZE >= sizeof(hal_uart_state_t));

    QSCI_GetDefaultConfig(&qsciConfig, uart_config->baudRate_Bps, uart_config->srcClock_Hz);

    if (kHAL_UartParityEven == uart_config->parityMode)
    {
        qsciConfig.eParityMode = kQSCI_ParityEven;
    }
    else if (kHAL_UartParityOdd == uart_config->parityMode)
    {
        qsciConfig.eParityMode = kQSCI_ParityOdd;
    }
    else
    {
        qsciConfig.eParityMode = kQSCI_ParityDisabled;
    }

    qsciConfig.bEnableRx = (bool)uart_config->enableRx;
    qsciConfig.bEnableTx = (bool)uart_config->enableTx;

    status = QSCI_Init(s_QsciAdapterBase[uart_config->instance], &qsciConfig);

    if (kStatus_Success != status)
    {
        return HAL_UartGetStatus(status);
    }

    uartHandle           = (hal_uart_state_t *)handle;
    uartHandle->instance = uart_config->instance;

#if (defined(UART_ADAPTER_NON_BLOCKING_MODE) && (UART_ADAPTER_NON_BLOCKING_MODE > 0U))

#if (defined(HAL_UART_TRANSFER_MODE) && (HAL_UART_TRANSFER_MODE > 0U))
    QSCI_TransferCreateHandle(s_QsciAdapterBase[uart_config->instance], &uartHandle->hardwareHandle,
                              (qsci_transfer_callback_t)HAL_UartCallback, handle);
#else
    s_UartState[uartHandle->instance] = uartHandle;
    /* Enable interrupt in */

    (void)EnableIRQWithPriority(s_QsciRxIRQ[uart_config->instance], HAL_UART_ISR_PRIORITY);
    (void)EnableIRQWithPriority(s_QsciTxIRQ[uart_config->instance], HAL_UART_ISR_PRIORITY);
    (void)EnableIRQWithPriority(s_QsciRxErrIRQ[uart_config->instance], HAL_UART_ISR_PRIORITY);
#endif

#endif

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartDeinit(hal_uart_handle_t handle)
{
    hal_uart_state_t *uartHandle;

    assert(handle);

    uartHandle = (hal_uart_state_t *)handle;

    QSCI_Deinit(s_QsciAdapterBase[uartHandle->instance]);

#if (defined(UART_ADAPTER_NON_BLOCKING_MODE) && (UART_ADAPTER_NON_BLOCKING_MODE > 0U))

#if !(defined(HAL_UART_TRANSFER_MODE) && (HAL_UART_TRANSFER_MODE > 0U))
    s_UartState[uartHandle->instance] = NULL;
#endif

#endif

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartReceiveBlocking(hal_uart_handle_t handle, uint8_t *data, size_t length)
{
    hal_uart_state_t *uartHandle;
    status_t status;
    assert(handle);
    assert(data);
    assert(length);

    uartHandle = (hal_uart_state_t *)handle;

#if (defined(UART_ADAPTER_NON_BLOCKING_MODE) && (UART_ADAPTER_NON_BLOCKING_MODE > 0U))
    if (NULL != uartHandle->rx.buffer)
    {
        return kStatus_HAL_UartRxBusy;
    }
#endif

    status = QSCI_ReadBlocking(s_QsciAdapterBase[uartHandle->instance], data, (uint32_t)length);

    return HAL_UartGetStatus(status);
}

hal_uart_status_t HAL_UartSendBlocking(hal_uart_handle_t handle, const uint8_t *data, size_t length)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(data);
    assert(length);

    uartHandle = (hal_uart_state_t *)handle;

#if (defined(UART_ADAPTER_NON_BLOCKING_MODE) && (UART_ADAPTER_NON_BLOCKING_MODE > 0U))
    if (NULL != uartHandle->tx.buffer)
    {
        return kStatus_HAL_UartTxBusy;
    }
#endif

    QSCI_WriteBlocking(s_QsciAdapterBase[uartHandle->instance], data, (uint32_t)length);

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartEnterLowpower(hal_uart_handle_t handle)
{
    assert(handle);

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartExitLowpower(hal_uart_handle_t handle)
{
    assert(handle);

    return kStatus_HAL_UartSuccess;
}

#if (defined(UART_ADAPTER_NON_BLOCKING_MODE) && (UART_ADAPTER_NON_BLOCKING_MODE > 0U))

#if (defined(HAL_UART_TRANSFER_MODE) && (HAL_UART_TRANSFER_MODE > 0U))

hal_uart_status_t HAL_UartTransferInstallCallback(hal_uart_handle_t handle,
                                                  hal_uart_transfer_callback_t callback,
                                                  void *callbackParam)
{
    hal_uart_state_t *uartHandle;

    assert(handle);
    assert(HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    uartHandle->callbackParam = callbackParam;
    uartHandle->callback      = callback;

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartTransferReceiveNonBlocking(hal_uart_handle_t handle, hal_uart_transfer_t *transfer)
{
    hal_uart_state_t *uartHandle;
    status_t status;
    assert(handle);
    assert(transfer);
    assert(HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    status = QSCI_TransferReceiveNonBlocking(&uartHandle->hardwareHandle, (qsci_transfer_t *)transfer, NULL);

    return HAL_UartGetStatus(status);
}

hal_uart_status_t HAL_UartTransferSendNonBlocking(hal_uart_handle_t handle, hal_uart_transfer_t *transfer)
{
    hal_uart_state_t *uartHandle;
    status_t status;
    assert(handle);
    assert(transfer);
    assert(HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    status = QSCI_TransferSendNonBlocking(&uartHandle->hardwareHandle, (qsci_transfer_t *)transfer);

    return HAL_UartGetStatus(status);
}

hal_uart_status_t HAL_UartTransferGetReceiveCount(hal_uart_handle_t handle, uint32_t *count)
{
    hal_uart_state_t *uartHandle;
    status_t status;
    assert(handle);
    assert(count);
    assert(HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    status = QSCI_TransferGetReceiveCount(&uartHandle->hardwareHandle, count);

    return HAL_UartGetStatus(status);
}

hal_uart_status_t HAL_UartTransferGetSendCount(hal_uart_handle_t handle, uint32_t *count)
{
    hal_uart_state_t *uartHandle;
    status_t status;
    assert(handle);
    assert(count);
    assert(HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    status = QSCI_TransferGetSendCount(&uartHandle->hardwareHandle, count);

    return HAL_UartGetStatus(status);
}

hal_uart_status_t HAL_UartTransferAbortReceive(hal_uart_handle_t handle)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    QSCI_TransferAbortReceive(&uartHandle->hardwareHandle);

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartTransferAbortSend(hal_uart_handle_t handle)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    QSCI_TransferAbortSend(&uartHandle->hardwareHandle);

    return kStatus_HAL_UartSuccess;
}

#else

/* None transactional API with non-blocking mode. */
hal_uart_status_t HAL_UartInstallCallback(hal_uart_handle_t handle,
                                          hal_uart_transfer_callback_t callback,
                                          void *callbackParam)
{
    hal_uart_state_t *uartHandle;

    assert(handle);
    assert(0U == HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    uartHandle->callbackParam = callbackParam;
    uartHandle->callback      = callback;

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartReceiveNonBlocking(hal_uart_handle_t handle, uint8_t *data, size_t length)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(data);
    assert(length);
    assert(0U == HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    if (NULL != uartHandle->rx.buffer)
    {
        return kStatus_HAL_UartRxBusy;
    }

    uartHandle->rx.bufferLength = (uint32_t)length;
    uartHandle->rx.bufferSofar  = 0;
    uartHandle->rx.buffer       = data;
    QSCI_EnableInterrupts(s_QsciAdapterBase[uartHandle->instance],
                          ((uint8_t)kQSCI_RxFullInterruptEnable) | ((uint8_t)kQSCI_RxErrorInterruptEnable));
    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartSendNonBlocking(hal_uart_handle_t handle, uint8_t *data, size_t length)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(data);
    assert(length);
    assert(0U == HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    if (NULL != uartHandle->tx.buffer)
    {
        return kStatus_HAL_UartTxBusy;
    }
    uartHandle->tx.bufferLength = (uint32_t)length;
    uartHandle->tx.bufferSofar  = 0;
    uartHandle->tx.buffer       = (volatile uint8_t *)data;
    QSCI_EnableInterrupts(s_QsciAdapterBase[uartHandle->instance], (uint8_t)kQSCI_TxEmptyInterruptEnable);
    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartGetReceiveCount(hal_uart_handle_t handle, uint32_t *reCount)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(reCount);
    assert(0U == HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    if (NULL != uartHandle->rx.buffer)
    {
        *reCount = uartHandle->rx.bufferSofar;
        return kStatus_HAL_UartSuccess;
    }
    return kStatus_HAL_UartError;
}

hal_uart_status_t HAL_UartGetSendCount(hal_uart_handle_t handle, uint32_t *seCount)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(seCount);
    assert(0U == HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    if (NULL != uartHandle->tx.buffer)
    {
        *seCount = uartHandle->tx.bufferSofar;
        return kStatus_HAL_UartSuccess;
    }
    return kStatus_HAL_UartError;
}

hal_uart_status_t HAL_UartAbortReceive(hal_uart_handle_t handle)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(0U == HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    if (NULL != uartHandle->rx.buffer)
    {
        QSCI_DisableInterrupts(s_QsciAdapterBase[uartHandle->instance],
                               ((uint8_t)kQSCI_RxDataRegFullFlag) | ((uint8_t)kQSCI_RxErrorInterruptEnable));
        uartHandle->rx.buffer = NULL;
    }

    return kStatus_HAL_UartSuccess;
}

hal_uart_status_t HAL_UartAbortSend(hal_uart_handle_t handle)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(0U == HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    if (NULL != uartHandle->tx.buffer)
    {
        QSCI_DisableInterrupts(s_QsciAdapterBase[uartHandle->instance], (uint8_t)kQSCI_TxEmptyInterruptEnable);
        uartHandle->tx.buffer = NULL;
    }

    return kStatus_HAL_UartSuccess;
}

#endif

#if (defined(HAL_UART_TRANSFER_MODE) && (HAL_UART_TRANSFER_MODE > 0U))
extern void QSCI_TransferHandleIRQ(qsci_transfer_handle_t *psHandle);
void HAL_UartIsrFunction(hal_uart_handle_t handle)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    QSCI_TransferHandleIRQ(&uartHandle->hardwareHandle);
}

#else

#define QSCI_IRQ_HANDER(x, vec) ivINT_QSCI##x##_##vec

void HAL_UartIsrFunction(hal_uart_handle_t handle)
{
    hal_uart_state_t *uartHandle;
    assert(handle);
    assert(0U == HAL_UART_TRANSFER_MODE);

    uartHandle = (hal_uart_state_t *)handle;

    HAL_UartInterruptHandle(uartHandle->instance);
}

#if defined(QSCI0)
#pragma interrupt alignsp saveall
void QSCI_IRQ_HANDER(0, RERR)(void);
void QSCI_IRQ_HANDER(0, RERR)(void)
{
    HAL_UartInterruptHandle(0);
}
#pragma interrupt off

#pragma interrupt alignsp saveall
void QSCI_IRQ_HANDER(0, RCV)(void);
void QSCI_IRQ_HANDER(0, RCV)(void)
{
    HAL_UartInterruptHandle(0);
}
#pragma interrupt off

#pragma interrupt alignsp saveall
#if defined(QSCI_TRIDLE_IRQS)
void QSCI_IRQ_HANDER(0, TRIDLE)(void);
void QSCI_IRQ_HANDER(0, TRIDLE)(void)
{
    HAL_UartInterruptHandle(0);
}
#elif defined(QSCI_TIDLE_IRQS)
void QSCI_IRQ_HANDER(0, TIDLE)(void);
void QSCI_IRQ_HANDER(0, TIDLE)(void)
{
    HAL_UartInterruptHandle(0);
}
#endif
#pragma interrupt off

#pragma interrupt alignsp saveall
void QSCI_IRQ_HANDER(0, TDRE)(void);
void QSCI_IRQ_HANDER(0, TDRE)(void)
{
    HAL_UartInterruptHandle(0);
}
#pragma interrupt off
#endif

#if defined(QSCI1)
#pragma interrupt alignsp saveall
void QSCI_IRQ_HANDER(1, RERR)(void);
void QSCI_IRQ_HANDER(1, RERR)(void)
{
    HAL_UartInterruptHandle(1);
}
#pragma interrupt off

#pragma interrupt alignsp saveall
void QSCI_IRQ_HANDER(1, RCV)(void);
void QSCI_IRQ_HANDER(1, RCV)(void)
{
    HAL_UartInterruptHandle(1);
}
#pragma interrupt off

#pragma interrupt alignsp saveall
#if defined(QSCI_TRIDLE_IRQS)
void QSCI_IRQ_HANDER(1, TRIDLE)(void);
void QSCI_IRQ_HANDER(1, TRIDLE)(void)
{
    HAL_UartInterruptHandle(1);
}
#elif defined(QSCI_TIDLE_IRQS)
void QSCI_IRQ_HANDER(1, TIDLE)(void);
void QSCI_IRQ_HANDER(1, TIDLE)(void)
{
    HAL_UartInterruptHandle(1);
}
#endif
#pragma interrupt off

#pragma interrupt alignsp saveall
void QSCI_IRQ_HANDER(1, TDRE)(void);
void QSCI_IRQ_HANDER(1, TDRE)(void)
{
    HAL_UartInterruptHandle(1);
}
#pragma interrupt off
#endif

#if defined(QSCI2)
#pragma interrupt alignsp saveall
void QSCI_IRQ_HANDER(2, RERR)(void);
void QSCI_IRQ_HANDER(2, RERR)(void)
{
    HAL_UartInterruptHandle(2);
}
#pragma interrupt off

#pragma interrupt alignsp saveall
void QSCI_IRQ_HANDER(2, RCV)(void);
void QSCI_IRQ_HANDER(2, RCV)(void)
{
    HAL_UartInterruptHandle(2);
}
#pragma interrupt off

#pragma interrupt alignsp saveall
#if defined(QSCI_TRIDLE_IRQS)
void QSCI_IRQ_HANDER(2, TRIDLE)(void);
void QSCI_IRQ_HANDER(2, TRIDLE)(void)
{
    HAL_UartInterruptHandle(2);
}
#elif defined(QSCI_TIDLE_IRQS)
void QSCI_IRQ_HANDER(2, TIDLE)(void);
void QSCI_IRQ_HANDER(2, TIDLE)(void)
{
    HAL_UartInterruptHandle(2);
}
#endif
#pragma interrupt off

#pragma interrupt alignsp saveall
void QSCI_IRQ_HANDER(2, TDRE)(void);
void QSCI_IRQ_HANDER(2, TDRE)(void)
{
    HAL_UartInterruptHandle(2);
}
#pragma interrupt off
#endif

#endif

#endif
