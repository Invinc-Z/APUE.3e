#include <stdbool.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    int fd[2];
    pid_t pid;
    char buf[1024];
    int n;

    if(pipe(fd) < 0){
        perror("pipe");
        exit(1);
    }

    if((pid = fork()) < 0){
        perror("fork");
        exit(1);
    }else if (pid == 0) {
        /* child */
        close(fd[0]);   // close child read end
        const char *msg = "hello through pipe\n";
        if (write(fd[1], msg, strlen(msg)) < 0) {
            perror("child write");
            _exit(1);
        }

        sleep(1);
        close(fd[1]); // close writer end => parent should eventually see EOF
        _exit(0);
    }

    // parent
    close(fd[1]);   // close write end
    for(;;) {
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(fd[0], &rset);

        printf("parent: calling select ...\n");
        fflush(stdout);

        n = select(fd[0] + 1, &rset, NULL, NULL, NULL);
        if (n < 0) {
            perror("select");
            exit(1);
        }

        printf("parent: select returned %d\n", n);

        if (FD_ISSET(fd[0], &rset)) {
            printf("parent: pipe read end is readable\n");

            n = read(fd[0], buf, sizeof(buf) -1);
            if(n < 0) {
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
    return EXIT_SUCCESS;
}
