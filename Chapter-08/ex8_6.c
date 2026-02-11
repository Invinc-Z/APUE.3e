#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PSCMD "ps -o pid,ppid,state,tty,command"
int main()
{
    pid_t pid;
    if((pid = fork()) < 0)
    {
        perror("fork error");
        exit(0);
    }
    else if(pid == 0)
        exit(0);

    sleep(5);
    system(PSCMD);
    return 0;
}

