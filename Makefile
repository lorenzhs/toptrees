CC=clang++
GCC=g++-4.9
# -flto requires use of the gold linker, so make sure that
# /usr/bin/ld -> ld.gold
BASEFLAGS=-std=c++11 -Wall -Wextra -Werror
FLAGS=-O3 -ffast-math -flto
DEBUGFLAGS=-O0 -g
MULTI=-pthread

EXECS=test testTT randomTree randomEval randomVerify coding
#EXECS

all: $(EXECS)

.PHONY: $(EXECS)

test:
	$(CC) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testPGO:
	$(GCC) $(FLAGS)=4 -DNDEBUG $(BASEFLAGS) $(EXTRA) -fprofile-generate -o test-p$(EXTRA) test.cpp
	./test-p$(EXTRA) ext/dblp.xml
	./test-p$(EXTRA) -r ext/dblp.xml
	$(GCC) $(FLAGS)=4 -DNDEBUG $(BASEFLAGS) $(EXTRA) -fprofile-use -o test-p$(EXTRA) test.cpp

testTT:
	$(CC) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o testTT$(EXTRA) testTT.cpp

testTTDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o testTT$(EXTRA) testTT.cpp

testTTNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o testTT$(EXTRA) testTT.cpp

randomTree:
	$(CC) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o randomTree$(EXTRA) randomTree.cpp

randomTreeDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o randomTree$(EXTRA) randomTree.cpp

randomTreeNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o randomTree$(EXTRA) randomTree.cpp

randomEval:
	$(CC) $(FLAGS) $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomEval$(EXTRA) randomEval.cpp

randomEvalDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomEval$(EXTRA) randomEval.cpp

randomEvalNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomEval$(EXTRA) randomEval.cpp

randomEvalPGO:
	$(GCC) $(FLAGS)=4 -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-generate -o randomEval-p$(EXTRA) randomEval.cpp
	./randomEval-p$(EXTRA) -n 100 -m 100000
	./randomEval-p$(EXTRA) -r -n 100 -m 100000
	$(GCC) $(FLAGS)=4 -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-use -fprofile-correction -o randomEval-p$(EXTRA) randomEval.cpp

randomVerify:
	$(CC) $(FLAGS) $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomVerify$(EXTRA) randomVerify.cpp

randomVerifyDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomVerify$(EXTRA) randomVerify.cpp

randomVerifyNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomVerify$(EXTRA) randomVerify.cpp

randomVerifyPGO:
	$(GCC) $(FLAGS)=4 -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-generate -o randomVerify-p$(EXTRA) randomVerify.cpp
	./randomVerify-p$(EXTRA) -n 100 -m 100000
	./randomVerify-p$(EXTRA) -r -n 100 -m 100000
	$(GCC) $(FLAGS)=4 -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-use -fprofile-correction -o randomVerify-p$(EXTRA) randomVerify.cpp

coding:
	$(CC) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o coding$(EXTRA) coding.cpp

codingDebug:
	$(CC) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o coding$(EXTRA) coding.cpp

codingNoDebug:
	$(CC) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o coding$(EXTRA) coding.cpp

codingPGO:
	$(GCC) $(FLAGS)=4 -DNDEBUG $(BASEFLAGS) $(EXTRA) -fprofile-generate -o coding-p$(EXTRA) coding.cpp
	./coding-p$(EXTRA) ext/dblp.xml
	./coding-p$(EXTRA) -r ext/dblp.xml
	$(GCC) $(FLAGS)=4 -DNDEBUG $(BASEFLAGS) $(EXTRA) -fprofile-use -o coding-p$(EXTRA) coding.cpp

#RULES

clean:
	rm -f $(EXECS) *.o *.so

cleanup:
	rm -f **/*~ *.ii *.s *.o

scan-build:
	scan-build -analyze-headers -enable-checker alpha.core.BoolAssignment -enable-checker alpha.core.CastSize -enable-checker alpha.core.FixedAddr -enable-checker alpha.core.IdenticalExpr -enable-checker alpha.core.PointerArithm -enable-checker alpha.core.PointerSub -enable-checker alpha.core.SizeofPtr -enable-checker alpha.cplusplus.NewDeleteLeaks -enable-checker alpha.cplusplus.VirtualCall -enable-checker alpha.security.ArrayBound -enable-checker alpha.security.ArrayBoundV2 -enable-checker alpha.security.MallocOverflow -enable-checker alpha.security.ReturnPtrRange -enable-checker alpha.security.taint.TaintPropagation -enable-checker alpha.unix.Chroot -enable-checker alpha.unix.MallocWithAnnotations -enable-checker alpha.unix.PthreadLock -enable-checker alpha.unix.SimpleStream -enable-checker alpha.unix.Stream -enable-checker alpha.unix.cstring.BufferOverlap -enable-checker alpha.unix.cstring.NotNullTerminated -enable-checker alpha.unix.cstring.OutOfBounds -enable-checker security.FloatLoopCounter -enable-checker security.insecureAPI.rand -enable-checker security.insecureAPI.strcpy $(CC) $(DEBUGFLAGS) $(BASEFLAGS) -o testTT testTT.cpp

cppcheck:
	cppcheck --enable=all --inconclusive .
