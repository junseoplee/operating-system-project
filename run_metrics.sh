#!/bin/bash

INPUT=$1
OUTPUT=$2
NUM_PROC=$3
NUM_THREAD=$4
MODE=$5

if [ ! -f "$INPUT" ]; then
  echo "입력 파일 $INPUT 이 존재하지 않습니다."
  exit 1
fi

if [ -z "$NUM_PROC" ] || [ -z "$NUM_THREAD" ]; then
  echo "사용법: $0 <입력파일> <출력파일> <프로세스 수> <스레드 수> [mutex|semaphore]"
  exit 1
fi

if [ "$MODE" == "semaphore" ]; then
  BINARY="./compressor_semaphore"
elif [ "$MODE" == "mutex" ]; then
  BINARY="./compressor_mutex"
else
  BINARY="./compressor"  # 기본 버전 (동기화 없음 또는 자유 실험용)
fi

echo ""
echo "1. 프로그램 실행 + 내부 측정 결과"
$BINARY "$INPUT" "$OUTPUT" "$NUM_PROC" "$NUM_THREAD"

echo ""
echo "2. 시스템콜 측정 결과"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "(Linux 환경에서 strace 사용)"
  strace -c $BINARY "$INPUT" "$OUTPUT" "$NUM_PROC" "$NUM_THREAD"
else
  echo "시스템콜 분석은 현재 OS에서 지원되지 않습니다."
fi
