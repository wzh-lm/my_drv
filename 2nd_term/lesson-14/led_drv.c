#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>


int major;
unsigned long pin_num;

static volatile unsigned long *gpfcon;
static volatile unsigned long *gpfdat;

static struct class *wzh_led_class;
static struct class_device *wzh_led_class_dev;

static int wzh_led_open(struct inode *inode, struct file *file)
{
	*gpfcon &= ~(3<<2*pin_num);
	*gpfcon |=   1<<2*pin_num ;

	*gpfdat |=   1<<pin_num;
	return 0;
}

static ssize_t wzh_led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	char pin_dat;
	int err;
	err = copy_from_user(&pin_dat, buf, 1);	
	if(err)
		return 1;
	if(pin_dat == 0)
	{
		*gpfdat |= 1<<pin_num;
	}
	else if(pin_dat == 1)
	{
		*gpfdat &= ~(1<<pin_num);
	}
	return 0;
}

static struct file_operations wzh_led_ops = {
	.open = wzh_led_open,
	.write = wzh_led_write,
};

static int wzh_led_probe(struct platform_device *pdev)
{
	struct resource *res;
	printk("match success!\n");
	major = register_chrdev(0, "wzh_led_chrdev", &wzh_led_ops);
	wzh_led_class = class_create(THIS_MODULE, "wzh_led_chrdev");
	wzh_led_class_dev = class_device_create(wzh_led_class, NULL, MKDEV(major, 0), NULL, "wzh_led");

	res     = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpfcon  = ioremap(res->start, res->end - res->start + 1); 
	gpfdat  = gpfcon + 1;
	
	res     = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	pin_num = res->start;
	return 0;
}

static int wzh_led_drv_remove(struct platform_device *pdev)
{
	printk("release!\n");
	unregister_chrdev(major, "wzh_led_chrdev");
	class_device_unregister(wzh_led_class_dev);
	class_destroy(wzh_led_class);
	
	iounmap(gpfcon);
	return 0;
}


static struct platform_driver wzh_led_drv ={
	.driver  = { 
		.name = "wzh_led",
	},
	.probe  = wzh_led_probe,
	.remove = wzh_led_drv_remove,
};


static int wzh_led_init(void)
{
	platform_driver_register(&wzh_led_drv);
	return 0;
}

static void wzh_led_exit(void)
{
	platform_driver_unregister(&wzh_led_drv);
}

module_init(wzh_led_init);
module_exit(wzh_led_exit);

MODULE_LICENSE("GPL");






