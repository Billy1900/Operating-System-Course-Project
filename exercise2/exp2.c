asmlinkage int sys_mycall(char* sourceFile,char* destFile)
{
    int source=sys_open(sourceFile,O_RDONLY,0);
    int  dest=sys_open(destFile,O_WRONLY|O_CREAT|O_TRUNC,0600);
    char buf[4096];
    mm_segment_t old_fs = get_fs();
    set_fs(get_ds());
    int i = 0;
    if(source>0 && dest>0){
        do{
            i=sys_read(source,buf,4096);
            sys_write(dest,buf,i);
        }while(i > 0);
    }
    else {
       printf("open file (src & des) failed!\n"); 
       return -1;
    }
    sys_close(source);
    sys_close(dest);
    set_fs(old_fs);
    return 1;
}
