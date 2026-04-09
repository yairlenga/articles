/*
 * currency_adjustments.c
 *
 * Example: large strcmp-based dispatch with date-dependent logic
 * (baseline version for optimization discussion)
 */

#include "ccy-lookup.h"
#include <stdbool.h>
#include <string.h>

static inline bool CCY_EQ(const char *s, const char *ccy)
{
    return strcmp(s, ccy) == 0 ;
}
// #define CCY_EQ(x, ccy) (strcmp(x, ccy) == 0)

#include "lookup_eq.inc"
