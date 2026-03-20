#include "apue.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
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
    int i, counter;
    pid_t pid;
    void *area;
    sem_t *sem_parent, *sem_child;

    if ((area = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED)
        err_sys("mmap error");

    /*
     * parent first parent = 1, child = 0
     */
    sem_parent = sem_open("/sem_parent_apue", O_CREAT | O_EXCL, 0600, 1);
    if (sem_parent == SEM_FAILED) {
        err_sys("sem_open parent error");
    }
    sem_child = sem_open("/sem_child_apue", O_CREAT | O_EXCL, 0600, 0);
    if (sem_child == SEM_FAILED) {
        err_sys("sem_open child error");
    }

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid > 0) {
        /* parent */
        for (i = 0; i < NLOOPS; i += 2) {
            if (sem_wait(sem_parent) < 0) {
                err_sys("sem_wait parent error");
            }
            if ((counter = update((long *)area)) != i)
                err_quit("parent: expected %d, got %d", i, counter);
            if (sem_post(sem_child) < 0) {
                err_sys("sem_post child error");
            }
        }
        waitpid(pid, NULL, 0);
        printf("shared area = %ld\n", *(long*)area);

        if (sem_close(sem_parent) < 0)
            err_sys("sem_close parent error");
        if (sem_close(sem_child) < 0) {
            err_sys("sem_close child error");
        }

        if (sem_unlink("/sem_parent_apue") < 0) {
            err_sys("sem_unlink parent error");
        }
        if (sem_unlink("/sem_child_apue") < 0) {
            err_sys("sem_unlink child error");
        }
    } else {
        /* child */
        for (i = 1; i < NLOOPS + 1; i += 2) {
            if (sem_wait(sem_child) < 0) {
                err_sys("sem_wait child error");
            }
            if ((counter = update((long *)area)) != i)
                err_quit("child: expected %d, got %d", i, counter);
            if (sem_post(sem_parent) < 0) {
                err_sys("sem_post parent error");
            }
        }
        if (sem_close(sem_parent) < 0)
            err_sys("sem_close parent error");
        if (sem_close(sem_child) < 0) {
            err_sys("sem_close child error");
        }
        _exit(0);
    }
}
