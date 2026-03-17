#include <stdio.h>

int main()
{
    int i = 0x12345678;
    char * pi = (char*)&i;
    if(*pi == 0x78)
    {
        printf("Little-Endian\n");
    }
    else {
        printf("Big-Endian\n");
    }
    return 0;
}
