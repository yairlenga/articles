default: all

.PHONY: FORCE

MODES = \
	01-strcmp 02-strcmp1 03-memcmp 04-charcmp \
	21-strin 22-strin-4 23-strin-cmp1 24-strin-memcmp 25-strin-filter 26-strin-filter4 \
	31-4cc 32-4cc-in 33-4cc-in4 34-4cc-filter 35-4cc-filter4 36-4cc-opt 

BUILD = build
BINARIES = ${MODES:%=$(BUILD)/ccy-%.exe}

OPT = -O2
CFLAGS = $(OPT) -g -Wall -Wextra -Werror -march=native

$(BUILD):
	mkdir $@

setup: build

all: $(BINARIES)

$(BUILD)/ccy-%.exe: test-ccy-lookup.c ccy-%.c ccy-lookup.h lookup_eq.inc lookup_eq.inc $(FORCE)
	$(CC) $(CFLAGS) $(filter %.c, $^) -o $@ $(LDFLAGS)

test.txt: $(BINARIES) $(FORCE)
	for BIN in $(BINARIES) ; do echo "Testing: $$BIN" ; ./$$BIN 1 ; done
	touch $@

test: test.txt

RUN_ARG = 1000000
RESULT = result
runall: $(BINARIES) $(FORCE)
	rm -f $(RESULT).out $(RESULT).tmp $(RESULT).txt
	@for BIN in $(BINARIES) ; do \
		echo "Running $$BIN $(RUN_ARG) ..." ; \
		time --format "$$BIN real %e user %U sys %S" --append -o $(RESULT).tmp ./$$BIN $(RUN_ARG); \
		done
	mv $(RESULT).tmp $(RESULT).out
	column -t < $(RESULT).out > $(RESULT).txt


BENCHMARK = build/benchmark
benchmark-file: $(BENCHMARK)-Og.txt $(BENCHMARK)-O.txt $(BENCHMARK)-O2.txt $(BENCHMARK)-O3.txt
	BL=`awk 'NR==1 { print $$5 }' $(BENCHMARK)-O.txt` ; \
	paste $(filter %.txt, $^) | \
	awk -v "BL=$$BL" '{ printf "%s %.2f %.2f %.2f %.2f\n",  gensub(".*/", "", 1, $$1), BL/$$5 , BL/$$12, BL/$$19, BL/$$26 }' | \
	column -t > $(BENCHMARK).txt
	( echo "test -Og -O -O2 -O3" ; cat $(BENCHMARK).txt) | md_table > $(BENCHMARK).md

benchmark:
	rm -f $(BENCHMARK).txt
	rm -f $(BINARIES) ; $(MAKE) runall OPT="-Og" RESULT=$(BENCHMARK)-Og
	rm -f $(BINARIES) ; $(MAKE) runall OPT="-O" RESULT=$(BENCHMARK)-O
	rm -f $(BINARIES) ; $(MAKE) runall OPT="-O2" RESULT=$(BENCHMARK)-O2
	rm -f $(BINARIES) ; $(MAKE) runall OPT="-O3" RESULT=$(BENCHMARK)-O3
	make benchmark-file

benchmark-clang:
	$(MAKE) benchmark CC=clang BENCHMARK=build/clang FORCE=FORCE

benchmark-gcc:
	$(MAKE) benchmark CC=gcc BENCHMARK=build/gcc FORCE=FORCE

benchmarks: benchmark-gcc benchmark-clang

clean:
	rm -f build/*.exe perf.data* *.out

realclean: clean
	rm -f $(BUILD) *.txt
