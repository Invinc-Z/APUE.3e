# UNIX System Overview

## Exercises

### 1.1

**Verify on your system that the directories dot and dot-dot are not the same, except in the root directory.**

```shell
$ ls -ali .
总计 12
6964481 drwxrwxr-x 2 zhuang zhuang 4096 11月  6 19:30 .
6961769 drwxrwxr-x 6 zhuang zhuang 4096 11月  6 19:27 ..

$ ls -ali /
总计 3960932
      2 drwxr-xr-x  23 root root       4096  8月 10 20:48 .
      2 drwxr-xr-x  23 root root       4096  8月 10 20:48 ..
```

` -i` print the index number of each file.

### 1.2

**In the output from the program in  [Figure 1.6](../apue.3e/figlinks/fig1.6), what happened to the processes with process IDs 852 and 853?**

**UNIX系统是多进程或者多任务系统，当这个程序运行的同时还有其他两个进程号为852和853的进程在运行。**

### 1.3

**In Section 1.7, the argument to `perror` is defined with the ISO C attribute `const,` whereas the integer argument to `strerror` isn't defined with this attribute. Why?**

函数原型如下：

```c
#include <stdio.h>
void perror(const char *s);
```

```c
#include <string.h>
char *strerror(int errnum);
```

`const`限定s指向的字符串不可更改，`strerror`的参数为值传递，本身就无法修改传入的参数。

### 1.4

**If the calendar time is stored as a signed 32-bit integer, in which year will it overflow? How can we extend the overflow point? Are these strategies compatible with existing applications?**

$\frac{2^{31}}{3600 \times 24 \times 365} \approx 68.096$

$1970 + 68.096 = 2038.096$

2038年溢出，扩展为64位表示。

将 `time_t` 数据类型定为 64 位整型，就可以解决该问题了。如果它现在是 32位整型，那么为使应用程序正常工作，应当对其重编译。但是这一问题还有更糟糕之处。某些文件系统及备份介质以 32 位整型存放时间。对于这些同样需要加以更新，但又需要能读旧的格式。

### 1.5

**If the process time is stored as a signed 32-bit integer, and if the system counts 100 ticks per second, after how many days will the value overflow?**

$\frac{2^{31}}{100 \times 3600 \times 24} \approx 248$
