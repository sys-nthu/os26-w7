/* 02-sweep.c: the read() loop of Part 2, for block sizes from 1 byte to 16 MiB.
 *
 *   ./02-sweep big.bin                     the whole sweep, one line per block size
 *   ./02-sweep big.bin BLOCK_SIZE BYTES    one run: read the first BYTES with BLOCK_SIZE per read()
 *
 * The sweep reads the file once first, so it is in the page cache: the timings measure
 * the read() calls and the copies, not the disk. Tiny blocks read only 16 MiB,
 * because 256 MiB one byte at a time would take about half a minute.
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }

/* the loop from the worksheet: read BYTES of the file, block_size bytes per call */
static void run(const char *path, size_t block_size, long long bytes, int print) {
    char *buf = malloc(block_size);
    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror(path); exit(1); }
    long long total = 0, calls = 0;
    double t0 = now();
    for (;;) {
        ssize_t n = read(fd, buf, block_size);   /* one system call */
        calls++;
        if (n <= 0) break;                       /* 0 means end of file */
        total += n;
        if (total >= bytes) break;
    }
    double t = now() - t0;
    close(fd); free(buf);
    if (print) printf("%10zu B %14lld %12.1f MB/s\n", block_size, calls, total / t / 1e6);
}

int main(int argc, char **argv) {
    if (argc != 2 && argc != 4) { fprintf(stderr, "usage: %s FILE [BLOCK_SIZE BYTES]\n", argv[0]); return 1; }
    if (argc == 4) { run(argv[1], strtoul(argv[2], NULL, 0), strtoll(argv[3], NULL, 0), 1); return 0; }
    run(argv[1], 1 << 20, 1LL << 40, 0);         /* warm up: bring the whole file into the page cache */
    printf("%12s %14s %17s\n", "block_size", "read() calls", "throughput");
    for (int e = 0; e <= 24; e += 2) {
        size_t bs = (size_t)1 << e;
        run(argv[1], bs, bs < 64 ? 16LL << 20 : 256LL << 20, 1);
    }
    return 0;
}
