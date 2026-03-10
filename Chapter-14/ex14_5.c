#include <poll.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/select.h>

unsigned int sleep_us(unsigned int microseconds) {
    struct timeval tv;
    tv.tv_sec = microseconds / 1000000;
    tv.tv_usec = microseconds % 1000000;
    if (select(0, NULL, NULL, NULL, &tv) < 0)
        return -1;
    return tv.tv_usec;
}

void sleep_us2(unsigned int microseconds) {
    struct pollfd dummy;
    int timeout;
    if ((timeout = microseconds / 1000) <= 0)
        timeout = 1;
    poll(&dummy, 0, timeout);
}

int main() {
    printf("sleep 3s\n");
    sleep_us(1000000 * 3);
    printf("sleep over!\n");

    printf("sleep 2s\n");
    sleep_us2(2000000);
    printf("sleep over!\n");

    return 0;
}
