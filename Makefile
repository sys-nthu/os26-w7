# Worksheet 7 hands-on: build every program used in the hands-on part.
#
#   ./setup.sh     install gcc, make, strace, dd, free, watch, git (run once)
#   make           build all programs (the number is the step in the hands-on part)
#   make big.bin   create the 1 GiB test file used by steps 1 to 3
#   make clean     remove the binaries and the test files
#
# -O2: the timings should measure the kernel and the caches, not an unoptimized loop.
CC      = gcc
CFLAGS  ?= -O2 -g -Wall -Wextra

PROGS := 01-coldread 02-sweep 04-fsyncs

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

big.bin:
	dd if=/dev/urandom of=$@ bs=1M count=1024 conv=fsync status=progress

clean:
	rm -f $(PROGS) *.o big.bin dirty.bin fsync-test.bin

.PHONY: all clean
