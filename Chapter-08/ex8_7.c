#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>

static void print_cloexec(int fd, const char* msg);
int main()
{
    DIR* dp = opendir("/");
    if(dp == NULL)
    {
        perror("opendir error");
        exit(1);
    }
    int dir_fd = dirfd(dp);
    print_cloexec(dir_fd, "opendir");

    int fd = open("/", O_RDONLY, O_DIRECTORY);
    if(fd == -1)
    {
        perror("open error");
        exit(1);
    }
    print_cloexec(fd, "open");
    return 0;
}

void print_cloexec(int fd, const char* msg)
{
    int flags = fcntl(fd, F_GETFD);
    if(flags == -1)
    {
        perror("fcntl error");
        exit(1);
    }

    printf("%s: FD_CLOEXEC is %s\n", msg, flags & FD_CLOEXEC ? "on" : "off");
}

