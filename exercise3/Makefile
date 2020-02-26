ifneq ($(KERNELRELEASE),)    
#kbuild syntax.
mymodule-objs :=test.o        //模块的文件组成
obj-m :=mydev.o        //生成的模块文件名
else
PWD :=$(shell pwd)
KVER :=$(shell uname -r)
KDIR :=/lib/modules/$(KVER)/build
all:
	$(MAKE) -C $(KDIR) M=$(PWD)
clean:
	rm -f *.cmd *.o *.mod *.ko
endif

