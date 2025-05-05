#!/bin/bash

INPUT=$1
OUTPUT=$2
NUM_PROC=$3
BINARY="./compressor"

if [ ! -f "$INPUT" ]; then
  echo "❌ 입력 파일 $INPUT 이(가) 존재하지 않습니다."
  exit 1
fi

if [ -z "$NUM_PROC" ]; then
  echo "❌ 사용법: $0 <입력파일> <출력파일> <프로세스 수>"
  exit 1
fi

echo -e "\n▶ 1. 프로그램 실행 + 내부 측정 결과"
$BINARY "$INPUT" "$OUTPUT" "$NUM_PROC"

echo -e "\n▶ 2. 시스템콜 측정 결과"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "(Linux 환경에서 strace 사용)"
  strace -c $BINARY "$INPUT" "$OUTPUT" "$NUM_PROC"
else
  echo "⚠️ 시스템콜 분석은 현재 OS에서 지원되지 않습니다."
fi
