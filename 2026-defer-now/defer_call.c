#include "defer_call.h"

#include <stdlib.h>

DeferLogLevel defer_log_level = DEFER_ERROR;

// Call the defer action, if the variable still hold non-null address

static bool get_ptr(DeferCall *call, void **out_ptr)
{
    void **pp = call->p_var ;
    if ( !pp ) return false ;
    if ( !*pp ) return false ;
    *out_ptr = *pp ;
    if ( call->reset ) {
        *pp = NULL ;
    }    
    return true ;
}

void defer_call_p(DeferCall *call)
{
    void *p ;
    if ( !get_ptr(call, &p) ) return ;
    typedef void (*cleanup)(void *ptr) ;
    cleanup fn = (cleanup) call->fn ;
    fn(p) ;
}

void defer_call_px(DeferCall *call)
{
    void *p ;
    if ( !get_ptr(call, &p) ) return ;
    typedef void (*cleanup)(void *ptr, void *cxt) ;
    cleanup fn = (cleanup) call->fn ;
    fn(p, call->cxt) ;
}

void defer_call_pm(DeferCall *call)
{
    void *p ;
    if ( !get_ptr(call, &p) ) return ;
    typedef void (*cleanup)(void *ptr, int mode) ;
    cleanup fn = (cleanup) call->fn ;
    fn(p, call->mode) ;
}

static bool get_handle(DeferCall *call, int *out_handle)
{
    int *p_var = call->p_var ;
    if ( !p_var ) return false ;
    int handle = *p_var ;
    if ( call->valid == DEFER_VALID_POS && handle <= 0 ) return false ;
    if ( call->valid == DEFER_VALID_NON_NEG && handle < 0 ) return false ;
    if ( call->reset ) {
        *p_var = call->valid == DEFER_VALID_NON_NEG ? -1 : 0 ;
    }
    *out_handle = handle ;
    return true ;
}

void defer_call_i(DeferCall *call)
{
    int handle ;
    if ( !get_handle(call, &handle)) return ;
    typedef void (*cleanup)(int ival) ;
    cleanup fn = (cleanup) call->fn ;
    fn(handle) ;
}

void defer_call_ix(DeferCall *call)
{
    int handle ;
    if ( !get_handle(call, &handle)) return ;
    typedef void (*cleanup)(int ival, void *cxt) ;
    cleanup fn = (cleanup) call->fn ;
    fn(handle, call->cxt) ;
}

void defer_call_im(DeferCall *call)
{
    int handle ;
    if ( !get_handle(call, &handle)) return ;
    typedef void (*cleanup)(int ival, int mode) ;
    cleanup fn = (cleanup) call->fn ;
    fn(handle, call->mode) ;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

void cleanup_fclose(void *fp_arg)
{
    FILE *fp = fp_arg ;
    if ( fp ) {
        if ( fclose(fp) != 0 ) {
            if ( defer_log_level >= DEFER_ERROR )
                fprintf(stderr, "%s: fclose failed: %s(%d)\n", __func__, strerror(errno), errno) ;
        }
    }
}

void cleanup_fd_close(int fd)
{
    if ( fd >= 0 ) {
        if ( close(fd) != 0 ) {
            if ( defer_log_level >= DEFER_ERROR )
                fprintf(stderr, "%s: close(fd=%d) failed: %s(%d)\n", __func__, fd, strerror(errno), errno) ;
        }
    }
}


#include <sys/socket.h>
void cleanup_sock_shutdown(int fd, int how)
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
