# Tree Compression with Top Trees Revisited

This implementation accompagnies the paper "Tree Compression with Top Trees Revisited" by Rajeev Raman and me (Lorenz Hübschle-Schneider), to be published in the SEA 2015 proceedings. A [technical Report](http://arxiv.org/abs/1506.04499) is available on the arXiv (arXiv:1506.04499 [cs.DS]).

The central part is the implementation of top tree compression, but this repository also contains RePair compression, the generation of random trees, and some utilities around it.

## Implementation

Nearly all of the actual code is implemented in the header files (`.h`). This allows the compiler to perform extensive optimisations. Each of the `.cpp` files in this folder can be compiled into an executable with GNU Make and the provided `Makefile`.

The executables are:

- `coding` reads an XML file, compresses it with our method, and computes the size of an encoding that is suitable for storage and unpacking. It does not produce an actual encoded output file. It supports both classical top tree compression as well as our RePair-inspired combiner. Usage information is available with the command line switches `-h` or `--help`
- `randomEval` applies the top tree compression algorithm to trees generated uniformly at random. Command line switches specify the number and size of trees to evaluate, the number of trees to evaluate in parallel (as threads), as well as the label alphabet size and the random seed. Help is available with the `-h` or `--help` switches.
- `randomVerify` works similarly to `randomEval`, but computes the top tree and unpacks it again, comparing the result of that with the input tree. This allows us to experimentally verify the correctness of our implementation, using both classic and RePair-like combining. Parameters are similar to `randomEval`.
- `test` apllies the compression algorithm to a single XML file and prints some statistics about the result. In most cases, `coding` should be used. Pass `-w` to write output DOT-files for top tree and Top DAG to `/tmp` and invoke the GraphViz `dot` command on them (warning: this can take a very long time for large graphs!). Pass `-r` for RePair-like combiner.
- `testTT` works similarly to `test` but performs unpacking of the Top DAG to verify correctness. Specify input file with `-i`, output folder for the trimmed and recovered XML files with `-o` (default: `/tmp`), and pass `-r` to use the RePair-inspired combiner.
- `repair` applies the RePair compression algorithm to the input file, printing the grammar and output string to stdout if `-v` is set.
- `randomTree` generates trees uniformly at random. Tree and alphabet size, seed, and output folder for an XML file (default: don't write) can be specified, as well as DOT graph plotting similar to `test`. Pass `-h` or `--help` for full usage information.

## Compiling

All algorithms are implemented in C++11 and have been tested with the GNU C++ compiler, `g++`, in version 4.9 and `clang++` in version 3.6. To build one of the above executables including debug assertions (which can cause significant overhead!), the executable name serves as `make` target, e.g. `make coding`. A version with debug information and without optimisations for debugging can be compiled by appending `Debug` to the executable name, while appending `NoDebug` disables assertions (example: `make randomEvalNoDebug`). For some executables, a target for profile guided optimisation builds is available as well, this might require changing of XML file paths in the Makefile. The suffix for these is `PGO`. The binaries will be suffixed with `-p`, e.g. `coding-p`.

**Notes:** `clang++` requires the `gold` linker and linker plugin to use link-time optimisations, which are enabled by default on all non-`Debug` builds. If this causes problems on your systems, remove `-flto` from line 6 and all occurences of `=$(NPROCS)` in the `Makefile`.

Prior to version 3.6, the `clang++` compiler did not support debug information with the upcoming `c++1y` standard. If you require support for `clang++` 3.4 or 3.5, remove `-g` from the flags. 

Additional `make` targets are available for static analysis with `cppcheck` (warning: many false positives, as it does not seem to have, among others, support for C++11 lambdas) and `scan-build`.

The code was tested and run under Debian GNU/Linux in the unstable distribution as of June 2015, but should work on all Linux-based systems. Adaptation for other operating systems will most likely require a change of paths (e.g. '/tmp'), the `makePathRecursive` function in `Common.h` and the `drawSvg` function in `DotGraphExporter.h`. Additionally, the Makefile requires adaptation to discover the number of available processors for link-time optimisation. POSIX threads (pthreads) are a requirement for the `randomEval` and `randomVerify` commands.

