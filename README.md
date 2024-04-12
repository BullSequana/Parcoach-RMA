This is ParcoachRMA. The parcoachRMA pass instruments STORE/LOAD writing/reading memory that is involved in a MPI RMA function. Some MPI RMA functions are also renamed.  


### Prerequisites

#### CMake `>= 3.12`
#### LLVM `= 10.0.x`


### Installation

#### Installation through Spack or EasyBuild
Spack and EasyBuild are provided with this repository. They can be used to install 
parcoachRMA in user space.

The recipes will build an [LLVM fork](https://github.com/flang-compiler/flang) that
supports flang (previously known as F18) and compile BullOpenMPI using that compiler.
The resulting ParcoachRMA can then be used to analyze Fortran programs.


#### Manually build parcoachRMA llvm pass and the runtime library associated:

```bash
cd path_to_parcoachRMA
mkdir build
cd build
cmake .. 
make
```

If CMake does not find LLVM, you can supply the path to your LLVM installation as follows  :

```bash
cmake .. -DLLVM_DIR=path_to_llvm_install/lib/cmake/llvm/
```

You can run 'ctest --verbose' to see the commands executing the tests
NOTE: ctest is not finished yet

### Run the pass on a single file

#### Using clang (not functional ATM)

```bash
cd path_to_parcoachRMA
clang -Xclang -load -Xclang build/src/parcoachRMA.* -c something.c
=
./build/parcoach something.c
```

#### Using opt

```bash
cd path_to_parcoachRMA
clang -c -emit-llvm something.c
opt -load  build/src/parcoachRMA.dylib -parcoach something.bc
=
./build/parcoachBC something.bc
```
		
#### To have a look at the IR 

```bash
cd path_to_parcoachRMA
./parcoachBC something.bc >  somethingINSTR.bc
llvm-dis somethingINSTR.bc
```
And open the generated file somethingINSTR.ll

#### To execute the program after the pass

```bash
cd path_to_parcoachRMA
clang -c -emit-llvm something.c
opt -load build/src/parcoachRMA.so -parcoach something.bc -o somethingINSTR.bc
clang -c somethingINSTR.bc
OMPI_CC=clang mpicc somethingINSTR.o -L./lib/ -lrma_analyzer
mpirun -np 4 --oversubscribe ./a.out
```
