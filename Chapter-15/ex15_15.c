#include "apue.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#define NLOOPS 1000
#define SIZE sizeof(long)
/* size of shared memory area */
static int update(long *ptr)
{
    return((*ptr)++); /* return value before increment */
}

int main(void)
{
    int shmid, i, counter;
    pid_t pid;
    void *area;

    if ((shmid = shmget(IPC_PRIVATE, SIZE, IPC_CREAT | 0600)) < 0) {
        err_sys("shmget error");
    }
    if ((area = shmat(shmid, NULL, 0)) == NULL) {
        err_sys("shmat error");
    }

    TELL_WAIT();
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid > 0) {
        /* parent */
        for (i = 0; i < NLOOPS; i += 2) {
            if ((counter = update((long *)area)) != i)
                err_quit("parent: expected %d, got %d", i, counter);
            TELL_CHILD(pid);
            WAIT_CHILD();
        }
        waitpid(pid, NULL, 0);
        printf("shared area = %ld\n", *(long*)area);
    } else {
        /* child */
        for (i = 1; i < NLOOPS + 1; i += 2) {
            WAIT_PARENT();
            if ((counter = update((long *)area)) != i)
                err_quit("child: expected %d, got %d", i, counter);
            TELL_PARENT(getppid());
        }
    }
    exit(0);
}
