CC=clang++
# -flto requires use of the gold linker, so make sure that
# /usr/bin/ld -> ld.gold
BASEFLAGS=-std=c++11 -Wall -Wextra -Werror
FLAGS=-O3 -ffast-math -flto
DEBUGFLAGS=-O0 -g

EXECS=test testTT random randomEval
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
	$(CC) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o testTT$(EXTRA) testTT.cpp

testTTDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o testTT$(EXTRA) testTT.cpp

testTTNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o testTT$(EXTRA) testTT.cpp

random:
	$(CC) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o random$(EXTRA) random.cpp

randomDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o random$(EXTRA) random.cpp

randomNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o random$(EXTRA) random.cpp

randomEval:
	$(CC) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o randomEval$(EXTRA) randomEval.cpp

randomEvalDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o randomEval$(EXTRA) randomEval.cpp

randomEvalNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o randomEval$(EXTRA) randomEval.cpp

#RULES

clean:
	rm -f $(EXECS) *.o *.so

cleanup:
	rm -f **/*~ *.ii *.s *.o

scan-build:
	scan-build -analyze-headers -enable-checker alpha.core.BoolAssignment -enable-checker alpha.core.CastSize -enable-checker alpha.core.FixedAddr -enable-checker alpha.core.IdenticalExpr -enable-checker alpha.core.PointerArithm -enable-checker alpha.core.PointerSub -enable-checker alpha.core.SizeofPtr -enable-checker alpha.cplusplus.NewDeleteLeaks -enable-checker alpha.cplusplus.VirtualCall -enable-checker alpha.security.ArrayBound -enable-checker alpha.security.ArrayBoundV2 -enable-checker alpha.security.MallocOverflow -enable-checker alpha.security.ReturnPtrRange -enable-checker alpha.security.taint.TaintPropagation -enable-checker alpha.unix.Chroot -enable-checker alpha.unix.MallocWithAnnotations -enable-checker alpha.unix.PthreadLock -enable-checker alpha.unix.SimpleStream -enable-checker alpha.unix.Stream -enable-checker alpha.unix.cstring.BufferOverlap -enable-checker alpha.unix.cstring.NotNullTerminated -enable-checker alpha.unix.cstring.OutOfBounds -enable-checker security.FloatLoopCounter -enable-checker security.insecureAPI.rand -enable-checker security.insecureAPI.strcpy $(CC) $(DEBUGFLAGS) $(BASEFLAGS) -o testTT testTT.cpp

cppcheck:
	cppcheck --enable=all --inconclusive .
