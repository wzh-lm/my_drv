#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/interrupt.h>
 
static struct class *irq_class;
static struct class_device *irq_class_device;
static DECLARE_WAIT_QUEUE_HEAD(irq_drv);

unsigned int flag_int ;
unsigned char key_val;


volatile unsigned long *gpf_con = NULL;
volatile unsigned long *gpf_dat = NULL;
volatile unsigned long *gpg_con = NULL;
volatile unsigned long *gpg_dat = NULL;

struct pin_desc{
	unsigned int pin;
	unsigned int pin_val;
};

struct pin_desc pin_desc[4] = {
	{S3C2410_GPF0 , 0x01},
	{S3C2410_GPF2 , 0x02},
	{S3C2410_GPG3 , 0x03},
	{S3C2410_GPG11, 0x04},
};

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	struct pin_desc * pin_point = (struct pin_desc *)dev_id;  //强制转换后复制给另一个新建指针
	key_val = pin_point->pin_val;
	wake_up_interruptible(&irq_drv);
	flag_int = 1;
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int irq_open(struct inode *inode, struct file *file)
{
	int error;
	error = request_irq(IRQ_EINT0,  buttons_irq, IRQT_FALLING, "S1", &pin_desc[0]);
	if(error)
		return 1;
	error = request_irq(IRQ_EINT2,  buttons_irq, IRQT_FALLING, "S2", &pin_desc[1]);
	if(error)
		return 1;
	error = request_irq(IRQ_EINT11, buttons_irq, IRQT_FALLING, "S3", &pin_desc[2]);
	if(error)
		return 1;
	error = request_irq(IRQ_EINT19, buttons_irq, IRQT_FALLING, "S4", &pin_desc[3]);
	if(error)
		return 1;
	return 0;
}

static int irq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	int send_fail;
	if (size != 1)
		return -EINVAL;
	//wait_event_interruptible(irq_drv,flag_int);
	send_fail = copy_to_user(buf,&key_val,1);
	if(send_fail)
		return -EINVAL;
	//flag_int = 0;
	return 0;		
}

static unsigned irq_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &irq_drv, wait);
	if(flag_int)
	{
		mask |= POLLIN|POLLRDNORM;
		flag_int = 0;
	}
	return mask;
}

static int irq_release(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0, &pin_desc[0]);
	free_irq(IRQ_EINT2, &pin_desc[1]);
	free_irq(IRQ_EINT11,&pin_desc[2]);
	free_irq(IRQ_EINT19,&pin_desc[3]);
	return 0;
}
	
static struct file_operations irq_ops = {
	.owner   = THIS_MODULE,
	.open    = irq_open,
	.read    = irq_read,
	.release = irq_release,
	.poll    = irq_poll,
};

int major;
static int irq_drv_init(void)
{
	major = register_chrdev(0,"irq_drv",&irq_ops);
	irq_class = class_create(THIS_MODULE,"irq_drv");
	irq_class_device = class_device_create(irq_class,NULL,MKDEV(major,0),NULL,"irq_drv");
	gpf_con = (unsigned long *)ioremap(0x56000050,16);
	gpf_dat = gpf_con + 1;
	gpg_con = (unsigned long *)ioremap(0x56000060,16);
	gpf_dat = gpg_con + 1;
	return 0;
}

static void irq_drv_exit(void)
{
	unregister_chrdev(major,"irq_drv");
	class_device_unregister(irq_class_device);
	class_destroy(irq_class);
	iounmap(gpf_con);
	iounmap(gpg_con);	
}

module_init(irq_drv_init);
module_exit(irq_drv_exit);

MODULE_LICENSE("GPL");

