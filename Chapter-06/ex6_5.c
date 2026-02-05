#include "apue.h"
#include <time.h>

int main()
{
    time_t currtime;
    struct tm * tm;
    char line[256] = {0};

    if((currtime = time(NULL)) == -1)
        err_sys("time error");
    if((tm = localtime(&currtime)) == NULL)
        err_sys("localtime error");
    if(strftime(line, 256, "%Y %m %d %A %T %Z\n", tm) == 0)
        err_sys("strftime error");
    fputs(line, stdout);
    exit(0);
}
