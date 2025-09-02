/*
 * Copyright (c) 2010-2014,2017 Wind River Systems, Inc.
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_TOOLCHAIN_IAR_H_
#define ZEPHYR_INCLUDE_TOOLCHAIN_IAR_H_

#ifndef ZEPHYR_INCLUDE_TOOLCHAIN_H_
#error Please do not include toolchain-specific headers directly, use <zephyr/toolchain.h> instead
#endif

/* This file is only tested with IAR 9.60.3 */

/**
 * @file
 * @brief IAR toolchain abstraction
 *
 * Macros to abstract compiler capabilities for IAR toolchain.
 */

#define TOOLCHAIN_HAS_PRAGMA_DIAG 1

#define TOOLCHAIN_HAS_C_GENERIC 1

#define TOOLCHAIN_HAS_C_AUTO_TYPE 1

#define TOOLCHAIN_HAS_ZLA 1

#define __CHAR_BIT__            __CHAR_BITS__
#define __SIZEOF_LONG__         4
#define __SIZEOF_LONG_LONG__    4

#ifndef _LINKER

#ifndef __ORDER_BIG_ENDIAN__
#define __ORDER_BIG_ENDIAN__            (1)
#endif

#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__         (2)
#endif

#ifndef __BYTE_ORDER__

#if defined(__LITTLE_ENDIAN__)

#if __LITTLE_ENDIAN__
#define __BYTE_ORDER__                  __ORDER_LITTLE_ENDIAN__
#else
#define __BYTE_ORDER__                  __ORDER_BIG_ENDIAN__
#endif

#else /* !defined(__LITTLE_ENDIAN__) */
#error "__BYTE_ORDER__ is not defined and cannot be automatically resolved"
#endif /* __LITTLE_ENDIAN__ */

#endif /* __BYTE_ORDER__ */


#undef BUILD_ASSERT /* clear out common version */
#define BUILD_ASSERT(EXPR, MSG...) static_assert(EXPR, "" MSG)

#ifdef __ICCARM_V8
#define ZRESTRICT __restrict
#else
#define ZRESTRICT restrict
#endif

#include <zephyr_headers/toolchain/common.h>
#include <stdbool.h>

#define ALIAS_OF(of) __attribute__((alias(#of)))

#define FUNC_ALIAS(real_func, new_alias, return_type) \
	return_type new_alias() ALIAS_OF(real_func)

#define CODE_UNREACHABLE __builtin_unreachable()
#define FUNC_NORETURN    __noreturn

#define _NODATA_SECTION(segment) __attribute__ ((section(#segment)))

/* Unaligned access */
#define UNALIGNED_GET(g)						\
__extension__ ({							\
	__packed struct  {				\
		__typeof__(*(g)) __v;					\
	} *__g = (__typeof__(__g)) (g);					\
	__g->__v;							\
})


#define UNALIGNED_PUT(v, p)                                             \
do {                                                                    \
	__packed struct {                            \
		__typeof__(*p) __v;                                     \
	} *__p = (__typeof__(__p)) (p);                                 \
	__p->__v = (v);                                                 \
} while (false)

/* Double indirection to ensure section names are expanded before
 * stringification
 */
#define __GENERIC_SECTION(segment) __attribute__((section(STRINGIFY(segment))))
#define Z_GENERIC_SECTION(segment) __GENERIC_SECTION(segment)

#define __GENERIC_DOT_SECTION(segment) \
	__attribute__((section("." STRINGIFY(segment))))
#define Z_GENERIC_DOT_SECTION(segment) __GENERIC_DOT_SECTION(segment)

#define ___in_section(a, b, c) \
	__attribute__((section("." Z_STRINGIFY(a)			\
				"." Z_STRINGIFY(b)			\
				"." Z_STRINGIFY(c))))
#define __in_section(a, b, c) ___in_section(a, b, c)

#define __in_section_unique(seg) ___in_section(seg, __FILE__, __COUNTER__)

#define __in_section_unique_named(seg, name) \
	___in_section(seg, __FILE__, name)

#ifdef __ZEPHYR__

/* When using XIP, using '__ramfunc' places a function into RAM instead
 * of FLASH. Make sure '__ramfunc' is defined only when
 * CONFIG_ARCH_HAS_RAMFUNC_SUPPORT is defined, so that the compiler can
 * report an error if '__ramfunc' is used but the architecture does not
 * support it.
 */
#if !defined(CONFIG_XIP)
#define __ramfunc
#elif defined(CONFIG_ARCH_HAS_RAMFUNC_SUPPORT)
#if defined(CONFIG_ARM)
#define __ramfunc	__attribute__((noinline))			\
			__attribute__((long_call, section(".ramfunc")))
#else
#define __ramfunc	__attribute__((noinline))			\
			__attribute__((section(".ramfunc")))
#endif
#endif /* !CONFIG_XIP */

#else /* !__ZEPHYR__ */

#define __ramfunc	__attribute__((noinline))			\
			__attribute__((section("RamFunction")))

#endif

#ifndef __fallthrough
#define __fallthrough
#endif

#ifndef __packed
#define __packed        __packed
#endif

#ifndef __aligned
#define __aligned(x)	__attribute__((aligned(x)))
#endif

#define __may_alias

#ifndef __printf_like
#define __printf_like(f, a)
#endif

#define __used		__attribute__((__used__))
#define __unused
#define __maybe_unused

#define __alignof __ALIGNOF__

#ifndef __deprecated
#define __deprecated	__attribute__((deprecated))
/* When adding this, remember to follow the instructions in
 * https://docs.zephyrproject.org/latest/develop/api/api_lifecycle.html#deprecated
 */
#endif

#ifndef __attribute_const__
#define __attribute_const__ __attribute__((__const__))
#endif

#ifndef __must_check
#define __must_check __attribute__((warn_unused_result))
#endif

#define ARG_UNUSED(x) (void)(x)

#define likely(x)   (__builtin_expect((bool)!!(x), true) != 0L)
#define unlikely(x) (__builtin_expect((bool)!!(x), false) != 0L)
#define POPCOUNT(x) __builtin_popcount(x)

#ifndef __no_optimization
/* TODO */
#define __no_optimization
#endif

#ifndef __weak
#define __weak __attribute__((__weak__))
#endif

#ifndef __attribute_nonnull
#define __attribute_nonnull(...)
#endif

/*
 * Be *very* careful with these. You cannot filter out __DEPRECATED_MACRO with
 * -wno-deprecated, which has implications for -Werror.
 */

/*
 * Expands to nothing and generates a warning. Used like
 *
 *   #define FOO __WARN("Please use BAR instead") ...
 *
 * The warning points to the location where the macro is expanded.
 */
#define __WARN(msg) __WARN1(IAR warning msg)
#define __WARN1(s) _Pragma(#s)

/* Generic message */
#ifndef __DEPRECATED_MACRO
#define __DEPRECATED_MACRO __WARN("Macro is deprecated")
/* When adding this, remember to follow the instructions in
 * https://docs.zephyrproject.org/latest/develop/api/api_lifecycle.html#deprecated
 */
#endif

/*
 * These macros generate absolute symbols for IAR
 */

/* create an extern reference to the absolute symbol */

#define GEN_OFFSET_EXTERN(name) extern const char name[]

#define GEN_ABS_SYM_BEGIN(name) \
	EXTERN_C void name(void); \
	void name(void)         \
	{

#define GEN_ABS_SYM_END }

#define compiler_barrier() do { \
	__asm volatile ("" ::: "memory"); \
} while (false)

/** @brief Return larger value of two provided expressions.
 *
 * Macro ensures that expressions are evaluated only once.
 *
 * @note Macro has limited usage compared to the standard macro as it cannot be
 *	 used:
 *	 - to generate constant integer, e.g. __aligned(Z_MAX(4,5))
 *	 - static variable, e.g. array like static uint8_t array[Z_MAX(...)];
 */
#define Z_MAX(a, b) ({ \
		/* random suffix to avoid naming conflict */ \
		__typeof__(a) _value_a_ = (a); \
		__typeof__(b) _value_b_ = (b); \
		(_value_a_ > _value_b_) ? _value_a_ : _value_b_; \
	})

/** @brief Return smaller value of two provided expressions.
 *
 * Macro ensures that expressions are evaluated only once. See @ref Z_MAX for
 * macro limitations.
 */
#define Z_MIN(a, b) ({ \
		/* random suffix to avoid naming conflict */ \
		__typeof__(a) _value_a_ = (a); \
		__typeof__(b) _value_b_ = (b); \
		(_value_a_ < _value_b_) ? _value_a_ : _value_b_; \
	})

/** @brief Return a value clamped to a given range.
 *
 * Macro ensures that expressions are evaluated only once. See @ref Z_MAX for
 * macro limitations.
 */
#define Z_CLAMP(val, low, high) ({                                             \
		/* random suffix to avoid naming conflict */                   \
		__typeof__(val) _value_val_ = (val);                           \
		__typeof__(low) _value_low_ = (low);                           \
		__typeof__(high) _value_high_ = (high);                        \
		(_value_val_ < _value_low_)  ? _value_low_ :                   \
		(_value_val_ > _value_high_) ? _value_high_ :                  \
					       _value_val_;                    \
	})

/**
 * @brief Calculate power of two ceiling for some nonzero value
 *
 * @param x Nonzero unsigned long value
 * @return X rounded up to the next power of two
 */
#define Z_POW2_CEIL(x) \
	((x) <= 2UL ? (x) : (1UL << (8 * sizeof(long) - __iar_builtin_CLZ((x) - 1))))

/**
 * @brief Check whether or not a value is a power of 2
 *
 * @param x The value to check
 * @return true if x is a power of 2, false otherwise
 */
#define Z_IS_POW2(x) (((x) != 0) && (((x) & ((x)-1)) == 0))

#define __noasan
#define __noubsan

#define FUNC_NO_STACK_PROTECTOR

#define __asm__ __asm          
          
#endif /* !_LINKER */
#endif /* ZEPHYR_INCLUDE_TOOLCHAIN_IAR_H_ */
