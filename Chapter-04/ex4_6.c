#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUF_SIZE 4096

int main(int argc, char* argv[])
{
    if(argc < 3){
        printf("Usage: ./a.out file1 file2\n");
        exit(1);
    }

    int fd1, fd2;
    char buf[BUF_SIZE] = {0};

    if((fd1 = open(argv[1], O_RDONLY)) == -1){
        printf("Failed to open %s.\n", argv[1]);
        exit(1);
    }

    if((fd2 = open(argv[2], O_RDWR|O_CREAT, 0666)) == -1){
        printf("Failed to open %s.\n", argv[2]);
        exit(1);
    }

    int flag = 0; // 0 空洞状态 1 非空洞状态
    int pos = 0;  // buf中的位置
    int global_pos = 0; // fd2文件位置
    int sret, i;
    while((sret = read(fd1, buf, BUF_SIZE))){
        flag = pos = 0;
        for(i = 0; i < sret; i++){
            if(flag == 0 && buf[i] != 0) // 在空洞状态遇到非空洞，记录下来位置，lseek 调整位置
            {
                pos = i;
                flag = 1;
                if(lseek(fd2, global_pos + i, SEEK_SET) == -1){
                    perror("lseek");
                    exit(1);
                }
            }else if (flag == 1 && buf[i] == 0) // 在非空洞状态遇到空洞，将之前的非空洞序列拷贝
            {
                if(write(fd2, buf + pos, i - pos) == -1){
                    perror("write");
                    exit(1);
                }
                flag = 0;
            }
        }

        if(pos != sret){
            if(write(fd2, buf + pos, sret - pos) == -1){
                perror("write");
                exit(1);
            }
        }
        global_pos += sret;
    }

    return 0;
}
