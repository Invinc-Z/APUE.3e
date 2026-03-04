#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define SIZE (1024L * 1024L * 1024L)   // 1GB

void sig_alrm(int signo)
{
    printf("SIGALRM caught!\n");
}

int main(void)
{
    FILE *fp;
    char *buf;
    size_t n;

    signal(SIGALRM, sig_alrm);

    buf = malloc(SIZE);
    if (buf == NULL) {
        perror("malloc");
        exit(1);
    }

    fp = fopen("bigoutput", "w");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    alarm(1);      // 1秒后发送 SIGALRM

    printf("Calling fwrite...\n");
    n = fwrite(buf, 1, SIZE, fp);
    printf("fwrite returned %zu\n", n);

    fclose(fp);
    return 0;
}
