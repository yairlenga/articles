default: all

BINARIES = gcc-13-test-cast-ok.exe gcc-14-test-cast-ok.exe clang-18-test-cast-ok.exe clang-19-test-cast-ok.exe

all: $(BINARIES)

gcc-13-test-cast-ok.exe: test-cast-ok.c safe-cast.h Makefile
	gcc-13 -o $@ $< -I. -Wall -Wextra

gcc-14-test-cast-ok.exe: test-cast-ok.c safe-cast.h Makefile
	gcc-14 -o $@ $< -I. -Wall -Wextra

clang-18-test-cast-ok.exe: test-cast-ok.c safe-cast.h Makefile
	clang-18 -o $@ $< -I. -Wall -Wextra

clang-19-test-cast-ok.exe: test-cast-ok.c safe-cast.h Makefile
	clang-19 -o $@ $< -I. -Wall -Wextra

clean:
	rm -f *.exe *.result *.err *.log

test.result: $(BINARIES)
	rm -f $@
	@for bin in $(BINARIES) ; do \
		echo "Running $$bin..." | tee -a $@ ; \
		./$$bin >> $@; \
	done

test-pass: test.result test.gold
	diff test.result test.gold

test-cast-fail.err: test-cast-fail.c
	awk '/ FAIL:/ { print FNR ":" $$0}' $< > $@

test-fail-%.err: test-cast-fail.c safe-cast.h Makefile
	$(CC) -o /dev/null $< -I. -Wall -Wextra 2> test-fail-$*.log || true
	awk '-F \\| ' '/ FAIL:/ { print +$$1 ":" $$2}' test-fail-$*.log | sort -n | uniq > $@

test-fail-%: test-fail-%.err test-cast-fail.err
	diff $@.err test-cast-fail.err

test-fail-gcc-13.err: CC = gcc-13
test-fail-gcc-14.err: CC = gcc-14
test-fail-clang-18.err: CC = clang-18 -ferror-limit=1000
test-fail-clang-19.err: CC = clang-19 -ferror-limit=1000

test-fail: test-fail-gcc-13 test-fail-gcc-14 test-fail-clang-18 test-fail-clang-19

test: test-pass test-fail
	echo "All tests passed!"