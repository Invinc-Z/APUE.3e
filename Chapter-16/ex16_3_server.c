#include "apue.h"
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define QLEN 10
#define MAXLISTEN 16
#define BUFLEN 128

static int initserver(int type, const struct sockaddr *addr, socklen_t alen, int qlen)
{
    int fd;
    int err = 0;
    if ((fd = socket(addr->sa_family, type, 0)) < 0)
        return(-1);
    if (bind(fd, addr, alen) < 0)
        goto errout;
    if (type == SOCK_STREAM || type == SOCK_SEQPACKET) {
        if (listen(fd, qlen) < 0)
            goto errout;
    }
    return(fd);
errout:
    err = errno;
    close(fd);
    errno = err;
    return(-1);
}

static void serve(int sockfd) {
    int clfd;
    FILE *fp;
    char buf[BUFLEN];

    clfd = accept(sockfd, NULL, NULL);
    if (clfd < 0) {
        err_ret("ruptimed: accept error");
        return;
    }
    if ((fp = popen("/usr/bin/uptime", "r")) == NULL) {
    // if ((fp = popen("ps -e --no-headers | wc -l", "r")) == NULL) {
        sprintf(buf, "error: %s\n", strerror(errno));
        send(clfd, buf, strlen(buf), 0);
    } else {
        while (fgets(buf, BUFLEN, fp) != NULL) {
            send(clfd, buf, strlen(buf), 0);
        }
    }
    close(clfd);
}

int main(int argc, char *argv[]) {
    struct addrinfo *ailist, *aip;
    struct addrinfo hint;
    int err, nlisten, i, maxfd, sockfd;
    int listenfd[MAXLISTEN];
    fd_set rset, allset;

    if (argc != 1) {
        err_quit("usage: ruptimed");
    }

    memset(&hint, 0, sizeof(hint));
    hint.ai_flags = AI_PASSIVE;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;

    if ((err = getaddrinfo(NULL, "8888", &hint, &ailist)) != 0)
        err_quit("ruptimed: getaddrinfo error: %s", gai_strerror(err));

    nlisten = 0;
    for (aip = ailist; aip != NULL; aip = aip->ai_next) {
        sockfd = initserver(SOCK_STREAM, aip->ai_addr, aip->ai_addrlen, QLEN);
        if (sockfd >= 0) {
            if (nlisten < MAXLISTEN) {
                listenfd[nlisten++] = sockfd;
            } else {
                close(sockfd);
                err_quit("too many listening sockets");
            }
        }
    }

    freeaddrinfo(ailist);

    if (nlisten == 0)
        err_quit("ruptimed: no endpoints could be initialized");

    FD_ZERO(&allset);
    maxfd = -1;
    for (i = 0; i < nlisten; i++) {
        FD_SET(listenfd[i], &allset);
        if (listenfd[i] > maxfd) {
            maxfd = listenfd[i];
        }
    }

    for (;;) {
        rset = allset;
        if (select(maxfd + 1, &rset, NULL, NULL, NULL) < 0) {
            if (errno == EINTR)
                continue;
            err_sys("select error");
        }

        for (i = 0; i < nlisten; i++) {
            if (FD_ISSET(listenfd[i], &rset)) {
                serve(listenfd[i]);
            }
        }
    }

    return EXIT_SUCCESS;
}
