#include "apue.h"
#include <string.h>
#include <stdlib.h>

#define WHITE " \t\n"
#define INITARGC 10

/*
 * buf[] contains white-space-separated arguments.
 * We convert it to an argv-style array of pointers, and call
 * the user's function (optfunc) to process the array.
 * We return -1 if there's a problem parsing buf or allocating memory;
 * otherwise we return whatever optfunc() returns.
 *
 * Note that the user's buf[] array is modified
 * (nulls placed after each token).
 */
int buf_args(char *buf, int (*optfunc)(int, char **))
{
    char    *ptr;
    char    **argv;
    int     argc;
    int     nalloc;
    int     ret;

    if (buf == NULL || optfunc == NULL)
        return -1;

    nalloc = INITARGC;
    argv = malloc(nalloc * sizeof(char *));
    if (argv == NULL)
        return -1;

    /*
     * First token becomes argv[0].
     */
    if (strtok(buf, WHITE) == NULL) {
        free(argv);
        return -1;      /* at least argv[0] is required */
    }

    argc = 0;
    argv[argc++] = buf;

    /*
     * Parse the remaining tokens.
     */
    while ((ptr = strtok(NULL, WHITE)) != NULL) {
        /*
         * Need one extra slot for the final NULL.
         */
        if (argc + 1 >= nalloc) {
            char **tmp;
            int newsize = nalloc * 2;

            tmp = realloc(argv, newsize * sizeof(char *));
            if (tmp == NULL) {
                free(argv);
                return -1;
            }
            argv = tmp;
            nalloc = newsize;
        }

        argv[argc++] = ptr;
    }

    argv[argc] = NULL;

    ret = (*optfunc)(argc, argv);

    free(argv);
    return ret;
}
