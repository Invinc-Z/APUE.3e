#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

int main(void)
{
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid > 0) {
        /* parent */
        printf("parent:\n");
        printf("  pid   = %ld\n", (long)getpid());
        printf("  pgrp  = %ld\n", (long)getpgrp());
        printf("  sid   = %ld\n", (long)getsid(0));
        exit(0);
    }

    /* child */
    printf("child (before setsid):\n");
    printf("  pid   = %ld\n", (long)getpid());
    printf("  pgrp  = %ld\n", (long)getpgrp());
    printf("  sid   = %ld\n", (long)getsid(0));

    if (setsid() < 0) {
        perror("setsid");
        exit(1);
    }

    printf("\nchild (after setsid):\n");
    printf("  pid   = %ld\n", (long)getpid());
    printf("  pgrp  = %ld\n", (long)getpgrp());
    printf("  sid   = %ld\n", (long)getsid(0));

    /* verify no controlling terminal */
    int fd = open("/dev/tty", O_RDONLY);
    if (fd < 0) {
        perror("open /dev/tty");
        printf("=> no controlling terminal\n");
    } else {
        printf("=> still has controlling terminal (unexpected)\n");
        close(fd);
    }

    exit(0);
}

