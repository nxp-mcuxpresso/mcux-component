/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FTP12557.h"

#define __SLCD_D_COM  (1U << 0U)
#define __SLCD_C_COM  (1U << 1U)
#define __SLCD_B_COM  (1U << 2U)
#define __SLCD_A_COM  (1U << 3U)
#define __SLCD_E_COM  (1U << 1U)
#define __SLCD_G_COM  (1U << 2U)
#define __SLCD_F_COM  (1U << 3U)

/* 12 S icons in total, apart from 2/3/4/5/6/7, other icons are the same. */
#define __SLCD_S_COM   (1U << 0U)
#define __SLCD_S2_COM  (1U << 3U)
#define __SLCD_S3_COM  (1U << 2U)
#define __SLCD_S4_COM  (1U << 2U)
#define __SLCD_S5_COM  (1U << 3U)
#define __SLCD_S6_COM  (1U << 2U)
#define __SLCD_S7_COM  (1U << 1U)

/* 5 D icons in total, apart from 1/5, other icons are the same. */
#define __SLCD_D_COM   (1U << 0U)
#define __SLCD_D1_COM  (1U << 1U)
#define __SLCD_D5_COM  (1U << 1U)

/* 5 T icons in total. */
#define __SLCD_T1_COM  (1U << 0U)
#define __SLCD_T2_COM  (1U << 1U)
#define __SLCD_T3_COM  (1U << 2U)
#define __SLCD_T4_COM  (1U << 3U)
#define __SLCD_T5_COM  (1U << 3U)

#define __SLCD_PIN5  1U
#define __SLCD_PIN6  2U
#define __SLCD_PIN7  3U
#define __SLCD_PIN8  4U
#define __SLCD_PIN9  5U
#define __SLCD_PIN10 6U
#define __SLCD_PIN11 7U
#define __SLCD_PIN12 8U
#define __SLCD_PIN13 9U
#define __SLCD_PIN14 10U
#define __SLCD_PIN15 11U
#define __SLCD_PIN16 12U
#define __SLCD_PIN17 13U
#define __SLCD_PIN18 14U
#define __SLCD_PIN19 15U
#define __SLCD_PIN20 16U

/* POS1 uses PIN5/6, POS2 uses PIN7/8, POS3 uses PIN10/11, POS4 uses PIN12/13,
 * POS5 uses PIN16/17, POS6 uses PIN18/19
 */
#define SLCD_PIN_NUM(x) (((x)>4)?(((x)*2)+2):(((x)<3)?(((x)*2)-1):((x)*2)))

/* Macros to define SLCD pin value of number 0-9. x can be from 1 to 6 represent 6 positions on panel. */
#define SLCD_POS_NUM0(x)                                                                                 \
static const uint16_t SLCD_Pos##x##_Num0[] = {                                                           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1U, __SLCD_A_COM | __SLCD_B_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM),                                   \
    0x0,                                                                                                 \
};

#define SLCD_POS_NUM1(x)                                                  \
static const uint16_t SLCD_Pos##x##_Num1[] = {                            \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_B_COM | __SLCD_C_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), 0U),                             \
    0x0,                                                                  \
};

#define SLCD_POS_NUM2(x)                                                                 \
static const uint16_t SLCD_Pos##x##_Num2[] = {                                           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_G_COM),                   \
    0x0,                                                                                 \
};

#define SLCD_POS_NUM3(x)                                                                                \
static const uint16_t SLCD_Pos##x##_Num3[] = {                                                          \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_G_COM),                                                 \
    0x0,                                                                                                \
};

#define SLCD_POS_NUM4(x)                                                  \
static const uint16_t SLCD_Pos##x##_Num4[] = {                            \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_B_COM | __SLCD_C_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_F_COM | __SLCD_G_COM),    \
    0x0,                                                                  \
};

#define SLCD_POS_NUM5(x)                                                                 \
static const uint16_t SLCD_Pos##x##_Num5[] = {                                           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_F_COM | __SLCD_G_COM),                   \
    0x0,                                                                                 \
};

#define SLCD_POS_NUM6(x)                                                                 \
static const uint16_t SLCD_Pos##x##_Num6[] = {                                           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM | __SLCD_G_COM),    \
    0x0,                                                                                 \
};

#define SLCD_POS_NUM7(x)                                                                 \
static const uint16_t SLCD_Pos##x##_Num7[] = {                                           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM | __SLCD_C_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), 0U),                                            \
    0x0,                                                                                 \
};

#define SLCD_POS_NUM8(x)                                                                                \
static const uint16_t SLCD_Pos##x##_Num8[] = {                                                          \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM | __SLCD_G_COM),                   \
    0x0,                                                                                                \
};

#define SLCD_POS_NUM9(x)                                                                                \
static const uint16_t SLCD_Pos##x##_Num9[] = {                                                          \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_F_COM | __SLCD_G_COM),                                  \
    0x0,                                                                                                \
};

SLCD_POS_NUM0(1)
SLCD_POS_NUM1(1)
SLCD_POS_NUM2(1)
SLCD_POS_NUM3(1)
SLCD_POS_NUM4(1)
SLCD_POS_NUM5(1)
SLCD_POS_NUM6(1)
SLCD_POS_NUM7(1)
SLCD_POS_NUM8(1)
SLCD_POS_NUM9(1)

SLCD_POS_NUM0(2)
SLCD_POS_NUM1(2)
SLCD_POS_NUM2(2)
SLCD_POS_NUM3(2)
SLCD_POS_NUM4(2)
SLCD_POS_NUM5(2)
SLCD_POS_NUM6(2)
SLCD_POS_NUM7(2)
SLCD_POS_NUM8(2)
SLCD_POS_NUM9(2)

SLCD_POS_NUM0(3)
SLCD_POS_NUM1(3)
SLCD_POS_NUM2(3)
SLCD_POS_NUM3(3)
SLCD_POS_NUM4(3)
SLCD_POS_NUM5(3)
SLCD_POS_NUM6(3)
SLCD_POS_NUM7(3)
SLCD_POS_NUM8(3)
SLCD_POS_NUM9(3)

SLCD_POS_NUM0(4)
SLCD_POS_NUM1(4)
SLCD_POS_NUM2(4)
SLCD_POS_NUM3(4)
SLCD_POS_NUM4(4)
SLCD_POS_NUM5(4)
SLCD_POS_NUM6(4)
SLCD_POS_NUM7(4)
SLCD_POS_NUM8(4)
SLCD_POS_NUM9(4)

SLCD_POS_NUM0(5)
SLCD_POS_NUM1(5)
SLCD_POS_NUM2(5)
SLCD_POS_NUM3(5)
SLCD_POS_NUM4(5)
SLCD_POS_NUM5(5)
SLCD_POS_NUM6(5)
SLCD_POS_NUM7(5)
SLCD_POS_NUM8(5)
SLCD_POS_NUM9(5)

SLCD_POS_NUM0(6)
SLCD_POS_NUM1(6)
SLCD_POS_NUM2(6)
SLCD_POS_NUM3(6)
SLCD_POS_NUM4(6)
SLCD_POS_NUM5(6)
SLCD_POS_NUM6(6)
SLCD_POS_NUM7(6)
SLCD_POS_NUM8(6)
SLCD_POS_NUM9(6)

/* Macro to define SLCD pin value array containing number 0-9. x can be from 1 to 6 represent 6 positions on panel. */
#define SLCD_POS_NUM_ARRAY(x)                                                                          \
static const uint16_t *SLCD_Pos##x##_Nums[] = {                                                        \
    SLCD_Pos##x##_Num0, SLCD_Pos##x##_Num1, SLCD_Pos##x##_Num2, SLCD_Pos##x##_Num3, SLCD_Pos##x##_Num4,\
    SLCD_Pos##x##_Num5, SLCD_Pos##x##_Num6, SLCD_Pos##x##_Num7, SLCD_Pos##x##_Num8, SLCD_Pos##x##_Num9,\
};

SLCD_POS_NUM_ARRAY(1)
SLCD_POS_NUM_ARRAY(2)
SLCD_POS_NUM_ARRAY(3)
SLCD_POS_NUM_ARRAY(4)
SLCD_POS_NUM_ARRAY(5)
SLCD_POS_NUM_ARRAY(6)

const uint16_t **SLCD_NumPos[] = {SLCD_Pos1_Nums, SLCD_Pos2_Nums, SLCD_Pos3_Nums, SLCD_Pos4_Nums, SLCD_Pos5_Nums, SLCD_Pos6_Nums};

/* Macros to define SLCD pin value of letter A-Z and a-z. Note that not all letters can be
 * displayed by this SLCD of 7 segements. x can be from 1 to 6 represent 6 positions on panel.
 */
#define SLCD_POS_LETTER_A(x)                                                             \
static const uint16_t SLCD_Pos##x##_Letter_A[] = {                                       \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM | __SLCD_C_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM | __SLCD_G_COM),    \
    0x0,                                                                                 \
};

#define SLCD_POS_LETTER_b(x)                                                         \
static const uint16_t SLCD_Pos##x##_Letter_b[] = {                                   \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_C_COM | __SLCD_D_COM),           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM | __SLCD_G_COM),\
    0x0,                                                                             \
};

#define SLCD_POS_LETTER_C(x)                                               \
static const uint16_t SLCD_Pos##x##_Letter_C[] = {                         \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1,  __SLCD_A_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM),     \
    0x0,                                                                   \
};

#define SLCD_POS_LETTER_c(x)                                          \
static const uint16_t SLCD_Pos##x##_Letter_c[] = {                    \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_D_COM),           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_G_COM),\
    0x0,                                                              \
};

#define SLCD_POS_LETTER_d(x)                                                             \
static const uint16_t SLCD_Pos##x##_Letter_d[] = {                                       \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_B_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_G_COM),                   \
    0x0,                                                                                 \
};

#define SLCD_POS_LETTER_E(x)                                                         \
static const uint16_t SLCD_Pos##x##_Letter_E[] = {                                   \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_D_COM),           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM | __SLCD_G_COM),\
    0x0,                                                                             \
};

#define SLCD_POS_LETTER_F(x)                                                         \
static const uint16_t SLCD_Pos##x##_Letter_F[] = {                                   \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM),                          \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM | __SLCD_G_COM),\
    0x0,                                                                             \
};

#define SLCD_POS_LETTER_g(x)                                                                            \
static const uint16_t SLCD_Pos##x##_Letter_g[] = {                                                      \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_F_COM | __SLCD_G_COM),                                  \
    0x0,                                                                                                \
};

#define SLCD_POS_LETTER_G(x)                                                             \
static const uint16_t SLCD_Pos##x##_Letter_G[] = {                                       \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM),                   \
    0x0,                                                                                 \
};

#define SLCD_POS_LETTER_H(x)                                                         \
static const uint16_t SLCD_Pos##x##_Letter_H[] = {                                   \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_B_COM | __SLCD_C_COM),           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM | __SLCD_G_COM),\
    0x0,                                                                             \
};

#define SLCD_POS_LETTER_h(x)                                                         \
static const uint16_t SLCD_Pos##x##_Letter_h[] = {                                   \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_C_COM),                          \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM | __SLCD_G_COM),\
    0x0,                                                                             \
};

#define SLCD_POS_LETTER_i(x)                                              \
static const uint16_t SLCD_Pos##x##_Letter_i[] = {                        \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_C_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), 0U),                             \
    0x0,                                                                  \
};

#define SLCD_POS_LETTER_I(x)                                              \
static const uint16_t SLCD_Pos##x##_Letter_I[] = {                        \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_B_COM | __SLCD_C_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), 0U),                             \
    0x0,                                                                  \
};

#define SLCD_POS_LETTER_j(x)                                                             \
static const uint16_t SLCD_Pos##x##_Letter_j[] = {                                       \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), 0U),                                            \
    0x0,                                                                                 \
};

#define SLCD_POS_LETTER_L(x)                                          \
static const uint16_t SLCD_Pos##x##_Letter_L[] = {                    \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_D_COM),           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM),\
    0x0,                                                              \
};

#define SLCD_POS_LETTER_l(x)                                          \
static const uint16_t SLCD_Pos##x##_Letter_l[] = {                    \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, 0U),                     \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM),\
    0x0,                                                              \
};

#define SLCD_POS_LETTER_n(x)                                          \
static const uint16_t SLCD_Pos##x##_Letter_n[] = {                    \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_C_COM),           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_G_COM),\
    0x0,                                                              \
};

#define SLCD_POS_LETTER_N(x)                                                             \
static const uint16_t SLCD_Pos##x##_Letter_N[] = {                                       \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM | __SLCD_C_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM),                   \
    0x0,                                                                                 \
};

#define SLCD_POS_LETTER_O(x)                                                                            \
static const uint16_t SLCD_Pos##x##_Letter_O[] = {                                                      \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM),                                  \
    0x0,                                                                                                \
};

#define SLCD_POS_LETTER_o(x)                                              \
static const uint16_t SLCD_Pos##x##_Letter_o[] = {                        \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_G_COM),    \
    0x0,                                                                  \
};

#define SLCD_POS_LETTER_p(x)                                                         \
static const uint16_t SLCD_Pos##x##_Letter_p[] = {                                   \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM),           \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM | __SLCD_G_COM),\
    0x0,                                                                             \
};

#define SLCD_POS_LETTER_q(x)                                                             \
static const uint16_t SLCD_Pos##x##_Letter_q[] = {                                       \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_B_COM | __SLCD_C_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_F_COM | __SLCD_G_COM),                   \
    0x0,                                                                                 \
};

#define SLCD_POS_LETTER_r(x)                                          \
static const uint16_t SLCD_Pos##x##_Letter_r[] = {                    \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, 0U),                     \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_G_COM),\
    0x0,                                                              \
};

#define SLCD_POS_LETTER_S(x)                                                             \
static const uint16_t SLCD_Pos##x##_Letter_S[] = {                                       \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_A_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_F_COM | __SLCD_G_COM),                   \
    0x0,                                                                                 \
};

#define SLCD_POS_LETTER_t(x)                                                         \
static const uint16_t SLCD_Pos##x##_Letter_t[] = {                                   \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_D_COM),                          \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM | __SLCD_G_COM),\
    0x0,                                                                             \
};

#define SLCD_POS_LETTER_U(x)                                                             \
static const uint16_t SLCD_Pos##x##_Letter_U[] = {                                       \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_B_COM | __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM | __SLCD_F_COM),                   \
    0x0,                                                                                 \
};

#define SLCD_POS_LETTER_u(x)                                              \
static const uint16_t SLCD_Pos##x##_Letter_u[] = {                        \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_C_COM | __SLCD_D_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_E_COM),                   \
    0x0,                                                                  \
};

#define SLCD_POS_LETTER_y(x)                                              \
static const uint16_t SLCD_Pos##x##_Letter_y[] = {                        \
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x) + 1, __SLCD_B_COM | __SLCD_C_COM),\
    SLCD_ENGINE_PIN_VAL(SLCD_PIN_NUM(x), __SLCD_F_COM | __SLCD_G_COM),    \
    0x0,                                                                  \
};

SLCD_POS_LETTER_A(1)
SLCD_POS_LETTER_b(1)
SLCD_POS_LETTER_C(1)
SLCD_POS_LETTER_c(1)
SLCD_POS_LETTER_d(1)
SLCD_POS_LETTER_E(1)
SLCD_POS_LETTER_F(1)
SLCD_POS_LETTER_g(1)
SLCD_POS_LETTER_G(1)
SLCD_POS_LETTER_H(1)
SLCD_POS_LETTER_h(1)
SLCD_POS_LETTER_i(1)
SLCD_POS_LETTER_I(1)
SLCD_POS_LETTER_j(1)
SLCD_POS_LETTER_L(1)
SLCD_POS_LETTER_l(1)
SLCD_POS_LETTER_n(1)
SLCD_POS_LETTER_N(1)
SLCD_POS_LETTER_O(1)
SLCD_POS_LETTER_o(1)
SLCD_POS_LETTER_p(1)
SLCD_POS_LETTER_q(1)
SLCD_POS_LETTER_r(1)
SLCD_POS_LETTER_S(1)
SLCD_POS_LETTER_t(1)
SLCD_POS_LETTER_U(1)
SLCD_POS_LETTER_u(1)
SLCD_POS_LETTER_y(1)

SLCD_POS_LETTER_A(2)
SLCD_POS_LETTER_b(2)
SLCD_POS_LETTER_C(2)
SLCD_POS_LETTER_c(2)
SLCD_POS_LETTER_d(2)
SLCD_POS_LETTER_E(2)
SLCD_POS_LETTER_F(2)
SLCD_POS_LETTER_g(2)
SLCD_POS_LETTER_G(2)
SLCD_POS_LETTER_H(2)
SLCD_POS_LETTER_h(2)
SLCD_POS_LETTER_i(2)
SLCD_POS_LETTER_I(2)
SLCD_POS_LETTER_j(2)
SLCD_POS_LETTER_L(2)
SLCD_POS_LETTER_l(2)
SLCD_POS_LETTER_n(2)
SLCD_POS_LETTER_N(2)
SLCD_POS_LETTER_O(2)
SLCD_POS_LETTER_o(2)
SLCD_POS_LETTER_p(2)
SLCD_POS_LETTER_q(2)
SLCD_POS_LETTER_r(2)
SLCD_POS_LETTER_S(2)
SLCD_POS_LETTER_t(2)
SLCD_POS_LETTER_U(2)
SLCD_POS_LETTER_u(2)
SLCD_POS_LETTER_y(2)

SLCD_POS_LETTER_A(3)
SLCD_POS_LETTER_b(3)
SLCD_POS_LETTER_C(3)
SLCD_POS_LETTER_c(3)
SLCD_POS_LETTER_d(3)
SLCD_POS_LETTER_E(3)
SLCD_POS_LETTER_F(3)
SLCD_POS_LETTER_g(3)
SLCD_POS_LETTER_G(3)
SLCD_POS_LETTER_H(3)
SLCD_POS_LETTER_h(3)
SLCD_POS_LETTER_i(3)
SLCD_POS_LETTER_I(3)
SLCD_POS_LETTER_j(3)
SLCD_POS_LETTER_L(3)
SLCD_POS_LETTER_l(3)
SLCD_POS_LETTER_n(3)
SLCD_POS_LETTER_N(3)
SLCD_POS_LETTER_O(3)
SLCD_POS_LETTER_o(3)
SLCD_POS_LETTER_p(3)
SLCD_POS_LETTER_q(3)
SLCD_POS_LETTER_r(3)
SLCD_POS_LETTER_S(3)
SLCD_POS_LETTER_t(3)
SLCD_POS_LETTER_U(3)
SLCD_POS_LETTER_u(3)
SLCD_POS_LETTER_y(3)

SLCD_POS_LETTER_A(4)
SLCD_POS_LETTER_b(4)
SLCD_POS_LETTER_C(4)
SLCD_POS_LETTER_c(4)
SLCD_POS_LETTER_d(4)
SLCD_POS_LETTER_E(4)
SLCD_POS_LETTER_F(4)
SLCD_POS_LETTER_g(4)
SLCD_POS_LETTER_G(4)
SLCD_POS_LETTER_H(4)
SLCD_POS_LETTER_h(4)
SLCD_POS_LETTER_i(4)
SLCD_POS_LETTER_I(4)
SLCD_POS_LETTER_j(4)
SLCD_POS_LETTER_L(4)
SLCD_POS_LETTER_l(4)
SLCD_POS_LETTER_n(4)
SLCD_POS_LETTER_N(4)
SLCD_POS_LETTER_O(4)
SLCD_POS_LETTER_o(4)
SLCD_POS_LETTER_p(4)
SLCD_POS_LETTER_q(4)
SLCD_POS_LETTER_r(4)
SLCD_POS_LETTER_S(4)
SLCD_POS_LETTER_t(4)
SLCD_POS_LETTER_U(4)
SLCD_POS_LETTER_u(4)
SLCD_POS_LETTER_y(4)

SLCD_POS_LETTER_A(5)
SLCD_POS_LETTER_b(5)
SLCD_POS_LETTER_C(5)
SLCD_POS_LETTER_c(5)
SLCD_POS_LETTER_d(5)
SLCD_POS_LETTER_E(5)
SLCD_POS_LETTER_F(5)
SLCD_POS_LETTER_g(5)
SLCD_POS_LETTER_G(5)
SLCD_POS_LETTER_H(5)
SLCD_POS_LETTER_h(5)
SLCD_POS_LETTER_i(5)
SLCD_POS_LETTER_I(5)
SLCD_POS_LETTER_j(5)
SLCD_POS_LETTER_L(5)
SLCD_POS_LETTER_l(5)
SLCD_POS_LETTER_n(5)
SLCD_POS_LETTER_N(5)
SLCD_POS_LETTER_O(5)
SLCD_POS_LETTER_o(5)
SLCD_POS_LETTER_p(5)
SLCD_POS_LETTER_q(5)
SLCD_POS_LETTER_r(5)
SLCD_POS_LETTER_S(5)
SLCD_POS_LETTER_t(5)
SLCD_POS_LETTER_U(5)
SLCD_POS_LETTER_u(5)
SLCD_POS_LETTER_y(5)

SLCD_POS_LETTER_A(6)
SLCD_POS_LETTER_b(6)
SLCD_POS_LETTER_C(6)
SLCD_POS_LETTER_c(6)
SLCD_POS_LETTER_d(6)
SLCD_POS_LETTER_E(6)
SLCD_POS_LETTER_F(6)
SLCD_POS_LETTER_g(6)
SLCD_POS_LETTER_G(6)
SLCD_POS_LETTER_H(6)
SLCD_POS_LETTER_h(6)
SLCD_POS_LETTER_i(6)
SLCD_POS_LETTER_I(6)
SLCD_POS_LETTER_j(6)
SLCD_POS_LETTER_L(6)
SLCD_POS_LETTER_l(6)
SLCD_POS_LETTER_n(6)
SLCD_POS_LETTER_N(6)
SLCD_POS_LETTER_O(6)
SLCD_POS_LETTER_o(6)
SLCD_POS_LETTER_p(6)
SLCD_POS_LETTER_q(6)
SLCD_POS_LETTER_r(6)
SLCD_POS_LETTER_S(6)
SLCD_POS_LETTER_t(6)
SLCD_POS_LETTER_U(6)
SLCD_POS_LETTER_u(6)
SLCD_POS_LETTER_y(6)

/* Macro to define SLCD pin value array containing uppercase letters. x can be from 1 to 6 represent 6 positions on panel. */
#define SLCD_POS_UPPER_LETTER_ARRAY(x)                                                                        \
static const uint16_t *SLCD_Pos##x##_Upper_Letters[] = {                                                      \
    SLCD_Pos##x##_Letter_A, NULL/*No B*/, SLCD_Pos##x##_Letter_C, NULL/*No D*/, SLCD_Pos##x##_Letter_E,       \
    SLCD_Pos##x##_Letter_F, SLCD_Pos##x##_Letter_G, SLCD_Pos##x##_Letter_H, SLCD_Pos##x##_Letter_I,           \
    NULL/*No J*/, NULL/*No K*/, SLCD_Pos##x##_Letter_L, NULL/*No M*/, SLCD_Pos##x##_Letter_N,                 \
    SLCD_Pos##x##_Letter_O, NULL/*No P*/, NULL/*No Q*/, NULL/*No R*/, SLCD_Pos##x##_Letter_S,                 \
    NULL/*No T*/, SLCD_Pos##x##_Letter_U, NULL/*No V*/, NULL/*No W*/, NULL/*No X*/, NULL/*No Y*/, NULL/*No Z*/\
};

/* Macro to define SLCD pin value array containing lowercase letters. x can be from 1 to 6 represent 6 positions on panel. */
#define SLCD_POS_LOWER_LETTER_ARRAY(x)                                                                 \
static const uint16_t *SLCD_Pos##x##_Lower_Letters[] = {                                               \
    NULL/*No a*/, SLCD_Pos##x##_Letter_b, SLCD_Pos##x##_Letter_c, SLCD_Pos##x##_Letter_d, NULL/*No e*/,\
    NULL/*No f*/, SLCD_Pos##x##_Letter_g, SLCD_Pos##x##_Letter_h, SLCD_Pos##x##_Letter_i,              \
    SLCD_Pos##x##_Letter_j, NULL/*No k*/, SLCD_Pos##x##_Letter_l, NULL/*No m*/, SLCD_Pos##x##_Letter_n,\
    SLCD_Pos##x##_Letter_o, SLCD_Pos##x##_Letter_p, SLCD_Pos##x##_Letter_q, SLCD_Pos##x##_Letter_r,    \
    NULL/*No s*/, SLCD_Pos##x##_Letter_t, SLCD_Pos##x##_Letter_u, NULL/*No v*/, NULL/*No w*/,          \
    NULL/*No x*/, SLCD_Pos##x##_Letter_y, NULL/*No z*/                                                 \
};

SLCD_POS_UPPER_LETTER_ARRAY(1)
SLCD_POS_UPPER_LETTER_ARRAY(2)
SLCD_POS_UPPER_LETTER_ARRAY(3)
SLCD_POS_UPPER_LETTER_ARRAY(4)
SLCD_POS_UPPER_LETTER_ARRAY(5)
SLCD_POS_UPPER_LETTER_ARRAY(6)

SLCD_POS_LOWER_LETTER_ARRAY(1)
SLCD_POS_LOWER_LETTER_ARRAY(2)
SLCD_POS_LOWER_LETTER_ARRAY(3)
SLCD_POS_LOWER_LETTER_ARRAY(4)
SLCD_POS_LOWER_LETTER_ARRAY(5)
SLCD_POS_LOWER_LETTER_ARRAY(6)

const uint16_t **SLCD_Pos_Upper_Letters[] = {SLCD_Pos1_Upper_Letters, SLCD_Pos2_Upper_Letters,
                                             SLCD_Pos3_Upper_Letters, SLCD_Pos4_Upper_Letters,
                                             SLCD_Pos5_Upper_Letters, SLCD_Pos6_Upper_Letters,};

const uint16_t **SLCD_Pos_Lower_Letters[] = {SLCD_Pos1_Lower_Letters, SLCD_Pos2_Lower_Letters,
                                             SLCD_Pos3_Lower_Letters, SLCD_Pos4_Lower_Letters,
                                             SLCD_Pos5_Lower_Letters, SLCD_Pos6_Lower_Letters,};

const uint16_t SLCD_Icon[] = {
    /* Character part 1*/
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN6, __SLCD_A_COM), /* A */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN6, __SLCD_B_COM), /* B */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN6, __SLCD_C_COM), /* C */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN6, __SLCD_D_COM), /* D */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN5, __SLCD_E_COM), /* E */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN5, __SLCD_F_COM), /* F */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN5, __SLCD_G_COM), /* G */

    /* Character part 2*/
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN8, __SLCD_A_COM), /* A */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN8, __SLCD_B_COM), /* B */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN8, __SLCD_C_COM), /* C */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN8, __SLCD_D_COM), /* D */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN7, __SLCD_E_COM), /* E */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN7, __SLCD_F_COM), /* F */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN7, __SLCD_G_COM), /* G */

    /* Character part 3*/
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN11, __SLCD_A_COM), /* A */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN11, __SLCD_B_COM), /* B */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN11, __SLCD_C_COM), /* C */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN11, __SLCD_D_COM),  /* D */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN10, __SLCD_E_COM),  /* E */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN10, __SLCD_F_COM),  /* F */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN10, __SLCD_G_COM),  /* G */

    /* Character part 4*/
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN13, __SLCD_A_COM),  /* A */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN13, __SLCD_B_COM),  /* B */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN13, __SLCD_C_COM),  /* C */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN13, __SLCD_D_COM),  /* D */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN12, __SLCD_E_COM),  /* E */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN12, __SLCD_F_COM),  /* F */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN12, __SLCD_G_COM),  /* G */

    /* Character part 5*/
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN17, __SLCD_A_COM),  /* A */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN17, __SLCD_B_COM),  /* B */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN17, __SLCD_C_COM),  /* C */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN17, __SLCD_D_COM),  /* D */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN16, __SLCD_E_COM),  /* E */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN16, __SLCD_F_COM),  /* F */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN16, __SLCD_G_COM),  /* G */

    /* Character part 6*/
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN19, __SLCD_A_COM),  /* A */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN19, __SLCD_B_COM),  /* B */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN19, __SLCD_C_COM),  /* C */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN19, __SLCD_D_COM),  /* D */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN18, __SLCD_E_COM),  /* E */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN18, __SLCD_F_COM),  /* F */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN18, __SLCD_G_COM),  /* G */

    /* 12 S icons */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN5, __SLCD_S_COM),   /* S1 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN9, __SLCD_S2_COM),  /* S2 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN9, __SLCD_S3_COM),  /* S3 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN15, __SLCD_S4_COM), /* S4 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN20, __SLCD_S5_COM), /* S5 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN20, __SLCD_S6_COM), /* S6 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN20, __SLCD_S7_COM), /* S7 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN20, __SLCD_S_COM),  /* S8 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN18, __SLCD_S_COM),  /* S9 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN16, __SLCD_S_COM),  /* S10 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN10, __SLCD_S_COM),  /* S11 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN7, __SLCD_S_COM),   /* S12 */

    /* 5 D icons */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN9, __SLCD_D1_COM),   /* D1 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN9, __SLCD_D_COM),    /* D2 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN12, __SLCD_D_COM),   /* D3 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN15, __SLCD_D_COM),   /* D4 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN15, __SLCD_D5_COM),  /* D5 */

    /* 5 T icons */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN14, __SLCD_T1_COM),   /* T1 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN14, __SLCD_T2_COM),    /* T2 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN14, __SLCD_T3_COM),   /* T3 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN14, __SLCD_T4_COM),   /* T4 */
    SLCD_ENGINE_PIN_VAL(__SLCD_PIN15, __SLCD_T5_COM),  /* T5 */
};
