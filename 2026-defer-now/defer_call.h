#ifndef _DEFER_CALL_H_
#define _DEFER_CALL_H_

// Default: include all modules (STDIO, STDLIB, ...)
#ifndef DEFER_USE_ALL
#define DEFER_USE_ALL 1
#endif

typedef void (*defer_call_fn) (void) ;
typedef struct defer_call {

    // Cleanup function
    defer_call_fn fn ;

    // Pointer to object, or pointer to object address (if is_addr is true)
    union {
        void **p_ref ;
        void *p_val ;
        int *i_ref ;
        int i_val ; 
    } ;

    // Extra argument passed to the cleanup function.
    void *cxt ;

} DeferCall ;

void defer_call_p_ref(DeferCall *call) ;
void defer_call_p_val(DeferCall *call) ;
void defer_call_i_ref(DeferCall *call) ;
void defer_call_i_val(DeferCall *call) ;

void defer_call_px_ref(DeferCall *call) ;
void defer_call_px_val(DeferCall *call) ;
void defer_call_ix_ref(DeferCall *call) ;
void defer_call_ix_val(DeferCall *call) ;

#define DEFER_CALL_CONCAT(a, b) a ## b
#define DEFER_CALL_VAR(counter) DEFER_CALL_CONCAT(defer_call_, counter)
#define DEFER_CALL_VALIDATE(check) ((void) sizeof(check, 0))

#define DEFER_CALL(cleanup_fn, check, _fn, ...) \
    [[gnu::cleanup(cleanup_fn)]] DeferCall DEFER_CALL_VAR(__COUNTER__) = ( \
        DEFER_CALL_VALIDATE(check), \
        (DeferCall) { .fn = (defer_call_fn) _fn, __VA_ARGS__ } \
    )

// cleanup functions with memory address
#define DEFER_CALL_P_ATX(_fn, var) \
    DEFER_CALL(defer_call_p_ref, _fn(var), _fn, .p_ref = (void **) &var)

#define DEFER_CALL_P_NOW(_fn, ptr) \
    DEFER_CALL(defer_call_p_val, _fn(ptr), _fn, .p_val = ptr)

// Cleanup functions with integer handle
#define DEFER_CALL_I_ATX(_fn, var) \
    DEFER_CALL(defer_call_i_ref, _fn(var), _fn, .i_ref = &var)

#define DEFER_CALL_I_NOW(_fn, val) \
    DEFER_CALL(defer_call_i_val, _fn(val), _fn, .i_val = val)


// Cleanup functions with extra context parameter
#define DEFER_CALL_PX_ATX(_fn, var, _cxt) \
    DEFER_CALL(defer_call_px_ref, _fn(var, _cxt), _fn, .p_ref = (void **) &var, .cxt = _cxt)

#define DEFER_CALL_PX_NOW(_fn, ptr, _cxt) \
    DEFER_CALL(defer_call_px_val, _fn(var, _cxt), _fn, .p_val = ptr, .cxt = _cxt)

#define DEFER_CALL_IX_ATX(_fn, var, _cxt) \
    DEFER_CALL(defer_call_ix_ref, _fn(var, _cxt), _fn, .i_ref = &var, .cxt = _cxt)

#define DEFER_CALL_IX_NOW(_fn, val, _cxt) \
    DEFER_CALL(defer_call_ix_val, _fn(val, _cxt), _fn, .i_val = val, .cxt = _cxt)

// User defined destrutor

#define DEFER_DESTROY(destroy_fn, p) \
    DEFER_CALL_P_ATX(destroy_fn, p)

#define DEFER_DESTROY_V(destroy_fn, p) \
    DEFER_CALL_P_NOW(destroy_fn, p)

// Malloc/calloc block, use "free"
#if defined(DEFER_USE_STDLIB) ? DEFER_USE_STDLIB : DEFER_USE_ALL
#include <stdlib.h>

#define DEFER_FREE(p) \
    DEFER_CALL_P_ATX(free, p)

#define DEFER_FREE_V(p) \
    DEFER_CALL_P_NOW(free(p), free, p)
#endif

// File Pointer, use fclose(fp)
#if defined(DEFER_USE_STDIO) ? DEFER_USE_STDIO : DEFER_USE_ALL
#include <stdio.h>

extern void cleanup_fclose(FILE *fp) ;

#define DEFER_FCLOSE(fp) \
    DEFER_CALL_P_ATX(cleanup_fclose, fp)

#define DEFER_FCLOSE_V(fp) \
    DEFER_CALL_P_NOW(cleanup_fclose, fp)

#endif

    // File Handle, use close(fd)
#if defined(DEFER_USE_UNISTD) ? DEFER_USE_UNISTD : DEFER_USE_ALL

extern void cleanup_fd_close(int fd) ;

#define DEFER_FD_CLOSE(fd) \
    DEFER_CALL_I_ATX(cleanup_fd_close, fd)

#define DEFER_FD_CLOSE_V(fd) \
    DEFER_CALL_I_NOW(cleanup_fd_close, fd)

// Socket handle, use shutdown(fd, 2), close(fd)
extern void cleanup_sock_close(int fd, int *how) ;

#define DEFER_SOCK_SHUTDOWN(fd, shutdown_how) \
    DEFER_CALL_IX_ATX(cleanup_sock_close, fd, (int [1]) { shutdown_how } )

#define DEFER_SOCK_SHUTDOWN_V(fd, shutdown_how) \
    DEFER_CALL_I_NOW(cleanup_sock_close, fd, (int [1]) { shutdown_how })

#endif

#endif

