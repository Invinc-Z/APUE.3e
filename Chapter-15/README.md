# Interprocess Communication

## Exercises

### 15.1

**In the program shown in Figure 15.6, remove the `close` right before the `waitpid` at the end of the parent code.
Explain what happens.**

[ex15_1.c](ex15_1.c)

If the write end of the pipe is never closed, the reader never sees an end of file.
The pager program blocks forever reading from its standard input.

### 15.2

**In the program in Figure 15.6, remove the `waitpid` at the end of the parent code.
Explain what happens.**

[ex15_2.c](ex15_2.c)

The parent terminates right after writing the last line to the pipe.
The read end of the pipe is automatically closed when the parent terminates.
But the parent is probably running ahead of the child by one pipe buffer,
    since the child (the `pager` program) is waiting for us to look at a page of output.
If we're running a shell, such as the Korn shell, with interactive command-line editing enabled,
the shell probably changes the terminal mode when our parent terminates and the shell prints a prompt.
This undoubtedly interferes with the pager program, which has also modified the terminal mode.
(Most pager programs set the terminal to noncanonical mode when awaiting input to proceed to the next page.)

### 15.3

**What happens if the argument to `popen` is a nonexistent command?
Write a small program to test this.**

[ex15_3.c](ex15_3.c)

```shell
$ ./a.out
sh: 1: nosuchcommand: not found
child exit status = 127
 ```

The `popen` function returns a file pointer because the shell is executed.
But the shell can't execute the nonexistent command, so it prints

`sh: line 1: ./a.out: No such file or directory`

on the standard error and terminates with an exit status of 127
(although the value depends on the type of shell).
`pclose` returns the termination status of the command as it is returned by `waitpid`.

### 15.4

**In the program shown in Figure 15.18, remove the signal handler, execute the program,
    and then terminate the child.
After entering a line of input, how can you tell that the parent was terminated by `SIGPIPE`?**

When the parent terminates, look at its termination status with the shell.
For the Bourne shell, Bourne-again shell, and Korn shell, the command is `echo $?`.
The number printed is 128 plus the signal number (141 = 128 + 13).

### 15.5

**In the program in Figure 15.18, use the standard I/O library
    for reading and writing the pipes instead of `read` and `write`.**

[ex15_5.c](ex15_5.c)

### 15.6

**The Rationale for POSIX.1 gives as one of the reasons for adding the `waitpid` function
that most pre-POSIX.1 systems can't handle the following:**

```c
if ((fp = popen("/bin/true", "r")) == NULL)
...
if ((rc = system("sleep 100")) == -1)
...
if (pclose(fp) == -1)
...
```

**What happens in this code if `waitpid` isn't available and `wait` is used instead?**

If `waitpid()` is unavailable and `wait()` is used instead,
`system()` cannot guarantee that it waits for the child it created for "sleep 100".
Since `popen("/bin/true", "r")` creates another child that exits almost immediately,
a call to `wait()` inside `system()` may reap the `popen` child instead.
Then `system()` can return too early, while the "sleep 100" child is still running.
Later, `pclose(fp)` tries to wait for the popen child, but that child has already been reaped,
    so `pclose()` fails, typically with -1 and errno set to `ECHILD`.

### 15.7

**Explain how `select` and `poll` handle an input descriptor that is a pipe,
    when the pipe is closed by the writer.
To determine the answer, write two small test programs: one using `select` and one using `poll`.
Redo this exercise, looking at an output descriptor that is a pipe, when the read end is closed.**

[ex15_7_select_input.c](ex15_7_select_input.c)

```bash
$ ./a.out
parent: calling select ...
parent: select returned 1
parent: pipe read end is readable
parent: read 19 bytes: hello through pipe
parent: calling select ...
parent: select returned 1
parent: pipe read end is readable
parent: read returned 0 (EOF, writer closed)
```

[ex15_7_select_output.c](ex15_7_select_output.c)

```bash
$ ./a.out
parent: calling select ...
parent: select returned 1
parent: writable=0 except=1
parent: write of 1 byte succeeded
parent: calling select ...
parent: select returned 1
parent: writable=0 except=1
parent: write error: Broken pipe
parent: got EPIPE (read end is closed)
```

[ex15_7_poll_input.c](ex15_7_poll_input.c)

```bash
$ ./a.out
parent: calling poll ...
parent: poll returned 1
parent: revents = 0x1 POLLIN
parent: read 19 bytes: hello through pipe
parent: calling poll ...
parent: poll returned 1
parent: revents = 0x10 POLLHUP
parent: read returned 0 (EOF, writer closed)
```


[ex15_7_poll_output.c](ex15_7_poll_output.c)

```bash
$ ./a.out
parent: calling poll ...
parent: poll returned 1
parent: revents = 0x4 POLLOUT
parent: write of 1 byte succeeded
parent: calling poll ...
parent: poll returned 1
parent: revents = 0xc POLLOUT POLLERR
parent: write error: Broken pipe
parent: got EPIPE (read end is closed)
```

For `select`:

When the write end of a pipe is closed, the read end is reported by `select` as readable.
If no data remains in the pipe, a read on that descriptor returns 0, indicating end-of-file.
`select` itself does not provide a separate hangup indication such as `POLLHUP`.

When the read end of a pipe is closed, `select` may still indicate the descriptor is writable,
but a subsequent write fails with `EPIPE` (or the process is terminated by `SIGPIPE` if the signal is not ignored).

For `poll`:

When the write end of a pipe is closed, poll returns `POLLHUP` in revents.
If unread data still remains in the pipe, poll may return both `POLLIN` and `POLLHUP`.
The application should still use `read(...) == 0` to determine end-of-file definitively.

When the read end of a pipe is closed,
`poll` typically returns `POLLOUT` and often `POLLERR`; again, a subsequent write fails with `EPIPE` or raises `SIGPIPE`.

### 15.8

**What happens if the cmdstring executed by `popen` with a type of "r" writes to its standard error?**

Anything written by the child to standard error appears wherever the parent's standard error would appear.
To send standard error back to the parent, include the shell redirection `2>&1` in the cmdstring.

### 15.9

**Since `popen` invokes a shell to execute its cmdstring argument, what happens when cmdstring terminates? (Hint: Draw all the processes involved.)**

The `popen` function forks a child, and the child executes the shell.
The shell in turn calls `fork`, and the child of the shell executes the command string.
When cmdstring terminates, the shell is waiting for this to happen.
The shell then exits, which is what the `waitpid` in `pclose` is waiting for.

### 15.10

**POSIX.1 specifically states that opening a FIFO for read–write is undefined.
Although most UNIX systems allow this, show another method for opening a FIFO
    for both reading and writing, without blocking.**

The trick is to open the FIFO twice: once for reading and once for writing.
We never use the descriptor that is opened for writing, but leaving that descriptor open prevents an end of file
    from being generated when the number of clients goes from 1 to 0.
Opening the FIFO twice requires some care, as a nonblocking open is required.
We have to do a nonblocking, read-only open first, followed by a blocking open for write-only.
(If we tried a nonblocking open for write-only first, it would return an error.)
We then turn off nonblocking for the read descriptor.

[ex15_10.c](ex15_10.c)

### 15.11

**Unless a file contains sensitive or confidential data, allowing other users to read the file causes no harm.
(It is usually considered antisocial, however, to go snooping around in other people's files.)
But what happens if a malicious process reads a message from a message queue that is being used by a server and several clients?
What information does the malicious process need to know to read the message queue?**

Randomly reading a message from an active queue would interfere with the client–server protocol,
    as either a client request or a server's response would be lost.
To read the queue, all that is needed is for the process to know the identifier for the queue and for the queue to allow world-read access.

### 15.12

**Write a program that does the following. Execute a loop five times:
    create a message queue, print the queue identifier, delete the message queue.
Then execute the next loop five times: create a message queue with a key of `IPC_PRIVATE`,
    and place a message on the queue.
After the program terminates, look at the message queues using `ipcs(1)`.
Explain what is happening with the queue identifiers.**

[ex15_12.c](ex15_12.c)

```bash
$ ./a.out
first loop: create and delete 5 queues
 loop1[0]: msqid = 0
 loop1[1]: msqid = 1
 loop1[2]: msqid = 2
 loop1[3]: msqid = 3
 loop1[4]: msqid = 4

second loop: create 5 queues and leave a message in each
 loop2[0]: msqid = 5
 loop2[1]: msqid = 6
 loop2[2]: msqid = 7
 loop2[3]: msqid = 8
 loop2[4]: msqid = 9

program exits now; run: ipcs -q
```

```bash
$ ipcs -q

--------- 消息队列 -----------
键        msqid      拥有者  权限     已用字节数 消息
0x00000000 5          zhuang     600        6            1
0x00000000 6          zhuang     600        6            1
0x00000000 7          zhuang     600        6            1
0x00000000 8          zhuang     600        6            1
0x00000000 9          zhuang     600        6            1
```

In the first loop, each message queue is created and then immediately removed with `IPC_RMID`.
Although the internal slot used by the kernel may be reused,
    the returned queue identifier does not necessarily repeat.
This is because a System V IPC identifier is usually not just a simple table index;
    it often also contains a sequence number.
When a queue is removed and a new one is created in the same slot,
    the sequence number changes, so the new queue gets a different identifier.

In the second loop, `msgget(IPC_PRIVATE, ...)` creates a brand-new queue every time.
The queues are not removed, and one message is placed on each of them.
After the program terminates, these queues remain in the kernel,
    so they appear in the output of `ipcs -q`.

The key shown by ipcs is typically `0x00000000` because `IPC_PRIVATE` is the special key value 0.
Even though the displayed key is the same, each call with `IPC_PRIVATE` creates a distinct new queue
    rather than locating an existing one.

Thus, the queue identifiers keep changing because they are kernel-generated IPC IDs,
    typically derived from both a slot number and a sequence number,
rather than being permanently tied to a single reusable numeric value.

### 15.13

**Describe how to build a linked list of data objects in a shared memory segment.
What would you store as the list pointers?**

We never store actual addresses in a shared memory segment,
since it's possible for the server and all the clients to attach the segment at different addresses.

Instead, when a linked list is built in a shared memory segment,
the list pointers should be stored as offsets to other objects in the shared memory segment.
These offsets are formed by subtracting the start of the shared memory segment from the actual address of the object.

### 15.14

**Draw a timeline of the program in Figure 15.33 showing the value of the variable i
    in both the parent and child, the value of the long integer in the shared memory region,
    and the value returned by the update function.
Assume that the child runs first after the `fork`.**

<img src="https://invinc-z-drawing-bed.oss-cn-shanghai.aliyuncs.com/img/image-20260319223453432.png" alt="Alternation between parent and child in Figure 15.33" style="zoom:67%;" />

### 15.15

**Redo the program in Figure 15.33 using the XSI shared memory functions from Section 15.9 instead of the shared memory-mapped region.**

[ex15_15.c](ex15_15.c)

### 15.16

**Redo the program in Figure 15.33 using the XSI semaphore functions from Section 15.8 to alternate between the parent and the child.**

[ex15_16.c](ex15_16.c)

### 15.17

**Redo the program in Figure 15.33 using advisory record locking to alternate between the parent and the child.**

This exercise cannot be implemented reliably using **only advisory record locking**.

The first reason is that **`fcntl` record locks are not inherited across `fork`**.
To alternate correctly, both the parent and the child need a well-defined initial synchronization state:
    the child must initially wait for the parent, and the parent must later be able to wait for the child.
With record locking, the parent can place a lock before `fork`, but the child does not inherit that lock as its own.
Therefore, the child must establish its part of the protocol only after it starts running.
This creates a startup race, because the parent may continue execution
    before the child has finished setting up the lock that the parent is supposed to wait on.

The second reason is that **record locks do not have semaphore-like semantics**.
A record lock is owned by a process, not shared as a synchronization token between processes.
One process cannot release a lock held by another process,
    and a process that tries to lock a region it already owns does not block in the same way a semaphore wait would.
Because of this, even if the startup race is artificially avoided, the later rounds of alternation still fail:
    after one round, the ownership of the lock bytes no longer automatically returns to the state required for the next round.
In other words, record locking does not naturally provide the “wait, signal,
    and reestablish the next waiting state” behavior that this problem requires.

So, in summary, this problem cannot be solved with only advisory record locking
    because record locks are both **non-inheritable across `fork`**
    and **unsuitable as a true synchronization primitive for strict turn-by-turn alternation**.


### 15.18

**Redo the program in Figure 15.33 using the POSIX semaphore functions from Section 15.10 to alternate between the parent and the child.**

[ex15_18.c](ex15_18.c)

