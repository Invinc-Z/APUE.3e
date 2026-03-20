#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static void print_revents(short revents)
{
    printf("parent: revents = 0x%x", revents);

    if (revents & POLLIN)   printf(" POLLIN");
    if (revents & POLLOUT)  printf(" POLLOUT");
    if (revents & POLLERR)  printf(" POLLERR");
    if (revents & POLLHUP)  printf(" POLLHUP");
    if (revents & POLLNVAL) printf(" POLLNVAL");

#ifdef POLLRDHUP
    if (revents & POLLRDHUP) printf(" POLLRDHUP");
#endif

    printf("\n");
}

int main(void)
{
    int fd[2];
    pid_t pid;
    struct pollfd pfd;
    int n;
    char ch = 'X';

    if (pipe(fd) < 0) {
        perror("pipe");
        exit(1);
    }

    /* Ignore SIGPIPE so write() reports EPIPE instead of killing the process */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        /* child: reader, then close read end */
        close(fd[1]);
        sleep(1);
        close(fd[0]);
        _exit(0);
    }

    /* parent: writer */
    close(fd[0]);

    pfd.fd = fd[1];
    pfd.events = POLLOUT;

    for (;;) {
        printf("parent: calling poll ...\n");
        fflush(stdout);

        pfd.revents = 0;
        n = poll(&pfd, 1, -1);
        if (n < 0) {
            perror("poll");
            exit(1);
        }

        printf("parent: poll returned %d\n", n);
        print_revents(pfd.revents);

        if (write(fd[1], &ch, 1) < 0) {
            printf("parent: write error: %s\n", strerror(errno));
            if (errno == EPIPE) {
                printf("parent: got EPIPE (read end is closed)\n");
                break;
            }
            exit(1);
        } else {
            printf("parent: write of 1 byte succeeded\n");
        }

        sleep(1);
    }

    close(fd[1]);
    wait(NULL);
    return 0;
}
