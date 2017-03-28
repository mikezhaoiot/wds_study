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
#define FIRST_DRV_DEVICE_NAME "first_name"

static int major; 
static struct class *first_class;
static struct class_device	*first_class_devs;

static int first_drv_open(struct inode *inode, struct file *file)
{
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
}

module_init(first_drv_init);
module_exit(first_drv_exit);
MODULE_LICENSE("GPL");





