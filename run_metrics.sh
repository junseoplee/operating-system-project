#!/bin/bash

INPUT=$1
OUTPUT=$2
NUM_PROCS=$3
NUM_THREADS=$4

if [ ! -f "$INPUT" ]; then
  echo "입력 파일 $INPUT 이(가) 존재하지 않습니다."
  exit 1
fi

if [ -z "$NUM_PROCS" ] || [ -z "$NUM_THREADS" ]; then
  echo "사용법: $0 <입력파일> <출력파일> <프로세스 수> <스레드 수>"
  exit 1
fi

echo -e "\n1. 프로그램 실행 + 내부 측정 결과"
./compressor "$INPUT" "$OUTPUT" "$NUM_PROCS" "$NUM_THREADS"

echo -e "\n2. 시스템콜 측정 결과"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "(Linux 환경에서 strace 사용)"
  strace -c ./compressor "$INPUT" "$OUTPUT" "$NUM_PROCS" "$NUM_THREADS"
else
  echo "시스템콜 분석은 현재 OS에서 지원되지 않습니다."
fi
