#include "apue.h"
#include <sys/msg.h>

#define MAXMSZ 512
#define KEY    0x123

struct mymesg {
    long mtype;
    char mtext[MAXMSZ + 1];
};

int main(int argc, char *argv[])
{
    int qnum;
    int qid;
    struct mymesg m;

    if (argc != 3)
        err_quit("usage: %s <queue-number:0|1|2> <message>", argv[0]);

    qnum = atoi(argv[1]);
    if (qnum < 0 || qnum > 2)
        err_quit("queue-number must be 0, 1, or 2");

    if ((qid = msgget(KEY + qnum, 0)) < 0)
        err_sys("msgget error");

    m.mtype = 1;
    strncpy(m.mtext, argv[2], MAXMSZ);
    m.mtext[MAXMSZ] = 0;

    if (msgsnd(qid, &m, strlen(m.mtext), 0) < 0)
        err_sys("msgsnd error");

    printf("sent to queue %d (qid=%d): %s\n", qnum, qid, m.mtext);
    exit(0);
}
