#!/usr/bin/env bash
set -euo pipefail

: "${ANDROID_NDK_HOME:=${ANDROID_NDK_ROOT:-}}"
if [ -z "${ANDROID_NDK_HOME}" ]; then
  echo "Set ANDROID_NDK_HOME or ANDROID_NDK_ROOT" >&2
  exit 1
fi

API="${API:-23}"
ABI="arm64-v8a"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/out/$ABI"
SRC="$ROOT/src/libshared_kuboom_bootstrap_v1.c"
mkdir -p "$OUT"

CC="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android${API}-clang"
if [ ! -x "$CC" ]; then
  echo "Compiler not found: $CC" >&2
  echo "Adjust ANDROID_NDK_HOME/API or use Windows .bat." >&2
  exit 1
fi

"$CC" -shared -fPIC -O2 -Wall -Wextra \
  -fvisibility=hidden \
  "$SRC" \
  -llog -o "$OUT/libshared.so"

file "$OUT/libshared.so" || true
echo "Built: $OUT/libshared.so"
