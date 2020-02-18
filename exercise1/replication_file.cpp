#include <stdio.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){

    if((argc == 3) && (strcmp(argv[1], argv[2]) != 0)){
        int fd_src, fd_dest, ret;

        fd_src = open(argv[1], O_RDONLY);
        if(fd_src < 0){
            perror("open argv[1]");
            return -1;
        }

        fd_dest = open(argv[2], O_WRONLY|O_CREAT, 0755);//S_IRUSR|S_IWUSR
        if(fd_dest < 0){
            close(fd_src);
            perror("open argv[2]");
            return -1;
        }

        char buf[BUFFER_SIZE] = "";
        do{
            memset(buf,0,sizeof(buf));
            ret = read(fd_src, buf, sizeof(buf));
            if(ret > 0)
                write(fd_dest, buf, ret);
        }while(ret > 0);

        close(fd_src);
        close(fd_dest);
    }

    return 0;
}

