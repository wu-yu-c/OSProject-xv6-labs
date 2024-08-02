#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#define BUFSIZE 1024
int main(int argc, char* argv[])
{
  // 缓冲区
  char buf[BUFSIZE]={"\0"};
  // 存放子进程 exec 的参数
  char* xargs[MAXARG];
  int n, i = 0,pos=0;
  if (argc < 2) {
    fprintf(2, "参数个数有误！\nusage: xargs command\n");
    exit(1);
  }
  // 复制命令行参数到xargs数组
  for (int j = 1; j < argc; j++) {
    xargs[i] = argv[j];
    i++;
  }
  // 从标准输入读取数据到缓冲区buf中
  while ((n = read(0, buf+pos, 1)) > 0) {
    if (pos >= BUFSIZE) {
        fprintf(2, "xargs: arguments too long.\n");
        exit(1);
    }
    if(buf[pos]=='\n'){
      buf[pos]='\0';
      xargs[i]=buf;
      if (fork() == 0) {
        exec(argv[1], xargs);
      }
      wait(0);
      pos=0;
    }
    else
      pos++;
  }
  exit(0);
}