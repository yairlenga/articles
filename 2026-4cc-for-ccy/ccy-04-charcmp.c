/*
 * currency_adjustments.c
 *
 * Example: large strcmp-based dispatch with date-dependent logic
 * (baseline version for optimization discussion)
 */
#include "ccy-lookup.h"

#include <string.h>

// #define CCY_EQ(x, ccy) (x[0] == ccy[0] && x[1] == ccy[1] && x[2] == ccy[2] && x[3] == ccy[3] )
static inline bool CCY_EQ(const char *s, const char *ccy)
{
   return s[0] == ccy[0] && s[1] == ccy[1] && s[2] == ccy[2] && s[3] == ccy[3] ;
}

#include "lookup_eq.inc"