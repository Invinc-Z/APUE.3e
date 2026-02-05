#include <stdio.h>
#include <time.h>
#include <limits.h>

int main(void)
{
    time_t t = (time_t)INT_MAX;   // 2147483647

    printf("max time_t: %ld\n", (long)t);
    printf("asctime:   %s", asctime(gmtime(&t)));

    t += 1;   // 溢出点

    printf("\nafter +1 second:\n");
    printf("time_t:    %ld\n", (long)t);
    printf("asctime:   %s", asctime(gmtime(&t)));

    return 0;
}

