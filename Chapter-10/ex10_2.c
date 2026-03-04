#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int sig2str(int signo, char * str)
{
    if(signo <= 0 || str == NULL)
        return -1;
    switch(signo)
    {
        case 1:
            strcpy(str, "HUP");
            break;
        case 2:
            strcpy(str, "INT");
            break;
        case 3:
            strcpy(str, "QUIT");
            break;
        case 4:
            strcpy(str, "ILL");
            break;
            // ...
        default:
            printf("Invalid signal number!\n");
            break;
    }

    return 0;
}

void sig_func(int signo)
{
    char buff[32] = {0};
    sig2str(signo, buff);
    printf("signal %d is: %s\n", signo, buff);
}

int main()
{
    char buff[32] = {0};
    signal(SIGINT, sig_func);
    for(int i = 1; i <= 4; i++)
    {
        memset(buff, 0, sizeof(buff));
        sig2str(i, buff);
        printf("signal %d is: %s\n", i, buff);
    }

    printf("Please enter Ctrl+C to catch interrupt signal...\n");
    pause();
    puts("");

    return 0;
}
