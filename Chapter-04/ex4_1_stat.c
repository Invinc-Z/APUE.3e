#include "apue.h"

int
main(int argc, char *argv[])
{
	int			i;
	struct stat	buf;
	char		ptr[32] = {0};

	for (i = 1; i < argc; i++) {
		printf("%s: ", argv[i]);
		if (stat(argv[i], &buf) < 0) {
			err_ret("stat error");
			continue;
		}
        memset(ptr, 0, sizeof(ptr));
		if (S_ISREG(buf.st_mode))
            strcpy(ptr, "regular");
		else if (S_ISDIR(buf.st_mode))
            strcpy(ptr, "directory");
		else if (S_ISCHR(buf.st_mode))
            strcpy(ptr, "character special");
		else if (S_ISBLK(buf.st_mode))
            strcpy(ptr, "block special");
		else if (S_ISFIFO(buf.st_mode))
            strcpy(ptr, "fifo");
		else if (S_ISLNK(buf.st_mode))
            strcpy(ptr, "symbolic link");
		else if (S_ISSOCK(buf.st_mode))
            strcpy(ptr, "socket");
		else
            strcpy(ptr, "** unknown mode **");
		printf("%s\n", ptr);
	}
	exit(0);
}
