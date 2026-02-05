# System Data Files and Information

## Exercises

### 6.1

**If the system uses a shadow file and we need to obtain the encrypted password,
how do we do so?**

For Linux, get the field `sp_pwdp` from the struct `spwd` in shadow file.

### 6.2

**If you have superuser access and your system uses shadow passwords,
implement the previous exercise.**

[ex6_2.c](ex6_2.c)

For Linux, 以超级用户运行 `sudo ./a.out`

### 6.3

**Write a program that calls `uname` and prints all the fields in the `utsname` structure.
Compare the output to the output from the `uname(1)` command.**

[ex6_3.c](ex6_3.c)

```plaintext
$ ./a.out
sysname: Linux
nodename: VM-Ubuntu2404
release: 6.14.0-37-generic
version: #37~24.04.1-Ubuntu SMP PREEMPT_DYNAMIC Thu Nov 20 10:25:38 UTC 2
machine: x86_64
domainname: (none)
```

```plaintext
$ uname --all
Linux VM-Ubuntu2404 6.14.0-37-generic #37~24.04.1-Ubuntu SMP PREEMPT_DYNAMIC Thu Nov 20 10:25:38 UTC 2 x86_64 x86_64 x86_64 GNU/Linux
```

### 6.4

**Calculate the latest time that can be represented by the `time_t` data type.
After it wraps around, what happens?**

`time_t` represents the number of seconds since 1970-01-01 00:00:00 UTC.

If `time_t` is a signed 32-bit integer, the maximum value is $2^{31} − 1 = 2147483647$ seconds,
which corresponds to 2038-01-19 03:14:07 UTC.

After it wraps around, the value overflows to $−2^{31}$,
causing the time to jump back to 1901-12-13 20:45:52 UTC.

[ex6_4.c](ex6_4.c)

`gcc -m32 ex6_4.c`

### 6.5

**Write a program to obtain the current time and print it using `strftime`,
so that it looks like the default output from `date(1)`.
Set the `TZ` environment variable to different values and see what happens.**

```bash
$ date
2026年 02月 05日 星期四 11:20:25 CST
```

[ex6_5.c](ex6_5.c)

```bash
./a.out
2026 02 05 Thursday 11:21:22 CST
$ TZ=US/Mountain ./a.out
2026 02 05 Thursday 03:24:01
```

