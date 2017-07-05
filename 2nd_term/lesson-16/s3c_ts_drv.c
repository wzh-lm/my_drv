#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/plat-s3c24xx/ts.h>

#include <asm/arch/regs-adc.h>
#include <asm/arch/regs-gpio.h>

struct adc_regs_t {
	unsigned long ADCCON;
	unsigned long ADCTSC;
	unsigned long ADCDLY;
	unsigned long ADCDAT0;
	unsigned long ADCDAT1;
	unsigned long ADCUPDN;
};

static struct input_dev *s3c_ts;
static volatile struct adc_regs_t *adc_regs;

static irqreturn_t s3c_tc_irq(int irq, void *dev_id)
{
	if(adc_regs->ADCDAT0 & (1<<15))
	{
		printk("Pen up\n");
		adc_regs->ADCTSC = 0xd3; 
	}
	else
	{
		printk("Pen down\n");
		adc_regs->ADCTSC = 0x1d3; 
	}
	return IRQ_HANDLED;
}

static int s3c_tc_init(void)
{
	struct clk *clk;
	/* 1.分配输入子系统结构体*/
	s3c_ts = input_allocate_device();
	
	/* 2.设置 */
	/* 2.1设置会产生哪些事件*/
	set_bit(EV_KEY, s3c_ts->evbit);
	set_bit(EV_ABS, s3c_ts->evbit);
	
	/* 2.2设置所产生事件的子事件有哪些*/
	set_bit(BTN_TOUCH, s3c_ts->keybit);
	input_set_abs_params(s3c_ts, ABS_X, 0, 0x3FF, 0, 0);
	input_set_abs_params(s3c_ts, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(s3c_ts, ABS_PRESSURE, 0, 1, 0, 0);
	
	/* 3.注册输入子系统结构体*/
	input_register_device(s3c_ts);
	
	/* 4.硬件相关配置*/
	/* 4.1使能时钟*/
	clk = clk_get(NULL, "adc");
	clk_enable(clk);
	
	/* 4.2设置触发方式*/
	adc_regs = ioremap(0x58000000, 24);

	/* 4.3设置ADCCON :暂未设置[0] 使能位*/
	adc_regs->ADCCON = (1<<14) | (49<<6);
	
	if(request_irq(IRQ_TC, s3c_tc_irq, IRQF_SAMPLE_RANDOM, "s3c_ts", NULL))
	{
		printk("ERROR:fail to request irq.");
	}
	adc_regs->ADCTSC = 0xd3; 
	return 0;
}

static void s3c_tc_exit(void)
{
	free_irq(IRQ_TC, NULL);
	input_unregister_device(s3c_ts);
	input_free_device(s3c_ts);
	iounmap(adc_regs);
}

module_init(s3c_tc_init);
module_exit(s3c_tc_exit);
MODULE_LICENSE("GPL");

