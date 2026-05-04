
default: all

NO_MALLOC_CACHE = GLIBC_TUNABLES=glibc.malloc.tcache_max=0:glibc.malloc.mxfast=0
CFLAGS = -g -Wall -Werror -D_DEFAULT_SOURCE
TESTS = test_defer_call test_defer_jump test_defer_closure test_defer_lambda

# -zexecstack

all: $(TESTS:%=%.exe)

clean:
	rm -f *.exe *.o

test_defer_call.exe: test_defer_call.c defer_call.c defer_call.h
	$(CC) $(CFLAGS) -o $@ $(filter %.c, $^)  $(LDFLAGS)

test_defer_jump.exe: test_defer_jump.c defer_now.h
	$(CC) $(CFLAGS) -o $@ $(filter %.c, $^)  $(LDFLAGS)

test_defer_closure.exe: test_defer_closure.c defer_closure.h
	$(CC) $(CFLAGS) -o $@ $(filter %.c, $^)  $(LDFLAGS)

test_defer_lambda.exe: test_defer_lambda.c defer_closure.h
	$(CC) $(CFLAGS) -o $@ $(filter %.c, $^)  $(LDFLAGS)

test:
	$(NO_MALLOC_CACHE) ./test_defer_call.exe
	valgrind ./test_defer_call.exe
	$(NO_MALLOC_CACHE) ./test_defer_jump.exe
	valgrind ./test_defer_jump.exe
	$(NO_MALLOC_CACHE) ./test_defer_closure.exe
	valgrind ./test_defer_closure.exe
	$(NO_MALLOC_CACHE) ./test_defer_lambda.exe
	valgrind ./test_defer_lambda.exe
