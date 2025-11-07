#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    char buf[4096] = {0};
    int fd = open("test.txt", O_RDWR|O_APPEND);
    read(fd, buf, sizeof(buf));
    printf("%s\n", buf);

    memset(buf, 0, sizeof(buf));
    lseek(fd, 7, SEEK_SET);
    read(fd, buf, sizeof(buf));
    printf("%s\n", buf);

    memset(buf, 0, sizeof(buf));
    strcpy(buf, "I am here. ");
    lseek(fd, 0, SEEK_SET);
    write(fd, buf, strlen(buf));

    memset(buf, 0, sizeof(buf));
    lseek(fd, 0, SEEK_SET);
    read(fd, buf, sizeof(buf));
    printf("%s\n", buf);

    close(fd);
    return 0;
}

