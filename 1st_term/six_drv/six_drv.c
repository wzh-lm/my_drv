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
static struct fasync_struct *button_async;
#if 0 //原子操作
static atomic_t v = ATOMIC_INIT(1);
#endif
static DECLARE_MUTEX(button_lock);
static DECLARE_WAIT_QUEUE_HEAD(button_press);

unsigned char key_val;
static int ev_press = 0;


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
	kill_fasync(&button_async, SIGIO, POLL_IN);  // SIGIO : 有IO  数据供读取  ;  POLL_IN : 有消息等待读取
	wake_up_interruptible(&button_press);
	ev_press = 1;
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int irq_open(struct inode *inode, struct file *file)
{
	int error;
	#if 0 
	if(!atomic_dec_and_test(&v))
	{
		atomic_inc(&v);    //原子变量增加1
		return -EBUSY;
	}
	#endif 
	down(&button_lock);
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
	wait_event_interruptible(button_press,ev_press);
	send_fail = copy_to_user(buf,&key_val,1);
	if(send_fail)
		return -EINVAL;
	ev_press = 0;
	return 0;		
}


static int irq_release(struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0, &pin_desc[0]);
	free_irq(IRQ_EINT2, &pin_desc[1]);
	free_irq(IRQ_EINT11,&pin_desc[2]);
	free_irq(IRQ_EINT19,&pin_desc[3]);
	#if 0 
	atomic_inc(&v);  
	#endif
	up(&button_lock);
	return 0;
}

static int button_fasync(int fd, struct file *file, int on)
{
	return fasync_helper(fd, file, on, &button_async);
}
	
static struct file_operations irq_ops = {
	.owner   = THIS_MODULE,
	.open    = irq_open,
	.read    = irq_read,
	.release = irq_release,
	.fasync  = button_fasync,
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

