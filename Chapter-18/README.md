# Terminal I/O

## Exercises

### 18.1

**Write a program that calls `tty_raw` and terminates (without resetting the terminal mode).
If your system provides the `reset(1)` command (all four systems described in this text do),
    use it to restore the terminal mode.**

[ex18_1.c](ex18_1.c)

The program simply calls `tty_raw(STDIN_FILENO)` and exits without calling `tty_reset`.
As a result, the terminal remains in raw mode after the process terminates.
The shell session then behaves abnormally (for example, no echo and no canonical input processing).
The terminal can be restored by running the `reset(1)` command.

### 18.2

**The `PARODD` flag in the `c_cflag` field allows us to specify even or odd parity.
The BSD `tip` program, however, also allows the parity bit to be 0 or 1. How does it do this?**

It builds a table for each of the 128 characters and sets the high-order bit (the parity bit)
    according to the user's specification.
It then uses 8-bit I/O, handling the parity generation itself.

### 18.3

**If your system's `stty(1)` command outputs the `MIN` and `TIME` values,
do the following exercise.
Log in to the system twice and start the vi editor from one login.
Use the `stty` command from your other login to determine which values vi sets `MIN` and `TIME` to
(since vi sets the terminal to noncanonical mode).
(If you are running a windowing system on your terminal, you can do this same test
    by logging in once and using two separate windows instead.)**

I opened two terminal windows.
In the first window, I ran `tty` to find the terminal device name, then started `vi`.
In the second window, I ran `stty -a < /dev/pts/N` on the first window's terminal device.
While `vi` was running, the terminal was in noncanonical mode, and the reported values were typically `MIN = 1` and `TIME = 0`.
This allows `vi` to read each keystroke immediately without waiting for a newline.
