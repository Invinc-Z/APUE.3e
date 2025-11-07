# Files and Directories

## Exercises

### 4.11

**In Section 4.22, our version of `ftw` never changes its directory. Modify this routine so that each time it encounters a directory, it uses the `chdir` function to change to that directory, allowing it to use the filename and not the pathname for each call to `lstat`. When all the entries in a directory have been processed, execute `chdir("..")`. Compare the time used by this version and the version in the text.**

测试时会受到缓存命中的影响，运行程序第二次的时间明显减少，采用的笨方法是开机就运行一个程序，运行完记录时间信息后**重启**再运行另一个程序。

不改变目录的原始版本[`ftw8.c`](ftw8.c)运行结果：

```shell
$ gcc ftw8.c -lapue
$ time ./a.out ~
regular files  =   25307, 77.41 %
directories    =    6812, 20.84 %
block special  =       0,  0.00 %
char special   =       0,  0.00 %
FIFOs          =       0,  0.00 %
symbolic links =     414,  1.27 %
sockets        =     160,  0.49 %
time: 6913370 us

real	0m6.920s
user	0m0.093s
sys		0m3.389s

$ time ./a.out /usr/
regular files  =  196743, 76.34 %
directories    =   26337, 10.22 %
block special  =       0,  0.00 %
char special   =       0,  0.00 %
FIFOs          =       0,  0.00 %
symbolic links =   34644, 13.44 %
sockets        =       0,  0.00 %
time: 35801839 us

real	0m35.834s
user	0m0.416s
sys		0m21.935s
```

改变目录使用文件名`lstat`的[`ex4_11_a.c`](ex4_11_a.c)运行结果：

```shell
$ gcc ex4_11.c -lapue
$ time ./a.out ~
regular files  =   25315, 77.41 %
directories    =    6812, 20.83 %
block special  =       0,  0.00 %
char special   =       0,  0.00 %
FIFOs          =       0,  0.00 %
symbolic links =     414,  1.27 %
sockets        =     161,  0.49 %
time: 6980234 us

real	0m6.987s
user	0m0.110s
sys		0m3.412s

$ time ./a.out /usr/
regular files  =  196743, 76.34 %
directories    =   26337, 10.22 %
block special  =       0,  0.00 %
char special   =       0,  0.00 %
FIFOs          =       0,  0.00 %
symbolic links =   34644, 13.44 %
sockets        =       0,  0.00 %
time: 30552446 us

real	0m30.559s
user	0m0.394s
sys		0m17.382s
```

改变目录且将文件名存在全局静态数组（全局数据区）而不是堆区的[`ex4_11_b.c`](ex4_11_b.c)运行结果：

```shell
$ gcc ex4_11.c -lapue
$ time ./a.out ~
regular files  =   25330, 77.42 %
directories    =    6812, 20.82 %
block special  =       0,  0.00 %
char special   =       0,  0.00 %
FIFOs          =       0,  0.00 %
symbolic links =     414,  1.27 %
sockets        =     163,  0.50 %
time: 6410887 us

real	0m6.416s
user	0m0.085s
sys		0m3.076s

$ time ./a.out /usr/
regular files  =  196743, 76.34 %
directories    =   26337, 10.22 %
block special  =       0,  0.00 %
char special   =       0,  0.00 %
FIFOs          =       0,  0.00 %
symbolic links =   34644, 13.44 %
sockets        =       0,  0.00 %
time: 31733553 us

real	0m31.738s
user	0m0.352s
sys		0m17.888s
```

从结果上看，整体来看有一些提升。其主要区别其实在`lstat`使用的是相对路径还是绝对路径。

在当前工作目录下，使用 `lstat` 访问文件时，**直接使用文件名（相对路径，如 `file.txt`）通常比使用绝对路径（如 `/home/user/cwd/file.txt`）更高效**，差异主要来自路径解析的开销，具体原因如下：

1. 路径解析步骤的差异

- **直接使用文件名（相对路径）**：  
  此时路径是当前工作目录（`cwd`）下的单组件路径（如 `file.txt`）。操作系统解析时，只需从 `cwd` 的 inode 出发，直接查找该文件名对应的目录项（dentry），步骤极少（通常1次目录查找）。

- **使用绝对路径**：  
  即使文件在当前目录，绝对路径仍需从根目录（`/`）开始，逐级解析每个目录组件（如 `/` → `home` → `user` → `cwd` → `file.txt`）。每个组件都需要查询对应目录的 inode 和目录项，步骤明显更多（组件数量等于路径层级数）。

2. 缓存的影响被削弱

操作系统的目录项（dentry）缓存和 inode 缓存能加速路径解析，但绝对路径的多级组件仍需逐个匹配缓存，而单组件相对路径只需匹配一次。即使缓存命中率很高，多组件的查询逻辑（如字符串比较、链表/哈希表查找）仍会产生额外的微小开销。

3. 实际开销的量级

- 单组件相对路径的解析几乎是“零额外开销”，直接基于 `cwd` 定位。
- 绝对路径的解析开销与路径层级成正比（例如，5级目录的绝对路径比相对路径多4次目录项查询）。  
  这种差异在单次调用中可能仅为几微秒，但在**高频调用场景**（如遍历目录下大量文件时反复调用 `lstat`）中，累积差异可能变得明显。

> `lstat` 处理绝对路径和相对路径的速度差异通常可以忽略不计，实际性能主要取决于：
>
> - 路径的总长度（组件数量）；
> - 路径组件是否命中系统缓存；
> - 系统调用的频率。

