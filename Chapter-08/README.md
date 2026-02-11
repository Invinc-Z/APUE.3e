# Process Control

## Exercises

### 8.1

**In Figure 8.3, we said that replacing the call to `_exit` with a call to `exit`
    might cause the standard output to be closed and printf to return −1.
Modify the program to check whether your implementation behaves this way.
    If it does not, how can you simulate this behavior?**

[ex8_1.c](ex8_1.c)

### 8.2

**Recall the typical arrangement of memory in Figure 7.6. Because the stack frames
corresponding to each function call are usually stored in the stack, and because after a
vfork the child runs in the address space of the parent, what happens if the call to vfork
is from a function other than main and the child does a return from this function after the
vfork? Write a test program to verify this, and draw a picture of what’s happening.**

[ex8_2.c](ex8_2.c)

When `vfork` is called, the parent's stack pointer points to the stack frame
    for the f1 function that calls `vfork`.
`vfork` causes the child to execute first, and the child returns from f1.
The child then calls f2, and its stack frame overwrites the previous stack frame for f1.
The child then zeros out the automatic variable buf, setting 1,000 bytes of the stack frame to 0.
The child returns from f2 and then calls `_exit`,
    but the contents of the stack beneath the stack frame for main have been changed.
The parent then resumes after the call to vfork and does a return from f1.
The return information is often stored in the stack frame,
and that information has probably been modified by the child.
After the parent resumes, the normal result is a core file.

### 8.3

**Rewrite the program in Figure 8.6 to use `waitid` instead of `wait`. Instead of calling
`pr_exit`, determine the equivalent information from the `siginfo` structure.**

[ex8_3.c](ex8_3.c)

### 8.4

When we execute the program in Figure 8.13 one time, as in

`$ ./a.out`

the output is correct. But if we execute the program multiple times,
one right after the other, as in

```shell
$ ./a.out ; ./a.out ; ./a.out
output from parent
ooutput from parent
ouotuptut from child
put from parent
output from child
utput from child
```

the output is not correct. What’s happening? How can we correct this? Can this problem
happen if we let the child write its output first?

In Figure 8.13, we have the parent write its output first. When the parent is done,
the child writes its output, but we let the parent terminate. Whether the parent
terminates or whether the child finishes its output first depends on the kernel’s
scheduling of the two processes (another race condition).
When the parent terminates, the shell starts up the next program, and this next program can
interfere with the output from the previous child.
We can prevent this from happening by not letting the parent terminate until the
child has also finished its output. Replace the code following the fork with the
following:

```c
else if (pid == 0) {
WAIT_PARENT();
/* parent goes first */
charatatime("output from child\n");
TELL_PARENT(getppid()); /* tell parent we’re done */
} else {
charatatime("output from parent\n");
TELL_CHILD(pid);
/* tell child we’re done */
WAIT_CHILD();
/* wait for child to finish */
}
```

[ex8_4_fixed.c](ex8_4_fixed.c)

We won’t see this happen if we let the child go first, since the shell doesn’t start
the next program until the parent terminates.

### 8.5

**In the program shown in Figure 8.20, we call `execl`, specifying the pathname of the
interpreter file. If we called `execlp` instead, specifying a filename of testinterp, and if
the directory /home/sar/bin was a path prefix, what would be printed as argv[2] when
the program is run?**

The same value (/home/sar/bin/testinterp) is printed for argv[2]. The
reason is that `execlp` ends up calling `execve` with the same pathname as when
we call `execl` directly. Recall Figure 8.15.

### 8.6

**Write a program that creates a zombie, and then call system to execute the ps(1) command
to verify that the process is a zombie.**

[ex8_6.c](ex8_6.c)

```bash
$ ./a.out
    PID    PPID S TT       COMMAND
   3109    3102 S pts/0    bash
   8466    3109 S pts/0    ./a.out
   8467    8466 Z pts/0    [a.out] <defunct>
   8468    8466 S pts/0    sh -c -- ps -o pid,ppid,state,tty,command
   8469    8468 R pts/0    ps -o pid,ppid,state,tty,command
```

### 8.7

**We mentioned in Section 8.10 that POSIX.1 requires open directory streams to be closed across an exec.
Verify this as follows: call `opendir` for the root directory,
    peek at your system's implementation of the `DIR` structure,
    and print the close-on-exec flag.
Then open the same directory for reading, and print the close-on-exec flag.**

[ex8_7.c](ex8_7.c)

```bash
$ ./a.out
opendir: FD_CLOEXEC is on
open: FD_CLOEXEC is off
```

