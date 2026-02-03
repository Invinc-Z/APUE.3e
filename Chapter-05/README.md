# Standard I/O Library

## Exercises

### 5.1

**Implement setbuf using setvbuf.**

```c
#include <stdio.h>

void my_setbuf(FILE *stream, char *buffer) {
    if (buffer == NULL) {
        // 如果 buffer 为 NULL，设置为无缓冲
        setvbuf(stream, NULL, _IONBF, 0);
    } else {
        // 否则，设置为全缓冲，缓冲区大小为 BUFSIZ
        setvbuf(stream, buffer, _IOFBF, BUFSIZ);
    }
}
```

### 5.2

**Type in the program that copies a file using line-at-a-time I/O (fgets and fputs) from
Figure 5.5, but use a MAXLINE of 4. What happens if you copy lines that exceed this length?
Explain what is happening.**

[5-2.txt](5-2.txt) 文本为：

```plaintext
Hello
world
```

1. 编译 `gcc ex5_2.c -g -lapue`
2. 调试 `gdb ./a.out`，设置断点`b 7`
3. 运行 `run < 5-2.txt`
4. 监视
```c
`display buf[0]`
`display buf[1]`
`display buf[2]`
`display buf[3]`
```

5. 打印的内容如下：
```plaintext
(gdb) display buf[0]
1: buf[0] = 72 'H'
(gdb) display buf[1]
2: buf[1] = 101 'e'
(gdb) display buf[2]
3: buf[2] = 108 'l'
(gdb) display buf[3]
4: buf[3] = 0 '\000'
```

6. 继续 `c`

```plaintext
1: buf[0] = 108 'l'
2: buf[1] = 111 'o'
3: buf[2] = 10 '\n'
4: buf[3] = 0 '\000'
```

观察可知，`fgets`读取一行，若超过`buf`的 size，将`buf`最后一位用零终止符填充，下一次接着该行没有读完的部分继续读。

### 5.3

**What does a return value of 0 from printf mean?**

当printf没有输出任何字符时，如 `printf("");`，函数调用返回0。

### 5.4

**The following code works correctly on some machines, but not on others. What could be the problem?**

```c
#include <stdio.h>
int main(void)
{
    char c;
    while ((c = getchar()) != EOF)
        putchar(c);
}
```

这是一个比较常见的错误。 `getc` 以及 `getchar` 的返回值是`int`类型，而不是`char`类型。

由于EOF经常定义为−1，那么如果系统使用的是有符号的字符类型，程序还可以正常工作。
但如果使用的是无符号字符类型，那么返回的EOF被保存到字符c后将不再是−1，所以，程序会进入死循环。
本书说明的4种平台都使用带符号字符，所以实例代码都能工作。

### 5.5

**How would you use the fsync function (Section 3.13) with a standard I/O stream?**

使用方法为：先调用 `fflush` 后调用 `fsync`。`fsync` 所使用的参数由 `fileno` 函数获得。

如果不调用 `fflush`，所有的数据仍然在内存缓冲区中，此时调用 `fsync` 将没有任何效果。

### 5.6

**In the programs in Figures 1.7 and 1.10, the prompt that is printed does not contain a
newline, and we don’t call fflush. What causes the prompt to be output?**

当程序以交互方式运行时，标准输入和标准输出都是行缓冲的。调用`fgets`时，标准输出会自动刷新。

### 5.7

**BSD-based systems provide a function called `funopen` that allows us to intercept read,
write, seek, and close calls on a stream. Use this function to implement fmemopen for
FreeBSD and Mac OS X.**

[ex5_7.c](ex5_7.c)
