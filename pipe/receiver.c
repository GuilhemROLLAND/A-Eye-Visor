#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>

int main(int argc, char const *argv[])
{
    int res, n;
    char buffer[100];
    res=open("fifo_test", O_RDONLY);
    n = read(res, buffer, 100);
    printf("Reader process having PID %d started\n", getpid());
    printf("Data received by receiver %d is : %s\n", getpid(), buffer);
    return 0;
}
