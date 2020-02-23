#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#define DEV_NAME "/dev/lnqdriver"

int main()
{
 int fd,num = 9999;
 
 fd = open(DEV_NAME,O_RDWR,S_IRUSR | S_IWUSR);
 if (fd < 0) 
 {
     printf("Open Device Failed!\n");
     return -1;
 }
 read(fd,&num,sizeof(int));
 printf("The lnqdriver is %d\n",num);
 printf("input a number written to lnqdriver: ");
 scanf("%d",&num);
 write(fd,&num,sizeof(int));
 read(fd,&num,sizeof(int));
 printf("The char you input is %d\n",num);
 
 close(fd);
 return 0;
}

