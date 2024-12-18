#ifndef _UTIL_MCUX_HELPER_H
#define _UTIL_MCUX_HELPER_H

#include <errno.h>

#include "fsl_common.h"

typedef long off_t;
typedef uint32_t pinctrl_soc_pin_t;
typedef uint32_t ssize_t;

#define __aligned(x)

#define __ramfunc  __attribute__((section("RamFunction")))
#define __weak     __attribute__((__weak__))


#if (defined(__ICCARM__))
#define __CHAR_BIT__            __CHAR_BITS__
#define __SIZEOF_LONG__         4
#define __SIZEOF_LONG_LONG__    4
#endif 

#define ALWAYS_INLINE inline

#ifndef ENOTSUP
#define ENOTSUP 134
#endif

#ifndef EINVAL
#define EINVAL 22
#endif

#ifndef ENOSYS
#define ENOSYS  88
#endif

#ifndef EIO
#define EIO     5
#endif

#ifndef ENOBUFS
#define ENOBUFS 105
#endif

#ifndef ENOENT
#define ENOENT 2
#endif

#ifndef ENODEV
#define ENODEV  19
#endif

#ifndef EACCES
#define EACCES  13
#endif

#ifndef EFAULT
#define EFAULT  14
#endif

#ifndef ENOMEM
#define ENOMEM  12
#endif

#ifndef MSEC_PER_SEC
#define MSEC_PER_SEC    1000U
#endif

#ifndef NSEC_PER_USEC
#define NSEC_PER_USEC 1000U
#endif

#define __syscall static inline

#define LOG_MODULE_REGISTER(ignoreA, ignoreB)  

#define _DO_CONCAT(x, y) x ## y
#define _CONCAT(x, y) _DO_CONCAT(x, y)

#define __subsystem
#define ARG_UNUSED(x) (void)(x)

#define __packed
#define irq_lock()        0
#define irq_unlock(x)     (void)(x)

#define LOG_ERR(...) (void) 0
#define LOG_WRN(...) (void) 0
#define LOG_DBG(...) (void) 0
#define LOG_INF(...) (void) 0

#if  (defined(__ICCARM__))
#define Z_DECL_ALIGN(type) __attribute__((aligned((__ALIGNOF__(type))))) type
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define Z_DECL_ALIGN(type) __aligned(__alignof(type)) type
#elif defined(__GNUC__) || defined(DOXYGEN_OUTPUT)

#else 
#error Toolchain not supported
#endif

#define BUILD_ASSERT(EXPR, MSG...)

#define Z_STRINGIFY(x) #x

#define __deprecated 
#if (defined(__ICCARM__))
#define ___in_section(a, b, c) \
          __attribute__((section("."Z_STRINGIFY(a)"."Z_STRINGIFY(b)"."Z_STRINGIFY(c))))
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define ___in_section(a, b, c) \
	__attribute__((section("." Z_STRINGIFY(a)			\
				"." Z_STRINGIFY(b)			\
				"." Z_STRINGIFY(c))))
#elif defined(__GNUC__) || defined(DOXYGEN_OUTPUT)
          
#else
#error Toolchain not supported      
#endif
          
#define __in_section(a, b, c) ___in_section(a, b, c)

#define __in_section_unique(seg) ___in_section(seg, __FILE__, __COUNTER__)

#define __in_section_unique_named(seg, name) \
	___in_section(seg, __FILE__, name)


extern char _image_ram_start[];
extern char _image_ram_end[];
extern char _image_ram_size[];

#define PR_EXC(...)  

#endif /* _UTIL_MCUX_HELPER_H */