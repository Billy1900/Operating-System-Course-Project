#include<sys/types.h>
#include <wait.h>
#include<stdio.h>
#include<unistd.h>


int main()
{
    pid_t time;
    pid_t cir;
    pid_t sum;

    if((time=fork())==-1){
        printf("fork error\n");
        return -1;
    }
    else if(time==0){
        execlp("./time",0);
    }else{
        if((cir=fork())==-1){
            printf("fork error\n");
            return -1;
        }
        if(cir==0){
            execlp("./cir",0);
        }else{
            if((sum=fork())==-1){
                printf("fork error\n");
                return -1;
            }
            if(sum==0){
                execlp("./sum",0);
            }else{//father process
                wait(&time);
                wait(&cir);
                wait(&sum);
            }
        }
    }

}
