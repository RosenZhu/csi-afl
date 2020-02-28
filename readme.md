# Overview
[UnTracer](https://github.com/FoRTE-Research/UnTracer-AFL) with a shared memory to record the examined blocks.

The current version is for non-PIE binaries.

## Install Dyninst
We use Dyninst to instrument target binaries. So firstly, install Dyninst [the branch](https://github.com/mxz297/dyninst).

```
git clone https://github.com/mxz297/dyninst.git
cd dyninst
git checkout fuzzing
```
Then, follow the instructions on [install instructions](https://github.com/mxz297/dyninst) to install Dyninst.

## Set up ENVs
```
export DYNINST_INSTALL=/path/to/dyninst/build/dir
export CSIFUZZ_PATH=/path/to/untracer

export DYNINSTAPI_RT_LIB=$DYNINST_INSTALL/lib/libdyninstAPI_RT.so
export LD_LIBRARY_PATH=$DYNINST_INSTALL/lib:$CSIFUZZ_PATH
export PATH=$PATH:$CSIFUZZ_PATH
```
## Install untracer
Enter the folder.
Change DYN_ROOT in makefile accordingly. Then
```
make clean && make all
```

## Run fuzzing

Fuzzing the target binary.

```
./untracer-dyn -i /path/to/seeds -o /path/to/output -t 500 -- /path/to/target/binary [params]
```