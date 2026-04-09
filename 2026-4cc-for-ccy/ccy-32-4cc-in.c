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

static inline bool ccy_in(const char *s, const char **ccy_list)
{
    [[gnu::aligned(4)]] char ccy[4] ;
    memcpy(ccy, s, sizeof(ccy)) ;
    while (*ccy_list) {      
        if (CCY_EQ(s, *ccy_list))
            return true;
        ccy_list++;
    }
    return false;
}

#define CCY_IN(ccy, ...) ({ \
    static const char *ccy_list[] = { __VA_ARGS__, NULL } ; \
    ccy_in(ccy, ccy_list) ; \
    })

#include "lookup_in.inc"