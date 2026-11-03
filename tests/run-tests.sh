#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, clang-dos
#
# Build the examples and run them on EmuTOS in Hatari, on an ST (68000),
# an STe, a TT (68030) and a Falcon (68030).
#
#   CC=.../bin/clang EMUTOS_DIR=... tests/run-tests.sh [OUT]
#
# CC is clang from the clang-dos toolchain (with the m68k-atari-tos
# sysroot in place, or SYSROOT set). OUT (default ./tests-out) gets the
# programs, their output and screenshots of the GEM example.
set -e
here=$(cd "$(dirname "$0")/.." && pwd)
CC=${CC:-clang}
out=${1:-$PWD/tests-out}
mkdir -p "$out"
flags="--target=m68k-atari-tos ${SYSROOT:+--sysroot=$SYSROOT}"
run=$here/tools/hatari-run.sh
fail=0

ok() { echo "ok   $*"; }
bad() { echo "FAIL $*"; fail=1; }

# Build.
for cpu in 68000 68030; do
  for opt in -O0 -O2 -Os; do
    n=$cpu$opt
    $CC $flags -mcpu=$cpu $opt -Wall "$here/examples/abitest.c" -o "$out/abi$n.tos" &&
      ok "build abitest $n" || bad "build abitest $n"
  done
done
$CC $flags -Os "$here/examples/hello.c" -o "$out/hello.tos" && ok "build hello" || bad "build hello"
$CC $flags -Os "$here/examples/sysinfo.c" -o "$out/sysinfo.tos" && ok "build sysinfo" || bad "build sysinfo"
$CC $flags -Os "$here/examples/gemhello.c" -o "$out/gemhello.prg" && ok "build gemhello" || bad "build gemhello"

check() { # name machine program text [-s shot]
  name=$1 machine=$2 prog=$3 text=$4
  shift 4
  if "$run" -m "$machine" -w "$text" -T 180 "$@" "$prog" >"$out/$name.log" 2>&1; then
    if grep -q FAIL "$out/$name.log"; then
      bad "$name"; grep FAIL "$out/$name.log"
    else
      ok "$name"
    fi
  else
    bad "$name (no \"$text\")"; tail -5 "$out/$name.log"
  fi
}

for m in st ste tt falcon; do
  check "hello-$m" $m "$out/hello.tos" "Hello from clang on TOS"
  check "sysinfo-$m" $m "$out/sysinfo.tos" "sysinfo done"
done
for n in 68000-O0 68000-O2 68000-Os; do
  check "abitest-$n-st" st "$out/abi$n.tos" "abitest:"
done
for n in 68030-O2 68030-Os; do
  check "abitest-$n-tt" tt "$out/abi$n.tos" "abitest:"
  check "abitest-$n-falcon" falcon "$out/abi$n.tos" "abitest:"
done
check "gemhello-st" st "$out/gemhello.prg" "gemhello: alert" -s "$out/gemhello-st.png"
check "gemhello-falcon" falcon "$out/gemhello.prg" "gemhello: alert" -s "$out/gemhello-falcon.png"
exit $fail
