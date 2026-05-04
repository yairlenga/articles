#ifndef _DEFER_JUMP_H_
#define _DEFER_JUMP_H_

    // Callback function take no parameters.
typedef void (*defer_body_fn)(void) ;

static inline void defer_jump(defer_body_fn *p)
{
    defer_body_fn fn = *p ;
    fn() ;
}

#define DEFER_CONCAT(a, b) a ## b

// Defering a lambda with indirect calls

#define DEFER_BUILD_JUMP(id, ...) \
    void DEFER_CONCAT(defer_fn, id)(void) { __VA_ARGS__; } \
    [[gnu::cleanup(defer_jump)]] \
    defer_body_fn DEFER_CONCAT(defer_var, id) = DEFER_CONCAT(defer_fn, id)

#define DEFER_INVOKE(...) \
    DEFER_BUILD_JUMP(__COUNTER__, __VA_ARGS__)

#endif