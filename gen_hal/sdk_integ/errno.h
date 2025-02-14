/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ERRNO_H_
#define ZEPHYR_INCLUDE_ERRNO_H_

#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ENOENT
#define ENOENT 2
#endif

#ifndef EIO
#define EIO 5
#endif

#ifndef ENOMEM
#define ENOMEM 12
#endif

#ifndef EACCES
#define EACCES 13
#endif

#ifndef EFAULT
#define EFAULT 14
#endif

#ifndef ENODEV
#define ENODEV 19
#endif

#ifndef EINVAL
#define EINVAL 22
#endif

#ifndef ENOSYS
#define ENOSYS 88
#endif

#ifndef ENOBUFS
#define ENOBUFS 105
#endif

#ifndef ENOTSUP
#define ENOTSUP 134
#endif

#ifndef EBUSY
#define EBUSY 16
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_ERRNO_H_ */
