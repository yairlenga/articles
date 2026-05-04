#ifndef _DEFER_NOW_H_
#define _DEFER_NOW_H_

typedef void (*defer_fn) (void *data, void *cxt) ;
typedef struct defer_call {

    // Cleanup function
    defer_fn fn ;

    // Pointer to object, or pointer to object address (if is_addr is true)
    union {
        void *ptr ;
        int ival ; 
    } ;

    // Extra argument passed to the cleanup function.
    void *cxt ;

} DeferCall ;

void defer_call_ptr_var_1(DeferCall *call) ;
void defer_call_ptr_val_1(DeferCall *call) ;
void defer_call_int_var_1(DeferCall *call) ;
void defer_call_int_val_1(DeferCall *call) ;

void defer_call_ptr_var_2(DeferCall *call) ;
void defer_call_ptr_val_2(DeferCall *call) ;
void defer_call_int_var_2(DeferCall *call) ;
void defer_call_int_val_2(DeferCall *call) ;

#define DEFER_ACTION_CONCAT(a, b) a ## b
#define DEFER_ACTION_VAR(counter) DEFER_ACTION_CONCAT(defer_call_, counter)
#define DEFER_ACTION(cleanup_fn, check, ...) \
    [[gnu::cleanup(cleanup_fn)]] DeferCall DEFER_ACTION_VAR(__COUNTER__) = \
        ((void) sizeof(check), (DeferCall) { __VA_ARGS__ })

// cleanup functions with memory address
#define DEFER_CALL_PTR_VAR_1(_fn, var) \
    DEFER_ACTION(defer_call_ptr_var_1, _fn(var), .fn = (defer_fn) _fn, .ptr = &var)

#define DEFER_CALL_PTR_VAL_1(_fn, ptr) \
    DEFER_ACTION(defer_call_ptr_val_1, _fn(ptr), .fn = (defer_fn) _fn, .ptr = ptr)

// Cleanup functions with integer handle
#define DEFER_CALL_INT_VAR_1(_fn, var) \
    DEFER_ACTION(defer_call_int_var_1, _fn(var), .fn = (defer_fn) _fn, .ptr = &var)

#define DEFER_CALL_INT_VAL_1(_fn, val) \
    DEFER_ACTION(defer_call_int_val_1, _fn(var), .fn = (defer_fn) fn, .ival = val)

// Cleanup functions with extra context parameter
#define DEFER_CALL_PTR_VAR_2(_fn, var, cxt) \
    DEFER_ACTION(defer_call_ptr_var_2, _fn(var, cxt), (defer_fn) .fn = _fn, .ptr = &var, .cxt = cxt)

#define DEFER_CALL_PTR_VAL_2(_fn, ptr, cxt) \
    DEFER_ACTION(defer_call_ptr_val_2, _fn(var, cxt), (defer_fn) .fn = _fn, .ptr = var, .cxt = cxt)

#define DEFER_CALL_INT_VAR_2(_fn, var, cxt) \
    DEFER_ACTION(defer_call_int_var_2, _fn(var, cxt), (defer_fn) .fn = _fn, .ptr = &var, .cxt = cxt)

#define DEFER_CALL_INT_VAL_2(_fn, val, cxt) \
    DEFER_ACTION(defer_call_int_val_2, _fn(var, cxt), (defer_fn) .fn = _fn, .ival = val, .cxt = cxt)

// Malloc/calloc block, use "free"
#include <stdlib.h>

#define DEFER_FREE(p) \
    DEFER_CALL_PTR_VAR_1(free, p)

#define DEFER_FREE_V(p) \
    DEFER_CALL_PTR_VAL_1(free(p), free, p)

// File Pointer, use fclose(fp)
#include <stdio.h>

#define DEFER_FCLOSE(fp) \
    DEFER_CALL_PTR_VAR_1(fclose, fp)

#define DEFER_FCLOSE_V(fp) \
    DEFER_CALL_PTR_VAL_1(fclose, fp)

// User defined destrutor

#define DEFER_DESTROY(destroy_fn, p) \
    DEFER_CALL_PTR_VAR_1(destroy_fn, p) ;

#define DEFER_DESTROY_V(destroy_fn, p) \
    DEFER_CALL_PTR_VAL_1(destroy_fn, p) ;

    // File Handle, use close(fd)

#define DEFER_FDCLOSE(fd) \
    DEFER_CALL_INT_var_1(close(fd), close, fd)

#define DEFER_FDCLOSE_V(fd) \
    DEFER_CALL_INT_VAL_1(close(fd), cleanup_fd_close, fd)

// Socket handle, use shutdown(fd, 2), close(fd)
extern void cleanup_socket_close(void *data, void *unused) ;

#define DEFER_SOCK_CLOSE(fd) \
    DEFER_CALL_INT_var_1(close(fd), cleanup_socket_close, fd)

#define DEFER_SOCK_CLOSE_V(fd) \
    DEFER_CALL_INT_var_1(close(fd), cleanup_socket_close, fd)


#endif

