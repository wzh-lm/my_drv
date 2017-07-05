#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>


struct lcd_regs_t {
	unsigned long	lcdcon1;
	unsigned long	lcdcon2;
	unsigned long	lcdcon3;
	unsigned long	lcdcon4;
	unsigned long	lcdcon5;
    unsigned long	lcdsaddr1;
    unsigned long	lcdsaddr2;
    unsigned long	lcdsaddr3;
    unsigned long	redlut;
    unsigned long	greenlut;
    unsigned long	bluelut;
    unsigned long	reserved[9];
    unsigned long	dithmode;
    unsigned long	tpal;
    unsigned long	lcdintpnd;
    unsigned long	lcdsrcpnd;
    unsigned long	lcdintmsk;
    unsigned long	lpcsel;
};

static struct lcd_regs_t *lcd_regs;
static struct fb_info *lcd_info;
static u32 pseudo_palette[16];//���üٵĵ�ɫ��

static volatile unsigned long *gpbcon = NULL;
static volatile unsigned long *gpbdat = NULL;
static volatile unsigned long *gpccon = NULL;
static volatile unsigned long *gpdcon = NULL;
static volatile unsigned long *gpgcon = NULL;

static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	u32 val;
	if(regno > 16)
		return 1;
	val = (((red & 0xffff)>>(16 - info->var.red.length))<<info->var.red.offset)|(((green & 0xffff)>>(16 - info->var.green.length))<<info->var.green.offset)|(((blue & 0xffff)>>(16 - info->var.blue.length))<<info->var.blue.offset);
	pseudo_palette[regno] = val;
	return 0;
}


static struct fb_ops s3c_lcdfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= s3c_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

 



static int lcd_init(void)
{
	//1.����ṹ��framebuffer
	lcd_info = framebuffer_alloc(0, NULL); //�����fb_info �ṹ��֮��Ŀռ� ��Ϊ0 ��
	//2.���ýṹ���ڲ���
	//2.1���ù̶�����
	strcpy(lcd_info->fix.id,"wzh_lcd");
	//lcd_info->fix.smem_start  �Դ���ʼ��ַ��  Ӧ���ǰ�
	lcd_info->fix.smem_len = 480*272*16/8;
	lcd_info->fix.type = FB_TYPE_PACKED_PIXELS;
	lcd_info->fix.visual = FB_VISUAL_TRUECOLOR;
	lcd_info->fix.line_length = 480*2;

	//2.2���ÿɱ����
	lcd_info->var.xres = 480;
	lcd_info->var.yres = 272;
	lcd_info->var.xres_virtual = 480;
	lcd_info->var.yres_virtual = 272;
	lcd_info->var.bits_per_pixel = 16;// diff_1
	
	lcd_info->var.red.length = 5;
	lcd_info->var.red.offset = 11;
	lcd_info->var.green.length = 6;
	lcd_info->var.green.offset = 5;
	lcd_info->var.blue.length = 5;
	lcd_info->var.blue.offset = 0;
	
	lcd_info->var.activate = FB_ACTIVATE_NOW;
	//2.3���ò������� 
	lcd_info->fbops = &s3c_lcdfb_ops;
	//2.4 ��������
	//lcd_info->screen_base     �Դ��׵�ַ���������
	lcd_info->screen_size = 480*272*2;
    lcd_info->pseudo_palette = pseudo_palette;

	//3.Ӳ������
	//3.1ӳ���������ַ
	gpbcon = ioremap(0x56000010, 8); //diff_2   size diff del gpgdat 
	gpbdat = gpbcon + 1;
	gpccon = ioremap(0x56000020, 4);
	gpdcon = ioremap(0x56000030, 4);
	gpgcon = ioremap(0x56000060, 4);
	lcd_regs = ioremap(0x4D000000,sizeof(struct lcd_regs_t));
	//3.2����GPIO���ر��⡢��Դ
	*gpbcon &= ~(3);
	*gpbcon |= 1;
	*gpbdat &= ~(1);  

	*gpccon = 0xaaaaaaaa;  
	*gpdcon = 0xaaaaaaaa;
	*gpgcon |= 3<<2*4;
	//3.3lcd������������ con1/con5����2  ��ʹ��λ�� ��ʱ�ر�
	lcd_regs->lcdcon1 = (4<<8)|(3<<5)|(12<<1);
	lcd_regs->lcdcon2 = (1<<24)|(271<<14)|(1<<6)|(9<<0);
	lcd_regs->lcdcon3 = (1<<19)|(479<<8)|(1<<0);
	lcd_regs->lcdcon4 = 40;
	lcd_regs->lcdcon5 = (1<<11)|(1<9)|(1<8)|(1<<0);

	//3.4�����Դ��ַ,����֪lcd ������
	lcd_info->screen_base = dma_alloc_writecombine(NULL, lcd_info->fix.smem_len, &lcd_info->fix.smem_start, GFP_KERNEL);
	lcd_regs->lcdsaddr1 = (lcd_info->fix.smem_start)>>1 & ~(3<<30);   //diff_3
	lcd_regs->lcdsaddr2 = ((lcd_info->fix.smem_start+lcd_info->fix.smem_len)>>1)&0x1fffff;
	lcd_regs->lcdsaddr3 = 480*16/16;

	//3.5�����⣬����Դ
	*gpbdat |= 1;
	lcd_regs->lcdcon1 |= 1;
	lcd_regs->lcdcon5 |= 1<<3;

	//4.ע��LCD�ṹ��
	register_framebuffer(lcd_info);
	return 0;
}

static void lcd_exit(void)
{
	unregister_framebuffer(lcd_info);
	*gpbdat &= ~(1);
	lcd_regs->lcdcon1 &= ~(1);
	lcd_regs->lcdcon5 &= ~(1<<3);
	dma_free_writecombine(NULL, lcd_info->fix.smem_len, lcd_info->screen_base, lcd_info->fix.smem_start);
	iounmap(gpbcon);
	iounmap(gpccon);
	iounmap(gpdcon);
	iounmap(gpgcon);
	iounmap(lcd_regs);
	framebuffer_release(lcd_info);
}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");

