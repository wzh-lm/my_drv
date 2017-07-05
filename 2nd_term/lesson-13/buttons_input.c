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

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>

struct pin_desc{
	int   irq_num;
	char *irq_name;
	int   key_val;
	int   pin;
};

static struct pin_desc pin_desc[4]={
	{IRQ_EINT0,  "S2", KEY_L,         S3C2410_GPF0},
	{IRQ_EINT2,  "S3", KEY_S,         S3C2410_GPF2},
	{IRQ_EINT11, "S4", KEY_ENTER,     S3C2410_GPG3},
	{IRQ_EINT19, "S5", KEY_LEFTSHIFT, S3C2410_GPG11},
};
static struct pin_desc *irq_pd;
static struct input_dev *buttons_dev;
static struct timer_list buttons_timer;

void buttons_timer_function(unsigned long pd)
{
	if(irq_pd == NULL)
		return ;
	if(s3c2410_gpio_getpin(irq_pd->pin))
	{
		input_event(buttons_dev, EV_KEY, irq_pd->key_val, 0);
		input_sync(buttons_dev);
	}
	else
	{
		input_event(buttons_dev,EV_KEY, irq_pd->key_val, 1);
		input_sync(buttons_dev);
	}
}

static irqreturn_t buttons_irq_handler(int irq, void *dev_id)
{
	irq_pd = (struct pin_desc *)dev_id;
	mod_timer(&buttons_timer, jiffies+HZ/100);
	return IRQ_RETVAL(IRQ_HANDLED);
}
	
static int buttons_init(void)
{
	int i,err;
	/* 1. 申请设备 */
	buttons_dev = input_allocate_device();
	
	/* 2. 设置，所支持的事件，及事件子类型 */
	set_bit(EV_KEY, buttons_dev->evbit);
	set_bit(EV_REP, buttons_dev->evbit);
	
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER, buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

	/* 3. 注册input 结构体，即列入链表 */
	input_register_device(buttons_dev);

	/* 4. 硬件相关操作 */
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer);
	for(i = 0; i < 4; i++)
	{
		err = request_irq(pin_desc[i].irq_num, buttons_irq_handler, IRQT_BOTHEDGE, pin_desc[i].irq_name, &pin_desc[i]);	
		if(err) 
			return 1;
	}
	return 0;
}

static void buttons_exit(void)
{
	int i;
	for(i = 0; i < 4; i++)
	{
		free_irq(pin_desc[i].irq_num, &pin_desc);
	}
	del_timer(&buttons_timer);
	input_unregister_device(buttons_dev);
	input_free_device(buttons_dev);
}

module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");


