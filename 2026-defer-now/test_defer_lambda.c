// #include "defer_invoke.h"
#include "defer_lambda.h"

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

int used_mem(void)
{
    struct mallinfo2 info = mallinfo2() ;
    return info.uordblks ;
}

void test_free(void)
{
    printf("%s: Before: used_mem=%d\n", __func__, used_mem()) ;
    char *x = calloc(10000, sizeof(*x)) ;
    DEFER_LAMBDA( x, free(x) ) ;
    int *y = calloc(200, sizeof(*y)) ;
    DEFER_LAMBDA( y, free(y) ) ;
    printf("%s: Exit: used_mem=%d\n", __func__, used_mem()) ;
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

void test_destroy(void)
{
    printf("%s: Before: used_mem=%d\n", __func__, used_mem()) ;
    FOO p1 = fooCreate("First") ;
    DEFER_LAMBDA( p1, fooDestroy(p1) ) ;
    {
        FOO p2 = fooCreate("Second") ;
        DEFER_LAMBDA( p2, fooDestroy(p2) ) ;
    }
    FOO p3 = fooCreate("Third") ;
    DEFER_LAMBDA( p3, fooDestroy(p3) ) ;

    printf("%s: Exit: used_mem=%d\n", __func__, used_mem()) ;
}

static FILE *work_fp ;

void test_file_close(void)
{
    work_fp = fopen("/dev/null", "r") ;
    DEFER_LAMBDA( work_fp, if ( work_fp) fclose(work_fp) ) ;
    printf("%s: work_fp=%p\n", __func__, work_fp) ;
}

int work_fd ;
void test_fd_close(void)
{
    work_fd = dup(STDIN_FILENO) ;
    DEFER_LAMBDA( work_fd, if ( work_fd != -1 ) { close(work_fd) ; } ) ;
    printf("%s: work_fd=%d, flags=%d\n", __func__, work_fd, fcntl(work_fd, F_GETFD)) ;
}

int work_sock ;
void test_sock_close(void)
{
    work_sock = socket(AF_LOCAL, SOCK_STREAM, 0) ;
    DEFER_LAMBDA( work_sock, if ( work_sock != -1 ) { shutdown(work_sock, SHUT_RDWR); close(work_sock) ; } ) ;
    printf("%s: work_fd=%d, flags=%d\n", __func__, work_fd, fcntl(work_fd, F_GETFD)) ;
}

int main(int argc, char **argv)
{
    (void) argc ;
    (void) argv ;

    printf("Starting: %s\n", __func__) ;
    test_free() ;
    malloc_trim(0) ;
    printf("=== After test_free: used_mem=%d\n", used_mem()) ;

    test_destroy() ;
    malloc_trim(0) ;
    printf("=== test_destroy: used_mem=%d\n", used_mem()) ;

    test_file_close() ;
    printf("=== test_file_close work_fp=%p\n", work_fp) ;

    test_fd_close() ;
    printf("=== test_fd_fclose: work_fd=%d, flags=%d\n", work_fd, fcntl(work_fd, F_GETFD)) ;
 
    test_sock_close() ;
    printf("=== test_sock_close: work_sock=%d, flags=%d\n", work_sock, fcntl(work_sock, F_GETFD)) ;

    malloc_trim(0) ;
    printf("=== After Trim mem_used=%d\n", used_mem()) ;
}