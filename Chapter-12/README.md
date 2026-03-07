# Thread Control

## Exercises

### 12.1

**Run the program in Figure 12.17 on a Linux system, but redirect the output into a file.
Explain the results.**

[ex12_1.c](ex12_1.c)

`./a.out`

```plaintext
thread started...
parent about to fork...
preparing locks...
parent unlocking locks...
parent returned from fork
child unlocking locks...
child returned from fork
```

`./a.out > outfile`

```plaintext
thread started...
parent about to fork...
preparing locks...
parent unlocking locks...
parent returned from fork
thread started...
parent about to fork...
preparing locks...
child unlocking locks...
child returned from fork
```

This is not a multithreading problem, as one might first guess.
The standard I/O routines are indeed thread-safe.
When we call fork, each process gets a copy of the standard I/O data structures.
When we run the program with standard output attached to a terminal,
    the output is line buffered, so every time we print a line,
    the standard I/O library writes it to our terminal.
However, if we redirect the standard output to a file,
    then the standard output is fully buffered.
The output is written when the buffer fills or the process closes the stream.
When we fork in this example, the buffer contains several printed lines not yet written,
so when the parent and the child finally flush their copies of the buffer,
the initial duplicate contents are written to the file.

### 12.2

**Implement `putenv_r`, a reentrant version of `putenv`.
Make sure that your implementation is async-signal safe as well as thread-safe.**

[ex12_2.c](ex12_2.c)

This function is a thread-safe function.
We cannot implement `putenv_r` as an async-signal safe function,
    because we have to do some malloc, and these function are not async-signal safe.

Moreover, we need to invoke some pthread lock function to implement thread-safe,
    the problem is that the pthread functions are not guaranteed to be async-signal safe,
    so we can't use them to make another function async-signal safe.

Some might suggest that one solution is to block all signals at the function `getenv_r` entry point.
However, the term async-signal-safe means the function can be invoked safely at the signal handlers
rather than block all signals firstly and restore it after unlock mutex.

### 12.3

**Can you make the `getenv` function shown in Figure 12.13 async-signal safe
by blocking signals at the beginning of the function and restoring the previous signal mask before
returning? Explain.**

Theoretically, if we arrange for all signals to be blocked when the signal handler runs,
    we should be able to make a function async-signal safe.
The problem is that we don't know whether any of the functions we call might unmask a signal that we've blocked,
    thereby making it possible for the function to be reentered through another signal handler.

And an async-signal-safe function is whether the function can be called safely at signal handlers,
    rather than block all signals. This is two things definitely.

### 12.4

**Write a program to exercise the version of `getenv` from Figure 12.13. Compile and run the
program on FreeBSD. What happens? Explain.**

[ex12_4.c](ex12_4.c)

On Linux is Ok.

On FreeBSD 8.0, the program drops core.
With gdb, we are able to see that the program initialization calls pthread functions,
    which call `getenv` to find the value of the `LIBPTHREAD_SPINLOOPS` and `LIBPTHREAD_YIELDLOOPS` environment variables.
However, our thread-safe version of `getenv` calls back into the pthread library while it is in an intermediate, inconsistent state.
In addition, the thread initialization functions call `malloc`, which, in turn,
    call `getenv` to find the value of the `MALLOC_OPTIONS` environment variable.

[ex12_4_flag.c](ex12_4_flag.c)

To get around this problem, we could make the reasonable assumption that program start-up is single threaded,
    and use a flag to indicate whether the thread initialization had been completed by our version of `getenv`.
While this flag is false, our version of `getenv` can operate as the non-reentrant version does
(and avoid all calls to pthread functions and malloc).
Then we could provide a separate initialization function to call `pthread_once`,
instead of calling it from inside `getenv`.
This requires that the program call our initialization function before calling `getenv`.
This solves our problem, because this can't be done until the program start-up initialization completes.
After the program calls our initialization function, our version of `getenv` operates in a thread-safe manner.

### 12.5

**Given that you can create multiple threads to perform different tasks within a program,
explain why you might still need to use fork.**

We still need fork if we want to run a program from within another program
(i.e., before calling exec).

### 12.6

**Reimplement the program in Figure 10.29 to make it thread-safe without using `nanosleep` or `clock_nanosleep`.**

[ex12_6.c](ex12_6.c)

### 12.7

**After calling `fork`, could we safely reinitialize a condition variable in the child process
    by first destroying the condition variable with `pthread_cond_destroy` and then initializing it with `pthread_cond_init`?**

The implementation of a condition variable most likely uses a mutex to protect its internal structure.
Because this is an implementation detail and therefore hidden,
    there is no portable way for us to acquire and release the lock in the fork handlers.
Since we can't determine the state of the internal lock in a condition variable after calling fork,
    it is unsafe for us to use the condition variable in the child process.

### 12.8

**The `timeout` function in Figure 12.8 can be simplified substantially. Explain how.**

The timeout function in Figure 12.8 is more complicated than necessary because it accepts an absolute timeout value,
then converts it into a relative interval before calling `clock_nanosleep`.

This conversion can be avoided in two ways.
First, the function could simply take a relative timeout argument in the first place.
Second, and more naturally, it could keep the absolute timeout argument and call `clock_nanosleep` with the `TIMER_ABSTIME` flag.
In that case, no subtraction from the current time is needed, so the code becomes substantially simpler.

[ex12_8a.c](ex12_8a.c)  relative timeout

[ex12_8b.c](ex12_8b.c)  absolute timeout
