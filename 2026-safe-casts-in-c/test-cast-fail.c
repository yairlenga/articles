#include "safe-cast.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define ASSERT_TYPE(type, expr) static_assert(\
    __builtin_types_compatible_p(type, __typeof__(expr)), "Type mismatch: expected " #type ", got " #expr)

static void test_cast() {
    char *pc ;
    struct foo { int a, b, c ; } bar ;

    (void) CAST(float, pc) ;      // FAIL: incompatible types
    (void) CAST(int, bar) ;       // FAIL: incompatible types
    (void) CAST(struct foo, pc) ; // FAIL: incompatible types
}

static void test_cast_val() {
    struct foo { int a, b, c ; } bar ;
    CAST_VAL(int, NULL) ;             // FAIL: NULL is pointer constant, not a value
    CAST_VAL(void *, "") ; ;          // FAIL: string literals are pointers, not a value
    CAST_VAL(void *, NULL) ;          // FAIL: NULL is pointer constant, not a value
    CAST_VAL(struct foo *, (struct foo *) NULL) ; // FAIL: NULL is pointer constant, not a value
    CAST_VAL(struct foo, bar) ;       // FAIL: struct foo is not a scalar value
    CAST_VAL(int **, (int **) NULL) ; // FAIL: NULL is pointer constant, not a value
    CAST_VAL(intptr_t, NULL) ;        // Fail: NULL is pointer constant, not a value

    char *pc = NULL ;
    void *pv = NULL ;
    CAST_VAL(int *, pc) ;            // FAIL: pc is a pointer, not a value
    CAST_VAL(int *, &pc) ;           // FAIL: &pc is a pointer, not a value
    CAST_VAL(const int *, pc) ;      // FAIL: pc is a pointer, not a value
    CAST_VAL(const int *, &pc) ;     // FAIL: &pc is a pointer, not a value
    CAST_VAL(int *, pv) ;            // FAIL: pv is a pointer, not a value
    CAST_VAL(int *, &pv) ;           // FAIL: &pv is a pointer, not a value

}

void test_cast_ptr(void) {
    CAST_PTR(int, (char) 'A') ;      // FAIL: char is not a pointer
    CAST_PTR(int, (unsigned char) 'A') ; // FAIL: unsigned char is not a pointer
    CAST_PTR(int, 42) ;              // FAIL: int is not a pointer
    CAST_PTR(int, 42U) ;             // FAIL: unsigned int is not a pointer
    CAST_PTR(int, 42L);              // FAIL: long is not a pointer
    CAST_PTR(int, 42LL);             // FAIL: long long is not a pointer
    CAST_PTR(int, 42ULL);            // FAIL: unsigned long long is not a pointer
    CAST_PTR(int, (short) 42);      // FAIL: short is not a pointer
    CAST_PTR(int, 3.14f);            // FAIL: float is not a pointer
    CAST_PTR(int, (double) 3.14);   // FAIL: double is not a pointer
    CAST_PTR(int, (bool) true) ;     // FAIL: bool is not a pointer
}

void test_cast_ptr1(void) {
    char *pc = NULL ;
    char x[10] ;

    CAST_PTR1(void *, pc) ;
    CAST_PTR1(void *, x) ;

    CAST_PTR1(void *, &pc) ;  // FAIL: &pc is char **, not a pointer to non-pointer type
    CAST_PTR1(void *, &x) ;   // FAIL: &x is char (*)[10], not a pointer to non-pointer type

}

void test_cast_unconst(void) {
#ifdef CAST_UNCONST
    char *pc = NULL ;
    void *pv = NULL ;
    struct foo *c_foo ;
    const char *pcc = NULL;

    CAST_UNCONST(char *, pc) ;     // FAIL: pc is not a pointer to const type
    CAST_UNCONST(void *, pv) ;     // FAIL: pv is not a pointer to const type
    CAST_UNCONST(int *, pcc) ;     // FAIL: pcc is pointer to const char, not pointer to const int
    CAST_UNCONST(int *, pc) ;      // FAIL: pc is pointer to char, not pointer to const char
    CAST_UNCONST(struct foo *, c_foo) ; // FAIL: c_foo is pointer to const struct foo, not pointer to non-const struct foo
#else
    printf("%s: Skipped - not supported by this compiler\n", __func__) ;
#endif
}

void test_cast_unconst1(void) {
#ifdef CAST_UNCONST1
    char *pc = NULL ;
    struct foo { int x, y, z ; } ;
    struct foo *c_foo = NULL ;

    CAST_UNCONST1(char *, pc) ;           // FAIL: pc is not a pointer to const type
    CAST_UNCONST1(struct foo *, c_foo) ;  // FAIL: c_foo not pointer to const struct
    printf("%s: Passed\n", __func__) ;
#else
    printf("%s: Skipped - not supported by this compiler\n", __func__) ;
#endif
}

#ifndef UNCONST_PTR
#define UNCONST_PTR(p) static_assert(0, "UNCONST_PTR is not supported by this compiler")
#endif
void test_unconst_ptr(void) {

    char *pc = NULL ;
    void *pv = NULL ;
    struct foo *c_foo ;

    UNCONST_PTR( (const char *) pc) ;       // OK
    UNCONST_PTR( (const char *) pv) ;       // OK
    UNCONST_PTR( (const char *) c_foo) ;    // OK

    UNCONST_PTR(pc) ;        // FAIL: pc is not a pointer to const type
    UNCONST_PTR(pv) ;        // FAIL: pv is not a pointer to const type
    UNCONST_PTR(c_foo) ;     // FAIL: c_foo is not a pointer to const type
}

#ifndef UNCONST_PTR1
#define UNCONST_PTR1(p) static_assert(0, "UNCONST_PTR1 is not supported by this compiler")
#endif
void test_unconst_ptr1(void) {
    char *pc = NULL, **pc2 ;

    struct foo { int x, y, z ; } ;
    struct foo *p_foo = NULL ;
    const struct foo **pc_foo2 = NULL ;
    struct foo **p_foo2 = NULL ;

    UNCONST_PTR1((const char *)pc) ;
    UNCONST_PTR1((const struct foo *) p_foo) ;

    UNCONST_PTR1(pc) ;          // FAIL: pc is not a pointer to const type
    UNCONST_PTR1(pc2) ;         // FAIL: pc2 is pointer to pointer to char, not pointer to const type
    UNCONST_PTR1(p_foo) ;     // FAIL: p_foo is not a pointer to const type
    UNCONST_PTR1(p_foo2) ;    // FAIL: p_foo2 is pointer to pointer to struct foo, not pointer to const type
    UNCONST_PTR1(p_foo2) ; // FAIL: p_foo2 is pointer to pointer to struct foo, not pointer to const type

    const char **pp2 = NULL ;
    char const **pp3 = NULL ;
    char * const *pp4 = NULL ;
    char ** const pp5 = NULL ;
    UNCONST_PTR1(pp2) ;       // FAIL: pointer to pointer
    UNCONST_PTR1(pp3) ;       // FAIL: pointer to const pointer
    UNCONST_PTR1(pp4) ;       // FAIL: const pointer to char
    UNCONST_PTR1(pp5) ;       // FAIL: pointer to pointer to const char
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