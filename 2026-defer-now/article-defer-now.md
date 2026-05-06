# Stop Waiting for defer: A Practical Cleanup Layer for C

## Not another macro trick. Just a small set of cleanup helpers that cover memory, files, descriptors, sockets, and custom objects.

## Introduction

I recently came across a post on Reddit: “I’m tired of waiting for the C language to finish specifying the defer function. What can I do?”

If you do a quick search, there is no shortage of answers. Over the years, many developers have built `defer` emulation in C - some clever, some portable, some tricky, and some break on newer compilers. The problem is not lack of ideas, but that many of them are not something you would standardize across a real codebase. (For a survey of implementations, see [(Un)portable defer in C](https://antonz.org/defer-in-c/))

My article is **not** about another attempt to implement `defer`, describe what you can do with it, compare it to other languages, or debate design choices. My goal here is much simpler: define a small, consistent set of cleanup macros that we can safely use in day-to-day C code. In practice, this approach eliminates most repetitive cleanup code in typical C functions, reduces error-handling boilerplate, and makes resource management predictable across the codebase.

The initial implementation will use compiler extensions. When a standard `defer` becomes available, these macros can be adapted to use it.

This approach is especially relevant for projects that run across multiple environments. In many cases older environments must be supported for years, which makes adapting new language features like `defer` a long-term process. Many legacy projects are also 5-10 years behind the current technology stack - which means they will not be able to leverage new `defer` anytime soon.

### What this gives you

- Uniform cleanup patterns across your codebase
- Fewer leaks and double-free bugs
- Cleaner error handling paths (especially with early returns)
- No dependency on future language features

### Example

The following patterns already cover most real-world resource management in typical C codebases:
```c

// calloc -> free
char *p = calloc(n, sizeof(*p)) ;
if ( !p ) ... ;
DEFER_FREE(p) ;

// fopen -> fclose
FILE *fp = fopen(filename, "r") ;
if ( ! fp ) ... ;
DEFER_FCLOSE(fp) ;


// Work file -> remove file
FILE *new_file = fopen(workfile, "w") ;
DEFER_REMOVE(workfile) ;
if ( ! new_file ) ... ;
DEFER_FCLOSE(new_file) ;
```

These macros follow a simple pattern: free, close, destroy, or restore. Once those are standardized, most cleanup becomes predictable.

If your codebase works with lower level objects (file descriptors, sockets, mutexes, ...), a few additional macros will be useful:

```c
// open -> close
int fd = open(filename, O_RDONLY) ;
if ( fd < 0 ) ... ;
DEFER_FD_CLOSE(fd) ;

// Socket -> shutdown/close
int sock = socket(AF_INET, SOCK_STREAM, 0) ;
if ( sock < 0 ) ... ;
DEFER_FD_CLOSE(sock) ;

if ( connect(...) ) ... ;
DEFER_SOCK_SHUTDOWN(sock, SHUT_RDWR) ;

// Mutex lock
pthread_mutex_lock(&lock) ;
DEFER_MUTEX_UNLOCK(&lock) ;

```
## Overview: Cleanup model

### Three axes of cleanup

Instead of treating each cleanup case separately, we can describe them using a small set of dimensions.

Looking at the common patterns of cleanup functions, we can classify them along three axes: Binding, Resource Identifier, and Context.

* Binding: ATX or VAL.
  * ATX - the cleanup function will be invoked on the final value of the variable (at scope exit)
  * VAL - the cleanup function will be invoked on the current value of the variable (at registration time)

* Resource identifier: Pointer vs Integer
  * P (pointer) Most resources are identified by their memory address
  * I (integer) Some resources are identified by their integer handle

* Extra Context:
  * None - cleanup function does not need any extra information
  * X - Extra information is needed for the cleanup operation

Each axis is independent. In practice, most code uses only a small subset of these combinations, but supporting all three provides a uniform model for a wide range of cleanup scenarios.

In practice:

- Most code uses: P + ATX
- Some cases need: I (file descriptors)
- Rare cases need: X (extra arguments)

### Binding: ATX vs VAL

In simple cases, we create a resource, and we perform the cleanup by calling the "destructor" function with the same object that was created
```c
{
    char *const x = calloc(n, sizeof(*x)) ;
    ...
    if ( ... ) { free(x) ; return ; }
    ...
    free(x) ;
}
```
Which we will implement with
```c
{
    char *const x = calloc(n, sizeof(*x)) ;
    DEFER_FREE(x) ;
    ...
}
```
This model works as long as the value of the resource does not change. However, there are cases where the resource address may change. One example is with `realloc`, when the `free` function should be invoked with the final value of `x`:
```c
{
    char *x = calloc(n, sizeof(*x)) ;
    DEFER_FREE(x) ;
    ...
    x = realloc(x, m*sizeof(*x)) ;
    ...
}
```
Another case is when the resource is released earlier in the function, and there is no need to perform the cleanup at the end.

```c
{
    char *x = calloc(n, sizeof(*x)) ;
    DEFER_FREE(x) ;
    ...
    free(x) ;
    x = NULL ;
    ...
    return ;
}
```
Releasing the memory when it's no longer needed is good practice. In this case, it's important to reset the value of the resource, so that the automatic invocation will not attempt to `free` the block again (which will likely crash the program).
> Most resources already have a natural "NA" value (e.g., NULL, -1), which can be used to mark them as already cleaned up.

We provide two variants for each cleanup action. The default version will use the ATX version ; it will invoke the cleanup on the final value of the resource. The _VAL variant will apply the cleanup using the value of the resource at the time of registration - for the (few) cases where it's needed.
```
    char *p1 = ... ;
    DEFER_FREE(p1) ;       // ATX: use value of p1 at scope exit

    char *p2 = ... ;
    DEFER_FREE_VAL(p2) ;   // use value of p2 at registration time.
```

### Resource Identifier: Pointer vs Integer

In many cases, resources are identified by their memory address, and the cleanup function only needs this memory address. Almost anything derived from malloc/calloc (or other allocators). This includes:
* File Object (`FILE *`) created by `fopen`, `fdopen` or `popen`
* Dynamically created strings created by `strdup`, `strndup`
* Network structures created by `getaddrinfo` and similar
* User defined objects created on the heap.

The second category of identifier is handles - resources that are identified by integer handle - in many cases, referencing system resources, outside our process space
* File descriptors (`open`, `creat`, `socket`, ...),
* Process identifiers (`fork`)
* IPC resources like shared memory (`shmget`)

### Extra Context

Certain cleanup functions require extra information. For example:
* The `shutdown` system calls take a parameter (int how).
* The `munmap` system call takes a length parameter

We generalize the support for extra parameters by adding support for extra pointer, which can be used to pass additional required parameters.

### Naming rules

To support all variations, we use consistent naming rules:
```
    DEFER_CALL_(P|I)(X)?_(ATX|VAL)
```
For example:
* DEFER_CALL_P_ATX(cleanup, var) - Call cleanup(void *), use final value of 'var'.
* DEFER_CALL_I_VAL(cleanup, fd) - Call cleanup(int), use value of `fd` at registration time.
* DEFER_CALL_IX_ATX(cleanup, sock, cxt) - Call cleanup(int, void *), use final value of `sock`, pass `cxt` with extra parameters.

## Inventory of provided macros

This project is intentionally small.

The goal is not to introduce a new abstraction layer, but to provide a set of useful macros that cover the common resource-cleanup patterns in C using DEFER-like macros.

The cleanup function is invoked with the value of the resource identifier at the end of the block.

> Rule: If the resource is released in the middle of the block - important to set its identifier to NULL (or other invalid value like -1) to prevent double-cleanup.

The core list covers the most common resource types.
* DEFER_FREE(void *p) for heap allocated memory
* DEFER_FCLOSE(FILE *fp) for `FILE *` streams
* DEFER_FD_CLOSE(int fd) for file descriptors

The full list (categories) includes

Memory:
* DEFER_FREE(void *p) - Core
* DEFER_FREE_PTR_ARRAY(void **a, int sz) - free list of pointers.

Files:
* DEFER_FCLOSE(FILE *fp) - core
* DEFER_REMOVE(const char *p)
* DEFER_PCLOSE(FILE *fp)
* DEFER_CLOSEDIR(DIR *d)

File Descriptors, Sockets:
* DEFER_FD_CLOSE(int fd) - core
* DEFER_SOCK_SHUTDOWN(int fd, int how)

Processes:
* DEFER_KILL(pid_t pid, int sig)

Synchronization:
* DEFER_MUTEX_UNLOCK(pthread_mutex_t *m)
* DEFER_RWLOCK_UNLOCK(pthread_rwlock_t *lock)

### CORE: DEFER_FREE(void *ptr)

To prevent memory leak - add `DEFER_FREE` after allocating the block with `malloc`, `calloc`, `realloc(NULL, ...)` or similar. The pattern cover resizing with with `realloc` `reallocarray` or similar, as long as resized address is stored to the same variable.

```c
{
    char *cp = malloc(n) ;
    if ( !cp ) return ERROR ;
    DEFER_FREE(cp) ;
    ...
    cp = realloc(cp, n + 100) ;
    ...
}
```
If the allocated memory can be freed before the end of the block, the resource identifier must be set to NULL.
```c
{
    char *ip = calloc(n, sizeof(*v)) ;
    if ( !ip ) return ERROR ;
    DEFER_FREE(ip) ;
    ...
    free(cp) ; 
    cp = NULL ;
}    
```

### CORE: DEFER_FCLOSE(FILE *fp)

To prevent leakage of `FILE *` object, `DEFER_FCLOSE` can be called after function that create `FILE *` - `fopen`, `fdopen`, `freopen`.
```c
{
    FILE *fp = fopen(filename, "r") ;
    DEFER_FCLOSE(fp) ;
}
```
If the file is closed before the end of the block, the resource identifier must be set to NULL. At that point, it can even be reused.
```c
{
    FILE *fp = fopen(filename, "r") ;
    if ( !fp ) return ERROR ;
    DEFER_FCLOSE(fp) ;
    ...
    
    ...
    fclose(fp) ;
    fp = NULL ;
    ...
    fp = fopen(filename2, "r") ;
    if ( !fp2 ) return ERROR ;
    ...
}
```

### CORE: DEFER_FD_CLOSE(int fd)
To prevent leakage of file descriptors, `DEFER_FD_CLOSE` can be called after any function that create a file descriptor (`open`, `create`, `socket`, `dup`, ...).

```c
{
    int fd = open(filename, O_RDONLY) ;
    DEFER_FD_CLOSE(fd) ;
    if ( fd < 0 ) return ERROR ;
    ...
}
```
If the file descriptor is explicitly closed in the block important to set the resource identifier to -1. Possible to set the resource identifier even before the first call.
```c
{
    int fd = -1 ;
    DEFER_FD_CLOSE(fd) ;
    ...
    fd = open(filename, O_RDONLY) ;
    if ( fd < 0 ) return ;
    ...
    close(fd) ;
    fd = -1 ;
    ...
    fd = open(file2, O_RDONLY) ;
    if ( fd < 0 ) return ;

}
```


### MEMORY: DEFER_FREE_PTR_ARRAY(void **a, int sz)

One common use case for managing list of large objects is to track list of pointers to created objects inside a fixed-size, or dynamic array of pointers. The DEFER_FREE_PTR_ARRAY can be used to call free on each element.

```c
struct foo { ... }

{
    struct foo **list = NULL ;
    DEFER_FREE(list) ;
    int pos = 0 ;
    DEFER_FREE_PTR_ARRAY(list, pos) ;
    ...
    for (...) {
        list = realloc(list, (pos+1)*sizeof(*list)) ;
        list[pos] = calloc(1, sizeof(*list[pos])) ;
        pos++ ;
        ...
    }
}
```
If using a fixed size array in zero-initialized memory possible to define the cleanup based on the array maximum size.
```c
#define MAX_FOO
struct foo { ... }

{
    int pos = 0 ;
    struct foo *list[MAX_FOO] = {} ;
    DEFER_FREE_PTR_ARRAY(list, MAX_FOO)  ;
    ...
    for (...) {
        list[pos] = calloc(1, sizeof(*list[pos])) ;
        pos++ ;
        ...
    }
}
```

### FILES: DEFER_REMOVE(const char *pathname)

When creating work files, it might be useful to `remove` the work file in addition to closing the `FILE *` object. This will ensure work files are removed when no longer needed.

```c
{
    FILE *fp = fopen(workfile, "w") ;
    DEFER_REMOVE(workfile) ;
    if ( !fp ) return ERROR ;
    DEFER_FCLOSE(fp) ;    
}
```

### FILES: DEFER_PCLOSE(FILE *fp)

```c
{
    char *fp = popen("ls", "r") ;
    DEFER_PCLOSE(fp) ;
    ...    
}
```


### FILES: DEFER_CLOSEDIR(DIR *dirp)

```c
{
    DIR *dir = opendir(dir_path) ;
    if ( !dir ) return ERROR ;
    DEFER_CLOSEDIR(dir) ;
}
```

### SOCKET: DEFER_SOCK_SHUTDOWN(int sock, int how)

Managing socket require executing shutdown once the socket has been connected (or after `listen`).
```c
{
    int sock = socket(...) ;
    if ( socket < 0 ) return ERROR ;
    DEFER_FD_CLOSE(sock) ;
    ...
    // Shutdown required only after connect
    if ( connect(sock, ...) < 0 ) return ERROR ;
    DEFER_SOCK_SHUTDOWN(sock, SHUT_RDWR) ;
    ...
}

```

### Process: DEFER_KILL(int pid, int sig)
```c
{
    int fd[2] ;
    if ( pipe(fd) < 0 ) return ERRORO ;
    DEFER_FD_CLOSE(fd[0]) ;
    DEFER_FD_CLOSE(fd[1]) ;
    pid_t pid = fork() ;
    if ( pid < 0 ) return ERROR ;
    if ( pid == 0 ) {
        close(fd[0]) ;
        fd[0] = -1 ;
        child_action() ;
        exit() ;
    } ;
    // Parent
    close(fd[1]) ;
    fd[1] = -1 ;
    DEFER_KILL(pid, SIGTERM) ;
    ... Read from child process
    while ( read(fd[0], ...) > 0 ) 
        if ( ... ) return ERROR ;
    }
}
```

### Synchronization: DEFER_MUTEX_UNLOCK(pthread_mutex_t *m)
```c
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER ;

{
    pthread_mutex_lock(&m) ;
    DEFER_MUTEX_UNLOCK(&m) ;

}
```
If the lock is released before the exit, the resource should be set to NULL to avoid double-cleanup

```c
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER ;

{
    pthread_mutex_t *mp = &my_mutex ;
    pthread_mutex_lock(mp) ;
    DEFER_MUTEX_UNLOCK(mp) ;
    ...
    // early release of the lock:
    pthread_mutex_unlock(mp) ;
    mp = NULL ;
}

```


### Synchronization: DEFER_RWLOCK_UNLOCK(pthread_mutex_t *lock)

```c
pthread_rwlock_t  my_lock = PTHREAD_RWLOCK_INITIALIZER ;

{
    pthread_rwlock_t lp = &my_lock ;
    // Read some data
    pthread_rwlock_rdlock(lp) ;
    DEFER_RWLOCK_UNLOCK(lp) ;
    ...
    // Release
    pthread_rwlock_unlock(lp) ;
    lp = NULL ;

    // Write some data
    lp = &my_lock ;
    pthread_rwlock_wrlock(lp ;
    ..

}
```