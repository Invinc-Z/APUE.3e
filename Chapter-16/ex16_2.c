#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <errno.h>

static void print_time_field(const char *name, time_t t)
{
    char buf[64];
    struct tm tm;

    if (localtime_r(&t, &tm) != NULL &&
        strftime(buf, sizeof(buf), "%F %T", &tm) > 0) {
        printf("%-10s = %lld (%s)\n", name, (long long)t, buf);
    } else {
        printf("%-10s = %lld\n", name, (long long)t);
    }
}

int main(void)
{
    int fd;
    struct stat st;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&st, 0, sizeof(st));
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    printf("fstat() on a socket:\n");
    printf("st_mode    = %#o", st.st_mode);
#ifdef S_ISSOCK
    if (S_ISSOCK(st.st_mode))
        printf(" (socket)");
#endif
    putchar('\n');

    printf("st_dev     = %lld\n", (long long)st.st_dev);
    printf("st_ino     = %lld\n", (long long)st.st_ino);
    printf("st_nlink   = %lld\n", (long long)st.st_nlink);
    printf("st_uid     = %lld\n", (long long)st.st_uid);
    printf("st_gid     = %lld\n", (long long)st.st_gid);
    printf("st_rdev    = %lld\n", (long long)st.st_rdev);
    printf("st_size    = %lld\n", (long long)st.st_size);
    printf("st_blksize = %lld\n", (long long)st.st_blksize);
    printf("st_blocks  = %lld\n", (long long)st.st_blocks);

    print_time_field("st_atime", st.st_atime);
    print_time_field("st_mtime", st.st_mtime);
    print_time_field("st_ctime", st.st_ctime);

    close(fd);
    return 0;
}
