#include<stdio.h>
#include<linux/unistd.h>
#include<asm/unistd.h>

int main(int argc,char **argv){
    int i=syscall(436,argv[1],argv[2]);
    printf("the result is %d",i);
    return 1;
}
