#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct mymsg {
    long mtype;
    char mtext[64];
};

int main(int argc, char *argv[])
{
    int i, msqid;
    struct mymsg msg;

    printf("first loop: create and delete 5 queues\n");
    for (i = 0; i < 5; i++) {
        msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
        if (msqid == -1) {
            perror("msgget");
            exit(1);
        }

        printf(" loop1[%d]: msqid = %d\n", i, msqid);

        if (msgctl(msqid, IPC_RMID, NULL) == -1) {
            perror("msgctl(IPC_RMID)");
            exit(1);
        }
    }

    printf("\nsecond loop: create 5 queues and leave a message in each\n");
    msg.mtype = 1;
    strcpy(msg.mtext, "hello");
    for (i = 0; i < 5; i++) {
        msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
        if(msqid == -1) {
            perror("msgget");
            exit(1);
        }

        printf(" loop2[%d]: msqid = %d\n", i, msqid);

        if(msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
    }

    printf("\nprogram exits now; run: ipcs -q\n");
    return EXIT_SUCCESS;
}
