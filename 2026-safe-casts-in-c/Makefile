default: all

BINARIES = gcc-13-test-cast-ok.exe gcc-14-test-cast-ok.exe clang-18-test-cast-ok.exe clang-19-test-cast-ok.exe

all: $(BINARIES)

gcc-13-test-cast-ok.exe: test-cast-ok.c safe-cast.h
	gcc-13 -o $@ $< -I. -Wall -Wextra

gcc-14-test-cast-ok.exe: test-cast-ok.c safe-cast.h
	gcc-14 -o $@ $< -I. -Wall -Wextra

clang-18-test-cast-ok.exe: test-cast-ok.c safe-cast.h
	clang-18 -o $@ $< -I. -Wall -Wextra

clang-19-test-cast-ok.exe: test-cast-ok.c safe-cast.h
	clang-19 -o $@ $< -I. -Wall -Wextra

clean:
	rm -f $(BINARIES)

test.result: $(BINARIES)
	rm -f $@
	@for bin in $(BINARIES) ; do \
		echo "Running $$bin..." | tee -a $@ ; \
		./$$bin | tee -a $@; \
	done

test: test.result test.gold
	diff test.result test.gold