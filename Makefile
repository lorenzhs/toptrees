CC=clang++
# -flto requires use of the gold linker, so make sure that
# /usr/bin/ld -> ld.gold
BASEFLAGS=-std=c++11 -Wall
FLAGS=-O3 -ffast-math -flto
DEBUGFLAGS=-O0 -g -Wextra
#-D_GLIBCXX_DEBUG

EXECS=test
#EXECS

all: $(EXECS)

.PHONY: $(EXECS)

test:
	$(CC) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

#RULES

clean:
	rm -f $(EXECS) *.o *.so

cleanup:
	rm -f **/*~ *.ii *.s *.o
