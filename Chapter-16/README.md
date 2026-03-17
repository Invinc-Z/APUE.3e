# Network IPC: Sockets

## Exercises

### 16.1

**Write a program to determine your system's byte ordering.**

[ex16_1.c](ex16_1.c)

### 16.2

**Write a program to print out which stat structure members are supported
    for sockets on at least two different platforms, and describe how the results differ.**

[ex16_2.c](ex16_2.c)

```shell
$ ./a.out
fstat() on a socket:
st_mode    = 0140777 (socket)
st_dev     = 9
st_ino     = 36513
st_nlink   = 1
st_uid     = 1000
st_gid     = 1000
st_rdev    = 0
st_size    = 0
st_blksize = 4096
st_blocks  = 0
st_atime   = 0 (1970-01-01 08:00:00)
st_mtime   = 0 (1970-01-01 08:00:00)
st_ctime   = 0 (1970-01-01 08:00:00)
```

### 16.3

**The program in Figure 16.17 provides service on only a single endpoint.
Modify the program to support service on multiple endpoints (each with a different address) at the same time.**

[ex16_3_server.c](ex16_3_server.c)

[ex16_3_client.c](ex16_3_client.c)

```shell
$ gcc ex16_3_client.c -lapue -o client
$ gcc ex16_3_server.c -lapue -o server
$ ./server &
$ ./client 127.0.0.1
15:25:23 up  2:27,  1 user,  load average: 0.21, 0.15, 0.09
```

For each endpoint we will be listening on, we need to bind the proper address
    and record an entry in an `fd_set` structure corresponding to each file descriptor.
We will use `select` to wait for connect requests to arrive on multiple endpoints.
Recall from Section 16.4 that a passive endpoint will appear to be readable
    when a connect request arrives on it.
When a connect request does arrive, we will accept the request and process it as before.

### 16.4

**Write a client program and a server program to return the number of processes currently
running on a specified host computer.**

change exercise 16.3 [ex16_3_server.c](ex16_3_server.c) `popen` arg `/usr/bin/uptime` to `ps -e --no-headers | wc -l`.

### 16.5

**In the program in Figure 16.18, the server waits for the child to execute the `uptime` command
and exit before accepting the next connect request.
Redesign the server so that the time to service one request doesn’t delay the processing of incoming connect requests.**

[ex16_15_server.c](ex16_5_server.c)

In the main procedure, we need to arrange to catch `SIGCHLD` by calling our signal function (Figure 10.18),
    which will use `sigaction` to install the handler specifying the restartable system call option.
Next, we need to remove the call to `waitpid` from our serve function.
After forking the child to service the request, the parent closes the new file descriptor
and resumes listening for additional connect requests.
Finally, we need a signal handler for `SIGCHLD`, as follows:

```c
void sigchld(int signo)
{
    while (waitpid((pid_t)-1, NULL, WNOHANG) > 0)
    ;
}
```

### 16.6

**Write two library routines: one to enable asynchronous (signal-based) I/O on a socket
and one to disable asynchronous I/O on a socket.
Use Figure 16.23 to make sure that the functions work on all platforms with as many socket types as possible.**

To enable asynchronous socket I/O, we need to establish socket ownership using the `F_SETOWN` fcntl command,
    and then enable asynchronous signaling using the `FIOASYNC` ioctl command.
To disable asynchronous socket I/O, we simply need to disable asynchronous signaling.
The reason we mix `fcntl` and `ioctl` commands is to find the methods that are most portable.
The code is shown in [ex16_6.c](ex16_6.c).

