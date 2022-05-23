#include <sys/types.h>
#include <sys/stat.h>
#include<stdio.h>

int main(int argc, char const *argv[])
{
    int res; 
    res = mkfifo("fifo_test", 0777);
    printf("Named pipe created\n");
    return 0;
}

