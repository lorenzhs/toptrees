CC=clang++
# -flto requires use of the gold linker, so make sure that
# /usr/bin/ld -> ld.gold
BASEFLAGS=-std=c++11 -Wall
FLAGS=-O3 -ffast-math -flto
DEBUGFLAGS=-O0 -g -Wextra
#-D_GLIBCXX_DEBUG

EXECS=test testTT
#EXECS

all: $(EXECS)

.PHONY: $(EXECS)

test:
	$(CC) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testTT:
	$(CC) $(FLAGS) $(BASEFLAGS) -o testTT testTT.cpp $(ADD_LIBS)

testTTDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) -o testTT testTT.cpp $(ADD_LIBS)

testTTNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) -o testTT testTT.cpp $(ADD_LIBS)

#RULES

clean:
	rm -f $(EXECS) *.o *.so

cleanup:
	rm -f **/*~ *.ii *.s *.o
