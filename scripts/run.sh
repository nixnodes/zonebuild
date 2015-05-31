#!/bin/sh

BASE_PATH=`dirname ${0}`

rm -f ${BASE_PATH}/build/tier1/* 

${BASE_PATH}/build_tier1_arpa.sh 10.0.0.0_8 1
${BASE_PATH}/build_tier1_arpa.sh 172.20.0.0_16 1
${BASE_PATH}/build_tier1_arpa.sh 172.22.0.0_16 1
${BASE_PATH}/build_tier1_arpa.sh 172.23.0.0_16 1
${BASE_PATH}/build_tier1_arpa.sh 172.31.0.0_16 1