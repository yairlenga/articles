default: all

MODES = \
	01-strcmp 02-strcmp1 03-memcmp 04-charcmp \
	21-strin 22-strin-4 23-strin-cmp1 24-strin-memcmp 25-strin-filter 26-strin-filter4 \
	31-4cc 32-4cc-in 33-4cc-in4 34-4cc-filter 35-4cc-filter4 36-4cc-opt 

BUILD = build
BINARIES = ${MODES:%=$(BUILD)/ccy-%.exe}

CFLAGS = -O2 -g -Wall -Wextra -Werror

$(BUILD):
	mkdir $@

setup: build

all: $(BINARIES)

$(BUILD)/ccy-%.exe: test-ccy-lookup.c ccy-lookup.h ccy-%.c Makefile
	$(CC) $(CFLAGS) $(filter %.c, $^) -o $@ $(LDFLAGS)

test.txt: $(BINARIES) Makefile
	for BIN in $(BINARIES) ; do echo "Testing: $$BIN" ; ./$$BIN 1 ; done
	touch $@

test: test.txt

RUN_ARG = 1000000
RESULT = result.out
result: $(BINARIES) Makefile
	rm -f $(RESULT) $(RESULT).tmp
	@for BIN in $(BINARIES) ; do \
		echo "Running $$BIN $(RUN_ARG) ..." ; \
		time --format "$$BIN real %e user %U sys %S" --append -o $(RESULT).tmp ./$$BIN $(RUN_ARG); \
		done
	mv $(RESULT).tmp $(RESULT)

BENCHMARK = benchmark
benchmark-file: $(BENCHMARK)-Og.txt $(BENCHMARK)-O.txt $(BENCHMARK)-O2.txt $(BENCHMARK)-O3.txt Makefile
	paste $(filter %.txt, $^) | \
		awk '{ print $$1, $$5, ":", $$12, ":", $$19, ":", $$26 }' | \
		column -t > $(BENCHMARK).txt

benchmark:
	rm -f $(BENCHMARK).txt
	rm -f $(BINARIES) ;	$(MAKE) result CFLAGS=-Og RESULT=$(BENCHMARK)-Og.txt
	rm -f $(BINARIES) ; make result CFLAGS=-O RESULT=$(BENCHMARK)-O.txt
	rm -f $(BINARIES) ; make result CFLAGS=-O2 RESULT=$(BENCHMARK)-O2.txt
	rm -f $(BINARIES) ; make result CFLAGS=-O3 RESULT=$(BENCHMARK)-O3.txt
	make benchmark-file

benchmark-clang:
	$(MAKE) benchmark CC=clang BENCHMARK=clang

benchmark-gcc:
	$(MAKE) benchmark CC=gcc BENCHMARK=gcc

benchmarks: benchmark-gcc benchmark-clang

clean:
	rm -f build/*.exe perf.data* a.out *.out

realclean: clean
	rm -f $(BUILD)
