# Worksheet 7 hands-on: buffering and caching

The programs for the hands-on part of the Week 7 worksheet
(<https://os26.sys-nthu.tw/weeks/w8.html#hands-on>), ready to build.
You still have to run each step yourself and think about what you see.

## Quick start

Open this repository in a GitHub Codespace
(<https://codespaces.new/sys-nthu/os26-w7>); it runs `setup.sh` and `make`
for you when it is created. On any other Linux machine, clone it and run:

```sh
./setup.sh      # once: checks gcc, make, strace, dd, free, watch, git and installs what is missing
make            # builds 01-coldread, 02-sweep, 04-fsyncs
make big.bin    # creates the 1 GiB test file for steps 1 to 3 (about 1 GiB of disk space)
```

`setup.sh` uses `apt-get` (with `sudo` if you are not root). On another
distribution, install the same tools by hand and run `make`.

None of the steps needs root. To make a file "cold", the programs ask the
kernel to drop it from the page cache with `posix_fadvise(POSIX_FADV_DONTNEED)`,
which any user may do for a file they can read. Your numbers will differ from
the worksheet's: a Codespace runs on a cloud disk, not on a local NVMe SSD.
Explaining *why* they differ is part of the exercise.

## What each step uses

Each file name starts with the number of the step in the hands-on part that uses it.

1. **How is RAM being used?** No program to build. Run `free -h`, then
   `cat big.bin > /dev/null`, then `free -h` again, and compare the
   *free*, *buff/cache* and *available* columns.
2. **Cold vs. warm, with and without readahead.** `./01-coldread big.bin`
   drops the file from the page cache, then reads it (1) cold and in order,
   (2) warm, (3) cold with readahead turned off (`POSIX_FADV_RANDOM`),
   (4) cold with 20,000 random 4 KiB reads and (5) warm with the same reads.
   It prints how much of the file is cached after each step (`mincore()`).
3. **The buffer-size sweep.** `./02-sweep big.bin` runs the `read()` loop
   from Part 2 of the worksheet for block sizes from 1 byte to 16 MiB and
   prints the number of `read()` calls and the throughput. Count the system
   calls of the two extremes with
   `strace -c ./02-sweep big.bin 1 100000` (100,000 bytes, 1 byte per call) and
   `strace -c ./02-sweep big.bin 1048576 104857600` (100 MiB, 1 MiB per call).
4. **Watch dirty pages.** `./03-dirty.sh` writes 256 MiB with `dd` (no
   `fsync`), prints how long `dd` took, then prints the `Dirty:` and
   `Writeback:` lines of `/proc/meminfo` every 0.5 s for 40 s. Run it a second
   time and type `sync` in another terminal while it counts.
5. **How long does fsync() take?** `./04-fsyncs` writes 1000 blocks of 4 KiB
   three ways: one `fsync()` at the end, an `fsync()` after every block, and
   `O_DIRECT`. `./04-fsyncs 100` uses fewer blocks if your disk is slow.
6. **How cat copies.** No program to build.
   `echo hello > A; strace -e trace=read,write,copy_file_range cat A > B`,
   then the same without `> B`. Which system call does `cat` use each time?

`make clean` removes the programs and the test files.
