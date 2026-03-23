#include "apue.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>

static int fd[2];   /* UNIX domain socket pair for descriptor passing */

    int
main(int argc, char *argv[])
{
    pid_t pid;
    int passfd;
    off_t off;

    if (argc != 2)
        err_quit("usage: %s <existing-file>", argv[0]);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0)
        err_sys("socketpair error");

    TELL_WAIT();

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {              /* child */
        close(fd[0]);

        if ((passfd = open(argv[1], O_RDONLY)) < 0)
            err_sys("open error");

        /* pass descriptor to parent */
        if (send_fd(fd[1], passfd) < 0)
            err_sys("send_fd error");

        /*
         * First reposition: set offset to 10
         */
        if (lseek(passfd, 10, SEEK_SET) == (off_t)-1)
            err_sys("lseek error");

        TELL_PARENT(getppid());         /* notify parent first offset set */
        WAIT_PARENT();                  /* wait until parent has checked */

        /*
         * Second reposition: set offset to 20
         */
        if (lseek(passfd, 20, SEEK_SET) == (off_t)-1)
            err_sys("lseek error");

        TELL_PARENT(getppid());         /* notify parent second offset set */
        WAIT_PARENT();                  /* wait until parent has checked */

        close(passfd);
        close(fd[1]);
        _exit(0);

    } else {                           /* parent */
        close(fd[1]);

        if ((passfd = recv_fd(fd[0], write)) < 0)
            err_sys("recv_fd error");

        WAIT_CHILD();                  /* wait child set first offset */

        if ((off = lseek(passfd, 0, SEEK_CUR)) == (off_t)-1)
            err_sys("parent lseek SEEK_CUR error");
        printf("parent sees current offset = %lld\n", (long long)off);

        TELL_CHILD(pid);               /* tell child continue */

        WAIT_CHILD();                  /* wait child set second offset */

        if ((off = lseek(passfd, 0, SEEK_CUR)) == (off_t)-1)
            err_sys("parent lseek SEEK_CUR error");
        printf("parent sees current offset = %lld\n", (long long)off);

        TELL_CHILD(pid);               /* allow child exit */

        close(passfd);
        close(fd[0]);
        waitpid(pid, NULL, 0);
    }

    exit(0);
}
