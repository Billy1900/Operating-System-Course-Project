# 要求
掌握添加设备驱动程序的方法

# 内容
采用模块方法，添加一个新的字符设备驱动程序，实现打开/关闭、读/写等基本操作,编写一个应用程序，测试添加的驱动程序

# sh
```
cd lnq_dev
make
sudo insmod lnq_dev.ko
```
And then log into root mode, compile the testdev.c and use it to write and read through device.
