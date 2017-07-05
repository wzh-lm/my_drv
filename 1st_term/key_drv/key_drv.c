#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *key_class;
static struct class_device *key_class_dev;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;
volatile unsigned long *gpgcon = NULL;
volatile unsigned long *gpgdat = NULL;

static int key_open(struct inode *inode , struct file *file)
{
	*gpfcon &= ~((3<<0*2)|(3<<2*2));
	*gpgcon &= ~((3<<3*2)|(3<<11*2));
	return 0;
}

static ssize_t key_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned char key_val[4];
	unsigned long val;
	if(count != sizeof(key_val))
	{
		return -EINVAL;
	}
	key_val[0] = *gpfdat & (1<<0) ? 1 : 0;
	key_val[1] = *gpfdat & (1<<2) ? 1 : 0;
	key_val[2] = *gpgdat & (1<<3) ? 1 : 0;
	key_val[3] = *gpgdat & (1<<11)? 1 : 0;

	val = copy_to_user(buf, key_val, sizeof(key_val));
	if(val != 0)		
	{
		printk(" ERROR: The data transfers fail !");
	}
	return 0;
}

static struct file_operations key_ops={
	.owner = THIS_MODULE,
	.open  = key_open,
	.read  = key_read,	
};


int major;

static int key_init(void)
{
	major = register_chrdev(0, "key_drv", &key_ops);

	key_class = class_create(THIS_MODULE,"key_drv");
	key_class_dev = class_device_create(key_class,NULL,MKDEV(major,0),NULL,"Buttons"); //mknod  /dev/Buttons c major 0;

	gpfcon = (unsigned long *)ioremap(0x56000050,16);
	gpfdat = gpfcon + 1;
	gpgcon = (unsigned long *)ioremap(0x56000060,16);
	gpgdat = gpgcon + 1;
	
	return 0;
}

static void key_exit(void)
{
	unregister_chrdev(major,"led_drv");
	class_device_unregister(key_class_dev);
	class_destroy(key_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
}


module_init(key_init);
module_exit(key_exit);

MODULE_LICENSE("GPL");


