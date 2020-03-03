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
tar -xzvf crash_inputs.tar.gz
tar -xzvf normal_inputs.tar.gz
tar -xzvf tcpdump.tar.gz
./CSIDyninst -i ./tcpdump -o ./output/tcpinst -b ./output -P
./CSIDyninst -i ./tcpdump -o ./output/tcpinst -b ./output -F
```

## run crash inputs
```
./output/tcpinst -nr ./crashes/input_name
```

## run normal inputs
```
./output/tcpinst -nr ./queue/input_name
```


## Run fuzzing

Fuzzing the target binary.

```
./csi-afl -i /path/to/seeds -o /path/to/output -t 500 -- /path/to/target/binary [params]
```