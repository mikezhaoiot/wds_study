/* 
 1. 驱动框架、填充
 2. 看原理图 
    EINT0  - GPF0 
	EINT2  - GPF2 
	EINT11 - GPG3
	EINT19 - GPG11 
  3. 查看数据手册
  	GPFCON 0x56000050
	GPFDAT 0x56000054 
	GPGCON 0x56000060
	GPGDAT 0x56000064
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

#define DEVICE_NAME  "button_drv"
#define BUTTON_MAJOR  221

static struct class *button_class;
static struct class_device *button_class_dev;
volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;
volatile unsigned long *gpgcon = NULL;
volatile unsigned long *gpgdat = NULL;

static irqreturn_t buttons_irq(int irq,void *dev_id)
{

	printk("-- buttons_irq --\n");
	return IRQ_HANDLED;
}


static int button_drv_open(struct inode *inode, struct file *file)
{
	request_irq(IRQ_EINT0, buttons_irq,IRQT_BOTHEDGE,"S2",1);
	request_irq(IRQ_EINT2, buttons_irq,IRQT_BOTHEDGE,"S3",1);
	request_irq(IRQ_EINT11,buttons_irq,IRQT_BOTHEDGE,"S4",1);
	request_irq(IRQ_EINT19,buttons_irq,IRQT_BOTHEDGE,"S5",1);
	printk("-- button drv open --\n");
	return 0;
}

static int button_drv_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	/* 返回四个引脚的电平*/
	unsigned char key_value[4];
	int regval;
	if(count != sizeof(key_value))
	{
		return -EINVAL;
	}

	/* 读GPF0 2 */
	regval = *gpfdat;
	key_value[0] = (regval & (1<<0)) ? 1 : 0;
	key_value[1] = (regval & (1<<2)) ? 1 : 0;

	/* 读GPG3 11 */
	regval = *gpgdat;
	key_value[2] = (regval & (1<<3))  ? 1 : 0;
	key_value[3] = (regval & (1<<11)) ? 1 : 0;

	copy_to_user(buff,key_value,sizeof(key_value));	//内核空间到用户空间
	return sizeof(key_value);
}
static ssize_t button_drv_write(struct file *file,const char __user *buf,size_t count,loff_t * ppos)
{
	return 0;
}
int button_drv_close(struct inode *inode,struct file *file)
{
	free_irq(IRQ_EINT0,1);
	free_irq(IRQ_EINT2,1);
	free_irq(IRQ_EINT11,1);
	free_irq(IRQ_EINT19,1);
	return 0;
}

static struct file_operations button_drv_fops = {
	.owner = THIS_MODULE,
	.open  = button_drv_open,
	.read  = button_drv_read,
	.write = button_drv_write,  
	.release = button_drv_close,
};


static int __init button_drv_init(void)
{
	int ret = 0;
	//注册驱动程序
	ret = register_chrdev(BUTTON_MAJOR,DEVICE_NAME,&button_drv_fops);	
	if(ret < 0)
	{
		printk("-- register fail --\n");
		return 0;
	}
	/* 自动创建设备节点 */
	button_class = class_create(THIS_MODULE,DEVICE_NAME);
	button_class_dev = class_device_create(button_class,NULL,MKDEV(BUTTON_MAJOR,0),NULL,"button");
	/* 物理地址转换成虚拟地址 */
	gpfcon = (volatile unsigned long *)ioremap(0x56000050,16);
	gpfdat = gpfcon + 1;
	gpgcon = (volatile unsigned long *)ioremap(0x56000060,16);
	gpgdat = gpgcon + 1;
	printk("-- button init --\n");
	return 0;
}
static void __exit button_drv_exit(void)
{
	unregister_chrdev(BUTTON_MAJOR,DEVICE_NAME);
	class_device_unregister(button_class_dev);
	class_destroy(button_class);
	iounmap(gpfcon);
	return 0;
}

module_init(button_drv_init);
module_exit(button_drv_exit);
MODULE_LICENSE("GPL");
