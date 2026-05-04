#ifndef _DEFER_NOW_H_
#define _DEFER_NOW_H_

    // Callback function take no parameters.
typedef void (*defer_body_fn)(void) ;

static inline void defer_jump(defer_body_fn *p)
{
    defer_body_fn fn = *p ;
    fn() ;
}

#define DEFER_CONCAT(a, b) a ## b

// Defering a lambda with indirect calls

#define DEFER_BUILD_JUMP(id, body) \
    void DEFER_CONCAT(defer_fn, id)(void) { body; } \
    [[gnu::cleanup(defer_jump)]] \
    defer_body_fn DEFER_CONCAT(defer_var, id) = DEFER_CONCAT(defer_fn, id)

#define DEFER_INVOKE_JUMP(body) \
    DEFER_BUILD_JUMP(__COUNTER__, body)

// Defering a lambda, with direct call

#define DEFER_BUILD_CLOSURE(id, body) \
    void DEFER_CONCAT(defer_fn, id)(int *p) { body; } \
    [[gnu::cleanup(DEFER_CONCAT(defer_fn, id))]] \
    int DEFER_CONCAT(defer_var, id) = id

#define DEFER_INVOKE_CLOSURE(body) \
    DEFER_BUILD_CLOSURE(__COUNTER__, body)

// Defering a lambda, indicating which variable will be exposes
  
#define DEFER_BUILD_LAMBDA(id, body, var) \
    void DEFER_CONCAT(defer_fn, id)(typeof(var) *p_var ) { __typeof__(*p_var) var = *p_var ; body; } \
    [[gnu::cleanup(DEFER_CONCAT(defer_fn, id))]] \
    __typeof__(var) DEFER_CONCAT(defer_var, id) = var

#define DEFER_INVOKE_LAMBDA(var, body) \
    DEFER_BUILD_LAMBDA(__COUNTER__, body)

#endif