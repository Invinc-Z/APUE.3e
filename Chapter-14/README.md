# Advanced I/O

## Exercises

### 14.1

**Write a test program that illustrates your system's behavior when a process is blocked
    while trying to write lock a range of a file and additional read-lock requests are made.
Is the process requesting a write lock starved by the processes read locking the file?**

[ex14_1.c](ex14_1.c)

On FreeBSD 8.0, Linux 3.2.0, and Mac OS X 10.6.8, the behavior is the same:
additional readers can starve pending writers. Running the program gives us

```plaintext
child 1: obtained read lock on file
child 2: obtained read lock on file
child 3: can’t set write lock: Resource temporarily unavailable
child 3 about to block in write-lock...
parent: obtained additional read lock while write lock is pending
killing child 1...
child 1: exit after pause
killing child 2...
child 2: exit after pause
killing child 3...
child 3: can’t write-lock file: Interrupted system call
```

On Solaris 10, readers don't starve waiting writers.
In this case, the parent is unable to obtain a read lock because there is a process waiting for a write lock.

### 14.2

**Take a look at your system's headers and examine the implementation of `select` and the four `FD_ macros`.**

On my Linux, the implementation as follows:

```c
/* The fd_set member is required to be an array of longs.  */
typedef long int __fd_mask;

/* Some versions of <linux/posix_types.h> define this macros.  */
#undef	__NFDBITS
/* It's easier to assume 8-bit bytes than to get CHAR_BIT.  */
#define __NFDBITS	(8 * (int) sizeof (__fd_mask))
#define	__FD_ELT(d)	((d) / __NFDBITS)
#define	__FD_MASK(d)	((__fd_mask) (1UL << ((d) % __NFDBITS)))

/* fd_set for select and pselect.  */
typedef struct
  {
    /* XPG4.2 requires this member name.  Otherwise avoid the name
       from the global namespace.  */
#ifdef __USE_XOPEN
    __fd_mask fds_bits[__FD_SETSIZE / __NFDBITS];
# define __FDS_BITS(set) ((set)->fds_bits)
#else
    __fd_mask __fds_bits[__FD_SETSIZE / __NFDBITS];
# define __FDS_BITS(set) ((set)->__fds_bits)
#endif
  } fd_set;

/* Maximum number of file descriptors in `fd_set'.  */
#define	FD_SETSIZE		__FD_SETSIZE

#ifdef __USE_MISC
/* Sometimes the fd_set member is assumed to have this type.  */
typedef __fd_mask fd_mask;

/* Number of bits per word of `fd_set' (some code assumes this is 32).  */
# define NFDBITS		__NFDBITS
#endif


/* Access macros for `fd_set'.  */
#define	FD_SET(fd, fdsetp)	__FD_SET (fd, fdsetp)
#define	FD_CLR(fd, fdsetp)	__FD_CLR (fd, fdsetp)
#define	FD_ISSET(fd, fdsetp)	__FD_ISSET (fd, fdsetp)
#define	FD_ZERO(fdsetp)		__FD_ZERO (fdsetp)

/* We don't use `memset' because this would require a prototype and
   the array isn't too big.  */
#define __FD_ZERO(s) \
  do {									      \
    unsigned int __i;							      \
    fd_set *__arr = (s);						      \
    for (__i = 0; __i < sizeof (fd_set) / sizeof (__fd_mask); ++__i)	      \
      __FDS_BITS (__arr)[__i] = 0;					      \
  } while (0)
#define __FD_SET(d, s) \
  ((void) (__FDS_BITS (s)[__FD_ELT(d)] |= __FD_MASK(d)))
#define __FD_CLR(d, s) \
  ((void) (__FDS_BITS (s)[__FD_ELT(d)] &= ~__FD_MASK(d)))
#define __FD_ISSET(d, s) \
  ((__FDS_BITS (s)[__FD_ELT (d)] & __FD_MASK (d)) != 0)
```

Most systems define the `fd_set` data type to be a structure that contains a single member: an array of long integers.
One bit in this array corresponds to each descriptor.
The four `FD_` macros then manipulate this array of longs, turning specific bits on and off and testing specific bits.

One reason that the data type is defined to be a structure containing an array
    and not simply an array is to allow variables of type `fd_set`
    to be assigned to one another with the C assignment statement.

### 14.3

**The system headers usually have a built-in limit on the maximum number of descriptors
that the `fd_set` data type can handle.
Assume that we need to increase this limit to handle up to 2,048 descriptors. How can we do this?**

To use this technique with contemporary systems, we need to do several things:
1. Before we include any header files, we need to define whatever symbol prevents us from including `<sys/select.h>`.
Some systems might protect he definition of the fd_set type with a separate symbol.
We need to define this, too.

    For example, on FreeBSD 8.0, we need to define `_SYS_SELECT_H_` to prevent the inclusion of `<sys/select.h>`
    and we need to define `_FD_SET` to prevent the inclusion of the definition for the `fd_set` data type.

2. Sometimes, for compatibility with older applications, `<sys/types.h>` or `<bits/typesizes.h>` defines the size of the `fd_set`,
    so we need to include it first, then undefine `FD_SETSIZE`.
Note that some systems use `__FD_SETSIZE` instead.

3. We need to redefine `FD_SETSIZE` (or `__FD_SETSIZE`) to the maximum file descriptor number we want to be able to use with `select`.

4. We need to undefine the symbols we defined in step 1.

5. Finally, we can include `<sys/select.h>`. Before we run the program,
    we need to configure the system to allow us to open as many file descriptors as we might need
    so that we can actually make use of `FD_SETSIZE` file descriptors.

In short, cancel the definition of original `__FD_SETSIZE`, and redefine it, then include `<sys/select.h>`.

### 14.4

**Compare the functions provided for signal sets (Section 10.11) and the `fd_set` descriptor sets.
Also compare the implementation of the two on your system.**

The following table lists the functions that do similar things.

| fd_set  | signal set  |
| ------- | ----------- |
| FD_ZERO | sigemptyset |
| FD_SET  | sigaddset   |
| FD_CLR  | sigdelset   |
| FD_ISSET| sigismember |

There is not an `FD_xxx` function that corresponds to `sigfillset`.
With signal sets, the pointer to the set is always the first argument,
and the signal number is the second argument.
With descriptor sets, the descriptor number is the first argument, and the pointer to the set is the next argument.

On my Linux, data type `sigset_t` is defined as follows:

```c
#define _SIGSET_NWORDS (1024 / (8 * sizeof (unsigned long int)))
typedef struct
{
  unsigned long int __val[_SIGSET_NWORDS];
} __sigset_t;
```

and `fd_set` is defined as follows:

```c
/* Number of descriptors that can fit in an `fd_set'.  */
#define __FD_SETSIZE		1024
typedef long int __fd_mask;
#define __NFDBITS	(8 * (int) sizeof (__fd_mask))
/* fd_set for select and pselect.  */
typedef struct
  {
    __fd_mask fds_bits[__FD_SETSIZE / __NFDBITS];
#endif
  } fd_set;
```

In my 64-bit machine, they can both simplify as follows:

```c
typedef struct
{
     unsigned long int arr[16];
} xxx_set;
```

Here, `sigset_t` use unsigned and `fd_set` use signed.
In reality, there's no obvious difference because we just make operation of bits.

### 14.5

**Implement the function `sleep_us`, which is similar to `sleep`, but waits for a specified number of microseconds.
Use either select or poll. Compare this function to the BSD `usleep` function.**

[ex14_5.c](ex14_5.c)


### 14.6

**Can you implement the functions `TELL_WAIT`, `TELL_PARENT`, `TELL_CHILD`,
`WAIT_PARENT`, and `WAIT_CHILD` from Figure 10.24 using advisory record locking instead of signals?
If so, code and test your implementation.**

No. What we would like to do is have `TELL_WAIT` create a temporary file
    and use 1 byte for the parent's lock and 1 byte for the child's lock.
`WAIT_CHILD` would have the parent wait to obtain a lock on the child’s byte,
    and `TELL_PARENT` would have the child release the lock on the child's byte.
The problem, however, is that calling fork releases all the locks in the child,
so the child can't start off with any locks of its own.

### 14.7

**Determine the capacity of a pipe using nonblocking writes. Compare this value with the value of `PIPE_BUF` from Chapter 2.**

[ex14_7.c](ex14_7.c)

```plaintext
./a.out
write ret -1, pipe capacity = 65536
```

The value can differ from the corresponding `PIPE_BUF` values,
because `PIPE_BUF` is defined to be the maximum amount of data that can be written to a pipe **atomically**.
Here, we calculate the amount of data that a pipe can hold independent of any atomicity constraints.

`PIPE_BUF` is only 4096 bytes on my system.
The 65536 value is the maxmimum amount of data allowed in the pipe,
while the 4096 value is the maximum amount of data that can be written atomically (in one go).

### 14.8

**Rewrite the program in Figure 14.21 to make it a filter:
    read from the standard input and write to the standard output,
    but use the asynchronous I/O interfaces.
What must you change to make it work properly?
Keep in mind that you should get the same results
    whether the standard output is attached to a terminal, a pipe, or a regular file.**

[ex14_8.c](ex14_8.c)

### 14.9

**Recall Figure 14.23. Determine the break-even point on your system where using `writev`
    is faster than copying the data yourself and using a single write.**

[ex14_9.c](ex14_9.c)

```plaintext
./a.out
     bytes      writev(ns)  copy+write(ns)       faster
        16        35069956        25142324         copy
        32        31321693        25353680         copy
        64        32037565        25307594         copy
       128        30532481        24456805         copy
       256        31219421        23903812         copy
       512        31820542        23876076         copy
      1024        31177242        24187951         copy
      2048        31156349        26104219         copy
      4096        31210473        29906796         copy
      8192        31227883        36786706       writev
     16384        31335572        46906516       writev
     32768        31388649       137575998       writev
```

To determine the break-even point, I benchmarked two approaches:

1. `writev(fd, iov, 2)`

2. copying the two pieces into a contiguous buffer with `memcpy`, then calling `write(fd, buf, total)`

To avoid disk effects, I wrote to `/dev/null` and repeated each test many times using `clock_gettime` for timing.

My results showed that for small writes, the extra complexity of `writev` made it slightly slower than memcpy + write.
As the total size increased, the cost of copying became dominant, and `writev` became faster.

On my system, the break-even point was approximately 8192 bytes. Below this size, memcpy + write was faster; above this size, writev was faster.

### 14.10

**Run the program in Figure 14.27 to copy a file and determine whether the last-access time for the input file is updated.**

Yes.

Whether the program in Figure 14.27 updates the last-access time
    for the input file depends on the operating system and the type of file system in which the file resides.
On all four platforms, the last-access time is updated
    when the file resides in the default file system type for the given operating system.

### 14.11

**In the program from Figure 14.27, close the input file after calling `mmap`
to verify that closing the descriptor does not invalidate the memory-mapped I/O.**

[ex14_11.c](ex14_11.c)

