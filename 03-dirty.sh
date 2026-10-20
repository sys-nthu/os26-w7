#!/usr/bin/env bash
# 03-dirty.sh: write 256 MiB without fsync and watch how long it stays dirty.
#
#   ./03-dirty.sh
#
# Prints the Dirty and Writeback lines of /proc/meminfo every 0.5 s for 40 s.
# dd returns long before the data is on the disk. Run it again and type "sync"
# in another terminal while it is counting.
set -euo pipefail
show() { awk '/^Dirty:|^Writeback:/ {printf "%s %6d MiB   ", $1, $2 / 1024} END {print ""}' /proc/meminfo; }
echo "before:   $(show)"
start=$(date +%s.%N)
dd if=/dev/zero of=dirty.bin bs=1M count=256 status=none
end=$(date +%s.%N)
awk -v a="$end" -v b="$start" 'BEGIN { printf "dd returned after %.3f s\n", a - b }'
for i in $(seq 1 80); do
  awk -v i="$i" 'BEGIN { printf "%5.1f s   ", i * 0.5 }'; show
  sleep 0.5
done
rm -f dirty.bin
