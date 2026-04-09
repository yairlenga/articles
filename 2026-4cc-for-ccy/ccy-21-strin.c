/*
 * currency_adjustments.c
 *
 * Example: large strcmp-based dispatch with date-dependent logic
 * (baseline version for optimization discussion)
 */

#include "ccy-lookup.h"

#include <string.h>
#include <stdbool.h>

// #define CCY_EQ(x, ccy) (strcmp(x, ccy)==0)
static inline bool CCY_EQ(const char *s, const char *ccy)
{
    return strcmp(s, ccy) == 0 ;
}

static inline bool ccy_in(const char *s, const char **ccy_list)
{
    while (*ccy_list) {      
        if (CCY_EQ(s, *ccy_list))
            return true;
        ccy_list++;
    }
    return false;
}

#define CCY_IN(ccy, ...) ccy_in(ccy, (const char *[]) { __VA_ARGS__, NULL })

#include "lookup_in.inc"