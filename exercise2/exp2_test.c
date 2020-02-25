#include<stdio.h>
#include<linux/unistd.h>
#include<asm/unistd.h>

int main(int argc,char **argv){
    int i=syscall(436,argv[1],argv[2]);
    if (i==1)
      printf("succeed!\n");
    else
      printd("failed!\n");
    return 1;
}
