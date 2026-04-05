#include "safe-cast.h"

#include <stdio.h>
#include <stdint.h>

#define ASSERT_TYPE(type, expr) static_assert(\
    __builtin_types_compatible_p(type, __typeof__(expr)), "Type mismatch: expected " #type ", got " #expr)

static void test_cast() {
    char *pc ;

    (void) CAST(int, pc) ;
    (void) CAST(int, NULL) ;
    (void) CAST(char *, 42) ;
    (void) CAST(int *, (int **) NULL) ;
}

static void test_cast_val() {
    struct foo { int a, b, c ; } bar ;
    CAST_VAL(int, NULL) ;
    CAST_VAL(void *, "") ; ;
    CAST_VAL(void *, NULL) ;
    CAST_VAL(struct foo *, (struct foo *) NULL) ;
    CAST_VAL(struct foo, bar) ;
    CAST_VAL(int **, (int **) NULL) ;
    CAST_VAL(intptr_t, NULL) ;

    char *pc = NULL ;
    void *pv = NULL ;
    CAST_VAL(int *, pc) ;
    CAST_VAL(int *, &pc) ;
    CAST_VAL(const int *, pc) ;
    CAST_VAL(const int *, &pc) ;
    CAST_VAL(int *, pv) ;
    CAST_VAL(int *, &pv) ;

}

void test_cast_ptr(void) {
    CAST_PTR(int, (char) 'A') ;
    CAST_PTR(int, (unsigned char) 'A') ;
    CAST_PTR(int, 42) ;
    CAST_PTR(int, 42U) ;
    CAST_PTR(int, 42L);
    CAST_PTR(int, 42LL);
    CAST_PTR(int, 42ULL);
    CAST_PTR(int, (short) 42);
    CAST_PTR(int, 3.14f);
    CAST_PTR(int, (double) 3.14);
    CAST_PTR(int, (bool) true) ;
}

void test_cast_ptr1(void) {
    char *pc = NULL ;
    char x[10] ;

    CAST_PTR1(void *, &pc) ;  // OK
    CAST_PTR1(void *, &x) ;   // OK

    CAST_PTR1(void *, &pc) ;  // FAIL
    CAST_PTR1(void *, &x) ;   // FAIL

}

void test_unconst_ptr(void) {
#ifdef UNCONST_PTR
    char *pc = NULL ;
    void *pv = NULL ;
    struct foo *c_foo ;

    UNCONST_PTR( (const char *) pc) ;       // OK
    UNCONST_PTR( (const char *) pv) ;       // OK
    UNCONST_PTR( (const char *) c_foo) ;    // OK

    UNCONST_PTR(pc) ;        // FAIL
    UNCONST_PTR(pv) ;        // FAIL
    UNCONST_PTR(c_foo) ;     // FAIL
#endif
}

void test_unconst_ptr1(void) {
#ifdef UNCONST_PTR
    char *pc = NULL, **pc2 ;

    struct foo { int x, y, z ; } ;
    struct foo *p_foo = NULL ;
    const struct foo **pc_foo2 = NULL ;
    struct foo **p_foo2 = NULL ;

    UNCONST_PTR1((const char *)pc) ;
    UNCONST_PTR1((const struct foo *) p_foo) ;

    UNCONST_PTR1(pc) ;
    UNCONST_PTR1(pc2) ;
    UNCONST_PTR1(p_foo) ;
    UNCONST_PTR1(p_foo2) ;
    UNCONST_PTR1(p_foo2) ;

    const char **pp2 = NULL ;
    char const **pp3 = NULL ;
    char * const *pp4 = NULL ;
    char ** const pp5 = NULL ;
    UNCONST_PTR1(pp2) ;
    UNCONST_PTR1(pp3) ;
    UNCONST_PTR1(pp4) ;
    UNCONST_PTR1(pp5) ;
#endif
}

int main(int argc, char *argv[]) {
    (void) argc, (void) argv;
    test_cast() ;
    test_cast_val();
    test_cast_ptr() ;
    test_cast_ptr1() ;
    test_cast_cptr() ;
    test_unconst_cptr1() ;
    test_unconst_ptr() ;
    test_unconst_ptr1() ;
    return 0;
}   