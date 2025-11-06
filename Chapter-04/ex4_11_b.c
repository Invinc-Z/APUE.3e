#include "apue.h"
#include <dirent.h>
#include <limits.h>
#include <sys/time.h>

/* function type that is called for each filename */
typedef	int	Myfunc(const char *, const struct stat *, int);

static Myfunc	myfunc;
static int		myftw(char *, Myfunc *);
static int		dopath(Myfunc *);

static long	nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

    int
main(int argc, char *argv[])
{
    struct timeval start, end;
    int ret;
    int timeuse;

    if (argc != 2)
        err_quit("usage:  ftw  <starting-pathname>");

    gettimeofday(&start, NULL);
    ret = myftw(argv[1], myfunc);		/* does it all */
    gettimeofday(&end, NULL);

    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (ntot == 0)
        ntot = 1;		/* avoid divide by 0; print 0 for all counts */
    printf("regular files  = %7ld, %5.2f %%\n", nreg,
           nreg*100.0/ntot);
    printf("directories    = %7ld, %5.2f %%\n", ndir,
           ndir*100.0/ntot);
    printf("block special  = %7ld, %5.2f %%\n", nblk,
           nblk*100.0/ntot);
    printf("char special   = %7ld, %5.2f %%\n", nchr,
           nchr*100.0/ntot);
    printf("FIFOs          = %7ld, %5.2f %%\n", nfifo,
           nfifo*100.0/ntot);
    printf("symbolic links = %7ld, %5.2f %%\n", nslink,
           nslink*100.0/ntot);
    printf("sockets        = %7ld, %5.2f %%\n", nsock,
           nsock*100.0/ntot);

    timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec -start.tv_usec;
    printf("time: %d us\n", timeuse);
    exit(ret);
}

/*
 * Descend through the hierarchy, starting at "pathname".
 * The caller's func() is called for every file.
 */
#define	FTW_F	1		/* file other than directory */
#define	FTW_D	2		/* directory */
#define	FTW_DNR	3		/* directory that can't be read */
#define	FTW_NS	4		/* file that we can't stat */

/* static char	*fullpath;		/1* contains full pathname for every file *1/ */
static char filepath[256]; // set it long not more
static size_t pathlen;

    static int					/* we return whatever func() returns */
myftw(char *pathname, Myfunc *func)
{
    strcpy(filepath, pathname);
    return(dopath(func));
}

/*
 * Descend through the hierarchy, starting at "fullpath".
 * If "fullpath" is anything other than a directory, we lstat() it,
 * call func(), and return.  For a directory, we call ourself
 * recursively for each name in the directory.
 */
    static int					/* we return whatever func() returns */
dopath(Myfunc* func)
{
    struct stat		statbuf;
    struct dirent	*dirp;
    DIR				*dp;
    int				ret;

    if (lstat(filepath, &statbuf) < 0)	/* stat error */
        return(func(filepath, &statbuf, FTW_NS));

    if (S_ISDIR(statbuf.st_mode) == 0)	/* not a directory */
        return(func(filepath, &statbuf, FTW_F));

    if ((ret = func(filepath, &statbuf, FTW_D)) != 0)
        return(ret);

    if ((dp = opendir(filepath)) == NULL)
        return(func(filepath, &statbuf, FTW_DNR));

    if (chdir(filepath) < 0) 
        err_quit("chdir error");

    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0  ||
            strcmp(dirp->d_name, "..") == 0)
            continue;		/* ignore dot and dot-dot */

        strcpy(filepath, dirp->d_name);

        if ((ret = dopath(func)) != 0)		/* recursive */
            break;	/* time to leave */
    }

    if (chdir("..") < 0) 
        err_quit("chdir to .. error");

    if (closedir(dp) < 0)
        err_ret("can't close directory %s", filepath);

    return(ret);
}

    static int
myfunc(const char *pathname, const struct stat *statptr, int type)
{
    switch (type) {
    case FTW_F:
        switch (statptr->st_mode & S_IFMT) {
        case S_IFREG:	nreg++;		break;
        case S_IFBLK:	nblk++;		break;
        case S_IFCHR:	nchr++;		break;
        case S_IFIFO:	nfifo++;	break;
        case S_IFLNK:	nslink++;	break;
        case S_IFSOCK:	nsock++;	break;
        case S_IFDIR:	/* directories should have type = FTW_D */
                        err_dump("for S_IFDIR for %s", pathname);
        }
        break;
    case FTW_D:
        ndir++;
        break;
    case FTW_DNR:
        err_ret("can't read directory %s", pathname);
        break;
    case FTW_NS:
        err_ret("stat error for %s", pathname);
        break;
    default:
        err_dump("unknown type %d for pathname %s", type, pathname);
    }
    return(0);
}
