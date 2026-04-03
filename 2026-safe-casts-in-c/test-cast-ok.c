#include "safe-cast.h"

#include <stdio.h>
#include <stdint.h>

#define ASSERT_TYPE(type, expr) static_assert(\
    __builtin_types_compatible_p(type, __typeof__(expr)), "Type mismatch: expected " #type ", got " #expr)

static void test_cast() {
    char *pc ;
    ASSERT_TYPE(int, CAST(int, *pc)) ;
    ASSERT_TYPE(int, CAST(int, 42)) ;
    ASSERT_TYPE(long, CAST(long, pc)) ;
    ASSERT_TYPE(long, CAST(long, &pc)) ;
    ASSERT_TYPE(intptr_t, CAST(intptr_t, pc)) ;
    ASSERT_TYPE(intptr_t, CAST(intptr_t, &pc)) ;
}

static void test_cast_val() {
    ASSERT_TYPE(int, CAST_VAL(int, (char) 'A')) ;
    ASSERT_TYPE(int, CAST_VAL(int, (unsigned char) 'A')) ;
    ASSERT_TYPE(int, CAST_VAL(int, 42)) ;
    ASSERT_TYPE(int, CAST_VAL(int, 42U)) ;
    ASSERT_TYPE(int, CAST_VAL(int, 42L));
    ASSERT_TYPE(int, CAST_VAL(int, 42LL));
    ASSERT_TYPE(int, CAST_VAL(int, 42ULL));
    ASSERT_TYPE(int, CAST_VAL(int, (short) 42));
    ASSERT_TYPE(int, CAST_VAL(int, 3.14f));
    ASSERT_TYPE(int, CAST_VAL(int, (double) 3.14));
    ASSERT_TYPE(int, CAST_VAL(int, (bool) true)) ;
}

void test_cast_ptr(void) {
    char *pc = NULL ;
    void *pv = NULL ;
    ASSERT_TYPE(int *, CAST_PTR(int *, pc)) ;
    ASSERT_TYPE(int *, CAST_PTR(int *, &pc)) ;
    ASSERT_TYPE(const int *, CAST_PTR(const int *, pc)) ;
    ASSERT_TYPE(const int *, CAST_PTR(const int *, &pc)) ;
    ASSERT_TYPE(int *, CAST_PTR(int *, pv)) ;
    ASSERT_TYPE(int *, CAST_PTR(int *, &pv)) ;
}

void test_cast_ptr1(void) {
    char *pc = NULL ;
    void *pv = NULL ;
    char x[10] ;

    ASSERT_TYPE(int *, CAST_PTR1(int *, pc)) ;
    ASSERT_TYPE(const int *, CAST_PTR1(const int *, pc)) ;

    ASSERT_TYPE(int *, CAST_PTR1(int *, pv)) ;
    ASSERT_TYPE(const int *, CAST_PTR1(const int *, pv)) ;
    ASSERT_TYPE(int *, CAST_PTR1(int *, x)) ;
    ASSERT_TYPE(const int *, CAST_PTR1(const int *, x)) ;
}

void test_unconst_ptr(void) {
    const char *pc = NULL ;
    const void *pv = NULL ;

    ASSERT_TYPE(char *, UNCONST_PTR(pc)) ;
    ASSERT_TYPE(void *, UNCONST_PTR(pv)) ;
}

void test_unconst_ptr1(void) {}

int main(int argc, char *argv[]) {
    (void) argc, (void) argv;
    test_cast() ;
    test_cast_val();
    test_cast_ptr() ;
    test_cast_ptr1() ;
    test_unconst_ptr() ;
    test_unconst_ptr1() ;
    return 0;
}   