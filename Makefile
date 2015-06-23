CX=clang++-3.6
PGO_CX=g++
# clang++ <= 3.6 doesn't support -g with -std=c++1y
DBG_CX=clang++-3.6
# -flto requires use of the gold linker, so make sure that
# /usr/bin/ld -> ld.gold when using clang++
BASEFLAGS=-std=c++1y -Wall -Wextra -Werror
FLAGS=-Ofast -ffast-math -flto
DEBUGFLAGS=-O0 -g
MULTI=-pthread

# this is going to fail miserably on non-Linux
NPROCS=$(shell grep -c ^processor /proc/cpuinfo)

EXECS=test testTT randomTree randomEval randomVerify coding repair testnav strip
#EXECS

all: $(EXECS)

pgo: testPGO randomEvalPGO randomVerifyPGO codingPGO repairPGO

test: test.cpp *.h
	$(CX) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testDebug: test.cpp *.h
	$(DBG_CX) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testNoDebug: test.cpp *.h
	$(CX) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o test$(EXTRA) test.cpp

testPGO: test.cpp *.h
	rm -f test.gcda
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -fprofile-generate -o test-p$(EXTRA) test.cpp
	./test-p$(EXTRA) data/others/dblp_small.xml
	./test-p$(EXTRA) -r data/others/dblp_small.xml
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -fprofile-use -o test-p$(EXTRA) test.cpp

testTT: testTT.cpp *.h
	$(CX) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o testTT$(EXTRA) testTT.cpp

testTTDebug: testTT.cpp *.h
	$(DBG_CX) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o testTT$(EXTRA) testTT.cpp

testTTNoDebug: testTT.cpp *.h
	$(CX) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o testTT$(EXTRA) testTT.cpp

randomTree: randomTree.cpp *.h
	$(CX) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o randomTree$(EXTRA) randomTree.cpp

randomTreeDebug: randomTree.cpp *.h
	$(DBG_CX) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o randomTree$(EXTRA) randomTree.cpp

randomTreeNoDebug: randomTree.cpp *.h
	$(CX) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o randomTree$(EXTRA) randomTree.cpp

randomEval: randomEval.cpp *.h
	$(CX) $(FLAGS) $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomEval$(EXTRA) randomEval.cpp

randomEvalDebug: randomEval.cpp *.h
	$(DBG_CX) $(DEBUGFLAGS) $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomEval$(EXTRA) randomEval.cpp

randomEvalNoDebug: randomEval.cpp *.h
	$(CX) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomEval$(EXTRA) randomEval.cpp

randomEvalPGO: randomEval.cpp *.h
	rm -f randomEval.gcda
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-generate -o randomEval-p$(EXTRA) randomEval.cpp
	./randomEval-p$(EXTRA) -n 100 -m 100000
	./randomEval-p$(EXTRA) -n 100 -m 100000 -r
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-use -fprofile-correction -o randomEval-p$(EXTRA) randomEval.cpp

randomVerify: randomVerify.cpp *.h
	$(CX) $(FLAGS) $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomVerify$(EXTRA) randomVerify.cpp

randomVerifyDebug: randomVerify.cpp *.h
	$(DBG_CX) $(DEBUGFLAGS) $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomVerify$(EXTRA) randomVerify.cpp

randomVerifyNoDebug: randomVerify.cpp *.h
	$(CX) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -o randomVerify$(EXTRA) randomVerify.cpp

randomVerifyPGO: randomVerify.cpp *.h
	rm -f randomVerify.gcda
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-generate -o randomVerify-p$(EXTRA) randomVerify.cpp
	./randomVerify-p$(EXTRA) -n 100 -m 100000
	./randomVerify-p$(EXTRA) -r -n 100 -m 100000
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-use -fprofile-correction -o randomVerify-p$(EXTRA) randomVerify.cpp

coding: coding.cpp *.h
	$(CX) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o coding$(EXTRA) coding.cpp

codingDebug: coding.cpp *.h
	$(DBG_CX) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o coding$(EXTRA) coding.cpp

codingNoDebug: coding.cpp *.h
	$(CX) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o coding$(EXTRA) coding.cpp

codingPGO: coding.cpp *.h
	rm -f coding.gcda
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -fprofile-generate -o coding-p$(EXTRA) coding.cpp
	./coding-p$(EXTRA) data/others/dblp_small.xml
	./coding-p$(EXTRA) -r data/others/dblp_small.xml
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -fprofile-use -o coding-p$(EXTRA) coding.cpp

repair: repair.cpp *.h
	$(CX) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o repair$(EXTRA) repair.cpp

repairDebug: repair.cpp *.h
	$(DBG_CX) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o repair$(EXTRA) repair.cpp

repairNoDebug: repair.cpp *.h
	$(CX) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o repair$(EXTRA) repair.cpp

repairPGO: repair.cpp *.h
	rm -f repair.gcda
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -fprofile-generate -o repair-p$(EXTRA) repair.cpp
	./repair-p$(EXTRA) data/others/dblp_small.xml
	$(PGO_CX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -fprofile-use -o repair-p$(EXTRA) repair.cpp

testnav: testnav.cpp *.h
	$(CX) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o testnav$(EXTRA) testnav.cpp

testnavDebug: testnav.cpp *.h
	$(DBG_CX) $(DEBUGFLAGS) $(BASEFLAGS) $(EXTRA) -o testnav$(EXTRA) testnav.cpp

testnavNoDebug: testnav.cpp *.h
	$(CX) $(FLAGS) -DNDEBUG $(BASEFLAGS) $(EXTRA) -o testnav$(EXTRA) testnav.cpp

strip: strip.cpp *.h
	$(CX) $(FLAGS) $(BASEFLAGS) $(EXTRA) -o strip$(EXTRA) strip.cpp

#RULES

clean:
	rm -f $(EXECS) *.o *.so

cleanup:
	rm -f **/*~ *.ii *.s *.o

scan-build:
	scan-build-3.6 -analyze-headers -enable-checker core.CallAndMessage -enable-checker core.DivideZero -enable-checker core.DynamicTypePropagation -enable-checker core.NonNullParamChecker -enable-checker core.NullDereference -enable-checker core.StackAddressEscape -enable-checker core.UndefinedBinaryOperatorResult -enable-checker core.VLASize -enable-checker core.builtin.BuiltinFunctions -enable-checker core.builtin.NoReturnFunctions -enable-checker core.uninitialized.ArraySubscript -enable-checker core.uninitialized.Assign -enable-checker core.uninitialized.Branch -enable-checker core.uninitialized.CapturedBlockVariable -enable-checker core.uninitialized.UndefReturn -enable-checker cplusplus.NewDelete -enable-checker cplusplus.NewDeleteLeaks -enable-checker deadcode.DeadStores -enable-checker security.insecureAPI.UncheckedReturn -enable-checker security.insecureAPI.getpw -enable-checker security.insecureAPI.gets -enable-checker security.insecureAPI.mkstemp -enable-checker security.insecureAPI.mktemp -enable-checker security.insecureAPI.vfork -enable-checker unix.API -enable-checker unix.Malloc -enable-checker unix.MallocSizeof -enable-checker unix.MismatchedDeallocator -enable-checker unix.cstring.BadSizeArg -enable-checker unix.cstring.NullArg  $(CX) $(DEBUGFLAGS) $(BASEFLAGS) -o coding-sb coding.cpp

cppcheck:
	cppcheck --enable=all --inconclusive .
