#include "apue.h"
#include <fcntl.h>
#include <sys/uio.h>
#include <time.h>

#define NLOOPS 200000

static long long nsec_diff(const struct timespec *a, const struct timespec *b) {
    return (b->tv_sec - a->tv_sec) * 1000000000LL + (b->tv_nsec - a->tv_nsec);
}

static long long test_writev(int fd, const char *p1, size_t n1, const char *p2,
        size_t n2) {
    struct iovec iov[2];
    struct timespec start, end;
    int i;

    iov[0].iov_base = (void *)p1;
    iov[0].iov_len = n1;
    iov[1].iov_base = (void *)p2;
    iov[1].iov_len = n2;

    if (clock_gettime(CLOCK_MONOTONIC, &start) < 0)
        err_sys("clock_gettime error");

    for (i = 0; i < NLOOPS; i++) {
        if (writev(fd, iov, 2) != (ssize_t)(n1 + n2))
            err_sys("writev error");
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end) < 0)
        err_sys("clock_gettime error");

    return nsec_diff(&start, &end);
}

static long long test_write_copy(int fd, const char *p1, size_t n1,
        const char *p2, size_t n2, char *buf) {
    struct timespec start, end;
    int i;

    if (clock_gettime(CLOCK_MONOTONIC, &start) < 0)
        err_sys("clock_gettime error");

    for (i = 0; i < NLOOPS; i++) {
        memcpy(buf, p1, n1);
        memcpy(buf + n1, p2, n2);
        if (write(fd, buf, n1 + n2) != (ssize_t)(n1 + n2))
            err_sys("write error");
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end) < 0)
        err_sys("clock_gettime error");

    return nsec_diff(&start, &end);
}

int main(void) {
    int fd;
    size_t total, n1, n2;
    char *p1, *p2, *buf;
    long long t_writev, t_copy;

    fd = open("/dev/null", O_WRONLY);
    if (fd < 0)
        err_sys("open /dev/null error");

    printf("%10s %15s %15s %12s\n", "bytes", "writev(ns)", "copy+write(ns)",
            "faster");

    for (total = 16; total <= 32768; total *= 2) {
        n1 = total / 2;
        n2 = total - n1;

        p1 = malloc(n1);
        p2 = malloc(n2);
        buf = malloc(total);
        if (p1 == NULL || p2 == NULL || buf == NULL)
            err_sys("malloc error");

        memset(p1, 'A', n1);
        memset(p2, 'B', n2);

        t_writev = test_writev(fd, p1, n1, p2, n2);
        t_copy = test_write_copy(fd, p1, n1, p2, n2, buf);

        printf("%10zu %15lld %15lld %12s\n", total, t_writev, t_copy,
                (t_writev < t_copy) ? "writev" : "copy");

        free(p1);
        free(p2);
        free(buf);
    }

    close(fd);
    exit(0);
}
