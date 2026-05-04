#ifndef _DEFER_NOW_H_
#define _DEFER_NOW_H_

    // Callback function take no parameters.
typedef void (*defer_fn)(void) ;

static inline void defer_invoke(defer_fn *p)
{
    defer_fn fn = *p ;
    fn() ;
}

#define DEFER_CONCAT(a, b) a ## b

//    void defer_fn32(int *p) { free(x); } [[gnu::cleanup(defer_fn32)]] int  defer_var32 = 32 ;

#define DEFER_BUILD_CALL1(id, action) \
    void DEFER_CONCAT(defer_fn, id)(int *p) { action; } \
    [[gnu::cleanup(DEFER_CONCAT(defer_fn, id))]] \
    int DEFER_CONCAT(defer_var, id) = id

#define DEFER_INVOKE1(action) \
    DEFER_BUILD_CALL1(__COUNTER__, action)

#define DEFER_BUILD_CALL2(id, action) \
    void DEFER_CONCAT(defer_fn, id)(void) { action; } \
    [[gnu::cleanup(defer_invoke)]] \
    defer_fn DEFER_CONCAT(defer_var, id) = DEFER_CONCAT(defer_fn, id)

#define DEFER_INVOKE2(action) \
    DEFER_BUILD_CALL2(__COUNTER__, action)

#define DEFER_BUILD_CALL3(id, action, var) \
    void DEFER_CONCAT(defer_fn, id)(typeof(var) *p_var ) { __typeof__(*p_var) var = *p_var ; action; } \
    [[gnu::cleanup(DEFER_CONCAT(defer_fn, id))]] \
    __typeof__(var) DEFER_CONCAT(defer_var, id) = var

#define DEFER_INVOKE3(action, var) \
    DEFER_BUILD_CALL2(__COUNTER__, action)



#endif