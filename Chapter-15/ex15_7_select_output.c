#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    int fd[2];
    pid_t pid;
    fd_set wset, eset;
    int n;
    char ch = 'X';

    if (pipe(fd) < 0) {
        perror("pipe");
        exit(1);
    }

    /*
     * Ignore SIGPIPE so that write() returns -1/EPIPE
     * instead of tereminating the process.
     */
    if (signal(SIGPIPE, SIG_IGN)  == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        close(fd[1]);   /* child doesn't write */
        sleep(1);       /* let parent block in select */
        close(fd[0]);   /* close read end */
        _exit(0);
    }

    /* parent: writer */
    close(fd[0]);

    for (;;) {
        FD_ZERO(&wset);
        FD_ZERO(&eset);
        FD_SET(fd[1], &wset);
        FD_SET(fd[1], &eset);

        printf("parent: calling select ...\n");
        fflush(stdout);

        n = select(fd[1] + 1, NULL, &wset, &wset, NULL);
        if(n < 0) {
            perror("select");
            exit(1);
        }

        printf("parent: select returned %d\n", n);
        printf("parent: writable=%d except=%d\n",
                FD_ISSET(fd[1], &wset) ? 1 : 0,
                FD_ISSET(fd[1], &eset) ? 1 : 0);

        /*
         * Try an actual write to see whether the pipe is still usable.
         */
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
    return EXIT_SUCCESS;
}
