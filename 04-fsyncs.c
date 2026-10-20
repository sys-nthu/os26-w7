/* 04-fsyncs.c: the price of durability.
 *
 *   ./04-fsyncs [N]        (default N = 1000)
 *
 * Writes N blocks of 4 KiB to fsync-test.bin in three ways:
 *   a. write() N times, then one fsync() at the end
 *   b. write() + fsync() after every block
 *   c. O_DIRECT: write() N times without the page cache, then one fsync()
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define FILE_NAME "fsync-test.bin"

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }

static double run(int n, int flags, int sync_each) {
    int fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC | flags, 0644);
    if (fd < 0) { perror("open " FILE_NAME); exit(1); }
    char *buf = aligned_alloc(4096, 4096);          /* O_DIRECT needs a page-aligned buffer */
    memset(buf, 'x', 4096);
    double t0 = now();
    for (int i = 0; i < n; i++) {
        if (write(fd, buf, 4096) != 4096) { perror("write"); exit(1); }
        if (sync_each) fsync(fd);
    }
    fsync(fd);
    double t = now() - t0;
    close(fd); free(buf); unlink(FILE_NAME);
    return t;
}

int main(int argc, char **argv) {
    int n = argc > 1 ? atoi(argv[1]) : 1000;
    double a = run(n, 0, 0), b = run(n, 0, 1), c = run(n, O_DIRECT, 0);
    printf("%d writes of 4 KiB\n", n);
    printf("a. write() x %d, then one fsync()   %9.2f ms  %9.1f us per write\n", n, a * 1e3, a / n * 1e6);
    printf("b. write() + fsync(), x %d          %9.2f ms  %9.1f us per write\n", n, b * 1e3, b / n * 1e6);
    printf("c. O_DIRECT write() x %d            %9.2f ms  %9.1f us per write\n", n, c * 1e3, c / n * 1e6);
    printf("b is %.0f times slower than a\n", b / a);
    return 0;
}
