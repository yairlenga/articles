#include "defer_call.h"

DeferLogLevel defer_log_level = DEFER_ERROR;

// Call the defer action, if the variable still hold non-null address

void defer_call_p(DeferCall *call)
{
    typedef void (*cleanup)(void *ptr) ;
    void **pp = call->p_var ;
    if ( pp && *pp ) {
        cleanup fn = (cleanup) call->fn ;
        fn(*pp) ;
    }
}

void defer_call_px(DeferCall *call)
{
    typedef void (*cleanup)(void *ptr, void *cxt) ;
    void **pp = call->p_var ;
    if ( pp && *pp ) {
        cleanup fn = (cleanup) call->fn ;
        fn(*pp, call->cxt) ;
    }
}

void defer_call_i(DeferCall *call)
{
    typedef void (*cleanup)(int ival) ;
    int *pp = call->p_var ;
    if ( pp ) {
        cleanup fn = (cleanup) call->fn ;
        fn(*pp) ;
    }
}

void defer_call_ix(DeferCall *call)
{
    typedef void (*cleanup)(int ival, void *cxt) ;
    int *pp = call->p_var ;
    if ( pp ) {
        cleanup fn = (cleanup) call->fn ;
        fn(*pp, call->cxt) ;
    }
}

void defer_call_pm(DeferCall *call)
{
    typedef void (*cleanup)(void *ptr, int mode) ;
    void **pp = call->p_var ;
    if ( pp && *pp ) {
        cleanup fn = (cleanup) call->fn ;
        fn(*pp, call->mode) ;
    }
}

void defer_call_im(DeferCall *call)
{
    typedef void (*cleanup)(int ival, int mode) ;
    int *pp = call->p_var ;
    if ( pp ) {
        cleanup fn = (cleanup) call->fn ;
        fn(*pp, call->mode) ;
    }
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

extern void cleanup_fclose(void *fp_arg)
{
    FILE *fp = fp_arg ;
    if ( fp ) {
        if ( fclose(fp) != 0 ) {
            if ( defer_log_level >= DEFER_ERROR )
                fprintf(stderr, "%s: fclose failed: %s(%d)\n", __func__, strerror(errno), errno) ;
        }
    }
}

extern void cleanup_fd_close(int fd)
{
    if ( fd >= 0 ) {
        if ( close(fd) != 0 ) {
            if ( defer_log_level >= DEFER_ERROR )
                fprintf(stderr, "%s: close(fd=%d) failed: %s(%d)\n", __func__, fd, strerror(errno), errno) ;
        }
    }
}


#include <sys/socket.h>
extern void cleanup_sock_shutdown(int fd, int how)
{
    if ( fd >= 0 ) {
        if ( shutdown(fd, how) != 0 ) {
            if ( defer_log_level >= DEFER_ERROR )
                fprintf(stderr, "%s: shutdown(fd=%d, how=%d) failed: %s(%d)\n", __func__,
                    fd, how,
                    strerror(errno), errno) ;
        }
    }
}

void cleanup_free_ptr_array(void *array_arg, int *p_count)
{
    void **arr = array_arg ;
    int count = *p_count ;
    for (int i=0 ; i<count ; i++ ) {
        if (arr[i] ) {
            free(arr[i]) ;
            arr[i] = NULL ;
        }
    }
}
