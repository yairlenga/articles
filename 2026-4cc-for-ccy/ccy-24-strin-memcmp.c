/*
 * currency_adjustments.c
 *
 * Example: large strcmp-based dispatch with date-dependent logic
 * (baseline version for optimization discussion)
 */

#include "ccy-lookup.h"

#include <string.h>
#include <stdbool.h>

// #define CCY_EQ(x, ccy) (x[0] == ccy[0] && strcmp(x+1, &(const char *) ccy[1])) == 0)
static inline bool CCY_EQ(const char *s, const char *ccy) {
    return memcmp(s, ccy, 4) == 0;
}

//#define CCY_IN(ccy, ...) ccy_in(ccy, (const char *[]) { __VA_ARGS__, NULL })
static inline bool ccy_in(const char *s, const char **ccy_list)
{
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