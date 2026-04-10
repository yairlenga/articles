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
#include "ccy-lookup.h"

#define CCY_EQ(x, ccy) (*(int *)x == *(int*) ccy )
typedef int64_t ccy_mask_t ;



static bool ccy_in(const char *s, const char **ccy_list)
{
    [[gnu::aligned(4)]] char ccy[4] ;
    memcpy(ccy, s, sizeof(ccy)) ;
 
    while (*ccy_list) {      
        if (CCY_EQ(ccy, *ccy_list))
            return true;
        ccy_list++;
    }
    return false;
}

static inline int ccy_hash(const char *s)
{
//    return (s[0] ^ s[1] ^ s[2] ^ s[3]) & (64-1);
   return ((*(int *) s) % 1031) & (64-1) ;

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