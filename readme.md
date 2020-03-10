# Overview
CSI-Fuzz aims to fuzz binaries efficiently, which uses the idea of full-speed fuzzing. 

The current version is for non-PIE binaries.

## Install Dyninst
We use Dyninst to instrument target binaries. So firstly, install Dyninst [the branch](https://github.com/mxz297/dyninst).\<br>
[Instruction for installing Capstone, libunwind and Dyninst](https://github.com/iu-parfunc/ShadowGuard/blob/master/bazel.sh); \<br>
For the branch of Dyninst, use `csifuzz`.

```
git clone https://github.com/mxz297/dyninst.git
cd dyninst
git checkout csifuzz
```
Then, follow the instructions on [install instructions](https://github.com/mxz297/dyninst) to install Dyninst.

## Set up ENVs
```
export DYNINST_INSTALL=/path/to/dyninst/build/dir
export CSIFUZZ_PATH=/path/to/csi-fuzz

export DYNINSTAPI_RT_LIB=$DYNINST_INSTALL/lib/libdyninstAPI_RT.so
export LD_LIBRARY_PATH=$DYNINST_INSTALL/lib:$CSIFUZZ_PATH
export PATH=$PATH:$CSIFUZZ_PATH
```
## Install CSIFuzz
Enter the folder csi-afl.
Change DYN_ROOT in makefile accordingly. Then
```
make clean && make all
```

## instrument binary
in the folder of csi-afl:
```
tar -xzvf cflow.tar.gz
./CSIDyninst -i ./cflow/cflow -o ./output/cflowinst -b ./output/ -P
./CSIDyninst -i ./cflow/cflow -o ./output/cflowinst -b ./output/ -F
```

## run with inputs
```
./output/cflowinst -T ./cflow/seed_dir/test.c 
```

## segfault
### bison
    ./bison -vdty -b ./output/ input_file 
    indirect addr: 0x44a42a
    44a42a:   ff d0   callq  *%rax

### cflow
    ./cflow -T input_file
    indirect addr: 0x4135d9
    4135d9:   ff d0   callq  *%rax

### exiv2
    ./exiv2 -pt input_file
    ?condition-taken edge: 4402b7, 4403e0
    4402b7:   89 d8    mov    %ebx,%eax

### objdump
    ./objdump -d input_file
    ? condition-not-taken edge: 407397, 4073a7
    407397:    48 8b 05 1a 4a 49 00    mov    0x494a1a(%rip),%rax    # 89bdb8 <synthcount>

