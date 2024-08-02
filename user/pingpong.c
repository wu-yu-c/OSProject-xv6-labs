#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int father[2], child[2];
    pipe(father);
    pipe(child);
    char info[8];
    //处理子进程
    if (fork() == 0) {
        read(father[0], info, 4);
        printf("%d: received %s\n", getpid(), info);
        write(child[1], "pong", strlen("pong"));
    }
    //处理父进程
    else {
        write(father[1], "ping", strlen("ping"));
        wait(0);
        read(child[0], info, 4);
        printf("%d: received %s\n", getpid(), info);
    }
    exit(0);
}
