/*
 * Copyright 2021, 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FSL_PM_CONFIG_H_
#define _FSL_PM_CONFIG_H_

#include "fsl_pm_device_config.h"

#ifndef FSL_PM_SUPPORT_NOTIFICATION
#define FSL_PM_SUPPORT_NOTIFICATION (0)
#endif /* FSL_PM_SUPPORT_NOTIFICATION */

#ifndef FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER
#define FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER (0)
#endif /* FSL_PM_SUPPORT_WAKEUP_SOURCE_MANAGER */

#ifndef FSL_PM_SUPPORT_LP_TIMER_CONTROLLER
#define FSL_PM_SUPPORT_LP_TIMER_CONTROLLER (0)
#endif /* FSL_PM_SUPPORT_LP_TIMER_CONTROLLER */

/*!
 * @brief If defined FSL_PM_SUPPORT_ALWAYS_ON_SECTION and set the macro to 1, then some critical
 * data of the power manager will be placed into the RAM section that is always powered on.
 * If this macro is not defined or this macro is set as 0, then upper software should use some
 * memory related constraint to guarantee some critical data of the system are retained in the selected power mode.
 */
#ifndef FSL_PM_SUPPORT_ALWAYS_ON_SECTION
#define FSL_PM_SUPPORT_ALWAYS_ON_SECTION (0)
#endif /* FSL_PM_SUPPORT_ALWAYS_ON_SECTION */

#ifndef PM_CONSTRAINT_COUNT
#define PM_CONSTRAINT_COUNT (0U)
#endif /* PM_CONSTRAINT_COUNT */

#ifndef PM_LP_STATE_COUNT
#define PM_LP_STATE_COUNT (0U)
#endif

#ifndef PM_RESC_GROUP_ARRAY_SIZE
#define PM_RESC_GROUP_ARRAY_SIZE ((PM_CONSTRAINT_COUNT) / 8U + 1U)
#endif /* PM_RESC_GROUP_ARRAY_SIZE */

#ifndef PM_RESC_MASK_ARRAY_SIZE
#define PM_RESC_MASK_ARRAY_SIZE ((PM_CONSTRAINT_COUNT) / 32U + 1U)
#endif /* PM_RESC_MASK_ARRAY_SIZE */

#define PM_RESOURCE_OFF          (0U)
#define PM_RESOURCE_PARTABLE_ON1 (1U)
#define PM_RESOURCE_PARTABLE_ON2 (2U)
#define PM_RESOURCE_FULL_ON      (4U)

/* The whole counter for constraint is allocated as 8-bits width. */
#define PM_PARTABLE_ON1_COUNTER_SIZE (3U)
#define PM_PARTABLE_ON1_COUNTER_MASK (0x7U)
#define PM_PARTABLE_ON2_COUNTER_SIZE (3U)
#define PM_PARTABLE_ON2_COUNTER_MASK (0x38U)
#define PM_FULL_ON_COUNTER_SIZE      (2U)
#define PM_FULL_ON_COUNTER_MASK      (0xC0U)

#ifndef PM_ENCODE_RESC
#define PM_ENCODE_RESC(opMode, rescShift) ((((uint32_t)(opMode)) << 8UL) | (uint32_t)(rescShift))
#endif /* PM_ENCODE_RESC */

#ifndef PM_DECODE_RESC
#define PM_DECODE_RESC(resConstraint)                         \
    opMode    = (uint32_t)(((uint32_t)resConstraint) >> 8UL); \
    rescShift = (uint32_t)(((uint32_t)resConstraint) & 0xFFUL)
#endif /* PM_DECODE_RESC */

/* Helper macros to define resource constraints with specific operation modes. */
#define PM_RESC_FULL_ON(rescId)   PM_ENCODE_RESC(PM_RESOURCE_FULL_ON, rescId)
#define PM_RESC_PART_ON1(rescId)  PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON1, rescId)
#define PM_RESC_PART_ON2(rescId)  PM_ENCODE_RESC(PM_RESOURCE_PARTABLE_ON2, rescId)
#define PM_RESC_OFF(rescId)  PM_ENCODE_RESC(PM_RESOURCE_OFF, rescId)

/* Helper macros for defining constraint masks for specific states. Note: These macros support up to 64 inputs only.  */
#define PM_DEFINE_RESC_MASK(...) \
    {.rescMask = { PM_DEFINE_MASK_ARRAY(__VA_ARGS__) }}

#define PM_DEFINE_MASK_ARRAY(...)    \
    PM_GENERATE_ARRAY_INIT(__VA_ARGS__)

#if (PM_RESC_MASK_ARRAY_SIZE == 1)
    #define PM_GENERATE_ARRAY_INIT(...) \
        [0] = PM_COLLECT_BITS_FOR_INDEX(0, __VA_ARGS__)

#elif (PM_RESC_MASK_ARRAY_SIZE == 2)
    #define PM_GENERATE_ARRAY_INIT(...) \
        [0] = PM_COLLECT_BITS_FOR_INDEX(0, __VA_ARGS__), \
        [1] = PM_COLLECT_BITS_FOR_INDEX(1, __VA_ARGS__)

#elif (PM_RESC_MASK_ARRAY_SIZE == 3)
    #define PM_GENERATE_ARRAY_INIT(...) \
        [0] = PM_COLLECT_BITS_FOR_INDEX(0, __VA_ARGS__), \
        [1] = PM_COLLECT_BITS_FOR_INDEX(1, __VA_ARGS__), \
        [2] = PM_COLLECT_BITS_FOR_INDEX(2, __VA_ARGS__)

#elif (PM_RESC_MASK_ARRAY_SIZE == 4)
    #define PM_GENERATE_ARRAY_INIT(...) \
        [0] = PM_COLLECT_BITS_FOR_INDEX(0, __VA_ARGS__), \
        [1] = PM_COLLECT_BITS_FOR_INDEX(1, __VA_ARGS__), \
        [2] = PM_COLLECT_BITS_FOR_INDEX(2, __VA_ARGS__), \
        [3] = PM_COLLECT_BITS_FOR_INDEX(3, __VA_ARGS__)
#else
    #error "The count of constraints exceed the capability of this macro "
#endif

#define PM_COLLECT_BITS_FOR_INDEX(idx, ...) \
    (0 PM_FOR_EACH_ARG(PM_ADD_BIT_IF_MATCH, idx, __VA_ARGS__))


#define PM_FOR_EACH_ARG(macro, idx, ...) \
    PM_FOR_EACH_IMPL(macro, idx, __VA_ARGS__, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, \
        PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END, PM_END,)

#define PM_FOR_EACH_IMPL(macro, idx, \
    a01,a02,a03,a04,a05,a06,a07,a08, \
    a09,a10,a11,a12,a13,a14,a15,a16, \
    a17,a18,a19,a20,a21,a22,a23,a24, \
    a25,a26,a27,a28,a29,a30,a31,a32, \
    a33,a34,a35,a36,a37,a38,a39,a40, \
    a41,a42,a43,a44,a45,a46,a47,a48, \
    a49,a50,a51,a52,a53,a54,a55,a56, \
    a57,a58,a59,a60,a61,a62,a63,a64, \
    a65,a66,a67,a68,a69,a70,a71,a72, \
    a73,a74,a75,a76,a77,a78,a79,a80, \
    a81,a82,a83,a84,a85,a86,a87,a88, \
    a89,a90,a91,a92,a93,a94,a95,a96, ...) \
    macro(idx, a01) macro(idx, a02) macro(idx, a03) macro(idx, a04) \
    macro(idx, a05) macro(idx, a06) macro(idx, a07) macro(idx, a08) \
    macro(idx, a09) macro(idx, a10) macro(idx, a11) macro(idx, a12) \
    macro(idx, a13) macro(idx, a14) macro(idx, a15) macro(idx, a16) \
    macro(idx, a17) macro(idx, a18) macro(idx, a19) macro(idx, a20) \
    macro(idx, a21) macro(idx, a22) macro(idx, a23) macro(idx, a24) \
    macro(idx, a25) macro(idx, a26) macro(idx, a27) macro(idx, a28) \
    macro(idx, a29) macro(idx, a30) macro(idx, a31) macro(idx, a32) \
    macro(idx, a33) macro(idx, a34) macro(idx, a35) macro(idx, a36) \
    macro(idx, a37) macro(idx, a38) macro(idx, a39) macro(idx, a40) \
    macro(idx, a41) macro(idx, a42) macro(idx, a43) macro(idx, a44) \
    macro(idx, a45) macro(idx, a46) macro(idx, a47) macro(idx, a48) \
    macro(idx, a49) macro(idx, a50) macro(idx, a51) macro(idx, a52) \
    macro(idx, a53) macro(idx, a54) macro(idx, a55) macro(idx, a56) \
    macro(idx, a57) macro(idx, a58) macro(idx, a59) macro(idx, a60) \
    macro(idx, a61) macro(idx, a62) macro(idx, a63) macro(idx, a64) \
    macro(idx, a65) macro(idx, a66) macro(idx, a67) macro(idx, a68) \
    macro(idx, a69) macro(idx, a70) macro(idx, a71) macro(idx, a72) \
    macro(idx, a73) macro(idx, a74) macro(idx, a75) macro(idx, a76) \
    macro(idx, a77) macro(idx, a78) macro(idx, a79) macro(idx, a80) \
    macro(idx, a81) macro(idx, a82) macro(idx, a83) macro(idx, a84) \
    macro(idx, a85) macro(idx, a86) macro(idx, a87) macro(idx, a88) \
    macro(idx, a89) macro(idx, a90) macro(idx, a91) macro(idx, a92) \
    macro(idx, a93) macro(idx, a94) macro(idx, a95) macro(idx, a96)

#define PM_ADD_BIT_IF_MATCH(idx, id) \
    PM_IF_NOT_END(id, | PM_BIT_IF_INDEX_MATCH(idx, id))

#define PM_CAT(a,b) PM_CAT_I(a,b)
#define PM_CAT_I(a,b) a##b
#define PM_PROBE() ~, 1
#define PM_SECOND(a, b, ...) b
#define PM_TEST_END_PM_END PM_PROBE()
//#define PM_TEST_END_      PM_PROBE()
#define PM_IS_PROBE(...)  PM_SECOND(__VA_ARGS__, 0)
#define PM_IS_END(x)      PM_IS_PROBE(PM_CAT(PM_TEST_END_, x))

#define PM_IF_NOT_END(id, expr)      PM_IF_NOT_END_I(PM_IS_END(id), expr)
#define PM_IF_NOT_END_I(cond, expr)  PM_IF_NOT_END_II(cond, expr)
#define PM_IF_NOT_END_II(cond, expr) PM_IF_NOT_END_##cond(expr)
#define PM_IF_NOT_END_0(expr) expr
#define PM_IF_NOT_END_1(expr)


#define PM_BIT_IF_INDEX_MATCH(idx, id) \
    (((uint32_t)(id) / 32U == (idx)) ? (1UL << ((uint32_t)(id) % 32U)) : 0UL)


#define PM_IS_CONS_SET(rescMasks, resId)    ((rescMasks->rescMask[(resId) / 32UL] & (1UL << ((resId) % 32UL))) != 0UL)

#define PM_FIND_CONS_OPERATE_MODE(resId, pSysRescGroup) \
    ({ \
        uint32_t _u32Tmp = ((pSysRescGroup)->groupSlice[(resId) / 8UL] >> (4UL * ((uint32_t)(resId) % 8UL))) & 0xFUL; \
        _u32Tmp |= (_u32Tmp >> 1UL); \
        _u32Tmp |= (_u32Tmp >> 2UL); \
        (uint8_t)(((_u32Tmp + 1UL) >> 1UL)); \
    })

#ifndef PM_ENCODE_WAKEUP_SOURCE_ID
#define PM_ENCODE_WAKEUP_SOURCE_ID(id) (id)
#endif /* PM_ENCODE_WAKEUP_SOURCE_ID */

#ifndef PM_DECODE_WAKEUP_SOURCE_ID
#define PM_DECODE_WAKEUP_SOURCE_ID(wsId) (wsId)
#endif /* PM_DECODE_WAKEUP_SOURCE_ID */

#endif /* _FSL_PM_CONFIG_H_ */
