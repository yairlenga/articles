default: all

MODES = \
	01-strcmp 02-strcmp1 03-memcmp 04-charcmp \
	21-strin 22-strin-static 23-strin-filter \
	31-4cc 32-4cc-in 33-4cc-filter 34-4cc-opt 

BINARIES = ${MODES:%=ccy-%.exe}

CFLAGS = -O2 -g

all: $(BINARIES)

ccy-%.exe: test-ccy-lookup.c ccy-lookup.h ccy-%.c Makefile
	$(CC) $(CFLAGS) $(filter %.c, $^) -o $@ $(LDFLAGS)

test: $(BINARIES) Makefile
	for BIN in $(BINARIES) ; do echo "Testing: $$BIN" ; ./$$BIN 1 ; done

RUN_ARG = 1000000
result.txt: Makefile $(BINARIES)
	rm -f $@ $@.tmp
	@for BIN in $(BINARIES) ; do \
		echo "Running $$BIN $(RUN_ARG) ..." ; \
		time --format "$$BIN real %e user %U sys %S" --append -o $@.tmp ./$$BIN $(RUN_ARG); \
		done
	mv $@.tmp $@

clean:
	rm -f *.exe perf.data* a.out *.txt