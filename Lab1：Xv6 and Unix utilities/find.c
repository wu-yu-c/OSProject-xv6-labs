#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
void find(char *path,char *file)
{
  char buf[512], *p;
  //声明文件描述符
  int fd;
  struct dirent de;
  struct stat st;
  //open函数打开路径，返回一个fd，若为-1则打开失败
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open path \"%s\"\n", path);
    return;
  }
  //通过fd获取一个已经打开的文件的状态信息，并将其存储在st中
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat path \"%s\"\n", path);
    close(fd);
    return;
  }

  switch(st.type){
    case T_DEVICE:
    case T_FILE:
      fprintf(2, "find: \"%s\" 不是目录类型\n", path);
      break;
    case T_DIR:
      if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
        fprintf(2, "find: path too long\n");
        break;
      }
      //复制路径
      strcpy(buf, path);
      p = buf+strlen(buf);
      *p++ = '/';
      while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
          continue;
        //不递归 "." 和 ".."
        if (!strcmp(de.name, ".") || !strcmp(de.name, ".."))
          continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
          fprintf(2, "find: cannot stat %s\n", buf);
          continue;
        }
        //若为目录类型，则递归查找
        else if(st.type==T_DIR)
          find(buf,file);    
        //若为文件类型且与目标文件名称相同，则输出
        else if (st.type == T_FILE && !strcmp(de.name, file))
          printf("%s\n", buf);
      }
      break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
  if(argc!=3){
    write(2,"请输入正确的指令格式！\nusage：find dirName fileName\n",
    strlen("请输入正确的指令格式！\nusage：find dirName fileName\n"));
    exit(1);
  }
  else{
    find(argv[1],argv[2]);
    exit(0);
  }
}
