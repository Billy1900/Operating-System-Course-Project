#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>



MODULE_LICENSE("GEL");
MODULE_AUTHOR("lnq");
#define DEV_NAME "lnqdriver"
static ssize_t GlobalRead(struct file *,char *,size_t,loff_t*);
static ssize_t GlobalWrite(struct file *,const char *,size_t,loff_t*);
static int char_major = 0;
static int GlobalData = 123456;


struct file_operations globalchar_fops ={
    .read = GlobalRead,
    .write = GlobalWrite
};

static int __init GlobalChar_init(void){
    int ret;
    ret = register_chrdev(char_major,DEV_NAME,&globalchar_fops);
    if(ret<0){
        printk(KERN_ALERT "GlobalChar Register Fail!\n");
        return -1;
    }
    else{
        printk(KERN_ALERT "GloblaChar Register Success!\n");
        char_major = ret;
        printk(KERN_ALERT "Major = %d\n",char_major);
    }
    return 0;
}

static void __exit GlobalChar_exit(void){
    unregister_chrdev(char_major,DEV_NAME);
    printk(KERN_ALERT "GlobalCharDev is dead now!\n");
    return;
}

static ssize_t GlobalRead(struct file *filp,char *buf,size_t len,loff_t *off){
    //GlobalData -= 1;
    if (copy_to_user(buf,&GlobalData,sizeof(int))){
        return -EFAULT;
    }
    return sizeof(int);
}

static ssize_t GlobalWrite(struct file *filp,const char *buf,size_t len,loff_t *off)
{
    if (copy_from_user(&GlobalData,buf,sizeof(int))){
        return -EFAULT;
    }
    return sizeof(int);
}

module_init(GlobalChar_init);
module_exit(GlobalChar_exit)

