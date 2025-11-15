# Files and Directories

## Exercises

### 4.1

**Modify the program in Figure 4.3 to use `stat` instead of `lstat`. What changes if one of the command-line arguments is a symbolic link?**

分别编译运行[ex4_1_lstat.c](ex4_1_lstat.c)和[ex4_1_stat.c](ex4_1_stat.c)，结果如下所示：

```shell
$ gcc ex4_1_lstat.c -lapue
$ ./a.out readme
readme: symbolic link

$ gcc ex4_1_stat.c -lapue
$ ./a.out readme 
readme: regular
```

`stat`追随符号链接到实际链接的文件。`lstat`函数不追随，取符号链接本身的信息。

### 4.2

**What happens if the file mode creation mask is set to 777 (octal)? Verify the results using your shell’s `umask` command.**

```shell
$ umask 0777
$ touch mask-test
$ ll mask-test 
---------- 1 zhuang zhuang 0 11月  9 15:13 mask-test
umask 0002
```

文件权限为全部被关闭。

### 4.3

**Verify that turning off user-read permission for a file that you own denies your access to the file.**

```shell
$ echo 'hello, world.' > hello
$ cat hello
hello, world.
$ ll hello
-rw-rw-r-- 1 zhuang zhuang 14 11月  9 15:19 hello
$ chmod u-r hello
$ ll hello
--w-rw-r-- 1 zhuang zhuang 14 11月  9 15:19 hello
$ cat hello
cat: hello: 权限不够
```

### 4.4

**Run the program in Figure 4.9 after creating the files foo and bar. What happens?**

在创建foo和bar之前编译运行[ex4_4.c](ex4_4.c)：

```shell
$ gcc ex4_4.c -lapue
$ umask
0002
$ ./a.out 
$ ll foo bar
-rw------- 1 zhuang zhuang 0 11月  9 15:33 bar
-rw-rw-rw- 1 zhuang zhuang 0 11月  9 15:33 foo
```

创建foo和bar之后运行：

```shell
$ rm bar foo
$ echo 'hello' > bar
$ echo 'world' > foo
$ ll bar foo
-rw-rw-r-- 1 zhuang zhuang 6 11月  9 15:46 bar
-rw-rw-r-- 1 zhuang zhuang 6 11月  9 15:46 foo
$ ./a.out 
$ ll bar foo
-rw-rw-r-- 1 zhuang zhuang 0 11月  9 15:47 bar
-rw-rw-r-- 1 zhuang zhuang 0 11月  9 15:47 foo
```

在 Unix/Linux 系统中，`creat` 函数（注意拼写是 `creat`，而非 `create`）的行为如下：

当使用 `creat` 函数创建**已存在的文件**时：
- **不会报错**，而是会**截断该文件（清空内容）**，但**不会改变文件原有的权限位**。
- 函数的本质是调用 `open` 系统调用，等价于 `open(path, O_WRONLY | O_CREAT | O_TRUNC, mode)`，其中 `O_TRUNC` 标志会导致已存在的文件被截断（内容清空），而 `mode` 参数（权限位）仅在文件**新创建**时有效，若文件已存在，则 `mode` 会被忽略，原权限保持不变。

总结：`creat` 处理已存在的文件时，不报错、不重写权限位，但会清空文件内容。

### 4.5

**In Section 4.12, we said that a file size of 0 is valid for a regular file. We also said that the `st_size` field is defined for directories and symbolic links. Should we ever see a file size of 0 for a directory or a symbolic link?**

目录的大小从不为0,因为总是包含`.`和`..`目录项，符号链接的大小符号链接的所指向的路径名的字符个数，而符号链接无法指向一个名字为空的文件。

比如：

```shell
$ ll readme
lrwxrwxrwx 1 zhuang zhuang 9 11月  9 15:04 readme -> README.md
```

`readme`文件大小为9，即指向的`README.md`文件名字的字符个数。

### 4.6

**Write a utility like `cp(1)` that copies a file containing holes, without writing the bytes of 0 to the output file.**

先使用代码[make_sparse_file.c](make_sparse_file.c)生成稀疏文件，再使用[ex4_6.c](ex4_6.c)复制稀疏文件。

```shell
$ gcc make_sparse_file.c 
$ ./a.out 
$ gcc ex4_6.c 
$ ./a.out sparse_file dest
$ ll sparse_file dest 
-rw-rw-r-- 1 zhuang zhuang 10485760 11月 14 17:50 dest
-rw-r--r-- 1 zhuang zhuang 10485760 11月 14 17:49 sparse_file
```

### 4.7

**Note in the output from the `ls` command in Section 4.12 that the files core and core.copy have different access permissions. If the umask value didn't change between the creation of the two files, explain how the difference could have occurred.**

内核在创建新文件时，对文件访问权限位有一个默认设置。在本示例中，该设置为 `rw-r--r--`。这个默认值可能会也可能不会被umask值修改。shell在为重定向创建新文件时，对文件访问权限位也有一个默认设置。在本示例中，该设置为`rw-rw-rw-`，而且这个值总会被我们当前的umask修改。在本示例中，我们的umask为0002。

### 4.8

**When running the program in Figure 4.16, we check the available disk space with the `df(1)` command. Why didn't we use the `du(1)` command?**

我们不能使用`du`命令，因为它要么需要文件名，比如 `du tempfile`。要么需要目录名，比如`du .`。但当`unlink`函数返回时，`tempfile`的目录项就不存在了。

刚才展示的`du .`命令不会计算`tempfile`仍占用的空间。

在这个示例中，我们必须使用`df`命令来查看文件系统上的实际可用空间量。

### 4.9

**In Figure 4.20, we show the `unlink` function as modifying the changed-status time of the file itself. How can this happen?**

如果要删除的链接不是指向该文件的最后一个链接，那么该文件不会被删除。在这种情况下，文件的状态更改时间会被更新。但如果要删除的链接是指向该文件的最后一个链接，更新这个时间就没有意义了，因为关于该文件的所有信息（索引节点）都会随文件一起被删除。

### 4.10

**In Section 4.22, how does the system's limit on the number of open files affect the `myftw` function?**

我们在使用`opendir`打开目录后，会递归调用函数`dopath`。假设`opendir`使用单个文件描述符，这意味着每次我们下降一级目录，就会使用另一个描述符（我们假设该描述符要到处理完目录并调用`closedir`时才会关闭）。这将我们能够遍历的目录的深度和个数限制在了进程的最大打开描述符数量之内。

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

### 4.12

**Each process also has a root directory that is used for resolution of absolute pathnames. This root directory can be changed with the `chroot` function. Look up the description for this function in your manuals. When might this function be useful?**

`chroot`函数被互联网文件传输协议（FTP）程序用于增强安全性。在系统上没有账户的用户（称为匿名FTP用户）会被放置在一个单独的目录中，并且会对该目录执行`chroot`操作。这可以防止用户访问这个新根目录之外的任何文件。

此外，`chroot`可用于在新位置构建文件系统层次结构的副本，然后在不更改原始文件系统的情况下修改这个新副本。例如，这可用于测试新软件包的安装。

只有超级用户可以执行`chroot`，而且一旦更改了某个进程的根目录，该进程（及其所有子进程）就永远无法回到原始根目录了。

### 4.13

**How can you set only one of the two time values with the `utimes` function?**

首先，调用`stat`来获取文件的三个时间；然后调用`utimes`来设置所需的值。在调用`utimes`时，我们不想更改的值应该是来自`stat`的对应值。

### 4.14

**Some versions of the `finger(1)` command output ‘‘New mail received ...’’ and ‘‘unread since ...’’ where ... are the corresponding times and dates. How can the program determine these two times and dates?**

`finger(1)`命令会对邮箱调用`stat`函数。最后修改时间是邮件最后一次被接收的时间，而最后访问时间是邮件最后一次被阅读的时间。

### 4.15

**Examine the archive formats used by the `cpio(1)` and `tar(1)` commands. (These descriptions are usually found in Section 5 of the UNIX Programmer ’s Manual.) How many of the three possible time values are saved for each file? When a file is restored, what value do you think the access time is set to, and why?**

`cpio`和`tar`都只在归档文件中存储修改时间（`st_mtime`）。访问时间不会被存储，因为它的值与归档文件创建的时间相对应，毕竟要将文件归档就必须读取该文件。`cpio`的`-a`选项会在读取每个输入文件后重置其访问时间。这样一来，创建归档文件就不会改变访问时间。（不过，重置访问时间确实会修改更改状态时间。）更改状态时间不会存储在归档文件中，因为即便对其进行了归档，在提取文件时我们也无法设置这个值。（`utimes`函数及其相关函数`futimens`和`utimensat`只能更改访问时间和修改时间。）

当归档文件被回读（提取）时，`tar`默认会将修改时间恢复为归档文件中的值。`tar`的`m`选项指示它不要从归档文件中恢复修改时间，而是将修改时间设置为提取的时间。在使用`tar`的所有情况下，提取后的访问时间都将是提取的时间。

相比之下，`cpio` 会将访问时间和修改时间设置为提取的时间。默认情况下，它不会尝试将修改时间设置为存档中的值。`cpio` 的 `-m` 选项会使其将访问时间和修改时间都设置为存档时的值。

### 4.16

**Does the UNIX System have a fundamental limitation on the depth of a directory tree? To find out, write a program that creates a directory and then changes to that directory, in a loop. Make certain that the length of the absolute pathname of the leaf of this directory is greater than your system's `PATH_MAX` limit. Can you call `getcwd` to fetch the directory's pathname? How do the standard UNIX System tools deal with this long pathname? Can you archive the directory using either `tar` or `cpio`?**

内核对目录树的深度没有固有的限制。尽管如此，许多命令在路径名超过`PATH_MAX`时会失败。图C.3中所示的程序创建了一个深度为1000级的目录树，每一级都是一个45个字符的名称。无法在Linux使用`getcwd`获取第1000级目录的绝对路径名。该程序能够在Linux系统上检索到路径名，但必须多次调用`realloc`才能获得足够大的缓冲区。在Linux 系统上运行这个程序[ex4_16.c](ex4_16.c)，结果如下：

```shell
$ gcc ex4_16.c -lapue
 ./a.out 
getcwd failed, size = 4096: Numerical result out of range
getcwd failed, size = 4196: Numerical result out of range
...
...
...
418 more lines
getcwd failed, size = 45896: Numerical result out of range
getcwd failed, size = 45996: Numerical result out of range
length = 46004
the 46,004-byte pathname is printed here
```

对这个目录（包含子目录和文件）尝试使用`cpio`归档，结果如下所示：

```shell
$ cd /tmp
find alonglonglonglonglonglonglonglonglonglongname/ | cpio -ovH newc > alongname.cpio
...
...
函数 stat 失败: 文件名过长
```

提示说许多文件名太长了，无法使用`cpio`来归档这个目录。

从报错信息也可以看出`cpio`系统命令调用了`stat`函数。而`stat`的参数`pathnme`的限制由文件系统的最大文件名长度 和系统级的最大路径名长度决定。

1. 文件名长度限制（单个文件名）
- 内核限制：在 Linux 系统中，单个文件名（不含路径）的最大长度由常量 `NAME_MAX` 定义（通常为 255 字节）。这意味着 `stat` 调用传入的单个文件名长度不能超过 255 字节，否则会返回错误 `ENAMETOOLONG`。
- `cpio` 的处理：如果文件名超过 `NAME_MAX`，`stat` 会失败，`cpio` 会报错并跳过该文件。
2. 路径名长度限制（完整路径）
- 内核限制：完整路径名（如 `/a/b/c/long_filename`）的最大长度由常量 `PATH_MAX` 定义（通常为 4096 字节，不同系统可能不同）。若路径长度超过 `PATH_MAX`，`stat` 会返回 `ENAMETOOLONG` 错误。
- `cpio` 的处理：`cpio` 本身不额外限制路径长度，但依赖 `stat` 的结果。如果路径过长导致 `stat`失败，`cpio` 会报错。

实际上，在所有四个平台上，`cpio`都无法归档这个目录。

可以在Linux系统上使用`tar`来归档这个目录。但是，在Linux系统上，无法从该归档文件中提取出目录结构。

```shell
$ cd /tmp
$ tar -cvf alongname.tar alonglonglonglonglonglonglonglonglonglongname/
...
$ rm -rf alonglonglonglonglonglonglonglonglonglongname/
$ tar -xvf alongname.tar
...
...：无法 mkdir: 文件名过长
...
...：无法 open: 文件名过长
tar: 由于前次错误，将以上次的错误状态退出
```

### 4.17

**In Section 3.16, we described the `/dev/fd` feature. For any user to be able to access these files, their permissions must be `rw-rw-rw-`. Some programs that create an output file delete the file first, in case it already exists, ignoring the return code:**

```c
unlink(path);
if ((fd = creat(path, FILE_MODE)) < 0)
	err_sys(...);
```

**What happens if path is `/dev/fd/1`?**

编译运行[ex_17.c](ex_17.c)，结果如下：

```shell
$ gcc ex_17.c -lapue
$ ./a.out /dev/fd/1
unlink failure: Operation not permitted

$ ls -ald /dev
drwxr-xr-x 19 root root 4340 11月 14 15:49 /dev
```

`/dev`目录的普通用户写入权限已关闭，以防止普通用户删除该目录中的文件名。这意味着解除链接的尝试会失败。

