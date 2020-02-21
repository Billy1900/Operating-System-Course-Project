# 要求
掌握添加系统调用的方法

# 内容
- 采用编译内核的方法，添加一个新的系统调用，实现文件拷贝功能
- 编写一个应用程序，测试新加的系统调用


# source
- [How to install Linux Kernel](https://www.fosslinux.com/1639/how-to-install-linux-kernel-4-10-1-in-ubuntu-16-04.htm)
- [Linux 添加与删除内核](https://gitchat.csdn.net/activity/5ca372102db0d83fd5a25fb9)
- [如何编译 Linux 内核](https://linux.cn/article-9665-1.html)
- [Ubuntu下载指定内核镜像](https://blog.csdn.net/Ciellee/article/details/101059527)


# Issue
### 1. Unable to find the ncurses libraries or the required header files.
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

### 2. openssl/opensslv.h: No such file or directory
<pre>
To fix this problem, you have to install OpenSSL development package, which is available in standard repositories of all modern Linux distributions.

To install OpenSSL development package on Debian, Ubuntu or their derivatives:
$ sudo apt-get install libssl-dev

To install OpenSSL development package on Fedora, CentOS or RHEL:
$ sudo yum install openssl-devel 
</pre>

### 3. [Kernel 5.1-rc1 breaks bcwml DKMS build (at least on Ubuntu)](https://www.reddit.com/r/linux/comments/b3nu83/kernel_51rc1_breaks_bcwml_dkms_build_at_least_on/)

**Problem:** 
I noticed that in the list of commits that went into version 5.1-rc1, there is one by Linus Torvalds that says `get rid of legacy 'get_ds()' function`

This function is used by DKMS on *buntu when building bcmwl, which is the non-free driver required by BCM 4352, e.g. on the Dell XPS 13 (9343) and some MacBooks, and some other Broadcom Wireless cards. I just tested it, and DKMS fails to build the module, with the following root error in the log:

`
/var/lib/dkms/bcmwl/6.30.223.271+bdcom/build/src/wl/sys/wl_cfg80211_hybrid.c:462:9: error: implicit declaration of function ‘get_ds’; did you mean ‘get_fs’? [-Werror=implicit-function-declaration]
  set_fs(get_ds());
         ^~~~~~
         get_fs
 `
 
**Solution:**
So, I had a quick look at the diff (which I found through the Kernel website) and from what I could tell, get_ds() was previously simply defined as

`#define get_ds()    (KERNEL_DS)`

and now all instances of it are removed; in all instances where get_ds() was previously used, it was replaced with KERNEL_DS.
