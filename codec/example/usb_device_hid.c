/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#if ((defined(USB_DEVICE_CONFIG_HID)) && (USB_DEVICE_CONFIG_HID > 0U))
#include "usb_device_hid.h"

#endif
