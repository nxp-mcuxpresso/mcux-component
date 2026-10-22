/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include "fsl_usbd_ehci_cmsis.h"

#include "usb_device_config.h"
#include "usb_device.h"
#include "usb_device_dci.h"
#include "usb_device_ehci.h"
#include "usb_phy.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Component ID definition, used by tools. */
#ifndef FSL_COMPONENT_ID
#define FSL_COMPONENT_ID "platform.drivers.usbd_ehci_cmsis"
#endif

/* Maximum number of endpoints configured in usb_device_config.h */
#define USBD_MAX_ENDPOINT_NUM (USB_DEVICE_CONFIG_ENDPOINTS)

/* Extract direction bit (bit 7) from endpoint address */
#define EP_DIR(ep_addr) (((ep_addr) >> 7) & 1U)

/* Extract endpoint number (bits 3:0) from endpoint address */
#define EP_NUM(ep_addr) ((ep_addr) & ARM_USB_ENDPOINT_NUMBER_MASK)

/* Driver status flags */
typedef struct {
    uint8_t initialized : 1; /*!< 1 = initialized,  0 = not initialized        */
    uint8_t powered     : 1; /*!< 1 = powered,       0 = not powered           */
    uint8_t low_power   : 1; /*!< 1 = in low-power (suspend) state, 0 = active */
    uint8_t reserved    : 5; /*!< Reserved, must be zero                       */
} DriverStatus_t;

/* Per-endpoint runtime information */
typedef struct {
    volatile uint32_t num_transferred; /*!< Number of bytes transferred in the last transfer */
} EP_Info_t;

/* Instance run-time information (read-write) */
typedef struct {
    ARM_USBD_SignalDeviceEvent_t   cb_device_event;                        /*!< Device event callback                                   */
    ARM_USBD_SignalEndpointEvent_t cb_endpoint_event;                      /*!< Endpoint event callback                                 */
    DriverStatus_t                 drv_status;                             /*!< Driver status flags                                     */
    volatile ARM_USBD_STATE        usbd_state;                             /*!< Current USB Device state                                */
    usb_device_handle              deviceHandle;                           /*!< SDK USB Device handle                                   */
    volatile uint32_t              setup_received;                         /*!< 1 = setup packet received and not yet read, 0 = read    */
    volatile uint8_t               setup_packet[8];                       /*!< Last received setup packet (8 bytes)                    */
    EP_Info_t                      ep_info[USBD_MAX_ENDPOINT_NUM][2];     /*!< Endpoint info array [endpoint number][direction 0=OUT 1=IN] */
} RW_Info_t;

/* Instance compile-time information (read-only), includes pointer to RW_Info_t */
typedef struct {
    RW_Info_t *ptr_rw_info; /*!< Pointer to run-time information     */
    uint8_t    ctrl;        /*!< USB EHCI controller index (1-based) */
    uint8_t    ctrl_id;     /*!< USB EHCI controller identifier      */
    uint8_t    irqn;        /*!< USB EHCI controller interrupt number */
    uint8_t    reserved;    /*!< Reserved, must be zero              */
} RO_Info_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static const ARM_DRIVER_VERSION    s_usbdDriverVersion      = {
    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 3),
    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)
};
static const ARM_USBD_CAPABILITIES s_usbdDriverCapabilities = {
    1U, /*!< VBUS detection           */
    1U, /*!< Signal VBUS On event     */
    1U, /*!< Signal VBUS Off event    */
    0U  /*!< Reserved (must be zero)  */
};

static       RW_Info_t usbd1_rw_info;
static const RO_Info_t usbd1_ro_info = {
    &usbd1_rw_info,
    1U,
    kUSB_ControllerEhci0,
    (uint8_t)USB1_HS_IRQn,
    0U
};

static ARM_DRIVER_VERSION    USBD_GetVersion                 (void);
static ARM_USBD_CAPABILITIES USBD_GetCapabilities            (void);
static int32_t               USBDn_Initialize                (const RO_Info_t * const ptr_ro_info, ARM_USBD_SignalDeviceEvent_t cb_device_event, ARM_USBD_SignalEndpointEvent_t cb_endpoint_event);
static int32_t               USBDn_Uninitialize              (const RO_Info_t * const ptr_ro_info);
static int32_t               USBDn_PowerControl              (const RO_Info_t * const ptr_ro_info, ARM_POWER_STATE state);
static int32_t               USBDn_DeviceConnect             (const RO_Info_t * const ptr_ro_info);
static int32_t               USBDn_DeviceDisconnect          (const RO_Info_t * const ptr_ro_info);
static ARM_USBD_STATE        USBDn_DeviceGetState            (const RO_Info_t * const ptr_ro_info);
static int32_t               USBDn_DeviceRemoteWakeup        (const RO_Info_t * const ptr_ro_info);
static int32_t               USBDn_DeviceSetAddress          (const RO_Info_t * const ptr_ro_info, uint8_t dev_addr);
static int32_t               USBDn_ReadSetupPacket           (const RO_Info_t * const ptr_ro_info, uint8_t *setup);
static int32_t               USBDn_EndpointConfigure         (const RO_Info_t * const ptr_ro_info, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_max_packet_size);
static int32_t               USBDn_EndpointUnconfigure       (const RO_Info_t * const ptr_ro_info, uint8_t ep_addr);
static int32_t               USBDn_EndpointStall             (const RO_Info_t * const ptr_ro_info, uint8_t ep_addr, bool stall);
static int32_t               USBDn_EndpointTransfer          (const RO_Info_t * const ptr_ro_info, uint8_t ep_addr, uint8_t *data, uint32_t num);
static uint32_t              USBDn_EndpointTransferGetResult (const RO_Info_t * const ptr_ro_info, uint8_t ep_addr);
static int32_t               USBDn_EndpointTransferAbort     (const RO_Info_t * const ptr_ro_info, uint8_t ep_addr);
static uint16_t              USBDn_GetFrameNumber            (const RO_Info_t * const ptr_ro_info);
static usb_status_t          USB_DeviceCallback              (usb_device_handle handle, uint32_t event, void *param);
static usb_status_t          USB_DeviceEndpointCb            (usb_device_handle handle, usb_device_endpoint_callback_message_struct_t *message, void *callbackParam);

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Returns driver version information */
static ARM_DRIVER_VERSION USBD_GetVersion(void)
{
    return s_usbdDriverVersion;
}

/* Returns driver capabilities */
static ARM_USBD_CAPABILITIES USBD_GetCapabilities(void)
{
    return s_usbdDriverCapabilities;
}

/* Initialize the USB Device Interface */
static int32_t USBDn_Initialize(const RO_Info_t * const         ptr_ro_info,
                                ARM_USBD_SignalDeviceEvent_t    cb_device_event,
                                ARM_USBD_SignalEndpointEvent_t  cb_endpoint_event)
{
    memset((void *)ptr_ro_info->ptr_rw_info, 0, sizeof(RW_Info_t));

    ptr_ro_info->ptr_rw_info->cb_device_event   = cb_device_event;
    ptr_ro_info->ptr_rw_info->cb_endpoint_event = cb_endpoint_event;
    ptr_ro_info->ptr_rw_info->drv_status.initialized = 1U;

    return ARM_DRIVER_OK;
}

/* De-initialize the USB Device Interface */
static int32_t USBDn_Uninitialize(const RO_Info_t * const ptr_ro_info)
{
    if (ptr_ro_info->ptr_rw_info->drv_status.powered != 0U)
    {
        (void)USBDn_PowerControl(ptr_ro_info, ARM_POWER_OFF);
    }

    memset((void *)ptr_ro_info->ptr_rw_info, 0, sizeof(RW_Info_t));

    return ARM_DRIVER_OK;
}

/* Control USB Device Interface power */
static int32_t USBDn_PowerControl(const RO_Info_t * const ptr_ro_info, ARM_POWER_STATE state)
{
    ARM_USBD_SignalDeviceEvent_t   cb_device_event;
    ARM_USBD_SignalEndpointEvent_t cb_endpoint_event;
    DriverStatus_t                 drv_status;
    uint8_t                        ep_num;
    uint8_t                        ep_dir;
    switch (state)
    {
        case ARM_POWER_FULL:
            if (ptr_ro_info->ptr_rw_info->drv_status.initialized == 0U)
            {
                return ARM_DRIVER_ERROR;
            }

            if (ptr_ro_info->ptr_rw_info->drv_status.low_power != 0U)
            {
                ptr_ro_info->ptr_rw_info->drv_status.low_power = 0U;
                if (USB_DeviceRun(ptr_ro_info->ptr_rw_info->deviceHandle) != kStatus_USB_Success)
                {
                    return ARM_DRIVER_ERROR;
                }
                break;
            }

            cb_device_event   = ptr_ro_info->ptr_rw_info->cb_device_event;
            cb_endpoint_event = ptr_ro_info->ptr_rw_info->cb_endpoint_event;
            drv_status        = ptr_ro_info->ptr_rw_info->drv_status;

            memset((void *)ptr_ro_info->ptr_rw_info, 0, sizeof(RW_Info_t));

            ptr_ro_info->ptr_rw_info->cb_device_event   = cb_device_event;
            ptr_ro_info->ptr_rw_info->cb_endpoint_event = cb_endpoint_event;
            ptr_ro_info->ptr_rw_info->drv_status        = drv_status;

            if (USB_DeviceInit(ptr_ro_info->ctrl_id, USB_DeviceCallback, &ptr_ro_info->ptr_rw_info->deviceHandle) != kStatus_USB_Success)
            {
                return ARM_DRIVER_ERROR;
            }
            if (EnableIRQ((IRQn_Type)ptr_ro_info->irqn) != kStatus_Success)
            {
                return ARM_DRIVER_ERROR;
            }

            ptr_ro_info->ptr_rw_info->drv_status.powered    = 1U;
            ptr_ro_info->ptr_rw_info->usbd_state.speed      = ARM_USB_SPEED_FULL;
            break;

        case ARM_POWER_OFF:
            if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
            {
                return ARM_DRIVER_OK;
            }

            for (ep_num = 0U; ep_num < USBD_MAX_ENDPOINT_NUM; ep_num++)
            {
                for (ep_dir = 0U; ep_dir < 2U; ep_dir++)
                {
                    (void)USBDn_EndpointTransferAbort(ptr_ro_info, (uint8_t)(ep_dir << 7) | ep_num);
                }
            }

            if (DisableIRQ((IRQn_Type)ptr_ro_info->irqn) != kStatus_Success)
            {
                return ARM_DRIVER_ERROR;
            }
            if (USB_DeviceDeinit(ptr_ro_info->ptr_rw_info->deviceHandle) != kStatus_USB_Success)
            {
                return ARM_DRIVER_ERROR;
            }
            (void)IRQ_ClearPendingIRQ((IRQn_Type)ptr_ro_info->irqn);

            ptr_ro_info->ptr_rw_info->drv_status.powered = 0U;

            cb_device_event   = ptr_ro_info->ptr_rw_info->cb_device_event;
            cb_endpoint_event = ptr_ro_info->ptr_rw_info->cb_endpoint_event;
            drv_status        = ptr_ro_info->ptr_rw_info->drv_status;

            memset((void *)ptr_ro_info->ptr_rw_info, 0, sizeof(RW_Info_t));

            ptr_ro_info->ptr_rw_info->cb_device_event   = cb_device_event;
            ptr_ro_info->ptr_rw_info->cb_endpoint_event = cb_endpoint_event;
            ptr_ro_info->ptr_rw_info->drv_status        = drv_status;
            break;

        case ARM_POWER_LOW:
            if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
            {
                return ARM_DRIVER_ERROR;
            }

            if (ptr_ro_info->ptr_rw_info->drv_status.low_power != 0U)
            {
                return ARM_DRIVER_OK;
            }

            if (USB_DeviceStop(ptr_ro_info->ptr_rw_info->deviceHandle) != kStatus_USB_Success)
            {
                return ARM_DRIVER_ERROR;
            }

            ptr_ro_info->ptr_rw_info->drv_status.low_power = 1U;
            break;

        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }

    return ARM_DRIVER_OK;
}

/* Connect the USB Device to the bus */
static int32_t USBDn_DeviceConnect(const RO_Info_t * const ptr_ro_info)
{
    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if (USB_DeviceRun(ptr_ro_info->ptr_rw_info->deviceHandle) != kStatus_USB_Success)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/* Disconnect the USB Device from the bus */
static int32_t USBDn_DeviceDisconnect(const RO_Info_t * const ptr_ro_info)
{
    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if (USB_DeviceStop(ptr_ro_info->ptr_rw_info->deviceHandle) != kStatus_USB_Success)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/* Get the current USB Device state */
static ARM_USBD_STATE USBDn_DeviceGetState(const RO_Info_t * const ptr_ro_info)
{
    return ptr_ro_info->ptr_rw_info->usbd_state;
}

/* Trigger USB Remote Wakeup signaling */
static int32_t USBDn_DeviceRemoteWakeup(const RO_Info_t * const ptr_ro_info)
{
    uint8_t state;

    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    state = 1U;
    if (USB_DeviceSetStatus(ptr_ro_info->ptr_rw_info->deviceHandle,
                            kUSB_DeviceStatusRemoteWakeup, &state) != kStatus_USB_Success)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/* Set the USB Device address.
 * The address is written directly to DEVICEADDR without setting the USBADRA
 * auto-advance bit, because this function is called after the STATUS IN
 * transfer has already completed. */
static int32_t USBDn_DeviceSetAddress(const RO_Info_t * const ptr_ro_info, uint8_t dev_addr)
{
    usb_device_ehci_state_struct_t *ehciState;
    uint8_t state;

    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    ehciState = (usb_device_ehci_state_struct_t *)((usb_device_struct_t *)ptr_ro_info->ptr_rw_info->deviceHandle)->controllerHandle;
    ehciState->registerBase->DEVICEADDR = (uint32_t)dev_addr << USBHS_DEVICEADDR_USBADR_SHIFT;

    state = (uint8_t)kUSB_DeviceStateAddress;
    if (USB_DeviceSetStatus(ptr_ro_info->ptr_rw_info->deviceHandle,
                            kUSB_DeviceStatusDeviceState, &state) != kStatus_USB_Success)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/* Read the setup packet received on the Control Endpoint */
static int32_t USBDn_ReadSetupPacket(const RO_Info_t * const ptr_ro_info, uint8_t *setup)
{
    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if (setup == NULL)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (ptr_ro_info->ptr_rw_info->setup_received == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    /* Re-read if a new setup packet arrives during the copy. */
    do
    {
        ptr_ro_info->ptr_rw_info->setup_received = 0U;
        for (uint8_t i = 0U; i < 8U; i++)
        {
            setup[i] = ptr_ro_info->ptr_rw_info->setup_packet[i];
        }
    } while (ptr_ro_info->ptr_rw_info->setup_received != 0U);

    return ARM_DRIVER_OK;
}

/* Configure a USB Endpoint */
static int32_t USBDn_EndpointConfigure(const RO_Info_t * const ptr_ro_info,
                                       uint8_t                 ep_addr,
                                       uint8_t                 ep_type,
                                       uint16_t                ep_max_packet_size)
{
    EP_Info_t                            *ptr_ep;
    uint8_t                               ep_num;
    uint8_t                               ep_dir;
    usb_device_endpoint_init_struct_t     epInit;
    usb_device_endpoint_callback_struct_t epCallback;

    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if (ptr_ro_info->ptr_rw_info->deviceHandle == NULL)
    {
        return ARM_DRIVER_ERROR;
    }

    ep_num = EP_NUM(ep_addr);
    if (ep_num >= USBD_MAX_ENDPOINT_NUM)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    ep_dir = EP_DIR(ep_addr);
    ptr_ep = &ptr_ro_info->ptr_rw_info->ep_info[ep_num][ep_dir];

    memset((void *)ptr_ep, 0, sizeof(EP_Info_t));

    memset(&epInit, 0, sizeof(epInit));
    epInit.maxPacketSize   = ep_max_packet_size;
    epInit.endpointAddress = ep_addr;
    epInit.transferType    = ep_type;
    epInit.zlt             = 0U;

    epCallback.callbackFn    = USB_DeviceEndpointCb;
    epCallback.callbackParam = (void *)((uint32_t)ep_addr);

    if (USB_DeviceInitEndpoint(ptr_ro_info->ptr_rw_info->deviceHandle,
                               &epInit, &epCallback) != kStatus_USB_Success)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/* Unconfigure a USB Endpoint */
static int32_t USBDn_EndpointUnconfigure(const RO_Info_t * const ptr_ro_info, uint8_t ep_addr)
{
    EP_Info_t *ptr_ep;
    uint8_t    ep_num;
    uint8_t    ep_dir;

    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    ep_num = EP_NUM(ep_addr);
    if (ep_num >= USBD_MAX_ENDPOINT_NUM)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    ep_dir = EP_DIR(ep_addr);
    ptr_ep = &ptr_ro_info->ptr_rw_info->ep_info[ep_num][ep_dir];

    if (USB_DeviceDeinitEndpoint(ptr_ro_info->ptr_rw_info->deviceHandle,
                                 ep_addr) != kStatus_USB_Success)
    {
        return ARM_DRIVER_ERROR;
    }

    memset((void *)ptr_ep, 0, sizeof(EP_Info_t));

    return ARM_DRIVER_OK;
}

/* Set or clear the STALL condition on a USB Endpoint */
static int32_t USBDn_EndpointStall(const RO_Info_t * const ptr_ro_info,
                                   uint8_t ep_addr, bool stall)
{
    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if (stall != 0U)
    {
        if (USB_DeviceStallEndpoint(ptr_ro_info->ptr_rw_info->deviceHandle,
                                    ep_addr) != kStatus_USB_Success)
        {
            return ARM_DRIVER_ERROR;
        }
    }
    else
    {
        if (USB_DeviceUnstallEndpoint(ptr_ro_info->ptr_rw_info->deviceHandle,
                                      ep_addr) != kStatus_USB_Success)
        {
            return ARM_DRIVER_ERROR;
        }
        if (USB_DeviceCancel(ptr_ro_info->ptr_rw_info->deviceHandle,
                             ep_addr) != kStatus_USB_Success)
        {
            return ARM_DRIVER_ERROR;
        }
    }

    return ARM_DRIVER_OK;
}

/* Start a read or write transfer on a USB Endpoint */
static int32_t USBDn_EndpointTransfer(const RO_Info_t * const ptr_ro_info,
                                      uint8_t ep_addr, uint8_t *data, uint32_t num)
{
    EP_Info_t *ptr_ep;
    uint8_t    ep_num;
    uint8_t    ep_dir;

    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    ep_num = EP_NUM(ep_addr);
    if (ep_num >= USBD_MAX_ENDPOINT_NUM)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    ep_dir = EP_DIR(ep_addr);
    ptr_ep = &ptr_ro_info->ptr_rw_info->ep_info[ep_num][ep_dir];

    ptr_ep->num_transferred = 0U;

    if (ep_dir != 0U)
    {
        if (USB_DeviceSendRequest(ptr_ro_info->ptr_rw_info->deviceHandle, ep_addr, data, num) !=
            kStatus_USB_Success)
        {
            return ARM_DRIVER_ERROR;
        }
    }
    else
    {
        if (USB_DeviceRecvRequest(ptr_ro_info->ptr_rw_info->deviceHandle, ep_addr, data, num) !=
            kStatus_USB_Success)
        {
            return ARM_DRIVER_ERROR;
        }
    }

    return ARM_DRIVER_OK;
}

/* Get the number of bytes transferred in the last endpoint transfer */
static uint32_t USBDn_EndpointTransferGetResult(const RO_Info_t * const ptr_ro_info,
                                                uint8_t ep_addr)
{
    uint8_t ep_num;

    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return 0U;
    }

    ep_num = EP_NUM(ep_addr);
    if (ep_num >= USBD_MAX_ENDPOINT_NUM)
    {
        return 0U;
    }

    return ptr_ro_info->ptr_rw_info->ep_info[ep_num][EP_DIR(ep_addr)].num_transferred;
}

/* Abort the current transfer on a USB Endpoint */
static int32_t USBDn_EndpointTransferAbort(const RO_Info_t * const ptr_ro_info, uint8_t ep_addr)
{
    uint8_t ep_num;

    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    ep_num = EP_NUM(ep_addr);
    if (ep_num >= USBD_MAX_ENDPOINT_NUM)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (USB_DeviceCancel(ptr_ro_info->ptr_rw_info->deviceHandle,
                         ep_addr) != kStatus_USB_Success)
    {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

/* Get the current USB Frame Number */
static uint16_t USBDn_GetFrameNumber(const RO_Info_t * const ptr_ro_info)
{
    uint16_t frame = 0U;

    if (ptr_ro_info->ptr_rw_info->drv_status.powered == 0U)
    {
        return 0U;
    }

    (void)USB_DeviceGetStatus(ptr_ro_info->ptr_rw_info->deviceHandle,
                              kUSB_DeviceStatusSynchFrame, (void *)(&frame));

    return frame;
}

/* SDK USB device event callback - maps SDK events to CMSIS events */
static usb_status_t USB_DeviceCallback(usb_device_handle handle,
                                       uint32_t event, void *param)
{
    RW_Info_t *ptr_rw_info;
    EP_Info_t *ptr_ep;
    uint8_t    speed;
    uint8_t    ep_addr;

    (void)param;

    if (handle == usbd1_rw_info.deviceHandle)
    {
        ptr_rw_info = &usbd1_rw_info;
    }
    else
    {
        return kStatus_USB_Error;
    }

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
            for (uint8_t i = 0U; i < USBD_MAX_ENDPOINT_NUM; i++)
            {
                ep_addr = i;
                ptr_ep  = &ptr_rw_info->ep_info[EP_NUM(ep_addr)][EP_DIR(ep_addr)];
                USB_DeviceUnstallEndpoint(ptr_rw_info->deviceHandle, ep_addr);
                ptr_ep->num_transferred = 0U;
                (void)USB_DeviceCancel(ptr_rw_info->deviceHandle, ep_addr);

                ep_addr = ARM_USB_ENDPOINT_DIRECTION_MASK | i;
                ptr_ep  = &ptr_rw_info->ep_info[EP_NUM(ep_addr)][EP_DIR(ep_addr)];
                USB_DeviceUnstallEndpoint(ptr_rw_info->deviceHandle, ep_addr);
                ptr_ep->num_transferred = 0U;
                (void)USB_DeviceCancel(ptr_rw_info->deviceHandle, ep_addr);
            }

            ptr_rw_info->usbd_state.active = 1U;
            ptr_rw_info->usbd_state.vbus   = 1U;

            USB_DeviceGetStatus(ptr_rw_info->deviceHandle, kUSB_DeviceStatusSpeed, &speed);
            if (ptr_rw_info->cb_device_event != NULL)
            {
                ptr_rw_info->cb_device_event(ARM_USBD_EVENT_RESET);
            }
            if (speed == USB_SPEED_FULL)
            {
                ptr_rw_info->usbd_state.speed = ARM_USB_SPEED_FULL;
            }
            else if (speed == USB_SPEED_HIGH)
            {
                ptr_rw_info->usbd_state.speed = ARM_USB_SPEED_HIGH;
                if (ptr_rw_info->cb_device_event != NULL)
                {
                    ptr_rw_info->cb_device_event(ARM_USBD_EVENT_HIGH_SPEED);
                }
            }
            break;

        case kUSB_DeviceEventSuspend:
            ptr_rw_info->usbd_state.active = 0U;
            if (ptr_rw_info->cb_device_event != NULL)
            {
                ptr_rw_info->cb_device_event(ARM_USBD_EVENT_SUSPEND);
            }
            break;

        case kUSB_DeviceEventResume:
            ptr_rw_info->usbd_state.active = 1U;
            if (ptr_rw_info->cb_device_event != NULL)
            {
                ptr_rw_info->cb_device_event(ARM_USBD_EVENT_RESUME);
            }
            break;

        case kUSB_DeviceEventLPMResume:
            ptr_rw_info->usbd_state.active = 1U;
            if (ptr_rw_info->cb_device_event != NULL)
            {
                ptr_rw_info->cb_device_event(ARM_USBD_EVENT_RESUME);
            }
            break;

        case kUSB_DeviceEventDetach:
            ptr_rw_info->usbd_state.active = 0U;
            ptr_rw_info->usbd_state.vbus   = 0U;
            if (ptr_rw_info->cb_device_event != NULL)
            {
                ptr_rw_info->cb_device_event(ARM_USBD_EVENT_VBUS_OFF);
            }
            break;

        case kUSB_DeviceEventAttach:
            ptr_rw_info->usbd_state.vbus = 1U;
            if (ptr_rw_info->cb_device_event != NULL)
            {
                ptr_rw_info->cb_device_event(ARM_USBD_EVENT_VBUS_ON);
            }
            break;

        default:
            break;
    }

    return kStatus_USB_Success;
}

/* SDK USB endpoint transfer-complete callback */
static usb_status_t USB_DeviceEndpointCb(
    usb_device_handle                              handle,
    usb_device_endpoint_callback_message_struct_t *message,
    void                                          *callbackParam)
{
    RW_Info_t *ptr_rw_info;
    EP_Info_t *ptr_ep;
    uint8_t    ep_addr = (uint8_t)((uint32_t)callbackParam & 0xFFU);

    if (handle == usbd1_rw_info.deviceHandle)
    {
        ptr_rw_info = &usbd1_rw_info;
    }
    else
    {
        return kStatus_USB_Error;
    }

    ptr_ep = &ptr_rw_info->ep_info[EP_NUM(ep_addr)][EP_DIR(ep_addr)];

    if ((ep_addr == 0x00U) && (message->isSetup == 1U))
    {
        for (uint8_t i = 0U; i < 8U; i++)
        {
            ptr_rw_info->setup_packet[i] = message->buffer[i];
        }
        ptr_rw_info->setup_received = 1U;
        if (ptr_rw_info->cb_endpoint_event != NULL)
        {
            ptr_rw_info->cb_endpoint_event(0U, ARM_USBD_EVENT_SETUP);
        }
    }
    else
    {
        ptr_ep->num_transferred = message->length;

        if (EP_DIR(ep_addr) != 0U)
        {
            if (ptr_rw_info->cb_endpoint_event != NULL)
            {
                ptr_rw_info->cb_endpoint_event(ep_addr, ARM_USBD_EVENT_IN);
            }
        }
        else
        {
            if (ptr_rw_info->cb_endpoint_event != NULL)
            {
                ptr_rw_info->cb_endpoint_event(ep_addr, ARM_USBD_EVENT_OUT);
            }
        }
    }

    return kStatus_USB_Success;
}

/* Per-instance wrappers that bind each public function to the USBD1 instance */
static int32_t        USBD1_Initialize              (ARM_USBD_SignalDeviceEvent_t cb_device_event, ARM_USBD_SignalEndpointEvent_t cb_endpoint_event) { return USBDn_Initialize              (&usbd1_ro_info, cb_device_event, cb_endpoint_event); }
static int32_t        USBD1_Uninitialize             (void)                                                                                          { return USBDn_Uninitialize             (&usbd1_ro_info); }
static int32_t        USBD1_PowerControl             (ARM_POWER_STATE state)                                                                         { return USBDn_PowerControl             (&usbd1_ro_info, state); }
static int32_t        USBD1_DeviceConnect            (void)                                                                                          { return USBDn_DeviceConnect            (&usbd1_ro_info); }
static int32_t        USBD1_DeviceDisconnect         (void)                                                                                          { return USBDn_DeviceDisconnect         (&usbd1_ro_info); }
static ARM_USBD_STATE USBD1_DeviceGetState           (void)                                                                                          { return USBDn_DeviceGetState           (&usbd1_ro_info); }
static int32_t        USBD1_DeviceRemoteWakeup       (void)                                                                                          { return USBDn_DeviceRemoteWakeup       (&usbd1_ro_info); }
static int32_t        USBD1_DeviceSetAddress         (uint8_t dev_addr)                                                                              { return USBDn_DeviceSetAddress         (&usbd1_ro_info, dev_addr); }
static int32_t        USBD1_ReadSetupPacket          (uint8_t *setup)                                                                                { return USBDn_ReadSetupPacket          (&usbd1_ro_info, setup); }
static int32_t        USBD1_EndpointConfigure        (uint8_t ep_addr, uint8_t ep_type, uint16_t ep_max_packet_size)                                 { return USBDn_EndpointConfigure        (&usbd1_ro_info, ep_addr, ep_type, ep_max_packet_size); }
static int32_t        USBD1_EndpointUnconfigure      (uint8_t ep_addr)                                                                               { return USBDn_EndpointUnconfigure      (&usbd1_ro_info, ep_addr); }
static int32_t        USBD1_EndpointStall            (uint8_t ep_addr, bool stall)                                                                   { return USBDn_EndpointStall            (&usbd1_ro_info, ep_addr, stall); }
static int32_t        USBD1_EndpointTransfer         (uint8_t ep_addr, uint8_t *data, uint32_t num)                                                  { return USBDn_EndpointTransfer         (&usbd1_ro_info, ep_addr, data, num); }
static uint32_t       USBD1_EndpointTransferGetResult(uint8_t ep_addr)                                                                               { return USBDn_EndpointTransferGetResult(&usbd1_ro_info, ep_addr); }
static int32_t        USBD1_EndpointTransferAbort    (uint8_t ep_addr)                                                                               { return USBDn_EndpointTransferAbort    (&usbd1_ro_info, ep_addr); }
static uint16_t       USBD1_GetFrameNumber           (void)                                                                                          { return USBDn_GetFrameNumber           (&usbd1_ro_info); }

/* USB1 high-speed device interrupt service routine */
void USB1_HS_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(usbd1_rw_info.deviceHandle);
}

ARM_DRIVER_USBD Driver_USBD1 = {
    USBD_GetVersion,
    USBD_GetCapabilities,
    USBD1_Initialize,
    USBD1_Uninitialize,
    USBD1_PowerControl,
    USBD1_DeviceConnect,
    USBD1_DeviceDisconnect,
    USBD1_DeviceGetState,
    USBD1_DeviceRemoteWakeup,
    USBD1_DeviceSetAddress,
    USBD1_ReadSetupPacket,
    USBD1_EndpointConfigure,
    USBD1_EndpointUnconfigure,
    USBD1_EndpointStall,
    USBD1_EndpointTransfer,
    USBD1_EndpointTransferGetResult,
    USBD1_EndpointTransferAbort,
    USBD1_GetFrameNumber
};
