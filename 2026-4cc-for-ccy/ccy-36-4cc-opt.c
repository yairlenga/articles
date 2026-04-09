/*
 * currency_adjustments.c
 *
 * Example: large strcmp-based dispatch with date-dependent logic
 * (baseline version for optimization discussion)
 */

#include "ccy-lookup.h"

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define CCY_EQ(x, ccy) (*(int *)x == *(int*) ccy )

#define CCY_EQ0(ccy, x) (x && CCY_EQ(ccy, x))

#define CCY_IN_8(ccy, x1, x2, x3, x4, x5, x6, x7, x8, x9, ...) \
    CCY_EQ0(ccy, x1) || CCY_EQ0(ccy, x2) || CCY_EQ0(ccy, x3) || CCY_EQ0(ccy, x4) || \
    CCY_EQ0(ccy, x5) || CCY_EQ0(ccy, x6) || CCY_EQ0(ccy, x7) || CCY_EQ0(ccy, x8)

#define CCY_IN(ccy, ...) CCY_IN_8(ccy, __VA_ARGS__, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)

/* =========================
 * Main function
 * ========================= */

 #include "lookup_in.inc"