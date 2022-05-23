#include <unistd.h>
#include<stdio.h>
#include<fcntl.h>

int main(int argc, char const *argv[])
{
    int res,n;
    res = open("fifo_test", O_WRONLY);
    write(res, "Message", 7);
    printf("Sender process having PID %d sent the data\n", getpid());
    return 0;
}
