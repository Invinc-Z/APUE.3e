#include <stdio.h>
#include <unistd.h>
#include <time.h>

int main()
{
    int i = 0;
    time_t curr;
    struct tm * p_tm;

    for(;;)
    {
        sleep(60);
        ++i;
        if(i % 5 == 0)
        {
            curr = time(NULL);
            printf("current time: %s", ctime(&curr));
            p_tm = localtime(&curr);
            printf("tm_sec = %d\n\n", p_tm->tm_sec);
            fflush(stdout);
        }
    }

    return 0;
}
