#include "defer_call.h"

// Call the defer action, if the variable still hold non-null address
// Reset to NULL after cleanup to avoid repeated calls.


void defer_call_p_ref(DeferCall *call)
{
    typedef void (*call_p_ref_fn)(void *ptr) ;
    void **pp = call->p_ref ;
    if ( pp ) {
        call->p_ref = NULL ;
        if ( *pp ) {
            call_p_ref_fn fn = (call_p_ref_fn) call->fn ;
            fn(*pp) ;
            *pp = NULL ;
        }
    }
}

void defer_call_i_ref(DeferCall *call)
{
    typedef void (*i_ref_fn)(int ival) ;
    int *pp = call->i_ref ;
    if ( pp ) {
        call->i_ref = NULL ;
        i_ref_fn fn = (i_ref_fn) call->fn ;
        fn(*pp) ;
    }
}

void defer_call_ix_ref(DeferCall *call)
{
    typedef void (*ix_ref_fn)(int ival, void *cxt) ;
    int *pp = call->i_ref ;
    if ( pp ) {
        call->i_ref = NULL ;
        ix_ref_fn fn = (ix_ref_fn) call->fn ;
        fn(*pp, call->cxt) ;
    }
}

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

extern void cleanup_fclose(FILE *fp)
{
    if ( fp ) {
        if ( fclose(fp) != 0 ) {
            fprintf(stderr, "%s: fclose failed: %s(%d)", __func__, strerror(errno), errno) ;
        }
    }
}

extern void cleanup_fd_close(int fd)
{
    if ( fd >= -1 ) {
        if ( close(fd) != 0 ) {
            fprintf(stderr, "%s: close failed: %s(%d)\n", __func__, strerror(errno), errno) ;
        }
    }
}


#include <sys/socket.h>
extern void cleanup_sock_close(int fd, int *how)
{
    if ( fd >= -1 ) {
        if ( how >= 0 ) {
            if ( shutdown(fd, *how) ) {
                fprintf(stderr, "%s: close failed: %s(%d)\n", __func__, strerror(errno), errno) ;
            }
        }
        if ( close(fd) != 0 ) {
            fprintf(stderr, "%s: close failed: %s(%d)\n", __func__, strerror(errno), errno) ;
        }
    }
}
