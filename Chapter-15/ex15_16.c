#include "apue.h"
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <sys/wait.h>
#define NLOOPS 1000
#define SIZE sizeof(long)

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short* array;
};

static void sem_op(int semid, int num, int op)
{
    struct sembuf sb;
    sb.sem_num = num;
    sb.sem_op = op;
    sb.sem_flg = 0;

    if (semop(semid, &sb, 1) < 0) {
        err_sys("semop error");
    }
}

/* size of shared memory area */
static int update(long *ptr)
{
    return((*ptr)++); /* return value before increment */
}

int main(void)
{
    int semid, i, counter;
    pid_t pid;
    void *area;

    union semun arg;
    unsigned short init[2];

    if ((semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600)) < 0) {
        err_sys("semget error");
    }

    if ((area = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED)
        err_sys("mmap error");

    init[0] = 1; /* parent */
    init[1] = 0; /* child */
    arg.array = init;

    if (semctl(semid, 0, SETALL, arg) < 0) {
        err_sys("semctl SETALL error");
    }

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid > 0) {
        /* parent */
        for (i = 0; i < NLOOPS; i += 2) {
            sem_op(semid, 0, -1);   /* P(parent) */
            if ((counter = update((long *)area)) != i)
                err_quit("parent: expected %d, got %d", i, counter);
            sem_op(semid, 1, +1);   /* V(child) */
        }
        waitpid(pid, NULL, 0);
        printf("shared area = %ld\n", *(long*)area);
        semctl(semid, 0, IPC_RMID);
    } else {
        /* child */
        for (i = 1; i < NLOOPS + 1; i += 2) {
            sem_op(semid, 1, -1);   /* P(child) */
            if ((counter = update((long *)area)) != i)
                err_quit("child: expected %d, got %d", i, counter);
            sem_op(semid, 0, +1);   /* V(parent) */
        }
        _exit(0);
    }
}
