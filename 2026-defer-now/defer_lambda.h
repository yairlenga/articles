#ifndef _DEFER_NOW_H_
#define _DEFER_NOW_H_

#define DEFER_CONCAT(a, b) a ## b

// Defering a lambda with indirect calls
  
#define DEFER_BUILD_LAMBDA(id, var, arg, ...) \
    void DEFER_CONCAT(defer_fn, id)(typeof(var) *p_var ) { __typeof__(*p_var) arg = *p_var ; __VA_ARGS__ ; } \
    [[gnu::cleanup(DEFER_CONCAT(defer_fn, id))]] \
    __typeof__(var) DEFER_CONCAT(defer_var, id) = var

#define DEFER_LAMBDA(var, ...) \
    DEFER_BUILD_LAMBDA(__COUNTER__, var, var, __VA_ARGS__)

#define DEFER_LAMBDA_X(var, arg, ...) \
    DEFER_BUILD_LAMBDA(__COUNTER__, var, arg, __VA_ARGS__)

#define DEFER_LAMBDA_ARG(var, ...) \
    DEFER_BUILD_LAMBDA(__COUNTER__, var, arg, __VA_ARGS__)

#endif