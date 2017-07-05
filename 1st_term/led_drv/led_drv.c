#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;

static struct class *led_class;
static struct class_device *led_class_dev[4];

int major,minor;

static ssize_t led_drv_open(struct inode *inode, struct file *file)
{
	minor = MINOR(inode->i_rdev);
	switch(minor)
	{
		case 0:
		{
			*gpfcon &= ~((3<<(4*2))|(3<<(5*2))|(3<<(6*2)));   //zeros 
			*gpfcon |=  (1<<(4*2))|(1<<(5*2))|(1<<(6*2));     //output	
			break;
		}
		case 1:
		{
			*gpfcon &= ~(3<<(4*2));
			*gpfcon |= 1<<(4*2);
			break;
		}
		case 2:
		{	
			*gpfcon &= ~(3<<(5*2));
			*gpfcon |= 1<<(5*2);
			break;
		}
		case 3:
		{
			*gpfcon &= ~(3<<(6*2));
			*gpfcon |= 1<<(6*2);
			break;
		}
	}
	
	printk("LED_ready...OK!\n");
	return 0;
}

static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;
	unsigned long i;
	i = copy_from_user(&val, buf, count);
	if(i != 0){
		printk("Fail to copy the dat from user!");
	}
	minor = MINOR(file->f_dentry->d_inode->i_rdev);
	switch(minor)
	{
		case 0:
		{
			if(val == 1)
			{		
				*gpfdat &= ~((1<<4)|(1<<5)|(1<<6));
			}
			else
			{
				*gpfdat |= (1<<4)|(1<<5)|(1<<6);
			}
			break;
		}
		case 1:
		{
			if(val == 1)
			{		
				*gpfdat &= ~(1<<4);
			}
			else
			{
				*gpfdat |= (1<<4);
			}
			break;
		}
		case 2:
		{
			if(val == 1)
			{		
				*gpfdat &= ~(1<<5);
			}
			else
			{
				*gpfdat |= (1<<5);
			}
			break;
		}
		case 3:
		{
			if(val == 1)
			{		
				*gpfdat &= ~(1<<6);
			}
			else
			{
				*gpfdat |= (1<<6);
			}
			break;
		}
	}
	return 0;
}

static struct file_operations firstdrv_ops={
	.owner  = THIS_MODULE,
	.open   = led_drv_open,
	.write  = led_drv_write,
};


static int led_drv_init(void)
{
	major = register_chrdev(0,"led_drv",&firstdrv_ops);
	led_class = class_create(THIS_MODULE,"led_drv");
	led_class_dev[0] = class_device_create(led_class,NULL,MKDEV(major,0),NULL,"leds");
	for(minor = 1; minor < 4; minor++)
	{
	led_class_dev[minor] = class_device_create(led_class,NULL,MKDEV(major,minor),NULL,"led%d",minor);
	}
	
	gpfcon = (volatile unsigned long *)ioremap(0x56000050,16); // first_address,map_size
	gpfdat = gpfcon + 1;
	return 0;
}

static void led_drv_exit(void)
{
	unregister_chrdev(major,"led_drv");
	for(minor = 0;minor < 4;minor++)
	{
		class_device_unregister(led_class_dev[minor]);
	}
	class_destroy(led_class);
	
	iounmap(gpfcon);
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");
