#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, clang-dos
#
# Run an Atari program in Hatari with EmuTOS, headless, and print what it
# writes to the console.
#
#   hatari-run.sh [options] PROGRAM [FILE...]
#
#   -m MACHINE  st (default), megast, ste, megaste, tt or falcon
#   -t TOS      the TOS image (default: EmuTOS from $EMUTOS_DIR, picked
#               for the machine)
#   -w TEXT     wait until the console output has TEXT (default: run
#               until the time limit)
#   -s FILE     save a screenshot (PNG) when the wait is over
#   -T SECONDS  give up after this long (default 120)
#   -r RES      TOS resolution: low, med, high, ttlow or ttmed
#
# PROGRAM is copied to drive C: and started by the desktop (TOS 1.04 and
# later, and EmuTOS, can start a program at boot). Name it .TOS or .TTP
# for a text program, .PRG for a GEM program. The FILEs are copied to C:
# too. Everything the program writes with GEMDOS or the BIOS to the
# console, and with NatFeats (natfeats.h) to the host, goes to standard
# output; the exit status is 0 if the wait text
# was seen (or, without -w, if Hatari ran until the time limit).
#
# EmuTOS is free software (GPL) and can be downloaded from
# https://emutos.sourceforge.io; set EMUTOS_DIR to the directory with its
# etos192us.img, etos256us.img and etos512us.img. No Atari ROM is needed.
set -e
machine=st tos= wait= shot= timeout=120 res=
while getopts m:t:w:s:T:r: opt; do
  case $opt in
  m) machine=$OPTARG ;;
  t) tos=$OPTARG ;;
  w) wait=$OPTARG ;;
  s) shot=$OPTARG ;;
  T) timeout=$OPTARG ;;
  r) res=$OPTARG ;;
  *) sed -n '5,25p' "$0"; exit 2 ;;
  esac
done
shift $((OPTIND - 1))
prog=${1:?usage: hatari-run.sh [options] PROGRAM [FILE...]}
shift

if [ -z "$tos" ]; then
  dir=${EMUTOS_DIR:-$HOME/emutos}
  case $machine in
  st | megast) img=etos192us.img ;;
  ste | megaste) img=etos256us.img ;;
  *) img=etos512us.img ;;
  esac
  tos=$(find "$dir" -name "$img" 2>/dev/null | head -1)
  [ -n "$tos" ] || { echo "no $img in $dir; set EMUTOS_DIR or use -t" >&2; exit 2; }
fi

work=$(mktemp -d)
trap 'kill $pid 2>/dev/null; rm -rf "$work"' EXIT
mkdir "$work/c" "$work/shots"
name=$(basename "$prog" | tr a-z A-Z)
cp "$prog" "$work/c/$name"
for f in "$@"; do cp "$f" "$work/c/$(basename "$f" | tr a-z A-Z)"; done

set -- --tos "$tos" --machine "$machine" --fast-forward yes --fast-boot yes \
  --sound off --statusbar no --confirm-quit no --mousewarp no \
  -d "$work/c" --auto "C:\\$name" --conout 2 --natfeats yes \
  --screenshot-dir "$work/shots" --cmd-fifo "$work/fifo" --log-level error
case $machine in
tt | falcon) set -- "$@" --memsize 14 ;;
*) set -- "$@" --memsize 1024 ;;
esac
[ -n "$res" ] && set -- "$@" --tos-res "$res"

SDL_VIDEODRIVER=${SDL_VIDEODRIVER:-dummy} SDL_AUDIODRIVER=${SDL_AUDIODRIVER:-dummy} \
  hatari "$@" >"$work/out" 2>"$work/err" &
pid=$!

end=$(($(date +%s) + timeout))
found=1
while kill -0 $pid 2>/dev/null; do
  if [ -n "$wait" ]; then
    cat "$work/out" "$work/err" 2>/dev/null | grep -q -- "$wait" && { found=0; break; }
  fi
  if [ "$(date +%s)" -ge "$end" ]; then
    [ -z "$wait" ] && found=0
    break
  fi
  sleep 1
done
if [ -n "$shot" ] && kill -0 $pid 2>/dev/null; then
  sleep 1
  echo "hatari-shortcut screenshot" >"$work/fifo"
  for i in 1 2 3 4 5 6 7 8 9 10; do
    ls "$work/shots"/*.png >/dev/null 2>&1 && break
    sleep 1
  done
  cp "$work/shots"/*.png "$shot" 2>/dev/null || echo "no screenshot" >&2
fi
kill -0 $pid 2>/dev/null && echo "hatari-shortcut quit" >"$work/fifo"
sleep 1
kill $pid 2>/dev/null || true
# The console output is VT52 text with CR LF line ends.
# Hatari's own messages and NatFeats output are in err.
tr -d '\r' <"$work/out"
cat "$work/err"
[ $found = 0 ] || echo "hatari-run.sh: timed out" >&2
rm -rf "$work"
trap - EXIT
exit $found
