#include "apue.h"
#include <signal.h>
#include <sys/resource.h>

#define BUFFSIZE 100

void sig_xfsz(int signo)
{
    const char msg[] = "caught SIGXFSZ\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
}

int main(void)
{
    int n;
    char buf[BUFFSIZE];

    struct rlimit rl;
    getrlimit(RLIMIT_FSIZE, &rl);
    rl.rlim_cur = 1024;   // 只改软限制
    if (setrlimit(RLIMIT_FSIZE, &rl) < 0)
    perror("setrlimit");

    signal_intr(SIGXFSZ, sig_xfsz);

    while ((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0)
    {
        ssize_t nwritten = write(STDOUT_FILENO, buf, n);

        if(nwritten != n){
            fprintf(stderr, "write return %zd\n", nwritten);
        }
    }
    if (n < 0)
        err_sys("read error");
    exit(0);
}
