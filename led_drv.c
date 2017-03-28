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
	/*GPF4��5��6 ����*/
	*gpfcon &= ~((0x3<<(4*2)) | (0x3<<(5*2))|(0x3<<(5*2)));	
	/*����Ϊ���*/
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
	copy_from_user(&val,buf,count);//�û��ռ䵽�ں˿ռ�
	if(!val)
	{
		//����LED
		*gpfdat &= ~((1<<4)|(1<<5)|(1<<6));
	}
	else
	{
		//���
		*gpfdat |= (1<<4)|(1<<5)|(1<<6);
	}
	printk("-- (%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	return 0;
}


/* ����ṹ���ַ��豸��������ĺ���
 * ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
 * ���ջ��������ṹ��ָ���Ķ�Ӧ����
 */
static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   first_drv_open,     
	.read	=	first_drv_read,	   
	.write	=	first_drv_write,	   
};


/*
 * ִ��insmod����ʱ�ͻ����������� 
 */
static int __init first_drv_init(void)
{
	/* ע���ַ��豸
	 * ����Ϊ���豸�š��豸���֡�file_operations�ṹ��
	 * ���������豸�žͺ;����file_operations�ṹ��ϵ�����ˣ�
	 */
	major = register_chrdev(FIRST_DRV_MAJOR, FIRST_DRV_DEVICE_NAME, &first_drv_fops);	
	printk("--  (%s(%d)--<%s) major = %d \n\n",__FILE__,__LINE__,__FUNCTION__,major);
	first_class = class_create(THIS_MODULE, FIRST_DRV_DEVICE_NAME);
    first_class_devs = class_device_create(first_class, NULL, MKDEV(major, 0), NULL, "xyz");	
	/*�����ַӳ��������ַ*/
	gpfcon = (volatile unsigned long *)ioremap(0x56000050,16);
	gpfdat  = gpfcon + 1;

	return 0;
}


/*
 * ִ��rmmod����ʱ�ͻ����������� 
 */
static void __exit first_drv_exit(void)
{
    /* ж���������� */
    unregister_chrdev(major, FIRST_DRV_DEVICE_NAME);
	printk("--  (%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	class_device_unregister(first_class_devs);
	class_destroy(first_class);
	iounmap(gpfcon);
}

module_init(first_drv_init);
module_exit(first_drv_exit);
MODULE_LICENSE("GPL");





