# Process Control

## Exercises

### 9.1

**Refer back to our discussion of the `utmp` and `wtmp` files in Section 6.8. Why are the logout
records written by the `init` process? Is this handled the same way for a network login?**

The `init` process learns when a terminal user logs out,
    because `init` is the parent of the login shell
    and receives the `SIGCHLD` signal when the login shell terminates.

For a network login, however, `init` is not involved.
Instead, the login entries in the utmp and wtmp files,
    and their corresponding logout entries,
    are usually written by the process that handles the login
    and detects the logout (`telnetd` in our example).

### 9.2

**Write a small program that calls `fork` and has the child create a new session.
Verify that the child becomes a process group leader and that the child no longer has a controlling terminal.**

[ex9_2.c](ex9_2.c)
