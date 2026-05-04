#ifndef _DEFER_CLOSURE_H_
#define _DEFER_CLOSURE_H_

    // Callback function take no parameters.
typedef void (*defer_body_fn)(void) ;

#define DEFER_CONCAT(a, b) a ## b

#define DEFER_BUILD_CLOSURE(id, ...) \
    void DEFER_CONCAT(defer_fn, id)(int *p) { __VA_ARGS__; } \
    [[gnu::cleanup(DEFER_CONCAT(defer_fn, id))]] \
    int DEFER_CONCAT(defer_var, id) = id

#define DEFER_INVOKE(...) \
    DEFER_BUILD_CLOSURE(__COUNTER__, __VA_ARGS__)

#endif