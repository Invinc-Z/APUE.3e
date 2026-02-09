# Process Environment

## Exercises

### 7.1

**On an Intel x86 system under Linux, if we execute the program that prints `hello, world`
and do not call exit or return, the termination status of the program
— which we can examine with the shell—is 13. Why?**

[ex7_1.c](ex7_1.c)

```bash
gcc -std=c89 ex7_1.c
./a.out
echo $?
```

It appears that the return value from printf (the number of characters output)
becomes the return value of main. To verify this theory, change the length of the
string printed and see if the new length matches the return value. Note that not
all systems exhibit this property. Also note that if you enable the ISO C extensions
in gcc, then the return value is always 0, as required by the standard.

### 7.2

**When is the output from the printfs in Figure 7.3 actually output?**

`main is done` is printed at main function,

`first/aecond exit handler` is printed when main return, and C start-up routine calls exit,
and exit function calls exit handler.

### 7.3

**Is there any way for a function that is called by main to examine the command-line
arguments without (a) passing argc and argv as arguments from main to the function or
(b) having main copy argc and argv into global variables?**

On most UNIX systems, there is no way to do this. Copies of argc and argv are
not kept in global variables like `environ` is.

### 7.4

**Some UNIX system implementations purposely arrange that, when a program is executed,
location 0 in the data segment is not accessible. Why?**

This provides a way to terminate the process when it tries to dereference a null
pointer, a common C programming error.

### 7.5

**Use the typedef facility of C to define a new data type Exitfunc for an exit handler.
Redo the prototype for atexit using this data type.**

```c
typedef void Exitfunc(void);
int atexit(Exitfunc* func);
```

### 7.6

**If we allocate an array of longs using calloc, is the array initialized to 0? If we allocate an
array of pointers using calloc, is the array initialized to null pointers?**

[ex7_6.c](ex7_6.c)

`calloc` initializes the memory that it allocates to all zero bits. ISO C does not
guarantee that this is the same as either a floating-point 0 or a null pointer.

### 7.7

**In the output from the size command at the end of Section 7.6, why aren’t any sizes given
for the heap and the stack?**

The heap and the stack aren't allocated until a program is executed by one of the `exec` functions.

### 7.8

**In Section 7.7, the two file sizes (879443 and 8378) don’t equal the sums of their respective
text and data sizes. Why?**

The exectable file(a.out) contains symbol table information that
can be useful in debugging a core file.
To improve this information, use the strip(1) command.
Stripping the two a.out files reduces their size.

```bash
$ gcc -static ex7_8.c
$ ls -l a.out
-rwxrwxr-x 1 zhuang zhuang 785360  2月  9 12:50 a.out
$ size a.out
  text	   data	    bss	    dec	    hex	filename
 668337	  23376	  22472	 714185	  ae5c9	a.out
$ strip -s a.out
$ ls -l a.out
-rwxrwxr-x 1 zhuang zhuang 706568  2月  9 12:54 a.out
$ gcc ex7_8.c
$ ls -l a.out
-rwxrwxr-x 1 zhuang zhuang 15960  2月  9 12:52 a.out
$ size a.out
   text	   data	    bss	    dec	    hex	filename
   1375	    600	      8	   1983	    7bf	a.out
$ strip -s a.out
$ ls -l a.out
-rwxrwxr-x 1 zhuang zhuang 14472  2月  9 12:57 a.out
```

### 7.9

**In Section 7.7, why does the size of the executable file differ so dramatically when we use
shared libraries for such a trivial program?**

When shared libraries are not used, a large portion of the executable file is
occupied by the standard I/O library.

### 7.10

**At the end of Section 7.10, we showed how a function can’t return a pointer to an automatic
variable. Is the following code correct?**

```c
int f1(int val)
{
    int num = 0;
    int *ptr = &num;
    if (val == 0) {
        int val;
        val = 5;
        ptr = &val;
    }
    return(*ptr + 1);
}
```

The code is incorrect, since it references the automatic integer val
    through a pointer after the automatic variable is no longer in existence.
Automatic variables declared after the left brace that
    starts a compound statement disappear after the matching right brace.
