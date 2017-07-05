#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>

static struct resource wzh_led_resource[] = {
	[0] = {
		.start = 0x56000050,
		.end   = 0x56000050 + 8 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = 4,
		.end   = 4,
		.flags = IORESOURCE_IRQ,
	}
};

static void wzh_led_dev_release(struct device * dev)
{
}

static struct platform_device wzh_led_dev = {
	.name  	 	  = "wzh_led",
	.id        	  = -1,
	.resource	  = wzh_led_resource,
	.num_resources = ARRAY_SIZE(wzh_led_resource),
	.dev          = {
		.release = wzh_led_dev_release,
	},
};

static int wzh_led_dev_init(void)
{
	platform_device_register(&wzh_led_dev);
	return 0;
}

static void wzh_led_dev_exit(void)
{
	platform_device_unregister(&wzh_led_dev);
}

module_init(wzh_led_dev_init);
module_exit(wzh_led_dev_exit);

MODULE_LICENSE("GPL");
