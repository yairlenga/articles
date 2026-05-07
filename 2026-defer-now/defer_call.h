#ifndef _DEFER_CALL_H_
#define _DEFER_CALL_H_

typedef void (*defer_call_fn) (void) ;
typedef struct defer_call {

    // Cleanup function
    defer_call_fn fn ;

    // Pointer to object, or pointer to object address (if is_addr is true)
    void *p_var ;

    // Extra argument passed to the cleanup function.
    void *cxt ;
    int mode ;

} DeferCall ;

typedef void (defer_cleanup_fn)(DeferCall *call) ;
defer_cleanup_fn defer_call_p, defer_call_i, defer_call_px, defer_call_ix, defer_call_pm, defer_call_im ;

#define DEFER_CALL_CONCAT(a, b) a ## b
#define DEFER_CALL_VAR(counter) DEFER_CALL_CONCAT(defer_call_, counter)
#define DEFER_CALL_VALIDATE(check) ((void) sizeof(check, 0))

#define DEFER_CALL(cleanup_fn, check, _fn, ...) \
    [[gnu::cleanup(cleanup_fn)]] DeferCall DEFER_CALL_VAR(__COUNTER__) = ( \
        DEFER_CALL_VALIDATE(check), \
        (DeferCall) { .fn = (defer_call_fn) _fn, __VA_ARGS__ } \
    )

// cleanup functions with memory address
#define DEFER_CALL_P(_fn, var) \
    DEFER_CALL(defer_call_p, _fn(var), _fn, .p_var = (void **) &var)

// Cleanup functions with integer handle
#define DEFER_CALL_I(_fn, var) \
    DEFER_CALL(defer_call_i, _fn(var), _fn, .p_var = &var)

// Cleanup functions with extra "mode" (integer) parameter
#define DEFER_CALL_PM(_fn, var, _mode) \
    DEFER_CALL(defer_call_pm, _fn(var, _mode), _fn, .p_var = (void **) &var, .mode = _mode)

#define DEFER_CALL_IM(_fn, var, _mode) \
    DEFER_CALL(defer_call_im, _fn(var, _mode), _fn, .p_var = &var, .mode = _mode)

// Cleanup functions with extra context parameter
#define DEFER_CALL_PX(_fn, var, _cxt) \
    DEFER_CALL(defer_call_px, _fn(var, _cxt), _fn, .p_var = (void **) &var, .cxt = _cxt)

#define DEFER_CALL_IX(_fn, var, _cxt) \
    DEFER_CALL(defer_call_ix, _fn(var, _cxt), _fn, .p_var = &var, .cxt = _cxt)

// Logging
typedef enum defer_log_level { DEFER_QUIET, DEFER_ERROR, DEFER_WARN, DEFER_DEBUG } DeferLogLevel ;
extern DeferLogLevel defer_log_level ;

// User defined destructor

#define DEFER_DESTROY(destroy_fn, p) \
    DEFER_CALL_P(destroy_fn, p)

#define DEFER_DESTROY_X(destroy_fn, p, cxt) \
    DEFER_CALL_PX(destroy_fn, p, cxt)

#define DEFER_DESTROY_M(destroy_fn, p, mode) \
    DEFER_CALL_PM(destroy_fn, p, mode)


// Malloc/calloc block, use "free"
#define DEFER_FREE(p) \
    DEFER_CALL_P(free, p)

#define DEFER_REMOVE(pathname) \
    DEFER_CALL_P(remove, pathname)

#define DEFER_PCLOSE(fp) \
    DEFER_CALL_P(pclose, fp)

// File Pointer, use fclose(fp)
void cleanup_fclose(void *fp_arg) ;
#define DEFER_FCLOSE(fp) \
    DEFER_CALL_P(cleanup_fclose, fp)

#define DEFER_CLOSEDIR(dirp) \
    DEFER_CALL_P(closedir, dirp)

// File handle, use close(fd)
extern void cleanup_fd_close(int fd) ;

#define DEFER_FD_CLOSE(fd) \
    DEFER_CALL_I(cleanup_fd_close, fd)

extern void cleanup_sock_shutdown(int fd, int how) ;

#define DEFER_SOCK_SHUTDOWN(fd, shutdown_how) \
    DEFER_CALL_IM(cleanup_sock_shutdown, fd, shutdown_how)

extern void cleanup_free_ptr_array(void *array, int *p_count) ;
#define DEFER_FREE_PTR_ARRAY(arr, v_count) \
    DEFER_CALL_PX(cleanup_free_ptr_array, arr, &v_count)

#define DEFER_MUTEX_UNLOCK(mutex) \
    DEFER_CALL_P(pthread_mutex_unlock, mutex)

#define DEFER_RWLOCK_UNLOCK(lock) \
    DEFER_CALL_P(pthread_rwlock_unlock, lock)

#endif
