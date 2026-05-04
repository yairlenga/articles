
default: all

NO_MALLOC_CACHE = GLIBC_TUNABLES=glibc.malloc.tcache_max=0:glibc.malloc.mxfast=0
CFLAGS = -g -Wall -Werror -D_DEFAULT_SOURCE
TESTS = test_defer_call test_defer_now

# -zexecstack

all: $(TESTS:%=%.exe)

clean:
	rm -f *.exe *.o

test_defer_call.exe: test_defer_call.c defer_call.c defer_call.h
	$(CC) $(CFLAGS) -o $@ $(filter %.c, $^)  $(LDFLAGS)

test_defer_now.exe: test_defer_now.c defer_call.c defer_call.h
	$(CC) $(CFLAGS) -o $@ $(filter %.c, $^)  $(LDFLAGS)

test:
	$(NO_MALLOC_CACHE) ./test_defer_call.exe
	valgrind ./test_defer_call.exe