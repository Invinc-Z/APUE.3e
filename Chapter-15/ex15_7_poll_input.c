#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_revents(short revents)
{
    printf("parent: revents = 0x%x", revents);

    if (revents & POLLIN)   printf(" POLLIN");
    if (revents & POLLHUP)  printf(" POLLHUP");
    if (revents & POLLERR)  printf(" POLLERR");
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
    char buf[1024];
    int n;
    struct pollfd pfd;

    if (pipe(fd) < 0) {
        perror("pipe");
        exit(1);
    }

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        /* child: writer */
        close(fd[0]); /* close read end */

        const char *msg = "hello through pipe\n";
        if (write(fd[1], msg, strlen(msg)) < 0) {
            perror("child write");
            _exit(1);
        }

        sleep(1); /* let parent block in poll first */
        close(fd[1]); /* close writer end */
        _exit(0);
    }

    /* parent: reader */
    close(fd[1]); /* close write end */

    pfd.fd = fd[0];
    pfd.events = POLLIN;

    for (;;) {
        printf("parent: calling poll ...\n");
        fflush(stdout);

        n = poll(&pfd, 1, -1);
        if (n < 0) {
            perror("poll");
            exit(1);
        }

        printf("parent: poll returned %d\n", n);
        print_revents(pfd.revents);

        if (pfd.revents & (POLLIN | POLLHUP)) {
            n = read(fd[0], buf, sizeof(buf) - 1);
            if (n < 0) {
                perror("read");
                exit(1);
            } else if (n == 0) {
                printf("parent: read returned 0 (EOF, writer closed)\n");
                break;
            } else {
                buf[n] = '\0';
                printf("parent: read %d bytes: %s", n, buf);
            }
        }
    }

    close(fd[0]);
    wait(NULL);
    return 0;
}
