#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

#define FIRST_DRV_MAJOR		  0 
#define FIRST_DRV_DEVICE_NAME "led_drv"

static int major; 
static struct class *first_class;
static struct class_device	*first_class_devs;
volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat= NULL;
static int first_drv_open(struct inode *inode, struct file *file)
{
	/*GPF4、5、6 清零*/
	*gpfcon &= ~((0x3<<(4*2)) | (0x3<<(5*2))|(0x3<<(5*2)));	
	/*设置为输出*/
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2))|(0x1<<(5*2)));		



	printk("--(%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	return 0;
}

static int first_drv_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{	
	printk("-- (%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{	
	int val;
	copy_from_user(&val,buf,count);//用户空间到内核空间
	if(!val)
	{
		//点亮LED
		*gpfdat &= ~((1<<4)|(1<<5)|(1<<6));
	}
	else
	{
		//灭灯
		*gpfdat |= (1<<4)|(1<<5)|(1<<6);
	}
	printk("-- (%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	return 0;
}


/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   first_drv_open,     
	.read	=	first_drv_read,	   
	.write	=	first_drv_write,	   
};


/*
 * 执行insmod命令时就会调用这个函数 
 */
static int __init first_drv_init(void)
{
	/* 注册字符设备
	 * 参数为主设备号、设备名字、file_operations结构；
	 * 这样，主设备号就和具体的file_operations结构联系起来了，
	 */
	major = register_chrdev(FIRST_DRV_MAJOR, FIRST_DRV_DEVICE_NAME, &first_drv_fops);	
	printk("--  (%s(%d)--<%s) major = %d \n\n",__FILE__,__LINE__,__FUNCTION__,major);
	first_class = class_create(THIS_MODULE, FIRST_DRV_DEVICE_NAME);
    first_class_devs = class_device_create(first_class, NULL, MKDEV(major, 0), NULL, "xyz");	
	/*物理地址映射成虚拟地址*/
	gpfcon = (volatile unsigned long *)ioremap(0x56000050,16);
	gpfdat  = gpfcon + 1;

	return 0;
}


/*
 * 执行rmmod命令时就会调用这个函数 
 */
static void __exit first_drv_exit(void)
{
    /* 卸载驱动程序 */
    unregister_chrdev(major, FIRST_DRV_DEVICE_NAME);
	printk("--  (%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	class_device_unregister(first_class_devs);
	class_destroy(first_class);
	iounmap(gpfcon);
}

module_init(first_drv_init);
module_exit(first_drv_exit);
MODULE_LICENSE("GPL");





