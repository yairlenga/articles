/*
 * currency_adjustments.c
 *
 * Example: large strcmp-based dispatch with date-dependent logic
 * (baseline version for optimization discussion)
 */

#include "ccy-lookup.h"

#include <string.h>
#include <stdint.h>

#define CCY_EQ(x, ccy) (*(int *)x == *(int*) ccy )

#include "lookup_eq.inc"
