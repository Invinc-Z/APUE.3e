#define _GNU_SOURCE
#include "apue.h"
#include <signal.h>
#include <errno.h>

void pr_mask(const char *str)
{
    sigset_t sigset;
    int errno_save;

    errno_save = errno;

    if (sigprocmask(0, NULL, &sigset) < 0) {
        err_ret("sigprocmask error");
    } else {
        printf("%s", str);

        /* 将 sigset 当作位数组处理 */
        unsigned long *bits = (unsigned long *)&sigset;
        size_t nwords = sizeof(sigset) / sizeof(unsigned long);
        // printf("sizeof(sigset) = %zu, sizeof(unsigned long) = %zu\n", sizeof(sigset), sizeof(unsigned long));
        printf("NSIG = %d\n", NSIG);

        for (size_t i = 0; i < nwords; i++) {

            unsigned long word = bits[i];

            while (word) {
                int bit = __builtin_ctzl(word);   /* 找最低位1 */

                int signo = i * sizeof(unsigned long) * 8 + bit + 1;

                if (signo < NSIG)   // 关键
                    printf("%s = %d\n", strsignal(signo), signo);

                word &= ~(1UL << bit);  /* 清掉最低位1 */
            }
        }
    }

    printf("\n");
    errno = errno_save;
}

int main()
{
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGQUIT);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGALRM);

    if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
        err_sys("sigprocmask error");
    }

    pr_mask("main: \n");

    return 0;
}
