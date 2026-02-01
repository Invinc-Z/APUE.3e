## 安装

### Ubuntu

```shell
sudo apt-get install libbsd-dev
cd apue.3e
make
......

sudo cp ./include/apue.h /usr/include/
sudo cp ./lib/libapue.a /usr/local/lib/
```

APUE主页给的[源码](http://www.apuebook.com/code3e.html)在Ubuntu环境下 `make` 会出错，本仓库已作出修改。主要修改如下：

- 在`db/Makefile`中的`-dylib` 是 macOS 系统链接器的选项，要删除掉。具体是将`db/Makefile`第12行

  `LDCMD=$(CC) -shared -Wl,-dylib -o libapue_db.so.1 -L$(ROOT)/lib -lapue -lc db.o`

  修改为：

  `LDCMD=$(CC) -shared -o libapue_db.so.1 -L$(ROOT)/lib -lapue -lc db.o`

- `major`和`minor`的宏在linux下定义在`<sys/sysmacros.h>`中，须在`/include/apue.h`中包含该头文件。具体是在`/include/apue.h`第28行添加：

  `#include <sys/sysmacros.h>      /* for major and minor Macros */  `

- 在 `stdio/buf.c`中，linux下`FILE`结构体的字段名称需要做一些修改。具体是将代码最后三个函数中`return`语句（98行，104行，111行）分别修改为：

  ```c
  return(fp->_flags & _IONBF);				 // return(fp->_flag & _IONBF);
  return(fp->_flags & _IONBF);				 // return(fp->_flag & _IONBF);
  return(fp->_IO_buf_end - fp->_IO_buf_base);  // return(fp->_base - fp->_ptr);
  ```

  注释为修改前的代码。

### 其他操作系统

本仓库代码已修改为适应 Linux 系统，其他操作系统需下载源码或将上述修改恢复，源码下载地址在[这里](http://www.apuebook.com/code3e.html)。

### 编译源代码并安装头文件和库文件

```shell
cd apue.3e
make
......

sudo cp ./include/apue.h /usr/local/include/
sudo cp ./lib/libapue.a /usr/local/lib/
```

---

## 编译

```she
gcc <filename.c> -lapue
```

