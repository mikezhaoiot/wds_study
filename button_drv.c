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
#include <linux/poll.h>


#define DEVICE_NAME  "button_drv"
#define BUTTON_MAJOR  221


/* 等待队列:
 * 当没有按键被按下时，如果有进程调用s3c24xx_buttons_read函数，
 * 它将休眠
 */
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，s3c24xx_buttons_read将它清0 */

static volatile int ev_press = 0;


static struct class *button_class;
static struct class_device *button_class_dev;
volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;
volatile unsigned long *gpgcon = NULL;
volatile unsigned long *gpgdat = NULL;

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

/*键值 ： 按下时，0x01 0x02 0x03 0x04*/
/*键值 ： 松开时，0x81 0x82 0x83 0x84*/
static unsigned char key_val;
struct pin_desc pins_desc[4] = {
	{S3C2410_GPF0,  0x01},
	{S3C2410_GPF2,  0x02},
	{S3C2410_GPG3,  0x03},
	{S3C2410_GPG11, 0x04},
};

static irqreturn_t buttons_irq(int irq,void *dev_id)
{
	struct pin_desc *pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	pinval = s3c2410_gpio_getpin(pindesc->pin);
	if(pinval)
	{
		/*松开*/
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/*按下*/
		key_val = pindesc->key_val;
	}
	ev_press = 1;                			/* 表示中断发生了 */
	wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */
	printk("-- buttons_irq --\n");
	return IRQ_HANDLED;
}


static int button_drv_open(struct inode *inode, struct file *file)
{
	request_irq(IRQ_EINT0, buttons_irq,IRQT_BOTHEDGE,"S2",&pins_desc[0]);
	request_irq(IRQ_EINT2, buttons_irq,IRQT_BOTHEDGE,"S3",&pins_desc[1]);
	request_irq(IRQ_EINT11,buttons_irq,IRQT_BOTHEDGE,"S4",&pins_desc[2]);
	request_irq(IRQ_EINT19,buttons_irq,IRQT_BOTHEDGE,"S5",&pins_desc[3]);
	printk("-- button drv open --\n");
	return 0;
}

static int button_drv_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	if(count != 1)
	{
		return -EINVAL;
	}
	/* 如果按键没有动作，则休眠*/
	wait_event_interruptible(button_waitq,ev_press);
	/*有按键动作，返回键值*/
	copy_to_user(buff,&key_val,1);
	ev_press = 0;
	return 1;
}
static ssize_t button_drv_write(struct file *file,const char __user *buf,size_t count,loff_t * ppos)
{
	return 0;
}

static unsigned button_drv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // 不会立即休眠

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}
int button_drv_close(struct inode *inode,struct file *file)
{
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11,&pins_desc[2]);
	free_irq(IRQ_EINT19,&pins_desc[3]);
	return 0;
}

static struct file_operations button_drv_fops = {
	.owner = THIS_MODULE,
	.open  = button_drv_open,
	.read  = button_drv_read,
	.write = button_drv_write,  
	.release = button_drv_close,
	.poll    = button_drv_poll,
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
