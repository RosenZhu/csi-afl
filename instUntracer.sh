#!/bin/bash

export PATH=$PATH:/home/xgzhu/apps/CSI-Fuzz/untracer-dyninst/csi-afl/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/xgzhu/apps/CSI-Fuzz/untracer-dyninst/csi-afl/
# $0: runfuzz.sh itself; $1: path to output directory
# $2: fuzzing seed dir;
# $3: path to target binary;  ${@:4}: parameters running targets
# bash runfuzz.sh ../outputs/becread1 ../target-bins/untracer_bins/binutils/readelf ../target-bins/untracer_bins/binutils/seed_dir/ -a @@

OUTDIR=${1}_untracer
SEEDS=$2
TARGET=$3
FUZZTIME=$4
WITHDICT=$5
TIMEOUT=$6
PARAMS=`echo ${@:7}`


NAME=`echo ${TARGET##*/}`
# INSTNAME=${NAME}_inst


mkdir $OUTDIR
mkdir ${OUTDIR}/CSI
# get addr files
./CSIDyninst -i ${TARGET} -b ${OUTDIR}/CSI -P
# instrument oracle
./CSIDyninst -i ${TARGET} -o ${OUTDIR}/CSI/${NAME}.oracle -b ${OUTDIR}/CSI -F
#instrument tracer
./CSIDyninst -i ${TARGET} -o ${OUTDIR}/CSI/${NAME}.tracer -b ${OUTDIR}/CSI -T
# instrument trimmer
./CSIDyninst -i ${TARGET} -o ${OUTDIR}/CSI/${NAME}.trimmer -b ${OUTDIR}/CSI -M
# instrument crasher
./CSIDyninst -i ${TARGET} -o ${OUTDIR}/CSI/${NAME}.crasher -b ${OUTDIR}/CSI -C

sleep 1
