#ifndef _SAFE_CAST_H_
#define _SAFE_CAST_H_

#define CAST(T, v) ((T)(v))

#define CAST_VAL(T, v) (CAST_REQUIRE_VALUE(v), (T) (v))
#define CAST_PTR(T, p) (CAST_REQUIRE_PTR(p), (T) (p))
#define CAST_PTR1(T, p) (CAST_REQUIRE_PTR1(p), (T)(p))
#define UNCONST_PTR(p) (CAST_REQUIRE_CONST_PTR(p), (CAST_TYPEOF_UNQUAL(*(p))*) (p))
#define UNCONST_PTR1(p) (CAST_REQUIRE_CONST_PTR1(p), (CAST_TYPEOF_UNQUAL(*(p)) *)(p))

#define CAST_GCC_CLASS_PTR (__builtin_classify_type(NULL))

static inline void cast_require_value(double v) { (void) v; }
#define CAST_REQUIRE_VALUE(v) ((void)sizeof(cast_require_value(v)))

static inline void cast_require_ptr(const void *v) { (void) v; }
#define CAST_REQUIRE_PTR(v)   ((void)sizeof(*v))

#if defined(__GNUC__) || defined(__clang__)
#define CAST_TYPEOF_UNQUAL(V) __typeof__(__typeof_unqual__(V))
#define CAST_TYPEOF(V) __typeof__(V)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define CAST_TYPEOF_UNQUAL(V) typeof_unqual(V)
#define CAST_TYPEOF(V) typeof(V)
#else
#define CAST_TYPEOF_UNQUAL(V) (V)
#endif

#define CAST_ASSERT(cond) \
    ((void)sizeof(char[ \
        (cond) ? 1 : -1 \
    ]))


// Check that p is pointing to a non-pointer type, to prevent accidentally using CAST_PTR1 when the argument is already a pointer.
// No need to verify that p is not a pointer, since the reference * will cause a compile error if p is not a pointer.

#define CAST_REQUIRE_PTR1(p) CAST_ASSERT( \
    __builtin_classify_type(p) == CAST_GCC_CLASS_PTR && \
    __builtin_classify_type(*p) != CAST_GCC_CLASS_PTR \
    )

#define CAST_IS_CONST_PTR(p) \
    __builtin_types_compatible_p( \
        __typeof__(p), \
        __typeof__((const __typeof__(*(p)) *)0))

#define CAST_REQUIRE_CONST_PTR(p) CAST_ASSERT( CAST_IS_CONST_PTR(p))

#define CAST_REQUIRE_CONST_PTR1(p) CAST_ASSERT( CAST_IS_PTR(p) && CAST_IS_CONST_PTR(*p))

#endif