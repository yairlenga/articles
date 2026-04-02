#include "safe-cast.h"

#include <stdio.h>

static void test_cast_ok()
{
    int i = 42;
    double d = 3.14;
    const char *str = "hello";

    int i1 = CAST(int, d);
    long l1 = CAST(long, i);
    double d1 = CAST(double, i) ;
    int *pi = CAST(int *, str) ;

    printf("%s: I1=%d L1=%ld d1=%f pi=%08x\n", __func__, i1, l1, d1, *(int *) pi) ;
}

static void test_cast_fail()
{
    int i = 42;
    double d = 3.14;
    const char *str = "hello";

    int i1 = CAST(int, &d);
    long l1 = CAST(long, &i);
    double d1 = CAST(double, &i) ;
    int *pi = CAST(int *, str[0]) ;

    printf("%s: I1=%d L1=%ld d1=%f pi=%08x\n", __func__, i1, l1, d1, *(int *) pi) ;
}

void test_ptr1(void)
{
    struct S { int x; };
    union U { int x; double y; };

    int *ip;
    int **ipp;
    struct S *sp;
    union U *up;
    int (*ap)[8];

    CAST_PTR1(void *, ip);   // OK
    CAST_PTR1(void *, sp);   // OK
    CAST_PTR1(void *, up);   // OK
    CAST_PTR1(void *, ap);   // OK

    CAST_PTR1(void *, ipp);  // FAIL

    CAST_PTR2(void *, ipp);  // OK
    CAST_PTR2(void *, ip);   // FAIL
    CAST_PTR2(void *, sp);   // FAIL
}

static void test_cast_val_ok(void)
{
    int i = 42;
    double d = 3.14;
    const char *str = "hello";

    int i1 = CAST_VAL(int, d);
    long l1 = CAST_VAL(long, i);
    double d1 = CAST_VAL(double, i) ;
    int s1 = CAST_VAL(int, str[0]) ;

    printf("%s: I1=%d L1=%ld d1=%f pi=%08x\n", __func__, i1, l1, d1, *(int *) pi) ;
}

static void test_cast_val_fail(void)
{
    int i = 42;
    double d = 3.14;
    const char *str = "hello";

    int i1 = CAST_VAL(int, &d);
    long l1 = CAST_VAL(long, &i);
    double d1 = CAST_VAL(double, &i) ;
    int *pi = CAST_VAL(int *, str) ;

    printf("%s: I1=%d L1=%ld d1=%f pi=%08x\n", __func__, i1, l1, d1, *(int *) pi) ;
}

static void test_cast_ptr_ok(void)
{
    int i = 42;
    double d = 3.14;
    const char *str = "hello";

    int *i1 = CAST_PTR(int *, &d);
    long *l1 = CAST_PTR(long *, &i);
    double *d1 = CAST_PTR(double *, &i) ;
    int *pi = CAST_PTR(int *, str) ;

    printf("%s: I1=%d L1=%ld d1=%f pi=%08x\n", __func__, *i1, *l1, *d1, *(int *) pi) ;
}




int main(int argc, char *argv[]) {
    test_cast_ok() ;
    test_cast_fail() ;
    return 0;
}   