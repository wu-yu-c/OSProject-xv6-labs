#include "kernel/types.h"
#include "user/user.h"
int main(int argc,char *argv[]) 
{
    if(argc!=2){
        write(2,"请输入正确的参数个数！\n用法：sleep time\n",strlen("请输入正确的参数个数！\n用法：sleep time\n"));
        exit(1);
    }
    int time=atoi(argv[1]);
    sleep(time);
    exit(0);
}
