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

#define DEVICE_NAME 	"led_drv"	/* 加载模式后，执行 "cat /proc/devices" 命令看到的设备名称*/
#define LED_MAJOR		 231		/* 主设备号 */


static struct class *led_class;
static struct class_device	*led_class_devs[4];


static char leds_status = 0x0;
static DECLARE_MUTEX(leds_lock);

static unsigned long gpio_va;

#define GPIO_OFT(x) ((x) - 0x56000000)
#define GPFCON	(*(volatile unsigned long *)(gpio_va + GPIO_OFT(0x56000050)))
#define GPFDAT  (*(volatile unsigned long *)(gpio_va + GPIO_OFT(0x56000054)))

static int led_drv_open(struct inode *inode, struct file *file)
{
	int minor = MINOR(inode->i_rdev);
	printk("-- open minor = %d--\n\n",minor);
	switch(minor)
	{
		case 0:	/* /dev/leds */
		{
			//GPF4
			GPFCON &= ~(0x3<<(4*2));
			GPFCON |= (1<<(4*2));
			//GPF5
			GPFCON &= ~(0x3<<(5*2));	//清零
			GPFCON |= (1<<(5*2));
			//GPF6
			GPFCON &= ~(0x3<<(6*2));
			GPFCON |= (1<<(6*2));

			GPFDAT |= (~(1<<4))|(~(1<<5))|(~(1<<6));

			//down(&leds_lock);
			//leds_status = 0x0;
			//up(&leds_lock);
			break;
		}
		case 1:	/* /dev/led1 */
		{
			s3c2410_gpio_cfgpin(S3C2410_GPF4,S3C2410_GPF4_OUTP);

			//s3c2410_gpio_setpin(S3C2410_GPF4,0);
			//down(&leds_lock);
			//leds_status &= ~(1<<0);
			//up(&leds_lock);
			break;
		}
		case 2:	/* /dev/led2 */
		{
			s3c2410_gpio_cfgpin(S3C2410_GPF5,S3C2410_GPF5_OUTP);

			//s3c2410_gpio_setpin(S3C2410_GPF5,0);
			//leds_status &= ~(1<<1);
			break;
		}
		case 3:	/* /dev/led3 */
		{
			s3c2410_gpio_cfgpin(S3C2410_GPF6,S3C2410_GPF6_OUTP);
			
			//s3c2410_gpio_setpin(S3C2410_GPF6,0);
			//down(&leds_lock);
			//leds_status &= ~(1<<2);
			//up(&leds_lock);
			break;
		}
		default:
			break;

	}
	printk("--(%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	return 0;
}

static int led_drv_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{	
	printk("-- (%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	return 0;
}

static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{	
	int minor; 
	char val;
	minor = MINOR(file->f_dentry->d_inode->i_rdev);
	copy_from_user(&val,buf,1);	//用户空间复制到内核空间
	printk("-- (%s(%d)--<%s) minor = %d \n",__FILE__,__LINE__,__FUNCTION__,minor);
	switch(minor)
	{
		case 0:		/* /dev/leds */
		{
			s3c2410_gpio_setpin(S3C2410_GPF4, (val & 0x1));
			s3c2410_gpio_setpin(S3C2410_GPF5, (val & 0x1));
			s3c2410_gpio_setpin(S3C2410_GPF6, (val & 0x1));

			//down(&leds_lock);
			//leds_status = val;
			//up(&leds_lock);
			break;
		}
		case 1:		/* /dev/led1 */
		{
			s3c2410_gpio_setpin(S3C2410_GPF4,val);
			#if 0
			if(val == 0)
			{
				down(&leds_lock);
				leds_status &= ~(1<<0);
				up(&leds_lock);	
			}
			else
			{
				down(&leds_lock);
				leds_status |= (1<<0);
				up(&leds_lock);
			}
			#endif 
			break;
		}
		case 2:
		{
			s3c2410_gpio_setpin(S3C2410_GPF5,val);
			#if 0
			if(val == 0)
			{
				down(&leds_lock);
				leds_status &= ~(1<<1);
				up(&leds_lock);	
			}
			else
			{
				down(&leds_lock);
				leds_status |= (1<<1);
				up(&leds_lock);
			}
			#endif 
			break;
		}
		case 3:
		{
			s3c2410_gpio_setpin(S3C2410_GPF6,val);
			#if 0
			if(val == 0)
			{
				down(&leds_lock);
				leds_status &= ~(1<<2);
				up(&leds_lock);	
			}
			else
			{
				down(&leds_lock);
				leds_status |= (1<<2);
				up(&leds_lock);
			}
			#endif 
			break;
		}
	}
	printk("-- (%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	return 0;
}


/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations led_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   led_drv_open,     
	.read	=	led_drv_read,	   
	.write	=	led_drv_write,	   
};


/*
 * 执行insmod命令时就会调用这个函数 
 */
static int __init led_drv_init(void)
{
	int ret = 0;
	int minor = 0;
		
	/*物理地址映射成虚拟地址*/
	gpio_va = ioremap(0x56000000,0x100000);
	if(!gpio_va)
	{
		return -EIO;
	}
	/* 注册字符设备
	 * 参数为主设备号、设备名字、file_operations结构；
	 * 这样，主设备号就和具体的file_operations结构联系起来了，
	 */
	ret = register_chrdev(LED_MAJOR, DEVICE_NAME, &led_drv_fops);	
	if(ret < 0)
	{
		printk(DEVICE_NAME "can't register major number");
		return ret;
	}

	led_class = class_create(THIS_MODULE, DEVICE_NAME);

    led_class_devs[0] = class_device_create(led_class, NULL, MKDEV(LED_MAJOR, 0), NULL, "leds");	/* /dev/leds */
	
	for(minor = 1; minor < 4; minor++)
	{
		led_class_devs[minor] = class_device_create(led_class,NULL,MKDEV(LED_MAJOR,minor),NULL,"led%d",minor);
	}

	printk(DEVICE_NAME " initialized\n");


	return 0;
}


/*
 * 执行rmmod命令时就会调用这个函数 
 */
static void __exit led_drv_exit(void)
{
	int minor;
    /* 卸载驱动程序 */
    unregister_chrdev(LED_MAJOR, DEVICE_NAME);	
	for (minor = 0; minor < 4; minor++)
	{
		class_device_unregister(led_class_devs[minor]);
	}
	class_destroy(led_class);
	iounmap(gpio_va);
}
/*驱动程序的初始化函数和卸载函数*/
module_init(led_drv_init);
module_exit(led_drv_exit);

/*描述驱动程序的一些信息*/
MODULE_DESCRIPTION("S3C2440 LED Driver");
MODULE_LICENSE("GPL");





