# Advanced IPC

## Exercises

### 17.1

**We chose to use UNIX domain datagram sockets in Figure 17.3, because they retain message boundaries.
Describe the changes that would be necessary to use regular pipes instead.
How can we avoid copying the messages two extra times?**

Regular pipes provide a byte stream interface.
To detect message boundaries, we'd have to add a header to each message to indicate the length.
But this still involves two extra copy operations: one to write to the pipe and one to read from the pipe.
A more efficient approach is to use the pipe only to signal the main thread that a new message is available.
We can use a single byte for this purpose.
With this approach, we need to move the `mymesg` structure to the `threadinfo` structure
    and use a mutex and a condition variable to prevent the helper thread
    from reusing the `mymesg` structure until the main thread is done with it.

```bash
$ gcc ex17_1.c -lapue
$ gcc ex17_1_client.c -o client -lapue
$ ./a.out
$ ./client 0 hello
```

### 17.2

**Write the following program using the file descriptor passing functions from this chapter
    and the parent–child synchronization routines from Section 8.9.
The program calls `fork`, and the child opens an existing file and passes the open descriptor to the parent.
The child then positions the file using lseek and notifies the parent.
The parent reads the file's current offset and prints it for verification.
If the file was passed from the child to the parent as we described,
    they should be sharing the same file table entry, so each time the child changes the file’s current offset,
    that change should also affect the parent’s descriptor.
Have the child position the file to a different offset and notify the parent again.**

[ex17_2.c](ex17_2.c)

```c
$ ./a.out README.md
parent sees current offset = 10
parent sees current offset = 20
```

### 17.3

**In Figures 17.20 and 17.21, we differentiated between declaring and defining the global variables. What is the difference?**

A declaration specifies the attributes (such as the data type) of a set of identifiers.
If the declaration also causes storage to be allocated, it is called a definition.

In the `opend.h` header, we declare the three global variables with the extern storage class.
These declarations do not cause storage to be allocated for the variables.
In the `main.c` file, we define the three global variables.
Sometimes, we'll also initialize a global variable when we define it, but we typically let the C default apply.

### 17.4

**Recode the `buf_args` function (Figure 17.23), removing the compile-time limit on the size of the argv array. Use dynamic memory allocation.**

[ex17_4.c](ex17_4.c)

### 17.5

**Describe ways to optimize the function loop in Figure 17.29 and Figure 17.30. Implement your optimizations.**

Both select and poll return the number of ready descriptors as the value of the function.
The loop that goes through the client array can terminate when the number of ready descriptors has been processed.

### 17.6

**In the `serv_listen` function (Figure 17.8), we unlink the name of the file representing the UNIX domain socket if the file already exists.
To avoid unintentionally removing a file that isn’t a socket, we could call `stat` first to verify the file type.
Explain the two problems with this approach.**

The first problem with the proposed solution is that there is a race
    between the call to `stat` and the call to `unlink` where the file can change.
The second problem is that if the name is a symbolic link pointing to the UNIX domain socket file,
then `stat` will report that the name is a socket (recall that the `stat` function follows symbolic links),
but when we call `unlink`, we will actually remove the symbolic link instead of the socket file.
To solve this problem, we should use `lstat` instead of `stat`, but this doesn't solve the first problem.

### 17.7

Describe two possible ways to pass more than one file descriptor with a single call to `sendmsg`.
Try them out to see if they are supported by your operating system.

The first option is to send both file descriptors in one control message.
Each file descriptor is stored in adjacent memory locations.
The following code shows this:

```c
struct msghdr msg;
struct cmsghdr *cmptr;
int *ip;
if ((cmptr = calloc(1, CMSG_LEN(2*sizeof(int)))) == NULL)
    err_sys("calloc error");
msg.msg_control = cmptr;
msg.msg_controllen = CMSG_LEN(2*sizeof(int));
/* continue initializing msghdr... */
cmptr->cmsg_len = CMSG_LEN(2*sizeof(int));
cmptr->cmsg_level = SOL_SOCKET;
cmptr->cmsg_type = SCM_RIGHTS;
ip = (int *)CMSG_DATA(cmptr);
*ip++ = fd1;
*ip = fd2;
```

This approach works on all four platforms covered in this book.

The second option is to pack two separate `cmsghdr` structures into a single message:

```c
struct msghdr msg;
struct cmsghdr *cmptr;
if ((cmptr = calloc(1, 2*CMSG_LEN(sizeof(int)))) == NULL)
    err_sys("calloc error");
msg.msg_control = cmptr;
msg.msg_controllen = 2*CMSG_LEN(sizeof(int));
/* continue initializing msghdr... */
cmptr->cmsg_len = CMSG_LEN(sizeof(int));
cmptr->cmsg_level = SOL_SOCKET;
cmptr->cmsg_type = SCM_RIGHTS;
*(int *)CMSG_DATA(cmptr) = fd1;
cmptr = CMPTR_NXTHDR(&msg, cmptr);
cmptr->cmsg_len = CMSG_LEN(sizeof(int));
cmptr->cmsg_level = SOL_SOCKET;
cmptr->cmsg_type = SCM_RIGHTS;
*(int *)CMSG_DATA(cmptr) = fd2;
```

Unlike the first approach, this method works only on FreeBSD 8.0.

