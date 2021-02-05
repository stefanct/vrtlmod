# vrtlmod - Verilated RTL Injection modifier

## Brief
Verilated RTL modifier (vrtlmod) is a LLVM-based tool modifying <a href="https://www.veripool.org/wiki/verilator" title="Verilator homepage">Verilator</a> output for cycle accurate target injection. Additionally, Vrtlmod takes <a href="https://gitlab.lrz.de/ge29noy/regpicker" title="regpicker git">RegPicker</a>'s XML output as target source.

## Dependencies
Besides standard (gmake, cmake, gcc, ...)

1. Verilator  - min. v4.1 (see: https://www.veripool.org/wiki/verilator and install guide)
2. libxml2
3. llvm			  - min. v9.0.1
4. Boost      - min. v1.72.0
5. (SystemC)  - ! make sure c++std is aligned with VRTL

## Build

1. **Required environment:**

```
	[BUILD]: export LLVM_DIR=<path/to/llvm/install/dir>/lib/cmake/llvm
	[BUILD]: export VERILATOR_ROOT=<path/to/verilator/repository> (not tested with installed Verilator, yet)
	[TEST]:  export REGPICKER_ROOT=<path/to/regpicker/repository>
	[TEST]:  export SystemCLanguage_DIR=<path/to/systemc/install/dir>

```

2. **Or via CMake command line arguments:**
[BUILD]: -DLLVM_DIR=<path/to/llvm/install/dir>/lib/cmake/llvm
[BUILD]: -DVERILATOR_ROOT=<path/to/verilator/repository> (not tested with installed Verilator, yet)
[TEST]:  -DREGPICKER_ROOT=<path/to/regpicker/repository>
[TEST]:  -DSystemCLanguage_DIR=<path/to/systemc/install/dir>

```
mkdir build && cd build
cmake .. [-DLLVM_DIR=...] [-DVERILATOR_ROOT=...] [-DREGPICKER_ROOT=...] [-DSystemCLanguage_DIR] [-DBUILD_DOC=On] [-DTEST_BUILD=On]
make
```

## Usage

1. **Required inputs:**

	- <a href="https://gitlab.lrz.de/ge29noy/regpicker" title="regpicker git">RegPicker</a>'s XML output: `<XML-file>`
	- <a href="https://www.veripool.org/wiki/verilator" title="Verilator homepage">Verilator</a> output: `<VRTL-files>` `<VRTL-Hpp-files>`

2. **Execution:**

```
vrtlmod [--systemc] --out=<outputdir> --regxml=<XML-file> <VRTL-Cpp-files> -- clang++ -I<VRTL-Hpp-dir> -I$LLVM_DIR/lib/clang/9.0.1/include -I$VERILATOR_ROOT/include [-I<path/to/systemc/include>]
```

The output can be found at `<outputdir>` in form of altered Cpp files (`<VRTL-Cpp-files>_vrtlmod.cpp`) and the built injection API inside `<outputdir>/vrtlmodapi` concluding:
- Target dictionary
- Injection classes

## Examples

The `-DTEST_BUILD=On` option in cmake enables a SystemC and C++ verilate->regpick->vrtlmod flow for the `test/fiapp/fiapp.sv` SystemVerilog example.
In addition, a SystemC (`test/fiapp/sc_fiapp_test.cpp`) and C++ (`test/fiapp/fiapp_test.cpp`) testbench showcases the usage of the generated fault injection API.

These tests are intended, both, as a form of unit-tests for `vrtlmod` as well as an example for its integration in other fault injection projects.
