default: all

MODES = \
	01-strcmp 02-strcmp1 03-memcmp 04-charcmp \
	21-strin 22-strin-cmp1 23-strin-filter 24-strin-c4 \
	31-4cc 32-4cc-in 33-4cc-filter 33-4cc-filter4 34-4cc-opt 

BINARIES = ${MODES:%=ccy-%.exe}

CFLAGS = -O2 -g -Wall -Wextra -Werror

all: $(BINARIES)

ccy-%.exe: test-ccy-lookup.c ccy-lookup.h ccy-%.c Makefile
	$(CC) $(CFLAGS) $(filter %.c, $^) -o $@ $(LDFLAGS)

test: $(BINARIES) Makefile
	for BIN in $(BINARIES) ; do echo "Testing: $$BIN" ; ./$$BIN 1 ; done

result: result.out

RUN_ARG = 100000
result.out: Makefile $(BINARIES)
	rm -f $@ $@.tmp
	@for BIN in $(BINARIES) ; do \
		echo "Running $$BIN $(RUN_ARG) ..." ; \
		time --format "$$BIN real %e user %U sys %S" --append -o $@.tmp ./$$BIN $(RUN_ARG); \
		done
	mv $@.tmp $@


benchmark.txt: result-Og.txt result-O.txt result-O2.txt result-O3.txt Makefile
	paste $(filter %.txt, $^) | \
		awk '{ print $$1, $$5, ":", $$12, ":", $$19, ":", $$26 }' | \
		column -t > $@

benchmark-gcc:
	rm -f benchmark.txt
	make clean ; make result.out CFLAGS=-Og && mv result.out result-Og.txt
	make clean ; make result.out CFLAGS=-O && mv result.out result-O.txt
	make clean ; make result.out CFLAGS=-O2 && mv result.out result-O2.txt
	make clean ; make result.out CFLAGS=-O3 && mv result.out result-O3.txt
	make benchmark.txt
	cp benchmark.txt $@.txt

benchmark-clang:
	rm -f benchmark.txt
	make clean ; make result.out CC=clang CFLAGS=-Og && mv result.out clang-Og.txt
	make clean ; make result.out CC=clang CFLAGS=-O && mv result.out clang-O.txt
	make clean ; make result.out CC=clang CFLAGS=-O2 && mv result.out clang-O2.txt
	make clean ; make result.out CC=clang CFLAGS=-O3 && mv result.out clang-O3.txt
	make benchmark.txt
	cp benchmark.txt $@.txt

benchmark: benchmark-gcc benchmark-clang


clean:
	rm -f *.exe perf.data* a.out *.out