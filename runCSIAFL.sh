#!/bin/bash

export PATH=$PATH:/home/xgzhu/data/csi-fuzz/test/csi-afl
export AFL_NO_UI=1 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/xgzhu/data/csi-fuzz/test/csi-afl
# $0: runfuzz.sh itself; $1: path to output directory
# $2: fuzzing seed dir;
# $3: path to target binary;  ${@:4}: parameters running targets
# bash runfuzz.sh ../outputs/becread1 ../target-bins/untracer_bins/binutils/readelf ../target-bins/untracer_bins/binutils/seed_dir/ -a @@

OUTDIR=${1}
SEEDS=$2
TARGET=$3
WITHDICT=$4
TIMEOUT=$5
# afl or aflfast
FUZZER=$6
COUNT_OUT=$7
PARAMS=`echo ${@:8}`


NAME=`echo ${TARGET##*/}`
# INSTNAME=${NAME}_inst

SEED_DIR=${OUTDIR}_${FUZZER}/out/queue
# output of count edges
RESULT_DIR=${COUNT_OUT}/${NAME}_${FUZZER}

if [ "$WITHDICT"x = "nodict"x ]
then
    COMMD="./csi-afl -i $SEED_DIR -o ${RESULT_DIR}/out -t $TIMEOUT -- ${TARGET} $PARAMS"
else
    COMMD="./csi-afl -i $SEED_DIR -o ${RESULT_DIR}/out -x ${WITHDICT} -t $TIMEOUT -- ${TARGET} $PARAMS"
fi

${COMMD}

rm -rf ${RESULT_DIR}/out/queue

# (
#     ${COMMD}
# )&
# sleep $FUZZTIME
# # ctrl-c
# ps -ef | grep "$COMMD" | grep -v 'grep' | awk '{print $2}' | xargs kill -2

rm ${RESULT_DIR}/CSI/${NAME}.oracle
# rm ${RESULT_DIR}/CSI/${NAME}.trimmer
rm ${RESULT_DIR}/CSI/${NAME}.tracer
# rm ${RESULT_DIR}/CSI/${NAME}.crasher

sleep 1

