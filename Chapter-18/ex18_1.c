#include "apue.h"

int main(void)
{
    if (tty_raw(STDIN_FILENO) < 0)
        err_sys("tty_raw error");

    exit(0);   /* terminate without resetting terminal mode */
}
