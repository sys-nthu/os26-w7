/* 01-coldread.c: cold vs warm page cache, and what readahead buys.
 *
 *   ./01-coldread big.bin
 *
 * Before every "cold" run, the program asks the kernel to drop FILE from the
 * page cache with posix_fadvise(POSIX_FADV_DONTNEED). That needs no root,
 * because the pages are clean (unchanged since they were read). mincore()
 * reports how much of the file is in the page cache at each point.
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define NRAND 20000                       /* random 4 KiB reads in the last test */

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }

/* the percentage of the file that is in the page cache right now */
static double resident(int fd, off_t size) {
    void *p = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    long pg = sysconf(_SC_PAGESIZE), n = (size + pg - 1) / pg, in = 0;
    unsigned char *v = malloc(n);
    mincore(p, size, v);
    for (long i = 0; i < n; i++) in += v[i] & 1;
    munmap(p, size); free(v);
    return 100.0 * in / n;
}

static void evict(int fd) { posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED); }

/* read the whole file in order, one 4 KiB page per pread() */
static double seq(int fd, off_t size) {
    char buf[4096];
    double t0 = now();
    for (off_t o = 0; o < size; o += sizeof buf) if (pread(fd, buf, sizeof buf, o) <= 0) break;
    return now() - t0;
}

/* NRAND reads of 4 KiB at random page offsets (always the same offsets) */
static double rnd(int fd, off_t size) {
    char buf[4096]; long pages = size / 4096;
    srand(42);
    double t0 = now();
    for (long i = 0; i < NRAND; i++) if (pread(fd, buf, sizeof buf, (off_t)(rand() % pages) * 4096) < 0) break;
    return now() - t0;
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s FILE   (try: make big.bin; %s big.bin)\n", argv[0], argv[0]); return 1; }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror(argv[1]); return 1; }
    struct stat st; fstat(fd, &st); off_t size = st.st_size;
    double mib = size / 1048576.0, t;

    evict(fd);
    printf("%s: %.0f MiB, %.1f %% in the page cache after dropping it\n\n", argv[1], mib, resident(fd, size));

    t = seq(fd, size);
    printf("1. cold, in order, readahead on   %8.3f s  %8.1f MiB/s   (now %.1f %% cached)\n", t, mib / t, resident(fd, size));
    t = seq(fd, size);
    printf("2. warm, in order                 %8.3f s  %8.1f MiB/s\n", t, mib / t);

    evict(fd); posix_fadvise(fd, 0, 0, POSIX_FADV_RANDOM);      /* turns readahead off for this file */
    t = seq(fd, size);
    printf("3. cold, in order, readahead off  %8.3f s  %8.1f MiB/s\n", t, mib / t);
    posix_fadvise(fd, 0, 0, POSIX_FADV_NORMAL);

    evict(fd);
    t = rnd(fd, size);
    printf("4. cold, %d random 4 KiB reads %8.3f s  %8.1f us per read   (now %.1f %% cached)\n", NRAND, t, t / NRAND * 1e6, resident(fd, size));
    t = rnd(fd, size);
    printf("5. warm, the same random reads    %8.3f s  %8.2f us per read\n", t, t / NRAND * 1e6);
    return 0;
}
