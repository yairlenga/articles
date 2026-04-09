/*
 * currency_adjustments.c
 *
 * Example: large strcmp-based dispatch with date-dependent logic
 * (baseline version for optimization discussion)
 */

#include "ccy-lookup.h"

#include <string.h>

static inline bool CCY_EQ(const char *x, const char *ccy) 
{
    return x[0] == ccy[0] && strcmp(x+1, ccy+1) == 0 ;
}
// #define CCY_EQ(x, ccy) (x[0] == ccy[0] && strcmp(x+1, ccy+1) == 0)

#include "lookup_eq.inc"