#include "defer_call.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
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
    printf("=== %s\n", __func__) ;
    {
        printf("%s: START: used_mem=%d\n", __func__, used_mem()) ;
        char *x = calloc(10000, sizeof(*x)) ;
        DEFER_FREE(x) ;
        int *y = calloc(200, sizeof(*y)) ;
        DEFER_FREE(y) ;
        printf("%s: FINISH: used_mem=%d\n", __func__, used_mem()) ;
    }
    printf("%s: AFTER: used_mem=%d\n", __func__, used_mem()) ;

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
    printf("=== %s\n", __func__) ;
    {
        printf("%s: START: used_mem=%d\n", __func__, used_mem()) ;
        FOO p1 = fooCreate("First") ;
        DEFER_DESTROY(fooDestroy, p1) ;
        {
            FOO p2 = fooCreate("Second") ;
            DEFER_DESTROY(fooDestroy, p2) ;
        }
        FOO p3 = fooCreate("Third") ;
        DEFER_DESTROY(fooDestroy, p3) ;

        printf("%s: FINISH: used_mem=%d\n", __func__, used_mem()) ;
    }
    printf("%s: AFTER: used_mem=%d\n", __func__, used_mem()) ;
}


void test_file_close(void)
{
    FILE *work_fp ;
    printf("=== %s\n", __func__) ;
    {
        work_fp = fopen("/dev/null", "r") ;
        printf("%s: BEFORE work_fp=%p\n", __func__, work_fp) ;
        DEFER_FCLOSE(work_fp) ;
        printf("%s: FINISH work_fp=%p\n", __func__, work_fp) ;
    }
    printf("%s: AFTER work_fp=%p\n", __func__, work_fp) ;
}

void test_fd_close(void)
{
    int work_fd ;
    printf("=== %s\n", __func__) ;
    {
        work_fd = dup(STDIN_FILENO) ;
        printf("%s: FINISH work_fd=%d, flags=%d\n", __func__, work_fd, fcntl(work_fd, F_GETFD)) ;
        DEFER_FD_CLOSE(work_fd) ;
        printf("%s: FINISH work_fd=%d, flags=%d\n", __func__, work_fd, fcntl(work_fd, F_GETFD)) ;
    }
    printf("%s: FINISH work_fd=%d, flags=%d\n", __func__, work_fd, fcntl(work_fd, F_GETFD)) ;
}

void test_sock_shutdown(void)
{
    printf("%s: BEFORE\n", __func__) ;
    int work_sock[2] ;
    {
        if ( socketpair(AF_LOCAL, SOCK_STREAM, 0, work_sock) != 0 ) {
            printf("%s: socketpair failed: %s(%d)\n", __func__, strerror(errno), errno) ;
            return ;
        }
        printf("%s: START: sock1=%d(F=%d), sock2=%d(F=%d)\n", __func__,
            work_sock[0], fcntl(work_sock[0], F_GETFD),
            work_sock[1], fcntl(work_sock[1], F_GETFD)) ;
    
        DEFER_FD_CLOSE(work_sock[0]) ;
        DEFER_FD_CLOSE(work_sock[1]) ;
        DEFER_SOCK_SHUTDOWN(work_sock[1], SHUT_RDWR) ;
        // Force shutdown
        {
            DEFER_SOCK_SHUTDOWN(work_sock[0], SHUT_RDWR) ;
            char msg[] = "abc" ;
            if ( send(work_sock[0], msg, sizeof(msg), 0) != sizeof(msg) ) {
                printf("%s: send failed: %s(%d)\n", __func__, strerror(errno), errno) ;
                return ;
            }
            if ( recv(work_sock[1], msg, sizeof(msg), 0) != sizeof(msg) ) {
                printf("%s: send failed: %s(%d)\n", __func__, strerror(errno), errno) ;
                return ;
            }
        }
        // Sending should fail
        {
            DEFER_SOCK_SHUTDOWN(work_sock[0], SHUT_RDWR) ;
            char msg[] = "abc" ;
            errno = 0 ;
            int bytes = send(work_sock[0], msg, sizeof(msg), MSG_NOSIGNAL) ;
            if ( bytes != -1 || errno != EPIPE ) {
                printf("%s: send not blocked: bytes=%d, Error %s(%d)\n", __func__, bytes, strerror(errno), errno) ;
                return ;
            }
        }
        printf("%s: FINISH: sock1=%d(F=%d), sock2=%d(F=%d)\n", __func__,
            work_sock[0], fcntl(work_sock[0], F_GETFD),
            work_sock[1], fcntl(work_sock[1], F_GETFD)) ;
    }
    printf("%s: AFTER: sock1=%d(F=%d), sock2=%d(F=%d)\n", __func__,
            work_sock[0], fcntl(work_sock[0], F_GETFD),
            work_sock[1], fcntl(work_sock[1], F_GETFD)) ;
}

int main(int argc, char **argv)
{
    (void) argc ;
    (void) argv ;

    printf("Starting: %s\n", __func__) ;
 
    test_free() ;
    malloc_trim(0) ;
 
    test_destroy() ;
    malloc_trim(0) ;
 
    test_file_close() ;
 
    test_fd_close() ;
 
    test_sock_shutdown() ;
 
    malloc_trim(0) ;
    printf("=== After Trim mem_used=%d\n", used_mem()) ;
}