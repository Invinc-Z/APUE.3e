#define _GNU_SOURCE
#include <stdio.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
    struct utsname u;

    if(uname(&u) != 0)
    {
        perror("utsname failed");
        exit(0);
    }
    printf("sysname: %s\n", u.sysname);
    printf("nodename: %s\n", u.nodename);
    printf("release: %s\n", u.release);
    printf("version: %s\n", u.version);
    printf("machine: %s\n", u.machine);
    printf("domainname: %s\n", u.domainname);

    return 0;
}
