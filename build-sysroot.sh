#!/bin/sh
# Build the m68k-atari-tos sysroot: picolibc with its TOS layer, the
# compiler-rt builtins, the TOS headers (osbind.h, aes.h, vdi.h, ...) and
# libgem.a, all for the 68000 (they run on every Atari).
#
#   BIN=... LLVM_SRC=... PICOLIBC_SRC=... ./build-sysroot.sh OUT
#
#   BIN           directory with clang, llvm-ar and the other LLVM tools
#   LLVM_SRC      clang-dos/llvm checkout (compiler-rt)
#   PICOLIBC_SRC  clang-dos/picolibc checkout
#   WORK          build directory (default: ./build-tos)
#
# OUT gets include/, lib/crt0.o, lib/tos.ld, lib/libc.a, lib/libm.a,
# lib/libgem.a and lib/libclang_rt.builtins.a. Everything in it can be
# redistributed.
set -e
out=${1:?usage: build-sysroot.sh OUT}
: "${BIN:?set BIN}" "${LLVM_SRC:?set LLVM_SRC}" "${PICOLIBC_SRC:?set PICOLIBC_SRC}"
here=$(cd "$(dirname "$0")" && pwd)
WORK=${WORK:-$PWD/build-tos}
mkdir -p "$out/include" "$out/lib" "$WORK"
out=$(cd "$out" && pwd) WORK=$(cd "$WORK" && pwd)
PATH=$BIN:$PATH
export PATH

# picolibc, with its malloc on top of the TOS layer's sbrk.
rm -rf "$WORK/picolibc" "$WORK/picolibc-install"
meson setup "$WORK/picolibc" "$PICOLIBC_SRC" \
  --cross-file "$PICOLIBC_SRC/scripts/cross-clang-m68k-atari-tos.txt" \
  -Dos-tos=true -Dmultilib=false -Dpicocrt=false \
  -Dsemihost=false -Dtests=false -Dposix-console=true -Dsingle-thread=true \
  -Dthread-local-storage=false -Dspecsdir=none \
  --prefix="$WORK/picolibc-install" >"$WORK/picolibc.log" 2>&1 ||
  { tail -20 "$WORK/picolibc.log"; exit 1; }
ninja -C "$WORK/picolibc" install >>"$WORK/picolibc.log" 2>&1 ||
  { grep -E 'error|FAILED' "$WORK/picolibc.log" | head -20; exit 1; }
cp -R "$WORK/picolibc-install/include/." "$out/include/"
for f in crt0.o tos.ld libc.a libm.a; do
  cp "$WORK/picolibc-install/lib/$f" "$out/lib/"
done

# compiler-rt builtins: 32-bit multiply and divide (the 68000 has only
# 16-bit ones), 64-bit arithmetic and soft float.
cmake -G Ninja -S "$LLVM_SRC/compiler-rt/lib/builtins" -B "$WORK/builtins" \
  -DCMAKE_SYSTEM_NAME=Generic -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER="$BIN/clang" -DCMAKE_ASM_COMPILER="$BIN/clang" \
  -DCMAKE_CXX_COMPILER="$BIN/clang++" \
  -DCMAKE_C_COMPILER_TARGET=m68k-atari-tos \
  -DCMAKE_CXX_COMPILER_TARGET=m68k-atari-tos \
  -DCMAKE_ASM_COMPILER_TARGET=m68k-atari-tos \
  -DCMAKE_C_FLAGS="-mcpu=68000" -DCMAKE_ASM_FLAGS="-mcpu=68000" \
  -DCMAKE_AR="$BIN/llvm-ar" -DCMAKE_RANLIB="$BIN/llvm-ranlib" \
  -DCMAKE_NM="$BIN/llvm-nm" \
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
  -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON -DCOMPILER_RT_BAREMETAL_BUILD=ON \
  -DCOMPILER_RT_OS_DIR=tos -DLLVM_CMAKE_DIR="$BIN/../lib/cmake/llvm" \
  >"$WORK/builtins.log" 2>&1 ||
  { tail -20 "$WORK/builtins.log"; exit 1; }
ninja -C "$WORK/builtins" >>"$WORK/builtins.log" 2>&1 ||
  { grep -E 'error|FAILED' "$WORK/builtins.log" | head -20; exit 1; }
cp "$WORK/builtins/lib/tos/libclang_rt.builtins-m68k.a" \
   "$out/lib/libclang_rt.builtins.a"

# The TOS headers and the GEM library.
cp -R "$here/include/." "$out/include/"
rm -rf "$WORK/gem" && mkdir -p "$WORK/gem"
for f in "$here"/gem/*.c; do
  "$BIN/clang" --target=m68k-atari-tos -mcpu=68000 -Os -Wall -Werror \
    --sysroot="$out" -ffunction-sections -c "$f" \
    -o "$WORK/gem/$(basename "$f" .c).o"
done
rm -f "$out/lib/libgem.a"
"$BIN/llvm-ar" rcs "$out/lib/libgem.a" "$WORK"/gem/*.o
echo "sysroot written to $out"
