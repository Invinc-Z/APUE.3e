#include "apue.h"
#include <fcntl.h>

#define FILE_NAME "counter.dat"
#define NLOOPS 10

static void increment_counter(int fd, const char* who)
{
    int val;

    lseek(fd, 0, SEEK_SET);
    if(read(fd, &val, sizeof(val)) != sizeof(val))
        err_sys("read error");
    val++;
    lseek(fd, 0, SEEK_SET);
    if(write(fd, &val, sizeof(val)) != sizeof(val))
        err_sys("write error");

    printf("%s incremented counter to %d\n", who, val);
    fflush(stdout);
}

int main()
{
    int fd;
    pid_t pid;

    if((fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0)
        err_sys("open error");

    int zero = 0;
    if(write(fd, &zero, sizeof(zero)) != sizeof(zero))
        err_sys("write error");


    TELL_WAIT();

    if((pid = fork()) < 0)
        err_sys("fork error");
    else if(pid == 0)
    {
        // child process
        for(int i = 0; i < NLOOPS; i++){
            WAIT_PARENT();
            increment_counter(fd, "child");
            TELL_PARENT(getppid());
        }
    }
    else
    {
        // parent process
        for(int i = 0; i < NLOOPS; i++){
            increment_counter(fd, "parent");
            TELL_CHILD(pid);
            WAIT_CHILD();
        }
    }

    close(fd);
    return 0;
}
