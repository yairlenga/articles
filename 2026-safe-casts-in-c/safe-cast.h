#ifndef _SAFE_CAST_H_
#define _SAFE_CAST_H_

#define CAST(T, v) ((T)(v))

#define CAST_VAL(T, v) (CAST_REQUIRE_VALUE(v), (T) (v))
#define CAST_PTR(T, v) (CAST_REQUIRE_PTR(v), (T) (v))
#define UNCONST_VAL(v) (CAST_REQUIRE_CONST(v), (CAST_TYPEOF_UNQUAL(v))(v))
#define UNCONST_PTR(v) (CAST_REQUIRE_CONST(*v), (CAST_TYPEOF_UNQUAL(*(v)) *)(v))

#define CAST_PTR1(T, v) (CAST_REQUIRE_PTR1(v), (T)(v))

static inline void cast_require_value(double v) { (void) v; }
#define CAST_REQUIRE_VALUE(v) ((void)sizeof(cast_require_value(v)))

static inline void cast_require_ptr(const void *v) { (void) v; }
#define CAST_REQUIRE_PTR(v)   ((void)sizeof(cast_require_ptr(v)))

#if defined(__GNUC__) || defined(__clang__)
#define CAST_TYPEOF_UNQUAL(V) __typeof_unqual__(V)
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

// Technically, this only checks that the pointed-to type is not const-qualified, but it's good enough for our purposes.
#define CAST_REQUIRE_CONST(v) CAST_ASSERT(!__builtin_types_compatible_p(__typeof__(v), __typeof__(const void *)))

#define CAST_REQUIRE_PTR1(v) ( \
    CAST_ASSERT(__builtin_types_compatible_p(__typeof__(v), __typeof__(void *))), \
    CAST_ASSERT(__builtin_types_compatible_p(__typeof__(v), __typeof__(const void *))) \
    )

#endif