#!/usr/bin/env bash
# Worksheet 7 hands-on: make sure the tools used in the hands-on part are installed.
# Safe to run more than once.
set -euo pipefail

# command to check  ->  Debian/Ubuntu package that provides it
declare -A PKG=(
  [gcc]=gcc
  [make]=make
  [strace]=strace
  [dd]=coreutils
  [free]=procps
  [watch]=procps
  [git]=git
)

missing=()
for cmd in "${!PKG[@]}"; do
  if command -v "$cmd" >/dev/null 2>&1; then
    printf '  ok       %s\n' "$cmd"
  else
    printf '  missing  %s\n' "$cmd"
    missing+=("${PKG[$cmd]}")
  fi
done

if ((${#missing[@]})); then
  if command -v apt-get >/dev/null 2>&1; then
    SUDO=""
    [ "$(id -u)" -ne 0 ] && SUDO="sudo"
    echo "Installing: ${missing[*]}"
    $SUDO apt-get update -qq
    $SUDO apt-get install -y -qq "${missing[@]}"
  else
    echo "This script only knows apt-get. Please install these yourself: ${missing[*]}" >&2
    exit 1
  fi
fi

echo
echo "Setup complete. Next: make"
