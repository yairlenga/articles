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

// #define CCY_EQ(x, ccy) (x[0] == ccy[0] && strcmp(x+1, &ccy[1]) == 0)
static inline bool CCY_EQ(const char *x, const char *ccy) {
    return x[0] == ccy[0] && strcmp(x+1, ccy+1) == 0 ;
}

typedef int64_t ccy_mask_t ;

static bool ccy_in(const char *s, const char **ccy_list)
{
    while (*ccy_list) {      
        if (CCY_EQ(s, *ccy_list))
            return true;
        ccy_list++;
    }
    return false;
}

static inline int ccy_hash(const char *s)
{
   return ((*(int *) s) % 1031) & (64-1) ;
//    return (s[0] ^ s[1] ^ s[2] ^ s[3]) & (64-1);

}

static inline ccy_mask_t ccy_mask(const char *ccy)
{
    return ((ccy_mask_t) 1) << ccy_hash(ccy) ;    
}

static ccy_mask_t ccylist_mask(const char **ccy_list) 
{
    ccy_mask_t mask = 0 ;
    while ( *ccy_list ) { 
        mask |= ccy_mask(*ccy_list) ;
        ccy_list++ ;
    } ;
    return mask ;
}

static inline bool ccy_in_mask(const char *ccy, ccy_mask_t mask)
{
    return ccy_mask(ccy) & mask ;
}

static inline bool ccy_in_fast(const char *ccy, ccy_mask_t *mask, const char **ccy_list)
{
    if ( __builtin_expect(!*mask, 0) ) *mask = ccylist_mask(ccy_list) ;
    return ccy_in_mask(ccy, *mask) && ccy_in(ccy, ccy_list) ;
}

#define CCY_IN(ccy, ...) ({ \
    static ccy_mask_t mask ; \
    static const char *ccy_list[] = { __VA_ARGS__, NULL } ; \
    ccy_in_fast(ccy, &mask, ccy_list) ; \
    })

#include "lookup_in.inc"