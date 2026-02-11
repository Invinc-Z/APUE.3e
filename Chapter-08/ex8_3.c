#include "apue.h"
#include <sys/wait.h>
int main(void)
{
    pid_t pid;
    int status;
    siginfo_t siginfo;
    if ((pid = fork()) < 0)
        err_sys("fork error");
    else if (pid == 0)
        exit(7);
    if (waitid(P_PID, pid, &siginfo, WEXITED) != 0)
        err_sys("waitid error");
    printf("exit value or signal is: %d\n", siginfo.si_status);

    if ((pid = fork()) < 0)
        err_sys("fork error");
    else if (pid == 0)
        abort();
    if (waitid(P_PID, pid, &siginfo, WEXITED) != 0)
        err_sys("wait error");
    printf("exit value or signal is: %d\n", siginfo.si_status);

    if ((pid = fork()) < 0)
        err_sys("fork error");
    else if (pid == 0)
        status /= 0; /* divide by 0 generates SIGFPE */
    if (waitid(P_PID, pid, &siginfo, WEXITED) != 0)
        err_sys("wait error");
    printf("exit value or signal is: %d\n", siginfo.si_status);
    exit(0);
}
