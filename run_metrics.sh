#!/bin/bash

INPUT=$1
OUTPUT=$2
BINARY="./compressor"

if [ ! -f "$INPUT" ]; then
  echo "❌ 입력 파일 $INPUT 이(가) 존재하지 않습니다."
  exit 1
fi

echo -e "\n▶ 1. 프로그램 실행 + 내부 측정 결과"
$BINARY "$INPUT" "$OUTPUT"

echo -e "\n▶ 2. 시스템콜 측정 결과"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "(Linux 환경에서 strace 사용)"
  strace -c $BINARY "$INPUT" "$OUTPUT"
else
  echo "⚠️ 이 스크립트는 현재 OS에서 지원되지 않습니다."
fi
