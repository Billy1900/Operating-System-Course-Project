## 要求
掌握添加系统调用的方法

## 内容
- 采用编译内核的方法，添加一个新的系统调用，实现文件拷贝功能
- 编写一个应用程序，测试新加的系统调用


## source
- [How to install Linux Kernel](https://www.fosslinux.com/1639/how-to-install-linux-kernel-4-10-1-in-ubuntu-16-04.htm)
- [Linux 添加与删除内核](https://gitchat.csdn.net/activity/5ca372102db0d83fd5a25fb9)
- [如何编译 Linux 内核](https://linux.cn/article-9665-1.html)
- [Ubuntu下载指定内核镜像](https://blog.csdn.net/Ciellee/article/details/101059527)


## Issue
<pre>
我们在更新CentOS或者Ubuntu的内核时，执行make menuconfig可能看如这样的错误：

*** Unable to find the ncurses libraries or the
*** required header files.
*** ‘make menuconfig’ requires the ncurses libraries.
***
*** Install ncurses (ncurses-devel) and try again.

解决办法如下：
CentOS：
yum install -y ncurses-devel

Ubuntu：
sudo apt-get install ncurses-dev
</pre>
