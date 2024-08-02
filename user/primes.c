#include "kernel/types.h"
#include "user/user.h"
void calculate(int *pd)
{
    int num1,num2,fd[2];
    if(read(pd[0],&num1,sizeof(int))){      
        printf("prime %d\n",num1);
        pipe(fd);
        if(fork()==0){
            while(read(pd[0],&num2,sizeof(int))){
                if(num2%num1!=0){
                    write(fd[1],&num2,sizeof(int));
                }
            }            
        }
        else{
            wait(0);
            close(pd[0]);
            close(fd[1]);
            calculate(fd);   
        }
    }
    return;
}

int main(int argc,char* argv[])
{
    int fd[2];
    pipe(fd);
    if(fork()==0){
        for(int i=2;i<36;i++)
            write(fd[1],&i,sizeof(int));        
    }
    else{
        wait(0);
        close(fd[1]);
        calculate(fd);
    }
    exit(0);
}
