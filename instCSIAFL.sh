#!/bin/bash

export PATH=$PATH:/home/xgzhu/data/csi-fuzz/test/csi-afl
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/xgzhu/data/csi-fuzz/test/csi-afl

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

mkdir $RESULT_DIR
mkdir ${RESULT_DIR}/CSI
# get addr files
./CSIDyninst -i ${TARGET} -b ${RESULT_DIR}/CSI -P
# instrument oracle
./CSIDyninst -i ${TARGET} -o ${RESULT_DIR}/CSI/${NAME}.oracle -b ${RESULT_DIR}/CSI -F
#instrument tracer
./CSIDyninst -i ${TARGET} -o ${RESULT_DIR}/CSI/${NAME}.tracer -b ${RESULT_DIR}/CSI -T
# # crasher
# ./CSIDyninst -i ${TARGET} -o ${RESULT_DIR}/CSI/${NAME}.crasher -b ${RESULT_DIR}/CSI -C
# # instrument trimmer
# ./CSIDyninst -i ${TARGET} -o ${RESULT_DIR}/CSI/${NAME}.trimmer -b ${RESULT_DIR}/CSI -M

sleep 1