# -flto requires use of the gold linker, so make sure that
# /usr/bin/ld -> ld.gold when using clang++
BASEFLAGS=-std=c++14 -Wall -Wextra -Werror $(EXTRA)
FLAGS=-Ofast -ffast-math -flto
DEBUGFLAGS=-O0 -g
MULTI=-pthread

# this is going to fail miserably on non-Linux
NPROCS=$(shell grep -c ^processor /proc/cpuinfo)
PGOFLAGS=$(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(EXTRA)

EXECS=test testTT randomTree randomEval randomVerify coding stringrepair testnav strip
#EXECS

all: $(EXECS)

pgo: testPGO randomEvalPGO randomVerifyPGO codingPGO stringrepairPGO

bin_release_%: $(subst bin_release_,,%).cpp *.h
	$(CXX) $(BASEFLAGS) $(FLAGS) -o $(subst .cpp,,$<)$(EXTRA) $<

bin_debug_%: $(subst bin_debug_,,%).cpp *.h
	$(CXX) $(DEBUGFLAGS) $(BASEFLAGS) -o $(subst .cpp,,$<)$(EXTRA) $<

bin_nodebug_%: $(subst bin_nodebug_,,%).cpp *.h
	$(CXX) $(BASEFLAGS) $(FLAGS) -DNDEBUG -o $(subst .cpp,,$<)$(EXTRA) $<

bin_prelease_%: $(subst bin_prelease_,,%).cpp *.h
	$(CXX) $(BASEFLAGS) $(FLAGS) $(MULTI) -o $(subst .cpp,,$<)$(EXTRA) $<

bin_pdebug_%: $(subst bin_pdebug_,,%).cpp *.h
	$(CXX) $(DEBUGFLAGS) $(BASEFLAGS) $(MULTI) -o $(subst .cpp,,$<)$(EXTRA) $<

bin_pnodebug_%: $(subst bin_pnodebug_,,%).cpp *.h
	$(CXX) $(BASEFLAGS) $(FLAGS) $(MULTI) -DNDEBUG -o $(subst .cpp,,$<)$(EXTRA) $<

test: bin_release_test
	@#significant comment
testDebug: bin_debug_test
testNoDebug: bin_nodebug_test

testPGO: test.cpp *.h
	rm -f test.gcda
	$(CXX) $(PGOFLAGS) -fprofile-generate -o test-p$(EXTRA) test.cpp
	./test-p$(EXTRA) data/others/dblp_small.xml
	./test-p$(EXTRA) -r data/others/dblp_small.xml
	$(CXX) $(PGOFLAGS) -fprofile-use -o test-p$(EXTRA) test.cpp

testTT: bin_release_testTT
	@#significant comment
testTTDebug: bin_debug_testTT
testTTNoDebug: bin_nodebug_testTT

randomTree: bin_release_randomTree
	@#significant comment
randomTreeDebug: bin_debug_randomTree
randomTreeNoDebug: bin_nodebug_randomTree

randomEval: bin_prelease_randomEval
	@#significant comment
randomEvalDebug: bin_pdebug_randomEval
randomEvalNoDebug: bin_pnodebug_randomEval

randomEvalPGO: randomEval.cpp *.h
	rm -f randomEval.gcda
	$(CXX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-generate -o randomEval-p$(EXTRA) randomEval.cpp
	./randomEval-p$(EXTRA) -n 100 -m 100000
	./randomEval-p$(EXTRA) -n 100 -m 100000 -r
	$(CXX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-use -fprofile-correction -o randomEval-p$(EXTRA) randomEval.cpp

randomVerify: bin_prelease_randomVerify
	@#significant comment
randomVerifyDebug: bin_pdebug_randomVerify
randomVerifyNoDebug: bin_pnodebug_randomVerify

randomVerifyPGO: randomVerify.cpp *.h
	rm -f randomVerify.gcda
	$(CXX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-generate -o randomVerify-p$(EXTRA) randomVerify.cpp
	./randomVerify-p$(EXTRA) -n 100 -m 100000
	./randomVerify-p$(EXTRA) -r -n 100 -m 100000
	$(CXX) $(FLAGS)=$(NPROCS) -DNDEBUG $(BASEFLAGS) $(MULTI) $(EXTRA) -fprofile-use -fprofile-correction -o randomVerify-p$(EXTRA) randomVerify.cpp


coding: bin_release_coding
	@#significant comment
codingDebug: bin_debug_coding
codingNoDebug: bin_nodebug_coding

codingPGO: coding.cpp *.h
	rm -f coding.gcda
	$(CXX) $(PGOFLAGS) -fprofile-generate -o coding-p$(EXTRA) coding.cpp
	./coding-p$(EXTRA) data/others/dblp_small.xml
	./coding-p$(EXTRA) -r data/others/dblp_small.xml
	$(CXX) $(PGOFLAGS) -fprofile-use -o coding-p$(EXTRA) coding.cpp

stringrepair: bin_release_stringrepair
	@#significant comment
stringrepairDebug: bin_debug_stringrepair
stringrepairNoDebug: bin_nodebug_stringrepair

stringrepairPGO: stringrepair.cpp *.h
	rm -f stringrepair.gcda
	$(CXX) $(PGOFLAGS) -fprofile-generate -o stringrepair-p$(EXTRA) stringrepair.cpp
	./stringrepair-p$(EXTRA) data/others/dblp_small.xml
	$(CXX) $(PGOFLAGS) -fprofile-use -o stringrepair-p$(EXTRA) stringrepair.cpp

testnav: bin_release_testnav
	@#significant comment
testnavDebug: bin_debug_testnav
testnavNoDebug: bin_nodebug_testnav

strip: bin_release_strip
	@#significant comment

#RULES

clean:
	rm -f $(EXECS) *.o *.so

cleanup:
	rm -f **/*~ *.ii *.s *.o

scan-build:
	scan-build -analyze-headers -enable-checker core.CallAndMessage -enable-checker core.DivideZero -enable-checker core.DynamicTypePropagation -enable-checker core.NonNullParamChecker -enable-checker core.NullDereference -enable-checker core.StackAddressEscape -enable-checker core.UndefinedBinaryOperatorResult -enable-checker core.VLASize -enable-checker core.builtin.BuiltinFunctions -enable-checker core.builtin.NoReturnFunctions -enable-checker core.uninitialized.ArraySubscript -enable-checker core.uninitialized.Assign -enable-checker core.uninitialized.Branch -enable-checker core.uninitialized.CapturedBlockVariable -enable-checker core.uninitialized.UndefReturn -enable-checker cplusplus.NewDelete -enable-checker cplusplus.NewDeleteLeaks -enable-checker deadcode.DeadStores -enable-checker security.insecureAPI.UncheckedReturn -enable-checker security.insecureAPI.getpw -enable-checker security.insecureAPI.gets -enable-checker security.insecureAPI.mkstemp -enable-checker security.insecureAPI.mktemp -enable-checker security.insecureAPI.vfork -enable-checker unix.API -enable-checker unix.Malloc -enable-checker unix.MallocSizeof -enable-checker unix.MismatchedDeallocator -enable-checker unix.cstring.BadSizeArg -enable-checker unix.cstring.NullArg  $(CXX) $(DEBUGFLAGS) $(BASEFLAGS) -o coding-sb coding.cpp

cppcheck:
	cppcheck --enable=all --inconclusive .
