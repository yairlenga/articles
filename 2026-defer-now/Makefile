
default: all

NO_MALLOC_CACHE = GLIBC_TUNABLES=glibc.malloc.tcache_max=0:glibc.malloc.mxfast=0
CFLAGS = -Wall -Werror

all: test1.exe

clean:
	rm -f *.exe *.o

test1.exe: test1.c defer_call.c defer_call.h
	$(CC) $(CFLAGS) -o $@ $(filter %.c, $^)  $(LDFLAGS)

test:
	$(NO_MALLOC_CACHE) ./test1.exe
	valgrind ./test1.exe