#!/bin/bash

INPUT=$1
OUTPUT=$2
NUM_WORKERS=$3
MODE=$4  # optional: thread or process
DEFAULT_MODE="process"

# 기본 실행 대상
BINARY="./compressor"

# 스레드 모드라면 실행 파일 변경
if [ "$MODE" == "thread" ]; then
  BINARY="./compressor_threaded"
elif [ "$MODE" != "" ] && [ "$MODE" != "$DEFAULT_MODE" ]; then
  echo "❌ 잘못된 실행 모드: $MODE (허용: process, thread)"
  exit 1
fi

if [ ! -f "$INPUT" ]; then
  echo "❌ 입력 파일 $INPUT 이(가) 존재하지 않습니다."
  exit 1
fi

if [ -z "$NUM_WORKERS" ]; then
  echo "❌ 사용법: $0 <입력파일> <출력파일> <프로세스/스레드 수> [process|thread]"
  exit 1
fi

echo -e "\n▶ 1. 프로그램 실행 + 내부 측정 결과"
$BINARY "$INPUT" "$OUTPUT" "$NUM_WORKERS"

echo -e "\n▶ 2. 시스템콜 측정 결과"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "(Linux 환경에서 strace 사용)"
  strace -c $BINARY "$INPUT" "$OUTPUT" "$NUM_WORKERS"
else
  echo "⚠️ 시스템콜 분석은 현재 OS에서 지원되지 않습니다."
fi
