# UNIX Standardization and Implementations

## Exercises

### 2.1

**We mentioned in Section 2.8 that some of the primitive system data types are defined in more than one header. For example, in FreeBSD 8.0, `size_t` is defined in 29 different headers. Because all 29 headers could be included in a program and because ISO C does not allow multiple `typedefs` for the same name, how must the headers be written?** 

下面是 FreeBSD 中使用的技术。在头文件`<machine/_types.h>`中定义可在多个头文件中出现的基本数据类型。例如：

```c
#ifndef _MACHINE__TYPES_H_
#define _MACHINE__TYPES_H_
typedef int __int32_t;
typedef unsigned int __uint32_t;
...
typedef __uint32_t __size_t;
...
#endif /* _MACHINE__TYPES_H_ */
```

在每个可以定义基本数据类型`size_t`的头文件中，包含下面的语句序列。

```c
#ifndef _SIZE_T_DECLARED
#define _SIZE_T_DECLARED
typedef __size_t size_t;
#endif
```

以这种方式，实际上只定义一次`size_t`。

### 2.2

**Examine your system's headers and list the actual data types used to implement the primitive system data types.**

在Ubuntu24.04系统上，以`gid_t`为例，该类型首先定义在`/usr/include/x86_64-linux-gnu/sys/types.h`：

```c
#ifndef __gid_t_defined
typedef __gid_t gid_t;
#define __gid_t_defined
#endif
```

而`__gid_t`定义在`/usr/include/x86_64-linux-gnu/bits/types.h`：

```c
# define __STD_TYPE     __extension__ typedef
#include <bits/typesizes.h> /* Defines __*_T_TYPE macros.  */ 
__STD_TYPE __PID_T_TYPE __pid_t;    /* Type of process identifications.  */   
```

` __PID_T_TYPE`定义在`/usr/include/x86_64-linux-gnu/bits/typesizes.h`：

```c
#define __PID_T_TYPE        __S32_TYPE
```

`__S32_TYPE`定义在`/usr/include/x86_64-linux-gnu/bits/types.h`：

```c
#define __S32_TYPE      int
```

### 2.3

**Update the program in [Figure 2.17](../apue.3e/figlinks/fig2.17) to avoid the needless processing that occurs when `sysconf` returns `LONG_MAX` as the limit for `OPEN_MAX`.**

**代码参考：**[figC.1](../apue.3e/figlinks/figC.1)
