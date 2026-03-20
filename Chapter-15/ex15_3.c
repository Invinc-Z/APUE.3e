#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(void)
{
    FILE *fp;
    char buf[1024];
    int status;

    fp = popen("nosuchcommand", "r");
    if (fp == NULL) {
        perror("popen");
        exit(1);
    }

    while (fgets(buf, sizeof(buf), fp) != NULL)
        fputs(buf, stdout);

    status = pclose(fp);

    if (status == -1) {
        perror("pclose");
        exit(1);
    }

    if (WIFEXITED(status))
        printf("child exit status = %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("child killed by signal = %d\n", WTERMSIG(status));

    return 0;
}
