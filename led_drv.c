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

#define DEVICE_NAME 	"led_drv"	/* ����ģʽ��ִ�� "cat /proc/devices" ��������豸����*/
#define LED_MAJOR		 231		/* ���豸�� */


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
			GPFCON &= ~(0x3<<(5*2));	//����
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
	copy_from_user(&val,buf,1);	//�û��ռ临�Ƶ��ں˿ռ�
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


/* ����ṹ���ַ��豸��������ĺ���
 * ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
 * ���ջ��������ṹ��ָ���Ķ�Ӧ����
 */
static struct file_operations led_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   led_drv_open,     
	.read	=	led_drv_read,	   
	.write	=	led_drv_write,	   
};


/*
 * ִ��insmod����ʱ�ͻ����������� 
 */
static int __init led_drv_init(void)
{
	int ret = 0;
	int minor = 0;
		
	/*�����ַӳ��������ַ*/
	gpio_va = ioremap(0x56000000,0x100000);
	if(!gpio_va)
	{
		return -EIO;
	}
	/* ע���ַ��豸
	 * ����Ϊ���豸�š��豸���֡�file_operations�ṹ��
	 * ���������豸�žͺ;����file_operations�ṹ��ϵ�����ˣ�
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
 * ִ��rmmod����ʱ�ͻ����������� 
 */
static void __exit led_drv_exit(void)
{
	int minor;
    /* ж���������� */
    unregister_chrdev(LED_MAJOR, DEVICE_NAME);	
	for (minor = 0; minor < 4; minor++)
	{
		class_device_unregister(led_class_devs[minor]);
	}
	class_destroy(led_class);
	iounmap(gpio_va);
}
/*��������ĳ�ʼ��������ж�غ���*/
module_init(led_drv_init);
module_exit(led_drv_exit);

/*�������������һЩ��Ϣ*/
MODULE_DESCRIPTION("S3C2440 LED Driver");
MODULE_LICENSE("GPL");





