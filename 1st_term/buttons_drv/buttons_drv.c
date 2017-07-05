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

#define GPF_DAT 0
#define GPG_DAT 1

static volatile unsigned long *gpfcon;
static volatile unsigned long *gpfdat;
static volatile unsigned long *gpgcon;
static volatile unsigned long *gpgdat;

int major;
unsigned char key_val;
unsigned int ev_press = 0;
static DECLARE_WAIT_QUEUE_HEAD(buttons_queue);
static DECLARE_MUTEX(buttons_lock);


static struct class *buttons_class;
static struct class_device *buttons_class_dev;
static struct timer_list buttons_timer;
static struct fasync_struct *buttons_fasync;




struct buttons_desc{
	unsigned int irq_num;
	char irq_name[5];	
	unsigned char pin_val;
	unsigned char which_gpdat;
	unsigned char gppin_num;
};

static struct buttons_desc buttons_desc[4] ={
	{IRQ_EINT0 , "S1", 1, GPF_DAT, 0},
	{IRQ_EINT2 , "S2", 2, GPF_DAT, 2},
	{IRQ_EINT11, "S3", 3, GPG_DAT, 3},
	{IRQ_EINT19, "S4", 4, GPG_DAT, 11},
};

static struct buttons_desc *irq_pd;

static void buttons_timer_function(unsigned long timer_desc)
{
	struct buttons_desc *buttons_desc_p = NULL;
	volatile unsigned long *gp_dat = NULL;
	buttons_desc_p = irq_pd ;
	if(buttons_desc_p == NULL)
	{
		return ;
	}
	//judge which gp port we should use
	if((buttons_desc_p->which_gpdat) == GPF_DAT)
	{
		gp_dat = gpfdat;
	}
	else
	{
		gp_dat = gpgdat;
	}
	
	if(!(*gp_dat & 1<<(buttons_desc_p->gppin_num)))
	{
		key_val = buttons_desc_p->pin_val;
	}
	else
	{
		key_val = buttons_desc_p->pin_val|0x80;
	}
	kill_fasync(&buttons_fasync, SIGIO, POLL_IN);
	wake_up_interruptible(&buttons_queue);
	ev_press = 1;
	return;
}

static irqreturn_t Buttons_irq(int irq, void *dev_id)
{
	irq_pd = (struct buttons_desc *)dev_id;
	mod_timer(&buttons_timer, jiffies + HZ/100);
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int Buttons_open(struct inode *inode, struct file *file)
{
	int Err,i;	
	if(file->f_flags&O_NONBLOCK)
	{
		if(down_trylock(&buttons_lock))
			return -EBUSY;
	}
	else
	{
		down(&buttons_lock);
	}
	for(i = 0; i < 4 ; i++)
	{
		Err = request_irq(buttons_desc[i].irq_num, Buttons_irq, IRQT_BOTHEDGE, buttons_desc[i].irq_name, &buttons_desc[i]);
		if(Err)
		{
			printk("Request irq failed!\n"); 
		}
	}
	*gpfcon &= ~((3<<2*4)|(3<<2*5)|(3<<2*6));
	*gpfcon |=   (1<<2*4)|(1<<2*5)|(1<<2*6);
	*gpfdat |=   (1<<4|1<<5|1<<6);
	
	printk("Open success! \n");
	return 0;
}

ssize_t Buttons_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{	
	int Err;
	//wait_event_interruptible(buttons_queue,ev_press);
	Err = copy_to_user(buf, &key_val, 1);
	if(Err)
	{
		printk("Kernel-Error:Something happened that make the message_tran fail!");
		return -EINVAL;
	}
	//ev_press = 0;
	return 0;
}

static int Buttons_fasync(int fd, struct file *file, int on)
{
	return fasync_helper(fd, file, on, &buttons_fasync);
}

static int Buttons_close(struct inode *inode , struct file *file)
{
	int i;
	for(i = 0; i < 4; i++)
		free_irq(buttons_desc[i].irq_num,&buttons_desc[i]);
	up(&buttons_lock);
	return 0;
}


struct file_operations Button_ops = {
	.owner   = THIS_MODULE,
	.open    = Buttons_open,
	.release = Buttons_close,
	.read    = Buttons_read,
	.fasync  = Buttons_fasync,
};

int Buttons_init(void)
{
	major = register_chrdev(0, "buttons_drv", &Button_ops);
	
	buttons_class     = class_create(THIS_MODULE, "button_drv");
	buttons_class_dev = class_device_create(buttons_class, NULL, MKDEV(major,0), NULL, "buttons_drv");

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;
	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1; 

	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer);
	return 0;
}

void Buttons_exit(void)
{
	unregister_chrdev(major, "buttons_drv");
	class_device_unregister(buttons_class_dev);
	class_destroy(buttons_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
}

module_init(Buttons_init);
module_exit(Buttons_exit);

MODULE_LICENSE("GPL");

