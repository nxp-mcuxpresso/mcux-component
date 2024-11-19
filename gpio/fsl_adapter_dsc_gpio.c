/*
 * Copyright 2020 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_device_registers.h"
#include "fsl_gpio.h"
#include "fsl_adapter_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief The pin config struct of gpio adapter. */

typedef struct _hal_gpio_pin
{
    uint16_t port : 3U;
    uint16_t reserved : 1U;
    uint16_t pin : 5U;
    uint16_t direction : 1U;
    uint16_t trigger : 3U;
    uint16_t reserved2 : 3U;
} hal_gpio_pin_t;

typedef struct _hal_gpio_state
{
    struct _hal_gpio_state *next;
    hal_gpio_callback_t callback;
    void *callbackParam;
    hal_gpio_pin_t pin;
} hal_gpio_state_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static GPIO_Type *const s_GpioPort[] = GPIO_BASE_PTRS;
static hal_gpio_state_t *s_GpioHead;

#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
/*! @brief Pointers to GPIO clocks for each instance. */
static const clock_ip_name_t s_gpioClocks[] = GPIO_CLOCKS;
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */

/*******************************************************************************
 * Code
 ******************************************************************************/

static void HAL_GpioInterruptHandle(uint8_t port)
{
    hal_gpio_state_t *head = s_GpioHead;
    uint16_t pinInterruptSetFlag;

    while (NULL != head)
    {
        if (head->pin.port == port)
        {
            break;
        }
        head = head->next;
    }

    if (NULL == head)
    {
        return;
    }

    head = s_GpioHead;

    pinInterruptSetFlag = GPIO_PortGetEdgeDetectedStatusFlags(s_GpioPort[port]);
    /* Clear external interrupt flag. */
    GPIO_PortClearEdgeDetectedStatusFlags(s_GpioPort[port], pinInterruptSetFlag);

    if (0U != pinInterruptSetFlag)
    {
        while (NULL != head)
        {
            if ((port == head->pin.port) && ((pinInterruptSetFlag & (1U << head->pin.pin)) == 0U))
            {
                if (NULL != head->callback)
                {
                    head->callback(head->callbackParam);
                }
                pinInterruptSetFlag &= ~(1U << head->pin.pin);
                if (0U == pinInterruptSetFlag)
                {
                    break;
                }
            }
            head = head->next;
        }
    }
}

static hal_gpio_status_t HAL_GpioConflictSearch(hal_gpio_state_t *head, uint8_t port, uint8_t pin)
{
    while (NULL != head)
    {
        if ((head->pin.port == port) && (head->pin.pin == pin))
        {
            return kStatus_HAL_GpioPinConflict;
        }
        head = head->next;
    }
    return kStatus_HAL_GpioSuccess;
}

static hal_gpio_status_t HAL_GpioAddItem(hal_gpio_state_t **head, hal_gpio_state_t *node)
{
    hal_gpio_state_t *p = *head;
    hal_gpio_state_t *q = *head;
    uint32_t regPrimask;

    regPrimask = DisableGlobalIRQ();

    if (NULL == p)
    {
        *head = node;
    }
    else
    {
        while (NULL != p)
        {
            if (p == node)
            {
                EnableGlobalIRQ(regPrimask);
                return kStatus_HAL_GpioPinConflict;
            }
            q = p;
            p = p->next;
        }

        q->next = node;
    }
    node->next = NULL;
    EnableGlobalIRQ(regPrimask);
    return kStatus_HAL_GpioSuccess;
}

static hal_gpio_status_t HAL_GpioRemoveItem(hal_gpio_state_t **head, hal_gpio_state_t *node)
{
    hal_gpio_state_t *p = *head;
    hal_gpio_state_t *q = NULL;
    uint32_t regPrimask;

    regPrimask = DisableGlobalIRQ();
    while (p != NULL)
    {
        if (p == node)
        {
            if (NULL == q)
            {
                *head = p->next;
            }
            else
            {
                q->next = p->next;
            }
            break;
        }
        else
        {
            q = p;
            p = p->next;
        }
    }
    EnableGlobalIRQ(regPrimask);
    return kStatus_HAL_GpioSuccess;
}

#if defined(GPIOA)
void GPIOA_DriverIRQHandler(void);
void GPIOA_DriverIRQHandler(void)
{
    HAL_GpioInterruptHandle(0);
}
#endif
#if defined(GPIOB)
void GPIOB_DriverIRQHandler(void);
void GPIOB_DriverIRQHandler(void)
{
    HAL_GpioInterruptHandle(1);
}
#endif
#if defined(GPIOC)
void GPIOC_DriverIRQHandler(void);
void GPIOC_DriverIRQHandler(void)
{
    HAL_GpioInterruptHandle(2);
}
#endif
#if defined(GPIOD)
void GPIOD_DriverIRQHandler(void);
void GPIOD_DriverIRQHandler(void)
{
    HAL_GpioInterruptHandle(3);
}
#endif
#if defined(GPIOE)
void GPIOE_DriverIRQHandler(void);
void GPIOE_DriverIRQHandler(void)
{
    HAL_GpioInterruptHandle(4);
}
#endif
#if defined(GPIOF)
void GPIOF_DriverIRQHandler(void);
void GPIOF_DriverIRQHandler(void)
{
    HAL_GpioInterruptHandle(5);
}
#endif
#if defined(GPIOG)
void GPIOG_DriverIRQHandler(void);
void GPIOG_DriverIRQHandler(void)
{
    HAL_GpioInterruptHandle(6);
}
#endif

hal_gpio_status_t HAL_GpioInit(hal_gpio_handle_t gpioHandle, hal_gpio_pin_config_t *pinConfig)
{
    hal_gpio_state_t *gpioState;
    hal_gpio_status_t status;

    assert(gpioHandle);
    assert(pinConfig);
    assert(HAL_GPIO_HANDLE_SIZE >= sizeof(hal_gpio_state_t));

    gpioState = (hal_gpio_state_t *)gpioHandle;

    if ((NULL != s_GpioHead) &&
        (kStatus_HAL_GpioSuccess != HAL_GpioConflictSearch(s_GpioHead, pinConfig->port, pinConfig->pin)))
    {
        return kStatus_HAL_GpioPinConflict;
    }

    status = HAL_GpioAddItem(&s_GpioHead, gpioState);
    if (kStatus_HAL_GpioSuccess != status)
    {
        return status;
    }

    gpioState->pin.pin       = pinConfig->pin;
    gpioState->pin.port      = pinConfig->port;
    gpioState->pin.direction = (uint16_t)pinConfig->direction;

    /* GPIO enable module clock. */
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    CLOCK_EnableClock(s_gpioClocks[pinConfig->port]);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */

    uint16_t pin_mask = ((uint16_t)1U) << (pinConfig->pin);

    /* GPIO pin set input first. */
    GPIO_PinSetDirection(s_GpioPort[pinConfig->port], (gpio_pin_t)pin_mask, kGPIO_DigitalInput);

    /* GPIO disbale interrupt as default. */
    GPIO_PinSetInterruptConfig(s_GpioPort[pinConfig->port], (gpio_pin_t)pin_mask, kGPIO_InterruptDisable);

    /* GPIO set default output level. */
    GPIO_PinWrite(s_GpioPort[pinConfig->port], (gpio_pin_t)pin_mask, (gpio_output_level_t)pinConfig->level);

    /* GPIO pin set direction. */
    GPIO_PinSetDirection(s_GpioPort[pinConfig->port], (gpio_pin_t)pin_mask,
                         ((kHAL_GpioDirectionOut == pinConfig->direction) ? kGPIO_DigitalOutput : kGPIO_DigitalInput));

    return kStatus_HAL_GpioSuccess;
}

hal_gpio_status_t HAL_GpioDeinit(hal_gpio_handle_t gpioHandle)
{
    hal_gpio_state_t *gpioState;

    assert(gpioHandle);

    gpioState = (hal_gpio_state_t *)gpioHandle;
    if ((uint16_t)kHAL_GpioDirectionIn == gpioState->pin.direction)
    {
        (void)HAL_GpioSetTriggerMode(gpioHandle, kHAL_GpioInterruptDisable);
    }
    (void)HAL_GpioRemoveItem(&s_GpioHead, gpioState);
    return kStatus_HAL_GpioSuccess;
}

hal_gpio_status_t HAL_GpioGetInput(hal_gpio_handle_t gpioHandle, uint8_t *pinState)
{
    hal_gpio_state_t *gpioState;

    assert(gpioHandle);
    assert(pinState);

    gpioState = (hal_gpio_state_t *)gpioHandle;

    uint16_t pin_mask = ((uint16_t)1U) << (gpioState->pin.pin);
    *pinState         = (GPIO_PinRead(s_GpioPort[gpioState->pin.port], (gpio_pin_t)pin_mask) != 0U) ? 1U : 0U;
    return kStatus_HAL_GpioSuccess;
}

hal_gpio_status_t HAL_GpioSetOutput(hal_gpio_handle_t gpioHandle, uint8_t pinState)
{
    hal_gpio_state_t *gpioState;

    assert(gpioHandle);

    gpioState         = (hal_gpio_state_t *)gpioHandle;
    uint16_t pin_mask = ((uint16_t)1U) << (gpioState->pin.pin);
    GPIO_PinWrite(s_GpioPort[gpioState->pin.port], (gpio_pin_t)pin_mask,
                  (pinState != 0U) ? kGPIO_OutputHigh : kGPIO_OutputLow);
    return kStatus_HAL_GpioSuccess;
}

hal_gpio_status_t HAL_GpioInstallCallback(hal_gpio_handle_t gpioHandle,
                                          hal_gpio_callback_t callback,
                                          void *callbackParam)
{
    hal_gpio_state_t *gpioState;

    assert(gpioHandle);

    gpioState = (hal_gpio_state_t *)gpioHandle;

    gpioState->callbackParam = callbackParam;
    gpioState->callback      = callback;

    return kStatus_HAL_GpioSuccess;
}

hal_gpio_status_t HAL_GpioGetTriggerMode(hal_gpio_handle_t gpioHandle, hal_gpio_interrupt_trigger_t *gpioTrigger)
{
    hal_gpio_state_t *gpioState;

    assert(gpioHandle);

    gpioState = (hal_gpio_state_t *)gpioHandle;

    if ((uint16_t)kHAL_GpioDirectionOut == gpioState->pin.direction)
    {
        return kStatus_HAL_GpioError;
    }

    *gpioTrigger = (hal_gpio_interrupt_trigger_t)gpioState->pin.trigger;
    return kStatus_HAL_GpioSuccess;
}

hal_gpio_status_t HAL_GpioSetTriggerMode(hal_gpio_handle_t gpioHandle, hal_gpio_interrupt_trigger_t gpioTrigger)
{
    hal_gpio_state_t *gpioState;
#if defined(FSL_FEATURE_SOC_GPIO_COUNT) && (FSL_FEATURE_SOC_GPIO_COUNT == 7)
    IRQn_Type portIrq[] = {GPIOA_IRQn, GPIOB_IRQn, GPIOC_IRQn, GPIOD_IRQn, GPIOE_IRQn, GPIOF_IRQn, GPIOG_IRQn};
#elif defined(FSL_FEATURE_SOC_GPIO_COUNT) && (FSL_FEATURE_SOC_GPIO_COUNT == 6)
    IRQn_Type portIrq[] = {GPIOA_IRQn, GPIOB_IRQn, GPIOC_IRQn, GPIOD_IRQn, GPIOE_IRQn, GPIOF_IRQn};
#endif
    gpio_interrupt_mode_t pinInt;

    bool bSupportedInterrupt = true;

    assert(gpioHandle);

    gpioState = (hal_gpio_state_t *)gpioHandle;

    if ((uint16_t)kHAL_GpioDirectionOut == gpioState->pin.direction)
    {
        return kStatus_HAL_GpioError;
    }

    switch (gpioTrigger)
    {
        case kHAL_GpioInterruptRisingEdge:
            pinInt = kGPIO_InterruptRisingEdge;
            break;
        case kHAL_GpioInterruptFallingEdge:
            pinInt = kGPIO_InterruptFallingEdge;
            break;
        case kHAL_GpioInterruptDisable:
            pinInt = kGPIO_InterruptDisable;
            break;
        default:
            bSupportedInterrupt = false;
            break;
    }

    if (!bSupportedInterrupt)
    {
        return kStatus_HAL_GpioError;
    }

    gpioState->pin.trigger = (uint16_t)gpioTrigger;

    /* initialize port interrupt */
    uint16_t pin_mask = ((uint16_t)1U) << (gpioState->pin.pin);
    GPIO_PinSetInterruptConfig(s_GpioPort[gpioState->pin.port], (gpio_pin_t)pin_mask, pinInt);
    if (kGPIO_InterruptDisable == pinInt)
    {
        (void)DisableIRQ(portIrq[gpioState->pin.port]);
    }
    else
    {
        (void)EnableIRQWithPriority(portIrq[gpioState->pin.port], HAL_GPIO_ISR_PRIORITY);
    }

    return kStatus_HAL_GpioSuccess;
}

hal_gpio_status_t HAL_GpioWakeUpSetting(hal_gpio_handle_t gpioHandle, uint8_t enable)
{
    assert(gpioHandle);

    return kStatus_HAL_GpioError;
}

hal_gpio_status_t HAL_GpioEnterLowpower(hal_gpio_handle_t gpioHandle)
{
    assert(gpioHandle);

    return kStatus_HAL_GpioError;
}

hal_gpio_status_t HAL_GpioExitLowpower(hal_gpio_handle_t gpioHandle)
{
    assert(gpioHandle);

    return kStatus_HAL_GpioError;
}
