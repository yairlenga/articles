#include "defer_call.h"

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

int used_mem(void)
{
    struct mallinfo2 info = mallinfo2() ;
    return info.uordblks ;
}

void test1(void)
{
    printf("%s: Before=%d\n", __func__, used_mem()) ;
    char *x = calloc(10000, sizeof(*x)) ;
    DEFER_FREE(x) ;
    int *y = calloc(200, sizeof(*y)) ;
    DEFER_FREE(y) ;
    printf("%s: After=%d\n", __func__, used_mem()) ;
}

typedef struct foo { char *name ; char x[1000] ; } *FOO ;

FOO fooCreate(const char *name)
{
    FOO p = calloc(1, sizeof(*p)) ;
    p->name = strdup(name);
    return p ;
}

void fooDestroy(FOO p)
{
    printf("%s: %s\n", __func__, p->name) ;
    free(p->name) ;
    free(p) ;
}

void test2(void)
{
    printf("%s: Before=%d\n", __func__, used_mem()) ;
    FOO p1 = fooCreate("First") ;
    DEFER_DESTROY(fooDestroy, p1) ;
    {
        FOO p2 = fooCreate("Second") ;
        DEFER_DESTROY(fooDestroy, p2) ;
    }
    FOO p3 = fooCreate("Third") ;
    DEFER_DESTROY(fooDestroy, p3) ;

    printf("%s: After=%d\n", __func__, used_mem()) ;
}


int main(int argc, char **argv)
{
    (void) argc ;
    (void) argv ;

    printf("Starting: %s\n", __func__) ;
    test1() ;
    printf("After test1: %d\n", used_mem()) ;
    test2() ;
    printf("After test2: %d\n", used_mem()) ;
    malloc_trim(0) ;
    printf("%s: Trim=%d\n", __func__, used_mem()) ;
}