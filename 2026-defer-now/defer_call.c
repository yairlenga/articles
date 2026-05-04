// Malloc/calloc block, use "free"

#include "defer_call.h"
#include <stdlib.h>

// Call the defer action, if the variable still hold non-null address
// Reset to NULL after cleanup to avoid repeated calls.

typedef void (*call_ptr_1_fn)(void *ptr) ;

void defer_call_ptr_var_1(DeferCall *call)
{
    void **p_var = call->ptr ;
    if ( p_var ) {
        if ( *p_var ) {
            call_ptr_1_fn fn = (call_ptr_1_fn) call->fn ;
            fn(*p_var) ;
//            *p_var = NULL ;
        }
//        call->ptr = NULL ;
    }
}

