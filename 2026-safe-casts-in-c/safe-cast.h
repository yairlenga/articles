#ifndef _SAFE_CAST_H_
#define _SAFE_CAST_H_

/*
 * Copyright (c) 2026 Yair Lenga
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software to use, copy, modify, and distribute it, subject to the
 * following condition:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

// safe-cast.h: Safe Casting in C - A header-only library for safer casts in C,
// using compile-time checks to prevent common casting mistakes.

// This library provides macros for casting values and pointers with additional 
// checks to ensure that the casts are valid and do not lead to undefined behavior.
// It includes checks for pointer types, value types, and const qualifiers, making
// it easier to write safer C code.

// Source: https://github.com/yairlenga/articles/tree/main/2026-safe-casts-in-c

// Public API

#define CAST(T, v) ((T)(v))

#define CAST_VAL(T, v) (CAST_REQUIRE_VALUE(v), CAST(T,v))
#define CAST_PTR(T, p) (CAST_REQUIRE_PTR(p), CAST(T,p))
#define CAST_PTR1(T, p) (CAST_REQUIRE_PTR1(p), CAST(T,p))

#define CAST_UNCONST(T, p) (CAST_REQUIRE_CONST_TYPE_PTR(T, p), CAST(T,p))
#define CAST_UNCONST1(T, p) (CAST_REQUIRE_CONST_TYPE_PTR1(T, p), CAST(T,p))

#define UNCONST_PTR(p) (CAST_REQUIRE_CONST_PTR(p), (CAST_TYPEOF_UNQUAL(*(p))*) (p))
#define UNCONST_PTR1(p) (CAST_REQUIRE_CONST_PTR1(p), (CAST_TYPEOF_UNQUAL(*(p))*) (p))

// Internal implementation details

// Check that v is a value, not a pointer, to prevent accidentally using CAST_VAL when the argument is a pointer.
static inline void cast_require_value(double v) { (void) v; }
#define CAST_REQUIRE_VALUE(v) ((void)sizeof(cast_require_value(v)))

// Check that p is a pointer, to prevent accidentally using CAST_PTR when the argument is not a pointer.
static inline void cast_require_ptr(const void *p) { (void) p; }
#define CAST_REQUIRE_PTR(p)   ((void)sizeof(cast_require_ptr(p)))

// Enable features based on GCC, CLANG version

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
    #define CAST_TYPEOF_UNQUAL(V) typeof_unqual(V)
    #define CAST_TYPEOF(V) typeof(V)

#elif defined(__GNUC__) || defined(__clang__)

    #define CAST_TYPEOF(V) __typeof__(V)

    #if (defined(__GNUC__) && __GNUC__ >= 14) || (defined(__clang__) && __clang_major__ >= 19)
        #define CAST_TYPEOF_UNQUAL(V) __typeof_unqual__(V)
    #else
        #undef UNCONST_PTR
        #undef UNCONST_PTR1
    #endif

#else
    #error "Compiler does not support typeof or __typeof__"
#endif

// Compile-time assertion macro. Evaluates the condition at compile time and causes a compilation error if the condition is false.

#define CAST_ASSERT(cond, msg) \
    ((void)sizeof(char[ \
        (cond) ? 1 : -1 \
    ]))

// Check that p is pointing to a non-pointer type, to prevent accidentally using CAST_PTR1 when the argument is already a pointer.
// No need to verify that p is not a pointer, since the reference * will cause a compile error if p is not a pointer.

#if defined(__GNUC__) || defined(__clang__)
    #define CAST_VOID_CLASS_TYPE (__builtin_classify_type(NULL))
#endif

#ifdef CAST_VOID_CLASS_TYPE

#define CAST_IS_PTR1(p) \
    (__builtin_classify_type(p) == CAST_VOID_CLASS_TYPE && \
     __builtin_classify_type(*p) != CAST_VOID_CLASS_TYPE)

#define CAST_IS_CONST_PTR(p) \
    __builtin_types_compatible_p( \
        __typeof__(p), \
        __typeof__((const __typeof__(*(p)) *)0))

#define CAST_IS_CONST_TYPE(T, p) \
    __builtin_types_compatible_p( \
        __typeof__(p), \
        __typeof__((const T )0))

#else
    // Without GCC, we can only check that p is a pointer,
    // but not that it points to a non-pointer type.
#define CAST_IS_PTR1(p) CAST_IS_PTR(p)

    // Without GCC, we can not reliably check if a pointer is const-qualified,
    // so we just check if it's a pointer.
#define CAST_IS_CONST_PTR(p) CAST_IS_PTR(p)

#endif

#define CAST_REQUIRE_PTR1(p) CAST_ASSERT( \
    CAST_IS_PTR1(p), \
    "Argument must be a pointer to non-pointer type")

#define CAST_REQUIRE_CONST_PTR(p) CAST_ASSERT( \
    CAST_IS_CONST_PTR(p), \
    "Argument must be a pointer to const type")

#define CAST_REQUIRE_CONST_PTR1(p) CAST_ASSERT( \
    CAST_IS_PTR1(p) && CAST_IS_CONST_PTR(p), \
    "Argument must be a pointer to const type")

        // Check that p type of const T *, to prevent accidentally using CAST_UNCONST when
        // the argument is not a pointer to const type, without using typeof_unqual which
        // is only available in C23 or newer GCC/CLANG versions.

#define CAST_REQUIRE_CONST_TYPE_PTR(T, p) CAST_ASSERT( \
    CAST_IS_CONST_TYPE(T, p), \
    "Argument must be a pointer to const type")

#define CAST_REQUIRE_CONST_TYPE_PTR1(T, p) CAST_ASSERT( \
    CAST_IS_CONST_TYPE(T, p) && CAST_IS_PTR1(p), \
    "Argument must be a pointer to const type")

#endif